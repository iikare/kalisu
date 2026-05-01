#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "asset.h"
#include "build_target.h"
#include "color.h"
#include "data.h"
#include "io.h"
#include "text.h"
#include "wrap.h"

extern vector<asset> assetSet;

using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

class controller {
 public:
  void init(vector<asset>& asset_set);
  void load(string fp);
  void update();
  void update_dropped_files();
  void unload();
  void close();
  void begin() {
    BeginDrawing();
    clearBackground(bg_col);
  }
  void end() { EndDrawing(); }

  bool loaded() { return load_flag; }

  bool run() { return !WindowShouldClose() && !close_flag; }
  void set_close() { close_flag = true; }

  const Font& get_font(const string& id, int size);

  unsigned int get_w() { return width; }
  unsigned int get_h() { return height; }
  float get_total_height() {
    float total = 0;
    for (const auto& p : pages) {
      total += p.height * (get_w() / (float)p.width);
    }
    return total;
  }

  void render_info();

  void find_system_breakpoints();

  vector<Texture2D> pages;
  vector<vector<pair<int, int>>> staves;
  vector<vector<pair<int, int>>> systems;
  vector<vector<int>> breakpoints;
  vector<vector<int>> mark_count;

  const char* FILTER_PDF = "pdf:pdf";
  ioController open_file = ioController(OSDIALOG_OPEN, FILTER_PDF);

  textController text;

  colorRGB text_col = colorRGB(10, 10, 10);
  colorRGB bg_col = colorRGB(240, 240, 240);
  colorRGB menu_col = colorRGB(222, 222, 222);
  colorRGB icon_col = colorRGB(34, 115, 150);
  colorRGB system_col = colorRGB(255, 0, 0);

  static constexpr int info_width = 500;
  static constexpr int info_height = 55;

 private:
  void init_data(const vector<asset>& asset_set);
  void find_staves(vector<int> b_ct, int w);
  int find_optimal_breakpoint(int page, int start, int end);

  string file_path;

  constexpr static float DPI_SCALE = 4.0f;

  fz_context* ctx;
  fz_document* doc;
  int page_ct;

  bool load_flag = false;

  unsigned int width = W_WIDTH;
  unsigned int height = W_HEIGHT;

  void update_fps();
  int c_mon = -1;
  bool close_flag = false;

  unordered_map<string, pair<asset, map<int, Font>>> fontMap;
};
