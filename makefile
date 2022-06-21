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
INPUTFILE := $(PROJECT_DIR)/ChangRobertsInput.json

#ALG := EXAMPLE_PEER
#ALGFILE := ExamplePeer

# ALG := BITCOIN_PEER
# ALGFILE := BitcoinPeer

# ALG := ETHEREUM_PEER
# ALGFILE := EthereumPeer

# ALG := PBFT_PEER
# ALGFILE := PBFTPeer

# ALG := RAFT_PEER
# ALGFILE := RaftPeer

# ALG := SMARTSHARDS_PEER
# ALGFILE := SmartShardsPeer

# ALG := LINEARCHORD_PEER
# ALGFILE := LinearChordPeer

# ALG := KADEMLIA_PEER
# ALGFILE := KademliaPeer

# ALG := ALTBIT_PEER
# ALGFILE := AltBitPeer

# ALG := STABLEDATALINK_PEER
# ALGFILE := StableDataLinkPeer

ALG := CHANGROBERTS_PEER
ALGFILE := ChangRobertsPeer
####### end algorithm configuration


CPPFLAGS := -Iinclude -MMD -MP
CXXFLAGS = -pthread  -D$(ALG)
CXX := g++

EXE := quantas.exe
OBJS := $(PROJECT_DIR)/main.o $(PROJECT_DIR)/$(ALGFILE).o


# extra debug and release flags
release:  CXXFLAGS += -O2 -s 
debug: CXXFLAGS += -O0 -g  -D_GLIBCXX_DEBUG

clang: CXX := clang++
clang: CXXFLAGS += -std=c++14

.PHONY: all clean run release debug

all: debug

release: $(EXE)
debug: $(EXE)
clang: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(EXE)

$(PROJECT_DIR)/%.o: $(PROJECT_DIR)/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(EXE) $(INPUTFILE)

clean:
	$(RM) $(EXE)
	$(RM) *.out
	$(RM) *.o
	$(RM) -r *.dSYM
	$(RM) $(PROJECT_DIR)/*.gch
	$(RM) $(PROJECT_DIR)/*.tmp
	$(RM) $(PROJECT_DIR)/*.o
	$(RM) $(PROJECT_DIR)/*.d
	$(RM) quantas_test/*.gch
	$(RM) quantas_test/*.tmp
	$(RM) quantas_test/*.o
	$(RM) quantas_test/*.d

-include $(OBJS:.o=.d)
