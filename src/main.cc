#include "aghfile.h"
#include "build_target.h"
#include "controller.h"
#include "log.h"
#include "misc.h"
#include "wrap.h"

controller ctr;

using std::max;
using std::min;

int main(int argc, char** argv) {
  ctr.init(assetSet);
  if (argc >= 2) {
    ctr.load(argv[1]);
  }

  float y_off = 0;
  float scr_spd = 30;
  bool show_info = false;

  while (ctr.run()) {
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
    if ((isKeyDown(KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL) && isKeyPressed(KEY_O)) ||
        (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ctr.loaded())) {
      ctr.open_file.dialog();
      if (ctr.open_file.pending()) {
        ctr.load(ctr.open_file.getPath());
        ctr.open_file.reset();
        y_off = 0;
      }
    }
    if (isKeyDown(KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL)) {
      // close document
      if (isKeyPressed(KEY_W)) {
        if (ctr.loaded()) {
          ctr.unload();
          show_info = false;
        }
        else {
          ctr.set_close();
        }
      }
      if (isKeyPressed(KEY_I)) {
        show_info = !show_info;
      }
    }

    if (isKeyPressed(KEY_SPACE) || isKeyPressed(KEY_PAGE_DOWN)) {
      float last_bp_abs_y = 0.0f;
      float current_page_abs_y = 0.0f;
      bool snap = false;

      for (unsigned int p = 0; p < ctr.pages.size(); p++) {
        const auto& tex = ctr.pages[p];
        float scale = (float)ctr.get_w() / (float)tex.width;

        for (auto& l : ctr.breakpoints[p]) {
          float bp_abs_y = current_page_abs_y + (l * scale);

          if (bp_abs_y + y_off > (float)ctr.get_h() + 1.0f) {
            y_off = -last_bp_abs_y;
            snap = true;
            break;
          }
          last_bp_abs_y = bp_abs_y;
        }
        if (snap) {
          break;
        }
        current_page_abs_y += (float)tex.height * scale;
      }
    }
    else if (isKeyPressed(KEY_PAGE_UP)) {
      float current_page_abs_y = 0.0f;
      float target_y = 0.0f;
      bool found_any_above = false;
      bool finished = false;

      for (unsigned int p = 0; p < ctr.pages.size(); p++) {
        const auto& tex = ctr.pages[p];
        float scale = (float)ctr.get_w() / (float)tex.width;

        for (auto& l : ctr.breakpoints[p]) {
          float bp_abs_y = current_page_abs_y + (l * scale);

          if (bp_abs_y < -y_off - 1.0f) {
            target_y = bp_abs_y;
            found_any_above = true;
          }
          else {
            finished = true;
            break;
          }
        }

        if (finished) {
          break;
        }
        current_page_abs_y += (float)tex.height * scale;
      }

      y_off = found_any_above ? -target_y : 0.0f;
    }

    int k = GetKeyPressed();
    if (k && k != KEY_I) {
      show_info = false;
    }

    if (isKeyPressed(KEY_HOME) || y_off > 0) {
      y_off = 0;
    }
    if (ctr.get_total_height() > ctr.get_h()) {
      float min_y = -(ctr.get_total_height() - ctr.get_h());
      if (isKeyPressed(KEY_END)) {
        y_off = min(y_off, min_y);
      }
      else {
        y_off = max(y_off, min_y);
      }
    }
    else {
      y_off = 0;
    }

    ctr.begin();

    float current_y = y_off;
    for (unsigned int p = 0; p < ctr.pages.size(); p++) {
      const auto& tex = ctr.pages[p];
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
    }

    if (!ctr.loaded()) {
      string load_text = "load a pdf w/ctrl-o";
      int load_text_size = 30;
      Vector2 load_text_bounds = measureTextEx(load_text, load_text_size);
      drawTextEx(load_text, ctr.get_w() / 2.0f - load_text_bounds.x / 2.0f,
                 ctr.get_h() / 2.0f - load_text_bounds.y / 20, ctr.text_col, 255, load_text_size);

      string info_text = "info - ctrl-i";
      int info_text_size = 18;
      Vector2 info_text_bounds = measureTextEx(info_text, info_text_size);
      drawTextEx(info_text, ctr.get_w() / 2.0f - info_text_bounds.x / 2.0f,
                 ctr.get_h() / 2.0f + load_text_bounds.y + 4 - info_text_bounds.y / 20, ctr.text_col, 255,
                 info_text_size);
    }

    if (show_info) {
      ctr.render_info();
    }
    ctr.end();

    ctr.update();
  }

  ctr.unload();
  ctr.close();
  return 0;
}
