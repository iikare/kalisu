ifeq ($(strip $(asan)),)
NONSTD=
else # use address sanitizer
NONSTD=-fsanitize=address -fno-omit-frame-pointer 
endif

ifeq ($(strip $(prof)),)
#NONSTD+=
else
NONSTD+=-pg -g 
endif

ifeq ($(strip $(arch)),)

CXX=clang++
LD=-lraylib $(shell pkg-config --libs mupdf) -lpthread -ldl -Wl

else ifeq ($(strip $(arch)),win)

CXX=x86_64-w64-mingw32-g++
CC=x86_64-w64-mingw32-ld
C=x86_64-w64-mingw32-gcc
DEPDEF=-DTARGET_WIN -I./dpd/mupdf/include
LFLAGSOSD=-lgdi32 -lwinmm -lopengl32 -lcomdlg32 -lole32 -luuid
SRCSOSD=$(OSDDIR)/osdialog.c $(OSDDIR)/osdialog_win.c

endif

ifeq ($(strip $(rel)),)
RELFLAGS=-Og -g
else # release build
RELFLAGS=-DTARGET_REL -O3 -ffast-math
# thinLTO doesn't exist for cross-compiler
ifeq ($(strip $(arch)),)
RELFLAGS+=-flto=thin
LD+=-flto=thin
else ifeq ($(strip $(arch)),win)
#LFLAGS+=-mwindows # remove console window
endif

endif


# annoying whitespace
ifeq ($(strip $(arch)),)
CFLAGS=--std=c++23 -Wall -Wextra $(NONSTD)$(RELFLAGS)$(DEPDEF)
else ifeq ($(strip $(arch)),win)
CFLAGS=--std=c++23 -Wall -Wextra $(NONSTD)$(RELFLAGS) $(DEPDEF)
endif

CFLAGSSTD=$(CFLAGS) -fno-exceptions

ifeq ($(strip $(arch)),)
LFLAGS+=$(LD) -lraylib $(LFLAGSOSD)
else ifeq ($(strip $(arch)),win)
LFLAGS+=-Wl,--start-group -L./dpd/mupdf/build/release -lmupdf -lmupdf-third -Wl,--end-group -L./dpd/raylib/src -lraylib $(LFLAGSOSD) -static -static-libgcc -static-libstdc++ data/i.res --verbose
endif

PREREQ_DIR=@mkdir -p $(@D)

SRCDIR=src
BUILDDIR=build
BINDIR=bin

NAME=$(addprefix $(BINDIR)/, kalisu)

SRCS=$(wildcard $(SRCDIR)/*.cc)
OBJS=$(patsubst $(SRCDIR)/%.cc, $(BUILDDIR)/%.o, $(SRCS))

all:
	@mkdir -p ./src/agh

ifeq ($(strip $(arch)),win)
	@./tool/cross.sh
	@./tool/objtype.sh win
else
	@./tool/objtype.sh linux
endif
	@$(MAKE) --no-print-directory $(NAME)

ifneq ($(strip $(rel)),)
ifeq ($(strip $(prof)),)
ifeq ($(strip $(arch)),win)
	@strip $(NAME).exe
else
	@strip $(NAME)
endif
endif
endif


re: clean
	@$(MAKE) --no-print-directory

f: clean
	@$(MAKE) rel=a relp=begin --no-print-directory
	@$(MAKE) arch=win rel=a relp=end --no-print-directory

$(NAME): $(OBJS) | $(@D)
	$(PREREQ_DIR)

	$(CXX) $(CFLAGSSTD) -o $(NAME) $(OBJS) $(LFLAGS)

$(OBJS): $(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	$(PREREQ_DIR)
	$(CXX) $(CFLAGSSTD) -o $@ -c $<

clean:
	@echo
	@echo "cleaning old build files"
	@echo
	@$(MAKE) --no-print-directory cleanbuild
	@$(MAKE) --no-print-directory cleanexec

cleanbuild:
	rm -f build/*.o
	rm -f src/agh/*

cleanexec:
	rm -f $(NAME) $(NAME).exe

.PHONY: all clean

