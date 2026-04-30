#include "controller.h"

#include "aghdef.h"
#include "log.h"
#include "misc.h"
#include "wrap.h"

using std::min;

void controller::init(vector<asset>& asset_set) {
#if defined(TARGET_REL)
  SetTraceLogLevel(LOG_ERROR);
#else
  SetTraceLogLevel(LOG_ERROR);
  // SetTraceLogLevel(LOG_DEBUG);
  //  SetTraceLogLevel(LOG_TRACE);
#endif

// allow resizing of window on windows
#if defined(TARGET_WIN)
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
#else
  // SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  SetConfigFlags(FLAG_MSAA_4X_HINT);
#endif

  const string window_title = string(W_NAME) + " " + string(W_VER);
  InitWindow(width, height, window_title.c_str());
  SetExitKey(KEY_F7);
  SetWindowMinSize(W_WIDTH, W_HEIGHT);

  update_fps();

  init_data(asset_set);
  text.init();
}

void controller::init_data(const vector<asset>& asset_set) {
  for (const auto& item : asset_set) {
    switch (item.assetType) {
      using enum ASSET;
      case FONT:
        fontMap.insert(make_pair(item.assetName, make_pair(item, map<int, Font>())));
        break;
      default:
        // these items aren't statically loaded
        break;
    }
  }
}

const Font& controller::get_font(const string& id, int size) {
  // find if a font with this id exists
  auto fit = fontMap.find(id);
  if (fit == fontMap.end()) {
    logW(LL_CRIT, "invalid font id -", id);
    // return nullptr;
    fit = fontMap.begin();
  }

  // if this font exists, find map of font size to font pointer
  auto it = fit->second.second.find(size);
  if (it == fit->second.second.end()) {
    // if end is reached, this font combination doesn't exist and needs to be
    // created

    asset& tmpFontAsset = fit->second.first;
    int lim = 255;
    int* loc = nullptr;

    Font tmp = LoadFontFromMemory(".otf", tmpFontAsset.data, tmpFontAsset.dataLen, size, loc, lim);

    SetTextureFilter(tmp.texture, TEXTURE_FILTER_BILINEAR);

    fit->second.second.insert(make_pair(size, tmp));

    // it = fontMap.find(size);
  }

  return fit->second.second.find(size)->second;
}

void controller::load(string fp) {
  logW(LL_INFO, "loading", fp);
  auto start = std::chrono::high_resolution_clock::now();
  if (load_flag) {
    unload();
  }
  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  fz_register_document_handlers(ctx);
  doc = fz_open_document(ctx, fp.c_str());
  page_ct = fz_count_pages(ctx, doc);

  for (int i = 0; i < page_ct; i++) {
    fz_page* page_ref = fz_load_page(ctx, doc, i);

    fz_rect rect = fz_bound_page(ctx, page_ref);
    float doc_width = rect.x1 - rect.x0;
    float doc_height = rect.y1 - rect.y0;

    const float page_limit_w = 4000.0f;
    float final_scale = DPI_SCALE;

    float scaled_width = doc_width * DPI_SCALE;
    float scaled_height = doc_height * DPI_SCALE;

    if (scaled_width > page_limit_w || scaled_height > page_limit_w) {
      float width_scale = page_limit_w / doc_width;
      float height_scale = page_limit_w / doc_height;

      final_scale = (width_scale < height_scale) ? width_scale : height_scale;
    }

    fz_matrix ctm = fz_scale(final_scale, final_scale);
    fz_pixmap* pix = fz_new_pixmap_from_page(ctx, page_ref, ctm, fz_device_rgb(ctx), 0);

    std::vector<int> b_ct_by_row(pix->h, 0);
    logQ(pix->h);
    unsigned char* pixels = pix->samples;

    constexpr unsigned int b_thr = 50;
    for (int y = 0; y < pix->h; y++) {
      int b_ct = 0;
      for (int x = 0; x < pix->w; x++) {
        int idx = (y * pix->w + x) * 3;
        if (pixels[idx] < b_thr && pixels[idx + 1] < b_thr && pixels[idx + 2] < b_thr) {
          b_ct++;
        }
      }

      b_ct_by_row[y] = b_ct;
    }

    find_staves(b_ct_by_row, pix->w);
    // logQ(b_ct_by_row);

    Image img = {pix->samples, pix->w, pix->h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8};

    Texture2D tex = LoadTextureFromImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    pages.emplace_back(tex);

    fz_drop_pixmap(ctx, pix);
    fz_drop_page(ctx, page_ref);
  }

  find_system_breakpoints();
  load_flag = true;

  logW(LL_INFO, "staves      |", inner_vec_ct(staves));
  logW(LL_INFO, "systems     |", inner_vec_ct(systems));
  logW(LL_INFO, "breakpoints |", inner_vec_ct(breakpoints));
  debug_time(start, "load");
}

void controller::find_staves(vector<int> b_ct, int w) {
  vector<int> lines;

  int threshold = static_cast<int>(w * 0.70f);

  for (unsigned int i = 0; i < b_ct.size(); ++i) {
    if (b_ct[i] >= threshold) {
      int start = i;
      while (i < b_ct.size() && b_ct[i] >= threshold) {
        i++;
      }
      int end = i - 1;

      int thickness = end - start + 1;
      if (thickness >= 1 && thickness < 15) {
        lines.push_back(start + (thickness / 2));
      }
    }
  }

  vector<pair<int, int>> page_staves;

  if (lines.size() < 5) {
    staves.push_back(page_staves);
    return;
  }

  size_t i = 0;
  while (i + 4 < lines.size()) {
    int g1 = lines[i + 1] - lines[i];
    int g2 = lines[i + 2] - lines[i + 1];
    int g3 = lines[i + 3] - lines[i + 2];
    int g4 = lines[i + 4] - lines[i + 3];

    float avg_gap = (g1 + g2 + g3 + g4) / 4.0f;
    float tol = avg_gap * 0.15f;

    bool spacing_ok = (abs(g1 - avg_gap) <= tol && abs(g2 - avg_gap) <= tol && abs(g3 - avg_gap) <= tol &&
                       abs(g4 - avg_gap) <= tol);

    if (spacing_ok && avg_gap > 3.0f) {
      page_staves.push_back({lines[i], lines[i + 4]});
      i += 5;
    }
    else {
      i++;
    }
  }

  staves.push_back(page_staves);

  vector<pair<int, int>> page_systems;
  for (unsigned int s = 0; s < page_staves.size(); s += 2) {
    pair<int, int> p = make_pair(page_staves[s].first, 0);

    if (s + 1 >= page_staves.size()) {
      p.second = page_staves.back().second;
    }
    else {
      p.second = page_staves[s + 1].second;
    }
    page_systems.push_back(p);
  }
  systems.push_back(page_systems);
}

void controller::find_system_breakpoints() {
  for (unsigned int p = 0; p < pages.size(); ++p) {
    // logQ(systems[p]);

    vector<int> page_breakpoints = {0};

    for (unsigned int sys = 0; sys < systems[p].size(); ++sys) {
      if (!sys) {
        continue;
      }
      // logQ(systems[p][sys-1].second, systems[p][sys].first,
      //(systems[p][sys-1].second + systems[p][sys].first)/2);

      page_breakpoints.push_back((systems[p][sys - 1].second + systems[p][sys].first) / 2);
    }

    breakpoints.push_back(page_breakpoints);
  }
}

void controller::update_fps() {
  if (c_mon != GetCurrentMonitor()) {
    c_mon = GetCurrentMonitor();
    SetTargetFPS(GetMonitorRefreshRate(c_mon));
  }
}

void controller::update() {
  width = GetScreenWidth();
  height = GetScreenHeight();

  update_fps();
  update_dropped_files();
}

void controller::update_dropped_files() {
  if (IsFileDropped()) {
    FilePathList dropFile = LoadDroppedFiles();
    constexpr unsigned int dropLimit = 1;

    if (dropFile.count > dropLimit) {
      logW(LL_WARN, "excess files dropped - max:", dropLimit);
    }

    for (unsigned int idx = 0; idx < min(dropLimit, dropFile.count); ++idx) {
      if (isValidExtension(getExtension(dropFile.paths[idx]))) {
        // logQ("valid dropped file:", dropFile.paths[idx]);
        load(dropFile.paths[idx]);
        break;
      }
    }
    UnloadDroppedFiles(dropFile);
  }
}

void controller::unload() {
  for (auto& p : pages) {
    UnloadTexture(p);
  }
  pages.clear();
  staves.clear();
  systems.clear();
  breakpoints.clear();

  if (load_flag) {
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
  }
  load_flag = false;
}

void controller::close() { CloseWindow(); }

void controller::render_info() {
  const int infoSideMargin = get_w() - info_width;
  const int infoTopMargin = get_h() - info_height;
  drawRectangle(infoSideMargin / 2.0f, infoTopMargin / 2.0f, info_width, info_height, menu_col);

  static constexpr int iconTextSize = 30;
  static constexpr double borderMargin = 20;

  const string copySym = "©";
  const string copy = " iika-re 2020-" + string(COPY_YEAR);
  static constexpr int copySymSize = 22;

  Vector2 iconTextVec = measureTextEx(W_NAME, iconTextSize);
  double iconTextX = (get_w() - iconTextVec.x) / 2.0f;
  double iconTextY = (get_h() - iconTextVec.y) / 2.0f + 2;

  drawTextEx(W_NAME, iconTextX, iconTextY, icon_col, 255, iconTextSize);

  double copySymWidth = measureTextEx(copySym, copySymSize).x;
  double copyWidth = measureTextEx(copy).x;
  double copyHeight = measureTextEx(copy).y;
  double licenseWidth = measureTextEx(ctr.text.getString("INFO_BOX_LICENSE_GPL3")).x;

  drawTextEx(ctr.text.getString("INFO_BOX_BUILD_DATE") + " " + string(BUILD_DATE),
             infoSideMargin / 2.0f + borderMargin, infoTopMargin / 2.0f + info_height - borderMargin - 20,
             ctr.text_col);

  auto versionString = ctr.text.getString("INFO_BOX_VER") + " " + string(W_VER);
#if !defined(TARGET_REL)
  versionString += " - " + string(COMMIT_HASH);
#endif

  drawTextEx(versionString, infoSideMargin / 2.0f + borderMargin,
             infoTopMargin / 2.0f + info_height + copyHeight - borderMargin - 20, ctr.text_col);

  drawTextEx(copySym, infoSideMargin / 2.0f + info_width - borderMargin - copySymWidth - copyWidth,
             infoTopMargin / 2.0f + info_height - borderMargin - 20, ctr.text_col, 255, copySymSize);
  drawTextEx(copy, infoSideMargin / 2.0f + info_width - borderMargin - copyWidth,
             infoTopMargin / 2.0f + info_height - borderMargin - 20, ctr.text_col);
  drawTextEx(ctr.text.getString("INFO_BOX_LICENSE_GPL3"),
             infoSideMargin / 2.0f + info_width - borderMargin - licenseWidth,
             infoTopMargin / 2.0f + info_height + copyHeight - borderMargin - 20, ctr.text_col);
}
