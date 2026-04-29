#include "misc.h"

#include <sys/stat.h>

#include <algorithm>
#include <cmath>
#include <iomanip>

#include "log.h"
#include "wrap.h"

using std::hex;
using std::min;
using std::stringstream;
using std::to_string;

rect pointToRect(const point& a, const point& b) {
  rect result = {0, 0, 0, 0};

  result.x = a.x - 2;
  result.y = min(a.y, b.y) - 4;
  result.width = b.x - a.x + 4;
  result.height = abs(b.y - a.y) + 8;

  return result;
}

point getMousePosition() {
  if (IsWindowFocused()) {
    return point(GetMouseX(), GetMouseY());
  }
  return {-1, -1};
}

void drawTextEx(const string& msg, const Vector2& pos, const colorRGB& col, unsigned char alpha, int size,
                const string& font) {
  drawTextEx(msg, pos.x, pos.y, col, alpha, size, font);
}
void drawTextEx(const string& msg, int x, int y, const colorRGB& col, unsigned char alpha, int size,
                const string& font) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, alpha};
  const Font& ft = ctr.get_font(font, size);
  DrawTextEx(ft, msg.c_str(), (const Vector2){static_cast<float>(x), static_cast<float>(y)}, ft.baseSize,
             TEXT_SPACING, color);
}

string getExtension(const string& path, bool len4) {
  unsigned long len = len4 ? 4 : 3;
  string p = path;
  transform(p.begin(), p.end(), p.begin(), ::tolower);
  return p.length() > len ? p.substr(p.size() - len) : p;
}

string getDirectory(const string& path) {
  string dir = path;
  int pos = dir.find_last_of("/\\");
  return dir.substr(0, pos);
}

bool isValidPath(const string& path) {
  struct stat info;
  // file doesn't exist in filesystem
  if (stat(path.c_str(), &info) == -1) {
    return false;
  }

  string ext = getExtension(path);

  return isValidExtension(ext);
}

bool isValidExtension(const string& ext) { return ext == "pdf"; }

string toHex(int dec) {
  stringstream stream;
  stream << hex << dec;
  return stream.str();
}

const Vector2 measureTextEx(const string& msg, int size, const string& font) {
  const Font& ft = ctr.get_font(font, size);
  return MeasureTextEx(ft, msg.c_str(), ft.baseSize, TEXT_SPACING);

  return {0.0f, 0.0f};
}
