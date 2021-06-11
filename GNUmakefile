# -*- mode: gnumakefile; tab-width: 4; indent-tabs-mode: t; -*-

# This Makefile does an out-of-source CMake build in ../build_`OS`_`CPU`
# eg:
#   ../build_linux_i386
# This is for users who like to configure & build covah with a single command.

define HELP_TEXT

Convenience Targets
   Provided for building COVAH, (multiple at once can be used).

   * debug:         Build a debug binary.
   * release:       Complete build with all options enabled.
   * deps:          Build all library dependencies.

   * developer:     Enable faster builds, error checking and tests, recommended for developers.
   * config:        Run cmake configuration tool to set build options.
   * ninja:         Use ninja build tool for faster builds.

   Note: passing the argument 'BUILD_DIR=path' when calling make will override the default build dir.
   Note: passing the argument 'BUILD_CMAKE_ARGS=args' lets you add cmake arguments.

Package Targets

   * package_debian:    Build a debian package.
   * package_pacman:    Build an arch linux pacman package.
   * package_archive:	  Build an archive package.

Testing Targets

   * test:
     Run automated tests with ctest.

Utilities

   * install:
     Installs Covah along with Pixar USD (Useful for updating python scripts).	 	 

   * icons:
     Updates PNG icons from SVG files.

     Optionally pass in variables: 'COVAH_BIN', 'INKSCAPE_BIN'
     otherwise default paths are used.

     Example
        make icons INKSCAPE_BIN=/path/to/inkscape

   * source_archive:
     Create a compressed archive of the source code.

   * update:
     Updates to latest source code revision.

   * format
     Format source code using clang (uses PATHS if passed in). For example::

        make format PATHS="source/covah/wm source/covah/covakernel"

Environment Variables

   * BUILD_CMAKE_ARGS:     Arguments passed to CMake.
   * BUILD_DIR:            Override default build path.
   * PYTHON:               Use this for the Python command (used for checking tools).
   * NPROCS:               Number of processes to use building (auto-detect when omitted).

Documentation Targets
   Not associated with building COVAH.

   * doc_all:     Generate sphinx C/C++ docs.
   * doc_man:     Generate manpage.

Information

   * help:              This help message.

endef
# HELP_TEXT (end)

# This makefile is not meant for Windows
ifeq ($(OS),Windows_NT)
    $(error On Windows, use "cmd //c make.bat" instead of "make")
endif

# System Vars
OS:=$(shell uname -s)
OS_NCASE:=$(shell uname -s | tr '[A-Z]' '[a-z]')
CPU:=$(shell uname -m)


# Source and Build DIR's
COVAH_DIR:=$(shell pwd -P)
BUILD_TYPE:=Release

# CMake arguments, assigned to local variable to make it mutable.
CMAKE_CONFIG_ARGS := $(BUILD_CMAKE_ARGS)

ifndef BUILD_DIR
	BUILD_DIR:=$(shell dirname "$(COVAH_DIR)")/build_$(OS_NCASE)
endif

# Dependencies DIR's
DEPS_INSTALL_SCRIPT:=$(COVAH_DIR)/build_files/build_environment/install_deps.py

ifndef DEPS_BUILD_DIR
    DEPS_BUILD_DIR:=$(shell dirname "$(COVAH_DIR)")/lib/linux_centos7_x86_64/build_env/build
endif

ifndef DEPS_INSTALL_DIR
	DEPS_INSTALL_DIR:=$(shell dirname "$(COVAH_DIR)")/lib/linux_centos7

	ifneq ($(OS_NCASE),darwin)
		# Add processor type to directory name
		DEPS_INSTALL_DIR:=$(DEPS_INSTALL_DIR)_$(CPU)
	endif
endif

# Prefer the python we ship.
ifndef PYTHON
	ifneq (, $(wildcard /usr/local/share/covah/1.50/python/bin/python3.9))
		PYTHON:=/usr/local/share/covah/1.50/python/bin/python3.9
	else
		PYTHON:=python3.9
	endif
endif

# For macOS python3 is not installed by default, so fallback to python binary
# in libraries, or python 2 for running make update to get it.
ifeq ($(OS_NCASE),darwin)
	ifeq (, $(shell command -v $(PYTHON)))
		PYTHON:=../lib/darwin/python/bin/python3.9m
		ifeq (, $(shell command -v $(PYTHON)))
			PYTHON:=python
		endif
	endif
endif

# -----------------------------------------------------------------------------
# additional targets for the build configuration

# support 'make debug'
ifneq "$(findstring debug, $(MAKECMDGOALS))" ""
	BUILD_DIR:=$(BUILD_DIR)_debug
	BUILD_TYPE:=Debug
endif
ifneq "$(findstring release, $(MAKECMDGOALS))" ""
	BUILD_DIR:=$(BUILD_DIR)_release
	CMAKE_CONFIG_ARGS:=-C"$(COVAH_DIR)/build_files/cmake/config/wabi_release.cmake" $(CMAKE_CONFIG_ARGS)
endif

ifneq "$(findstring developer, $(MAKECMDGOALS))" ""
	CMAKE_CONFIG_ARGS:=-C"$(COVAH_DIR)/build_files/cmake/config/covah_developer.cmake" $(CMAKE_CONFIG_ARGS)
endif

# -----------------------------------------------------------------------------
# build tool

ifneq "$(findstring ninja, $(MAKECMDGOALS))" ""
	CMAKE_CONFIG_ARGS:=$(CMAKE_CONFIG_ARGS) -G Ninja
	BUILD_COMMAND:=ninja
	DEPS_BUILD_COMMAND:=ninja
else
	ifneq ("$(wildcard $(BUILD_DIR)/build.ninja)","")
		BUILD_COMMAND:=ninja
	else
		BUILD_COMMAND:=make -s
	endif

	ifneq ("$(wildcard $(DEPS_BUILD_DIR)/build.ninja)","")
		DEPS_BUILD_COMMAND:=ninja
	else
		DEPS_BUILD_COMMAND:=make -s
	endif
endif

# -----------------------------------------------------------------------------
# COVAH binary path

# Allow passing in own COVAH_BIN so developers who don't
# use the default build path can still use utility helpers.
ifeq ($(OS), Darwin)
	COVAH_BIN?="$(BUILD_DIR)/bin/COVAH.app/Contents/MacOS/COVAH"
else
	COVAH_BIN?="$(BUILD_DIR)/bin/covah"
endif


# -----------------------------------------------------------------------------
# Get the number of cores for threaded build
# You may increase the value of 'RESPONSIVENESS'
# to reduce the number of cores.
ifndef NPROCS
	NPROCS:=1
	RESPONSIVENESS:=12
	ifeq ($(OS), Linux)
		NPROCS:=$(shell nproc)
	endif
	ifeq ($(OS), NetBSD)
		NPROCS:=$(shell getconf NPROCESSORS_ONLN)
	endif
	ifneq (,$(filter $(OS),Darwin FreeBSD))
		NPROCS:=$(shell sysctl -n hw.ncpu)
	endif
	NPROCS:=$(shell expr $(NPROCS) - $(RESPONSIVENESS))
endif


# -----------------------------------------------------------------------------
# Macro for configuring cmake

CMAKE_CONFIG = cmake $(CMAKE_CONFIG_ARGS) \
                     -H"$(COVAH_DIR)" \
                     -B"$(BUILD_DIR)" \
                     -DCMAKE_BUILD_TYPE_INIT:STRING=$(BUILD_TYPE)


# -----------------------------------------------------------------------------
# Tool for 'make config'

# X11 specific
ifdef DISPLAY
	CMAKE_CONFIG_TOOL = cmake-gui
else
	CMAKE_CONFIG_TOOL = ccmake
endif


# -----------------------------------------------------------------------------
# Build COVAH
all: .FORCE
	@echo
	@echo Configuring COVAH in \"$(BUILD_DIR)\" ...

#	# if test ! -f $(BUILD_DIR)/CMakeCache.txt ; then \
#	# 	$(CMAKE_CONFIG); \
#	# fi

#	# do this always incase of failed initial build, could be smarter here...
	@$(CMAKE_CONFIG)

	@echo
	@echo Building Covah & Pixar USD...
	$(BUILD_COMMAND) -C "$(BUILD_DIR)" -j $(NPROCS) install
	@echo
	@echo edit build configuration with: "$(BUILD_DIR)/CMakeCache.txt" run make again to rebuild.
	@echo Covah successfully built, run from: /usr/local/share/covah/1.50/bin/covah
	@echo

debug: all
release: all
developer: all
ninja: all

# -----------------------------------------------------------------------------
# Run Install (So you don't have to rebuild everytime, python scripts, etc)
install: .FORCE
	@echo
	@echo Installing Covah and Pixar USD...

	@echo
	cd "$(BUILD_DIR)" ; ninja install
	@echo
	@echo Covah and Pixar USD successfully installed,
	@echo Run Covah from: /usr/local/share/covah/1.50/bin/covah
	@echo Run Python or test Pixar UsdView from: $(PYTHON).
	@echo

# -----------------------------------------------------------------------------
# Build all required dependencies to build Covah and Pixar USD

# 
# Build dependencies
DEPS_TARGET = install
ifneq "$(findstring clean, $(MAKECMDGOALS))" ""
	DEPS_TARGET = clean
endif

deps: .FORCE
	@echo
	@echo Configuring dependencies in \"$(DEPS_BUILD_DIR)\"

#	#@python3.9 $(DEPS_INSTALL_SCRIPT) --build clean
	@python3.9 $(DEPS_INSTALL_SCRIPT) --build all

	@echo
	@echo Dependencies successfully built and installed to $(DEPS_INSTALL_DIR).
	@echo

# -----------------------------------------------------------------------------
# Configuration (save some cd'ing around)
config: .FORCE
	$(CMAKE_CONFIG_TOOL) "$(BUILD_DIR)"


# -----------------------------------------------------------------------------
# Help for build targets
export HELP_TEXT
help: .FORCE
	@echo "$$HELP_TEXT"

# -----------------------------------------------------------------------------
# Packages
#
package_debian: .FORCE
	cd build_files/package_spec ; DEB_BUILD_OPTIONS="parallel=$(NPROCS)" sh ./build_debian.sh

package_pacman: .FORCE
	cd build_files/package_spec/pacman ; MAKEFLAGS="-j$(NPROCS)" makepkg

package_archive: .FORCE
	make -C "$(BUILD_DIR)" -s package_archive
	@echo archive in "$(BUILD_DIR)/release"


# -----------------------------------------------------------------------------
# Tests
#
test: .FORCE
	$(PYTHON) ./build_files/utils/make_test.py "$(BUILD_DIR)"

# -----------------------------------------------------------------------------
# Utilities
#

source_archive: .FORCE
	./build_files/utils/make_source_archive.sh

INKSCAPE_BIN?="inkscape"
icons: .FORCE
	COVAH_BIN=$(COVAH_BIN) INKSCAPE_BIN=$(INKSCAPE_BIN) \
		"$(COVAH_DIR)/release/datafiles/covah_icons_update.py"
	COVAH_BIN=$(COVAH_BIN) INKSCAPE_BIN=$(INKSCAPE_BIN) \
		"$(COVAH_DIR)/release/datafiles/prvicons_update.py"

update: .FORCE
	$(PYTHON) ./build_files/utils/make_update.py

format: .FORCE
	$(PYTHON) source/tools/utils_maintenance/clang_format_paths.py $(PATHS)


# -----------------------------------------------------------------------------
# Documentation
#

doc_all: .FORCE
	cd doc/sphinx; make html
	@echo "docs written into: '$(COVAH_DIR)/doc/_build'"

doc_man: .FORCE
	$(PYTHON) doc/manpage/covah.1.py $(COVAH_BIN) covah.1

clean: .FORCE
	$(BUILD_COMMAND) -C "$(BUILD_DIR)" clean

.PHONY: all

.FORCE:
