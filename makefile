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
LD=-lraylib $(shell pkg-config --libs mupdf) -lpthread -ldl

else ifeq ($(strip $(arch)),win)

CXX=x86_64-w64-mingw32-g++
CC=x86_64-w64-mingw32-ld
C=x86_64-w64-mingw32-gcc
DEPDEF=-DTARGET_WIN

endif

ifeq ($(strip $(rel)),)
ifneq ($(strip $(torch)),)
#RELFLAGS=-O3
RELFLAGS=-Og -g
else
RELFLAGS=-Og -g
endif
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
LFLAGS+=-static -static-libgcc -static-libstdc++ -L./dpd/raylib/src -lraylib $(LFLAGSOSD) data/misc/i.res
endif

PREREQ_DIR=@mkdir -p $(@D)

SRCDIR=src
BUILDDIR=build
BINDIR=bin

NAME=$(addprefix $(BINDIR)/, stave_viewer)

SRCS=$(wildcard $(SRCDIR)/*.cc)
OBJS=$(patsubst $(SRCDIR)/%.cc, $(BUILDDIR)/%.o, $(SRCS))

all:
	@mkdir -p ./src/agh

#ifneq ($(strip $(relp)),end)
	#@$(MAKE) --no-print-directory pre
#endif
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
	@$(MAKE) rel=a relp=begin torch=y --no-print-directory
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

