-include Makefile.local

# Building
## Release or Debug
#
#  make BUILD_TYPE=Debug
#  make BUILD_TYPE=Release
#
# To build either in debug mode, or in release mode
#
# Debugging
## Run in debugger
#
#  make tests DEBUG=1 ARGS=--gtest_filter=World*
#
# to only build the tests targets, and run it via gdb afterwards.
# It will automatically start the execution and forward the arguments
# that are given via ARGS to the target
#
## Verbose output
#
#  make Q=
#
# This will print out information about the commands that are executed to build
# and run the target(s)

Q                 = @
OS               := $(shell uname)
LOCAL_CONFIG_DIR  = ~/.local/share/engine

VALGRIND         ?=
ifeq ($(VALGRIND),)
VALGRIND_CMD     ?=
else
VALGRIND_CMD     ?= valgrind
endif

DEBUG            ?=
ifeq ($(DEBUG),)
DEBUG_CMD        ?=
else
DEBUGGER         := $(shell (gdb --help >/dev/null 2>&1 && echo GDB) || (lldb --help >/dev/null 2>&1 && echo LLDB))
ifeq ($(DEBUGGER),GDB)
DEBUG_CMD        ?= gdb -ex run --args
else ifeq ($(DEBUGGER),LLDB)
DEBUG_CMD        ?= lldb -b -o run
else
DEBUG_CMD        ?=
endif
endif

BUILD_TYPE       ?= Debug
# override this in your Makefile.local to use a different directory
BUILDDIRPATH     ?= ./
#BUILDDIR         ?= $(BUILDDIRPATH)build-$(shell echo $(BUILD_TYPE) | tr '[:upper:]' '[:lower:]')
BUILDDIR         ?= $(BUILDDIRPATH)build/$(BUILD_TYPE)

CMAKE_OPTIONS    ?=

#VOGL_OPTIONS     ?= --vogl_force_debug_context --vogl_exit_after_x_frames 2000
VOGL_OPTIONS     ?= --vogl_force_debug_context
VOGL             ?=
ifeq ($(VOGL),)
VOGL_CMD         ?=
else
VOGL_BIN         ?= vogl
VOGL_CMD         ?= $(VOGL_BIN) trace --vogl_tracepath $(BUILDDIR) --vogl_tracefile $@.trace.bin $(VOGL_OPTIONS)
ARGS_TMP         := $(ARGS)
ARGS              = "--args $(ARGS_TMP)"
endif

MAKE_PID     := $$PPID
JOB_FLAG     := $(filter -j%, $(subst -j ,-j,$(shell ps T | grep "^\s*$(MAKE_PID).*$(MAKE)")))
MAKE_OPTIONS := --no-print-directory -C $(BUILDDIR)

ifeq ($(OS),Darwin)
CMAKE_GENERATOR ?= "Xcode"
CMAKE_BINARY    ?= /Applications/CMake.app/Contents/bin/cmake
DARWIN          := 1
else ifeq ($(OS),Linux)
CMAKE_GENERATOR ?= "Eclipse CDT4 - Unix Makefiles"
CMAKE_BINARY    ?= cmake
LINUX           := 1
else
CMAKE_GENERATOR ?= "MSYS Makefiles"
CMAKE_BINARY    ?= cmake
WINDOWS         := 1
endif
INSTALL_DIR     ?= $(BUILDDIRPATH)$(OS)

all: build

run: shapetool

.PHONY: clangtidy
clangtidy:
	$(Q)mkdir -p $(BUILDDIR)/tidy
	$(Q)cd $(BUILDDIR)/tidy; $(CMAKE_BINARY) -DCMAKE_CXX_CLANG_TIDY:STRING="clang-tidy-4.0;-checks=readability-uniqueptr-delete-release,readability-non-const-parameter,readability-redundant-smartptr-get,performance-unnecessary-value-param,performance-unnecessary-copy-initialization,performance-inefficient-string-concatenation,performance-implicit-cast-in-loop,performance-for-range-copy,performance-faster-string-find,modernize-make-shared,clang-analyzer-security.*;-fix" $(CURDIR) $(CMAKE_OPTIONS) && cmake --build .

.PHONY: cmake
cmake:
	$(Q)mkdir -p $(BUILDDIR)
	$(Q)cd $(BUILDDIR); $(CMAKE_BINARY) -G$(CMAKE_GENERATOR) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CURDIR) $(CMAKE_OPTIONS)

define COMPILE
$(if $(LINUX),\
	$(Q)$(MAKE) $(MAKE_OPTIONS) $(JOB_FLAG) $(1) \
$(else),\
	$(if $(DARWIN),\
		$(Q)cd $(BUILDDIR); xcodebuild build -target $(1) install -project tests.xcodeproj -configuration $(BUILD_TYPE) \
	$(else),\
		$(Q)$(MAKE) $(MAKE_OPTIONS) $(JOB_FLAG) $(1) \
	)
)
endef

.PHONY: build
build: cmake
	$(call COMPILE, install)

clean:
	$(Q)rm -rf $(BUILDDIR)

clean-local-config:
	$(Q)rm -rf $(LOCAL_CONFIG_DIR)

edit-local-config:
	$(Q)$(EDITOR) $(LOCAL_CONFIG_DIR)/shapetool/shapetool.vars

doc: cmake
	$(call COMPILE, $@)

server client voxedit shapetool shadertool noisetool databasetool uitool tests testmesh testcamera testdepthbuffer testtexture testvoxelfont testplane flatc: cmake
	$(call COMPILE, $@)
	$(call COMPILE, copy-data-shared)
	$(call COMPILE, copy-data-$@)
	$(Q)cd $(BUILDDIR); $(VALGRIND_CMD) $(DEBUG_CMD) $(VOGL_CMD) ./$@ $(ARGS)

rcon: cmake
	$(call COMPILE, $@)
	$(Q)cd $(BUILDDIR); $(VALGRIND_CMD) $(DEBUG_CMD) $(VOGL_CMD) ./$@ $(ARGS)

test-material-color: cmake
	$(call COMPILE, tests)
	$(Q)cd $(BUILDDIR); $(VALGRIND_CMD) $(DEBUG_CMD) ./tests --gtest_color=yes --gtest_filter=MaterialTest* -- $(ARGS)
	$(Q)xdg-open build/material.png

test-ambient-occlusion: cmake
	$(call COMPILE, tests)
	$(Q)cd $(BUILDDIR); $(VALGRIND_CMD) $(DEBUG_CMD) ./tests --gtest_color=yes --gtest_filter=AmbientOcclusionTest* -- $(ARGS)

.PHONY: remotery
remotery:
	$(Q)xdg-open file://$(CURDIR)/tools/remotery/index.html

.PHONY: tags
tags:
	$(Q)ctags -R src

define UPDATE_GIT
	$(Q)if [ ! -d $(1).sync ]; then \
		git clone $(2) $(1).sync; \
	else \
		cd $(1).sync && git pull --rebase && cd ..; \
	fi;
endef

define UPDATE_HG
	$(Q)if [ ! -d $(1).sync ]; then \
		hg clone $(2) $(1).sync; \
	else \
		cd $(1).sync && hg pull && hg update && cd ..; \
	fi;
endef

# TODO turbobadger (but currently we have custom patches)
updatelibs:
	$(call UPDATE_GIT,assimp,https://github.com/assimp/assimp.git)
	rm -rf contrib/libs/assimp/code/* contrib/libs/assimp/include/*
	cp -r assimp.sync/code/* contrib/libs/assimp/code
	cp -r assimp.sync/include/* contrib/libs/assimp/include
	git checkout contrib/libs/assimp/include/assimp/revision.h
	$(call UPDATE_GIT,flatbuffers,https://github.com/google/flatbuffers.git)
	rm -rf contrib/libs/flatbuffers/flatbuffers/* contrib/libs/flatbuffers/compiler/*
	mkdir -p contrib/libs/flatbuffers/compiler/src
	cp -r flatbuffers.sync/include/flatbuffers/* contrib/libs/flatbuffers/flatbuffers
	cp -r flatbuffers.sync/src/* contrib/libs/flatbuffers/compiler
	cp -r flatbuffers.sync/grpc/src/* contrib/libs/flatbuffers/compiler/src
	rm contrib/libs/flatbuffers/compiler/flathash.cpp
	$(call UPDATE_GIT,libenet,https://github.com/lsalzman/enet.git)
	cp -r libenet.sync/*.[ch] contrib/libs/libenet
	cp -r libenet.sync/include/* contrib/libs/libenet/include
	$(call UPDATE_GIT,glm,https://github.com/g-truc/glm.git)
	rm -rf contrib/libs/glm/glm/*
	cp -r glm.sync/glm/* contrib/libs/glm/glm
	rm contrib/libs/glm/glm/CMakeLists.txt
	$(call UPDATE_HG,sdl2,https://hg.libsdl.org/SDL)
	rm -rf contrib/libs/sdl2/src/* contrib/libs/sdl2/include/*
	cp -r sdl2.sync/src/* contrib/libs/sdl2/src
	cp -r sdl2.sync/include/* contrib/libs/sdl2/include
	mv contrib/libs/sdl2/include/SDL_config.h contrib/libs/sdl2/config/
	rm contrib/libs/sdl2/include/SDL_config.h.in
	git add contrib/libs
