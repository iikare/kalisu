#pragma once

#include <string>
#include <vector>

#include "build_target.h"

using std::make_pair;
using std::pair;
using std::string;
using std::vector;

class controller {
 public:
  void init();
  void load(string fp);
  void update();
  void unload();
  void begin() {
    BeginDrawing();
    ClearBackground(WHITE);
  }
  void end() { EndDrawing(); }

  unsigned int get_w() { return width; }
  unsigned int get_h() { return height; }
  float get_total_height() {
    float total = 0;
    for (const auto& p : pages) {
      total += p.height * (get_w() / (float)p.width);
    }
    return total;
  }

  void find_system_breakpoints();

  int page_ct;
  vector<Texture2D> pages;
  vector<vector<pair<int, int>>> staves;
  vector<vector<pair<int, int>>> systems;
  vector<vector<int>> breakpoints;

 private:
  void find_staves(vector<int> b_ct, int w);

  void update_fps();
  int c_mon = -1;

  string file_path;
  const string prog_name = "stave_viewer";
  unsigned int width = 800;
  unsigned int height = 1000;

  constexpr static float DPI_SCALE = 4.0f;

  fz_context* ctx;
  fz_document* doc;
};
