#include "build_target.h"
#include "controller.h"
#include "wrap.h"
#include "log.h"

controller ctr;

using std::max;

int main(int argc, char** argv) {

  ctr.init();
  if (argc >= 2) {
    //logW(LL_CRIT, "usage: ./stave_viewer [pdf]");
    ctr.load(argv[1]);
  }

  float y_off = 0.0f;
  float scr_spd = 30.0f;

  while (!WindowShouldClose()) {
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      y_off += wheel * scr_spd;
    }
    if (isKeyDown(KEY_DOWN)) {
      y_off -= scr_spd * 0.1f;
    }
    if (isKeyDown(KEY_UP)) {
      y_off += scr_spd * 0.1f;
    }
    if (isKeyDown(KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL)) {
      // open pdf
      if (isKeyPressed(KEY_O)) {
        ctr.open_file.dialog();
        if (ctr.open_file.pending()) {
          ctr.load(ctr.open_file.getPath());
          ctr.open_file.reset();
        }
      }
    }

    // system detection
    if (isKeyPressed(KEY_SPACE)) {
      bool snap = false;
      float last_bp_abs_y = 0.0f;
      float current_page_abs_y = 0.0f;

      for (unsigned int p = 0; p < ctr.pages.size(); p++) {
        const auto& tex = ctr.pages[p];
        float scale = (float)ctr.get_w() / (float)tex.width;
        float page_h = (float)tex.height * scale;

        for (auto& l : ctr.breakpoints[p]) {
          float bp_abs_y = current_page_abs_y + (l * scale);

          if (bp_abs_y + y_off > ctr.get_h()) {
            y_off = -last_bp_abs_y;
            snap = true;
            break;
          }

          last_bp_abs_y = bp_abs_y;
        }

        if (snap) {
          break;
        }

        current_page_abs_y += page_h;
      }
    }

    if (y_off > 0) {
      y_off = 0;
    }
    if (ctr.get_total_height() > ctr.get_h()) {
      float min_y = -(ctr.get_total_height() - ctr.get_h());
      y_off = max(y_off, min_y);
    }
    else {
      y_off = 0;
    }

    ctr.begin();

    float current_y = y_off;
    int p = 0;
    for (const auto& tex : ctr.pages) {
      float scale = ctr.get_w() / (float)tex.width;
      float page_h = (float)tex.height * scale;

      if (current_y + page_h > 0 && current_y < ctr.get_h()) {
        DrawTextureEx(tex, {0, current_y}, 0.0f, scale, WHITE);
      }

      // debug
      for (const auto& l : ctr.systems[p]) {
        if (current_y + l.first * scale > 0 && current_y + l.first * scale < ctr.get_h()) {
          DrawLine(0, current_y + l.first * scale, ctr.get_w(), current_y + l.first * scale, RED);
        }
        if (current_y + l.second * scale > 0 && current_y + l.second * scale < ctr.get_h()) {
          DrawLine(0, current_y + l.second * scale, ctr.get_w(), current_y + l.second * scale, RED);
        }
      }
      for (const auto& l : ctr.breakpoints[p]) {
        if (current_y + l * scale > 0 && current_y + l * scale < ctr.get_h()) {
          DrawLine(0, current_y + l * scale, ctr.get_w(), current_y + l * scale, SKYBLUE);
        }
      }

      current_y += page_h;
      p++;
    }

    ctr.end();

    ctr.update();
  }

  ctr.unload();
  ctr.close();
  return 0;
}
