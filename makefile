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

################
#
#  Configure this for the specific input file.
#  Make sure to include the path to the input file 
#  from the location of this makefile.

# INPUTFILE := quantas/BitcoinPeer/BitcoinSpeedTest.json

# INPUTFILE := quantas/ExamplePeer/ExampleInput.json

INPUTFILE := quantas/BitcoinPeer/BitcoinInput.json

################

EXE := quantas.exe

# compiles all the cpps in Common and main.cpp
COMMON_SRCS := $(wildcard quantas/Common/*.cpp)
COMMON_OBJS := $(COMMON_SRCS:.cpp=.o)
OBJS := $(COMMON_OBJS) quantas/main.o
# compiles all cpps specified as necessary in the INPUTFILE
ALGS := $(shell sed -n '/"algorithms"/,/]/p' $(INPUTFILE) \
         | sed -n 's/.*"\([^"]*\.cpp\)".*/quantas\/\1/p')
OBJS += $(ALGS:.cpp=.o)

# necessary flags
CPPFLAGS := -Iinclude -MMD -MP
CXXFLAGS := -pthread
CXX := g++
GCC_VERSION := $(shell $(CXX) -dumpversion)
GCC_MIN_VERSION := 8

# check the version of the GCC compiler being used
check-version:
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then echo "Default version of g++ must be higher than 8."; fi
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then echo "To change the default version visit: https://linuxconfig.org/how-to-switch-between-multiple-gcc-and-g-compiler-versions-on-ubuntu-20-04-lts-focal-fossa"; fi
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then exit 1; fi

# extra debug and release flags
release: CXXFLAGS += -O3 -s -std=c++17
debug: CXXFLAGS += -O0 -g -D_GLIBCXX_DEBUG -std=c++17

# flags for clang
clang: CXX := clang++
clang: CXXFLAGS += -std=c++17

.PHONY: clean run release debug

release: check-version $(EXE)
debug: check-version $(EXE)

clang: release
	@echo running with input: $(INPUTFILE)
	@./$(EXE) $(INPUTFILE)

%.o: %.cpp
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXE): $(OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $(EXE)

# Define a helper function to check dmesg for errors
define check_failure
    @echo "Make target '$@' failed! Checking dmesg..."; \
    dmesg | tail -20 | grep -i -E 'oom|killed|segfault|error' || echo "No relevant logs found."
endef

run: release
	@echo running with input: $(INPUTFILE)
	@./$(EXE) $(INPUTFILE); exit_code=$$?; \
	if [ $$exit_code -ne 0 ]; then $(call check_failure); exit $$exit_code; fi

run_debug: debug
	@gdb --ex "set print thread-events off" --ex run --ex backtrace --args ./$(EXE) $(INPUTFILE); exit_code=$$?; \
	if [ $$exit_code -ne 0 ]; then $(call check_failure); exit $$exit_code; fi

# in the future this could be generalized to go through every file in "Tests"
rand_test: quantas/Tests/randtest.cpp quantas/Common/Distribution.cpp
	$(CXX) -pthread -std=c++17 $^ -o $@.exe
	./$@.exe

TESTS = check-version rand_test test_Example test_Bitcoin test_Ethereum test_PBFT test_Raft test_LinearChord test_Kademlia test_AltBit test_StableDataLink

############################### Compile and run all tests - uses a wild card.
test: $(TESTS)
	@make --no-print-directory clean
	@echo all tests successful

test_%: ALGFILE = $*Peer
test_%: CXXFLAGS += -O0 -g  -D_GLIBCXX_DEBUG -std=c++17
test_%:
	@make --no-print-directory clean
	@echo Testing $(ALGFILE)
	@$(CXX) $(CXXFLAGS) -c -o quantas/main.o quantas/main.cpp
	@$(CXX) $(CXXFLAGS) -c -o quantas/$(ALGFILE)/$(ALGFILE).o quantas/$(ALGFILE)/$(ALGFILE).cpp
	@$(CXX) $(CXXFLAGS) -c -o quantas/Common/Distribution.o quantas/Common/Distribution.cpp
	@$(CXX) $(CXXFLAGS)  quantas/main.o quantas/$(ALGFILE)/$(ALGFILE).o quantas/Common/Distribution.o -o $(EXE)
	@./$(EXE) quantas/$(ALGFILE)/$*Input.json; exit_code=$$?; \
	if [ $$exit_code -ne 0 ]; then $(call check_failure); exit $$exit_code; fi
	@$(RM) quantas/$(ALGFILE)/*.o
	@echo $(ALGFILE) successful

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

-include $(OBJS:.o=.d)
