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


PROJECT_DIR := quantas

################
#
#  configure this for the specific algorithm and input file
#

INPUTFILE := ExampleInput.json

# ALGFILE := TrailPeer

ALGFILE := ExamplePeer

# ALGFILE := BitcoinPeer

# ALGFILE := EthereumPeer

# ALGFILE := PBFTPeer

# ALGFILE := RaftPeer

# ALGFILE := SmartShardsPeer

# ALGFILE := LinearChordPeer

# ALGFILE := KademliaPeer

# ALGFILE := AltBitPeer

# ALGFILE := StableDataLinkPeer

# ALGFILE := ChangRobertsPeer

# ALGFILE := DynamicPeer

# ALGFILE := KPTPeer

# ALGFILE := KSMPeer

# ALGFILE := CycleOfTreesPeer

####### end algorithm configuration

####### All algorithms


INPUTFILE := $(PROJECT_DIR)/$(ALGFILE)/$(INPUTFILE)


CPPFLAGS := -Iinclude -MMD -MP
CXXFLAGS = -pthread -include $(PROJECT_DIR)/$(ALGFILE)/$(ALGFILE).hpp
CXX := g++


GCC_VERSION := $(shell $(CXX) -dumpversion)
GCC_MIN_VERSION := 8

check-version:
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then echo "Default version of g++ must be higher than 8."; fi
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then echo "To change the default version visit: https://linuxconfig.org/how-to-switch-between-multiple-gcc-and-g-compiler-versions-on-ubuntu-20-04-lts-focal-fossa"; fi
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then exit 1; fi

EXE := quantas.exe
OBJS = $(PROJECT_DIR)/main.o $(PROJECT_DIR)/$(ALGFILE)/$(ALGFILE).o $(PROJECT_DIR)/Common/Distribution.o


# extra debug and release flags
release: CXXFLAGS += -O3 -s -std=c++17
debug: CXXFLAGS += -O0 -g  -D_GLIBCXX_DEBUG -std=c++17

clang: CXX := clang++
clang: CXXFLAGS += -std=c++17

.PHONY: all clean run release debug

all: release

release: check-version $(EXE)
debug: check-version $(EXE)

clang: all
	./$(EXE) $(INPUTFILE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(EXE)

$(PROJECT_DIR)/%.o: $(PROJECT_DIR)/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(EXE) $(INPUTFILE)

run_debug: debug
	./$(EXE) $(INPUTFILE)

# in the future this could be generalized to go through every file in "Tests"
rand_test: $(PROJECT_DIR)/Tests/randtest.cpp $(PROJECT_DIR)/Common/Distribution.cpp
	$(CXX) -pthread -std=c++17 $^ -o $@.exe
	./$@.exe

TESTS = check-version rand_test test_Example test_Bitcoin test_Ethereum test_PBFT test_Raft test_SmartShards test_LinearChord test_Kademlia test_AltBit test_StableDataLink test_ChangRoberts test_Dynamic test_KPT test_KSM

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
	@./$(EXE) quantas/$(ALGFILE)/$*Input.json
	@$(RM) quantas/$(ALGFILE)/*.o
	@echo $(ALGFILE) successful

clean:
	@$(RM) *.exe
	@$(RM) *.out
	@$(RM) *.o
	@$(RM) -r *.dSYM
	@$(RM) $(PROJECT_DIR)/*.gch
	@$(RM) $(PROJECT_DIR)/*.tmp
	@$(RM) $(PROJECT_DIR)/*.o
	@$(RM) $(PROJECT_DIR)/*.d
	@$(RM) $(PROJECT_DIR)/Common/*.gch
	@$(RM) $(PROJECT_DIR)/Common/*.tmp
	@$(RM) $(PROJECT_DIR)/Common/*.o
	@$(RM) $(PROJECT_DIR)/Common/*.d
	@$(RM) $(PROJECT_DIR)/$(ALGFILE)/*.gch
	@$(RM) $(PROJECT_DIR)/$(ALGFILE)/*.tmp
	@$(RM) $(PROJECT_DIR)/$(ALGFILE)/*.o
	@$(RM) $(PROJECT_DIR)/$(ALGFILE)/*.d
	@$(RM) quantas_test/*.gch
	@$(RM) quantas_test/*.tmp
	@$(RM) quantas_test/*.o
	@$(RM) quantas_test/*.d

# enables recursive glob patterns for bash
cleanRec: SHELL := /bin/bash -O globstar
cleanRec:
	@$(RM) **/*.out
	@$(RM) **/*.o
	@$(RM) **/*.d
	@$(RM) **/*.dSYM
	@$(RM) **/*.gch
	@$(RM) **/*.tmp
	@$(RM) **/*.exe

-include $(OBJS:.o=.d)
