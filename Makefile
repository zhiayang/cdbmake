# Makefile
# Copyright (c) 2021, zhiayang
# Licensed under the Apache License Version 2.0.

WARNINGS        = -Wno-padded -Wno-cast-align -Wno-unreachable-code -Wno-packed -Wno-missing-noreturn -Wno-float-equal -Wno-unused-macros -Werror=return-type -Wextra -Wno-unused-parameter -Wno-trigraphs -Wconversion

COMMON_CFLAGS   = -Wall -O2

CC              := clang
CXX             := clang++

CFLAGS          = $(COMMON_CFLAGS) -std=c99 -fPIC -O3
CXXFLAGS        = $(COMMON_CFLAGS) -Wno-old-style-cast -std=c++20 -fno-exceptions

CXXSRC          = $(shell find source -iname "*.cpp" -print)
CXXOBJ          = $(CXXSRC:.cpp=.cpp.o)
CXXDEPS         = $(CXXOBJ:.o=.d)

DEFINES         :=
INCLUDES        := -Isource -Iexternal

OUTPUT_BIN      := build/cdbmake

.PHONY: all clean build
.DEFAULT_GOAL = all

all: build

test: build
	@$(OUTPUT_BIN)

build: $(OUTPUT_BIN)

$(OUTPUT_BIN): $(CXXOBJ)
	@echo "  $(notdir $@)"
	@mkdir -p build
	@$(CXX) $(CXXFLAGS) $(WARNINGS) $(DEFINES) -Iexternal -o $@ $^

%.cpp.o: %.cpp Makefile
	@echo "  $(notdir $<)"
	@$(CXX) $(CXXFLAGS) $(WARNINGS) $(INCLUDES) $(DEFINES) -MMD -MP -c -o $@ $<

%.c.o: %.c Makefile
	@echo "  $(notdir $<)"
	@$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	-@find source -iname "*.cpp.d" | xargs rm
	-@find source -iname "*.cpp.o" | xargs rm
	-@rm -f $(OUTPUT_BIN)

-include $(CXXDEPS)
-include $(CDEPS)










