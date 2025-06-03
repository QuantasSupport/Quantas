# Copyright 2022

# This file is part of QUANTAS.  QUANTAS is free software: you can
# redistribute it and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
# QUANTAS is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.  You should have received a
# copy of the GNU General Public License along with QUANTAS. If not,
# see <https://www.gnu.org/licenses/>.

############################### Input ###############################

#  Configure this for the specific input file.
#  Make sure to include the path to the input file 

# INPUTFILE := quantas/ExamplePeer/ExampleInput.json

# INPUTFILE := quantas/PBFTPeer/_PBFTInput.json

# INPUTFILE := quantas/AltBitPeer/AltBitUtility.json

# INPUTFILE := quantas/BitcoinPeer/BitcoinInput.json

# INPUTFILE := quantas/PBFTPeer/PBFTInput.json

INPUTFILE := quantas/PBFTPeer/PBFTInputV2.json

############################### Variables and Flags ###############################

EXE := quantas.exe

# compiles all the cpps in Common and main.cpp
COMMON_SRCS := $(wildcard quantas/Common/*.cpp)
COMMON_OBJS := $(COMMON_SRCS:.cpp=.o)

ABSTRACT_OBJS := $(COMMON_OBJS) quantas/Common/Abstract/abstractSimulation.o quantas/Common/Abstract/Channel.o quantas/Common/Abstract/Network.o
CONCRETE_OBJS := $(COMMON_OBJS) quantas/Common/Concrete/concreteSimulation.o quantas/Common/Concrete/ipUtil.o

# compiles all cpps specified as necessary in the INPUTFILE
ALGS := $(shell sed -n '/"algorithms"/,/]/p' $(INPUTFILE) \
         | sed -n 's/.*"\([^"]*\.cpp\)".*/quantas\/\1/p')
ALG_OBJS += $(ALGS:.cpp=.o)

# necessary flags
CXX := g++
CXXFLAGS := -pthread -std=c++17
GCC_VERSION := $(shell $(CXX) $(CXXFLAGS) -dumpversion)
GCC_MIN_VERSION := 8

############################### Build Types ###############################

# release for faster runtime, debug for debugging
release: CXXFLAGS += -O3 -s
release: check-version $(EXE)
debug: CXXFLAGS += -O0 -g -D_GLIBCXX_DEBUG
debug: check-version $(EXE)

############################### Running Commands ###############################

# When running on windows use make clang
clang: CXX := clang++
clang: release
	@echo running with input: $(INPUTFILE)
	@./$(EXE) $(INPUTFILE)

# When running on Linux use make run
run: release
	@echo running with input: $(INPUTFILE)
	@./$(EXE) $(INPUTFILE); exit_code=$$?; \
	if [ $$exit_code -ne 0 ]; then $(call check_failure); exit $$exit_code; fi

############################### Debugging ###############################

# runs the program with full Valgrind to trace memory leaks
run_memory: debug
	@echo running: $(INPUTFILE) with valgrind
	@valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
		 ./$(EXE) $(INPUTFILE)

# runs the program with Valgrind to see if there are any memory leaks
run_simple_memory: debug
	@echo ""
	@echo running: $(INPUTFILE) with valgrind
	@valgrind --leak-check=full ./$(EXE) $(INPUTFILE) 2>&1 \
		| grep -E "HEAP SUMMARY|in use|LEAK SUMMARY|definitely lost: |indirectly lost: |possibly lost: |still reachable: |ERROR SUMMARY"
	@echo ""

# runs the program with GDB for more advanced error viewing
run_debug: debug
	@gdb --ex "set print thread-events off" --ex run --ex backtrace --args ./$(EXE) $(INPUTFILE); exit_code=$$?; \
	if [ $$exit_code -ne 0 ]; then $(call check_failure); exit $$exit_code; fi

############################### Tests ###############################

# Test thread based random number generation
rand_test: quantas/Tests/randtest.cpp
	@echo "Testing thread based random number generation..."
	@make --no-print-directory clean
	@$(CXX) $(CXXFLAGS) $^ -o $@.exe
	@./$@.exe
	@echo ""
	
# in the future this could be generalized to go through every file in a Tests
# folder such that the input files need not be listed here
TEST_INPUTS := quantas/ExamplePeer/ExampleInput.json quantas/AltBitPeer/AltBitUtility.json quantas/PBFTPeer/PBFTInput.json quantas/BitcoinPeer/BitcoinInput.json

test: check-version rand_test
	@make --no-print-directory clean
	@echo "Running memory tests on all test inputs..."
	@echo ""
	@for file in $(TEST_INPUTS); do \
		$(MAKE) --no-print-directory run_simple_memory INPUTFILE="$$file"; \
	done

############################### Helpers ###############################

# Define a helper function to check dmesg for errors
define check_failure
    @echo "Make target '$@' failed! Checking dmesg..."; \
    dmesg | tail -20 | grep -i -E 'oom|killed|segfault|error' || echo "No relevant logs found."
endef

%.o: %.cpp
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# check the version of the GCC compiler being used is above a threshold
check-version:
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then echo "Default version of g++ must be higher than 8."; fi
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then echo "To change the default version visit: https://linuxconfig.org/how-to-switch-between-multiple-gcc-and-g-compiler-versions-on-ubuntu-20-04-lts-focal-fossa"; fi
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then exit 1; fi

$(EXE): $(ALG_OBJS) $(ABSTRACT_OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $(EXE)

############################### Cleanup ###############################

# enables recursive glob patterns for bash to clean out unecessary files
clean: SHELL := /bin/bash -O globstar
clean:
	@$(RM) **/*.out
	@$(RM) **/*.o
	@$(RM) **/*.d
	@$(RM) **/*.dSYM
	@$(RM) **/*.gch
	@$(RM) **/*.tmp
	@$(RM) **/*.exe

clean_txt: SHELL := /bin/bash -O globstar
clean_txt:
	@$(RM) **/*.txt

# -include $(OBJS:.o=.d)

############################### PHONY ###############################

# All make commands found in this file
.PHONY: clean run release debug $(EXE) %.o clang run_memory run_simple_memory run_debug check-version rand_test test clean_txt
