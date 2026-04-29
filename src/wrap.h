#pragma once

#include <string>

#include "build_target.h"
#include "color.h"
#include "box.h"

using std::string;

void clearBackground();
void clearBackground(const colorRGB& col);

void drawPixel(float x, float y, const colorRGB& col, unsigned char alpha = 255);

void drawRectangle(float x, float y, float w, float h, const colorRGB& col, unsigned char alpha = 255);
void drawRectangleLines(float x, float y, float w, float h, float width, const colorRGB& col);
void drawLine(float xi, float yi, float xf, float yf, const colorRGB& col);
void drawLineEx(float xi, float yi, float xf, float yf, float thick, const colorRGB& col, unsigned char alpha = 255);
void drawLineBezier(float xi, float yi, float xf, float yf, float thick, const colorRGB& col);

void drawCircle(float x, float y, float r, const colorRGB& col, float alpha = 255);

void drawRing(const Vector2& center, float iRad, float oRad, const colorRGB& col, float alpha = 255,
              float startAngle = 0, float endAngle = 360);

void drawGradientLineH(const Vector2& a, const Vector2& b, float thick, const colorRGB& col, float alphaA,
                       float alphaB);

void drawTextureEx(const Texture2D& tex, const Vector2& pos, float rot = 0.0f, float scale = 1.0f);
void drawTexturePro(const Texture2D& tex, const rect& source, const rect& dest, const Vector2& origin, float rot);


template <class T>
bool isKeyPressed(T key) {
  return IsKeyPressed(key);
}
template <class T, class... U>
bool isKeyPressed(T key, U... keys) {
  return IsKeyPressed(key) || isKeyPressed(keys...);
}

template <class T>
bool isKeyDown(T key) {
  return IsKeyDown(key);
}
template <class T, class... U>
bool isKeyDown(T key, U... keys) {
  return IsKeyDown(key) || isKeyDown(keys...);
}

ostream& operator<<(ostream& out, const Vector2& vec);
ostream& operator<<(ostream& out, const Vector3& vec);
