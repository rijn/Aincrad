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
DEPS = arguments.o config.o util.o
OBJS_DIR = objs
OPTIMIZE = off
INCLUDES = -I./src/ -I$(OBJS_DIR)/ -I./src/lib/
VPATH = ./src/ ./src/lib/ $(OBJS_DIR)
WARNINGS = -pedantic -Wall -Werror -Wfatal-errors -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = $(INCLUDES) -std=c++11 -stdlib=libc++ -stdlib=libc++ -lpthread $(WARNINGS)
CXXFLAGS = $(INCLUDES) -std=c++11 -stdlib=libc++ -stdlib=libc++ -MMD -MP $(WARNINGS)
-include $(OBJS_DIR)/*.d

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
debug: clean pre-compile $(EXENAME)-debug echo-done

pre-compile: echo-compile $(OBJS_DIR)

echo-compile:
	@echo "compiling..."

echo-done:
	@echo -e "done."

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

.PHONY: clean
clean:
	@rm -f $(wildcard *.d) $(wildcard *.o) $(wildcard *.cgo) $(wildcard *.cga) $(EXENAME) $(CCMONAD) $(IDFILE)
	@rm -rf .objs
