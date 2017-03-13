#    ▄▄   ▄▄▄▄▄  ▄▄   ▄   ▄▄▄  ▄▄▄▄▄    ▄▄   ▄▄▄▄
#    ██     █    █▀▄  █ ▄▀   ▀ █   ▀█   ██   █   ▀▄
#   █  █    █    █ █▄ █ █      █▄▄▄▄▀  █  █  █    █
#   █▄▄█    █    █  █ █ █      █   ▀▄  █▄▄█  █    █
#  █    █ ▄▄█▄▄  █   ██  ▀▄▄▄▀ █    ▀ █    █ █▄▄▄▀

EXENAME = aincrad

SHELL = /bin/bash

ROOT_DIR=$(shell pwd)

CXX = clang++
LD = clang++
OBJS = $(EXENAME).o
DEPS = arguments.o config.o util.o client.o server.o package.o command.o
OBJS_DIR = objs
OPTIMIZE = off
INCLUDES = -I./src/ -I$(OBJS_DIR)/ -I./src/lib/ -I/usr/local/include
DEFINE = -DASIO_HAS_STD_ATOMIC
VPATH = ./src/ ./src/lib/ $(OBJS_DIR)
WARNINGS = -pedantic -Wall -Werror -Wfatal-errors -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = $(INCLUDES) -std=c++14 -stdlib=libc++ -lpthread -L/usr/local/lib -lboost_system -lboost_system-mt $(WARNINGS)
CXXFLAGS = $(INCLUDES) $(DEFINE) -std=c++14 -stdlib=libc++ -MMD -MP $(WARNINGS)
-include $(OBJS_DIR)/*.d

BUILD_SCRIPTS_DIR    = ./build-scripts
TAGS_TARGETS         = $(BUILD_SCRIPTS_DIR)/tags.mk
YCM_FLAGS_TARGETS    = $(BUILD_SCRIPTS_DIR)/ycm_flags.mk
YCM_FLAGS_TEMPLATE   = $(BUILD_SCRIPTS_DIR)/ycm_extra_conf_template.py

.PHONY: all
all: release

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

ifeq ($(strip $(OPTIMIZE)),on)
CXXFLAGS += -O2 -DOPTIMIZE
else ifeq ($(strip $(OPTIMIZE)),off)
CXXFLAGS += -g -O0
else
$(warning Invalid value specified for OPTIMIZE. Should be on or off)
CXXFLAGS += -g -O0
endif

.PHONY: release debug
release: pre-compile $(EXENAME) echo-done
debug: pre-compile $(EXENAME)-debug echo-done

pre-compile: echo-compile $(OBJS_DIR)

echo-compile:
	@echo "compiling..."

echo-done:
	@echo -e "\033[;32mdone.\033[0m"

$(EXENAME): $(DEPS:%.o=$(OBJS_DIR)/%.o) $(OBJS:%.o=$(OBJS_DIR)/%.o)
	@echo -e " ld\t$<"
	@$(LD) $^ $(LDFLAGS) -o $@

$(EXENAME)-debug: $(OBJS:%.o=$(OBJS_DIR)/%-debug.o) $(DEPS:%.o=$(OBJS_DIR)/%-debug.o)
	@echo -e " ld\t$(EXENAME)"
	@$(LD) $^ $(LDFLAGS) -o $@

$(OBJS_DIR)/%.o:%.cpp
	@echo -e " cc\t$<"
	@$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJS_DIR)/%-debug.o: %.cpp
	@echo -e " cc\t$<"
	@$(CXX) -c $(CXXFLAGS) -DDEBUG $< -o $@

svn.o: svn.h svn.cpp svn_libsvn.cpp
ifeq ($(IS_LIBSVN),0)
	$(CXX) $(CXXFLAGS) -I/usr/include/apr-1 -I/usr/include/apr-1.0 -I/usr/include/subversion-1 -Wno-deprecated-declarations -c svn_libsvn.cpp -o $@ || $(CXX) $(CXXFLAGS) -c svn.cpp -o $@
else
	$(CXX) $(CXXFLAGS) -c svn.cpp -o $@
endif

.PHONY: help
help:
	@echo -ne "\033[;32m"
	@echo "Please use \`make <target>' where <target> is one of"
	@echo -e "\033[0m"
	@echo "  help     to show this help"
	@echo "  new      clean and build release"
	@echo "  release  to build release version"
	@echo "  debug    to build debug version with define"
	@echo "  cloc     show code statistics"
	@echo "  tags     to generate tags file"
	@echo "  ycm_extra_conf"
	@echo "       	  to generate .ycm_extra_conf.py file for YCM completion"

.PHONY: help
cloc:
	@echo -e "\033[;32m"
	@cloc . --exclude-list-file="src/lib/optionparser.h" --exclude-dir="build-scripts" --ignore-whitespace --exclude-lang="D"
	@echo -e "\033[0m"

include $(TAGS_TARGETS)
include $(YCM_FLAGS_TARGETS)

.PHONY: new
new: clean all

.PHONY: clean
clean:
	@rm -f $(wildcard *.d) $(wildcard *.o) $(wildcard *.cgo) $(wildcard *.cga) $(EXENAME) $(CCMONAD) $(IDFILE)
	@rm -rf $(OBJS_DIR)

BOOST_PATH = $(shell brew info boost | sed -n 4p | cut -d ' ' -f 1)
PATCH_PATH = $(BOOST_PATH)/include/boost/asio/detail/

.PHONY: patch
patch:
	@echo "Boost Path = $(BOOST_PATH)"
	@echo -n "Patch to path $(PATCH_PATH) ?"
	@read -rs -t5 -n 1 yn;

	@echo ""

	@echo "patching..."

	cp -f "./asio_patch/fenced_block.hpp" "$(PATCH_PATH)fenced_block.hpp"
	cp -f "./asio_patch/std_fenced_block.hpp" "$(PATCH_PATH)std_fenced_block.hpp"

	@echo -e "\033[;32mdone.\033[0m"

