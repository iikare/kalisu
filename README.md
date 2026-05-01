# kalisu - score reader

Navigate piano scores by system or group of systems.

`kalisu` — ಕಲಿಸು (teach)

# dependencies
This project requires several dependencies, many of which are submodules to this repository. Some of these modules are optional and/or platform specific.

* `osdialog` (supplied as a submodule)
* `raylib`*  (see below)
* `mupdf`**  (see below)
* `make`
* `xxd`

Cross-compilation will require you to recompile `raylib` and `mupdf` for the target platform. For this purpose, the included submodule must be used. Compiling for
a linux platform does not require the use of the bundled `raylib` or `mupdf` submodule. For these cases, the build process will link against the system `raylib` and `mupdf`
libraries.

The current build script assumes you are building on a Linux environment with raylib installed in the appropriate directories (`/lib/libraylib.so` and `/usr/include/raylib.h`). This project should be compiler-agnostic, but for best results, use `clang`/`clang++`. For cross-compilation to Windows, the following tools are required:

* `x86_64-w64-mingw32-gcc`
* `x86_64-w64-mingw32-g++`
* `x86_64-w64-mingw32-ar`
* `x86_64-w64-mingw32-ld`
* `x86_64-w64-mingw32-ranlib`

# build
Don't forget to `git clone --recurse-submodules` when obtaining the source code!

Currently, only Linux and to a lesser extent, Windows, are supported, however, the dependencies and codebase are
purposely OS-agnostic, and with some tweaking can run on MacOS as
well. Contributions to improve cross-platform interoperability are welcomed.

Simply run `make` in the project's root directory. The resulting executable
will be formed at `./bin/kalisu`

Cross-compilation to Windows is done through `make arch=win`, and the following executable is formed at `./bin/kalisu.exe`

# usage
If executing `kalisu` from the command-line, up to one optional argument is accepted:
```sh
./bin/kalisu path/to/file.pdf
```

Otherwise, use the built-in graphical commands. Attempting to load a non-score pdf file is not supported and will lead to undefined behavior.

# license
`kalisu` is licensed under GPLv3.
