#pragma once

#include <algorithm>
#include <vector>

using std::find;
using std::is_enum;
using std::vector;

template <class T>
class enumChecker {
 public:
  enumChecker() : items(0), itemsLastFrame(0) {
    static_assert(is_enum<T>::value, "class requires an enum type");
  }

  void add(T item) {
    if (!contains(item)) {
      items.push_back(item);
    }
  }

  void remove(T item) { items.remove(item); }

  void clear() {
    itemsLastFrame = items;
    items.clear();
  }

  bool contains(T item) { return find(items.begin(), items.end(), item) != items.end(); }

  bool contains(T item1, T item2...) { return contains(item1) || contains(item2); }

  bool containsLastFrame(T item) {
    return find(itemsLastFrame.begin(), itemsLastFrame.end(), item) != itemsLastFrame.end();
  }

  bool containsLastFrame(T item1, T item2...) { return containsLastFrame(item1) || containsLastFrame(item2); }

 private:
  vector<T> items;
  vector<T> itemsLastFrame;
};

enum class ASSET { FONT, RENDER_TEXTURE, TEXTURE, SHADER, IMAGE, MODEL, NONE };

enum class langType {
  LANG_EN,
  LANG_NONE,
};
