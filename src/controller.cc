#include "controller.h"

#include "build_target.h"
#include "log.h"

void controller::init() {
  SetTraceLogLevel(LOG_ERROR);
  InitWindow(width, height, prog_name.c_str());
  SetExitKey(KEY_F7);
  update_fps();

  ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
  fz_register_document_handlers(ctx);
}

void controller::load(string fp) {
  logQ(fp);
  doc = fz_open_document(ctx, fp.c_str());
  page_ct = fz_count_pages(ctx, doc);

  for (int i = 0; i < page_ct; i++) {
    fz_page* page_ref = fz_load_page(ctx, doc, i);
    fz_rect rect = fz_bound_page(ctx, page_ref);
    fz_matrix ctm = fz_scale(DPI_SCALE, DPI_SCALE);
    fz_pixmap* pix = fz_new_pixmap_from_page(ctx, page_ref, ctm, fz_device_rgb(ctx), 0);

    std::vector<int> b_ct_by_row(pix->h, 0);
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
}

void controller::unload() {
  for (auto& p : pages) {
    UnloadTexture(p);
  }
  fz_drop_document(ctx, doc);
  fz_drop_context(ctx);
  CloseWindow();
}
