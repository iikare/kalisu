#if defined(TARGET_REL)
  #define NO_DEBUG
#endif

#if defined(TARGET_WIN)
extern "C" {
  #include "../dpd/mupdf/include/mupdf/fitz.h"
  #include "../dpd/raylib/src/raylib.h"
  #include "../dpd/raylib/src/rlgl.h"
}
  #define _USE_MATH_DEFINES
#else
  #include <mupdf/fitz.h>
  #include <raylib.h>
#endif
