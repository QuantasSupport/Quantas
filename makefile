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

# Configurable usage to override the hardcoded input:
#   make run INPUTFILE=quantas/ExamplePeer/ExampleInput.json
#   make run_distributed_concrete INPUTFILE=quantas/KademliaPeer/KademliaConcreteInput.json LEADER=localhost FOLLOWERS=localhost,localhost
#
# Keep INPUTFILE pointed at a convenient default for abstract runs.
# Concrete runs should normally be launched through make run_distributed_concrete.
# The launcher supplies QUANTAS_IS_LEADER and QUANTAS_LEADER_IP/PORT, so
# concrete JSON files do not need machine-specific leader addresses.

INPUTFILE := quantas/ExamplePeer/ExampleInput.json
# INPUTFILE := quantas/ExamplePeer/ExampleConcreteInput1.json
# INPUTFILE := quantas/KademliaPeer/KademliaConcreteInput.json
# INPUTFILE := quantas/ChordPeer/ChordConcreteInput.json

# INPUTFILE := quantas/AltBitPeer/AltBitInput.json
# INPUTFILE := quantas/AltBitPeer/AltBitFaultToleranceInput.json

# INPUTFILE := quantas/PBFTPeer/PBFTInput.json

# INPUTFILE := quantas/BitcoinPeer/BitcoinPeerInput.json
# INPUTFILE := quantas/BitcoinPeer/BitcoinConcreteInput.json
# INPUTFILE := quantas/BitcoinPeer/BitcoinParasiteSweep.json
# INPUTFILE := quantas/EthereumPeer/EthereumParasiteSweep.json
# INPUTFILE := quantas/EthereumPeer/EthereumPeerInput2.json

# INPUTFILE := quantas/EthereumPeer/EthereumPeerInput.json


# INPUTFILE := quantas/KademliaPeer/KademliaPeerInput.json
# INPUTFILE := quantas/KademliaPeer/KademliaScaleAbstractPaper.json

# INPUTFILE := quantas/ChordPeer/ChordPeerInput.json
# INPUTFILE := quantas/ChordPeer/ChordScaleConcretePaper.json
# INPUTFILE := quantas/ChordPeer/ChordScaleAbstractPaper.json

# INPUTFILE := quantas/RaftPeer/RaftInput.json

# INPUTFILE := quantas/StableDataLinkPeer/StableDataLinkInput.json
# INPUTFILE := quantas/StableDataLinkPeer/StableDataLinkFaultToleranceInput.json

# INPUTFILE := quantas/LightningPeer/LightningPeerSweep.json

# INPUTFILE := quantas/BrachaPeer/BrachaPeerInput.json

############################### Variables and Flags ###############################

EXE := quantas.exe

ifeq ($(MAKELEVEL),0)
MAKEFLAGS += -j16
endif
.DEFAULT_GOAL := help

# compiles all the cpps in Common and main.cpp
COMMON_SRCS := $(wildcard quantas/Common/*.cpp)
COMMON_OBJS := $(COMMON_SRCS:.cpp=.o)

MODE ?= abstract

ABSTRACT_SIM_OBJS := quantas/Common/Abstract/abstractSimulation.o quantas/Common/Abstract/Channel.o quantas/Common/Abstract/Network.o quantas/Common/Concrete/NetworkInterfaceConcrete.o quantas/Common/Concrete/ProcessCoordinator.o quantas/Common/Concrete/ipUtil.o
CONCRETE_SIM_OBJS := quantas/Common/Concrete/NetworkInterfaceConcrete.o quantas/Common/Concrete/ProcessCoordinator.o quantas/Common/Concrete/concreteSimulation.o quantas/Common/Concrete/ipUtil.o quantas/Common/Abstract/Channel.o

ifeq ($(MODE),concrete)
SIM_OBJS := $(COMMON_OBJS) $(CONCRETE_SIM_OBJS)
else
SIM_OBJS := $(COMMON_OBJS) $(ABSTRACT_SIM_OBJS)
endif

# compiles all cpps specified as necessary in the INPUTFILE
# Guard this so make does not block on stdin when INPUTFILE is unset.
ifneq ($(strip $(INPUTFILE)),)
ALGS := $(shell sed -n '/"algorithms"/,/]/p' "$(INPUTFILE)" \
         | sed -n 's/.*"\([^"]*\.cpp\)".*/quantas\/\1/p')
else
ALGS :=
endif
ALG_OBJS += $(ALGS:.cpp=.o)

# necessary flags
CXX := g++
CXXFLAGS := -pthread -std=c++17
GCC_VERSION := $(shell $(CXX) $(CXXFLAGS) -dumpversion)
GCC_MIN_VERSION := 8
BUILD_SIGNATURE_FILE := .build/.last_build_signature
BUILD_SIGNATURE := MODE=$(MODE);CXX=$(CXX);CXXFLAGS=$(CXXFLAGS);ALGS=$(ALGS);SIM_OBJS=$(SIM_OBJS)

############################### Build Types ###############################

# release for faster runtime, debug for debugging
release: CXXFLAGS += -O3 -s
release: check-version $(EXE)
debug: CXXFLAGS += -O0 -g -D_GLIBCXX_DEBUG 
# -fsanitize=address,undefined -fno-omit-frame-pointer # flag helps with double delete errors
debug: check-version $(EXE)

############################### Running Commands ###############################

# When running on windows use make clang
clang: CXX := clang++
clang: release
	@echo running with input: $(INPUTFILE)
	@./$(EXE) $(INPUTFILE)

# When running on Linux use make run
run: release
	
	@if [ -n "$(RUN_DIR)" ]; then \
		export QUANTAS_RUN_DIR="$(RUN_DIR)"; \
	elif [ -z "$$QUANTAS_RUN_DIR" ]; then \
		export QUANTAS_RUN_DIR="$(CURDIR)/experiments/$$(date +%Y%m%d_%H%M%S)"; \
	fi; \
	mkdir -p "$$QUANTAS_RUN_DIR"; \
	if [ -z "$$QUANTAS_HOSTNAME" ]; then \
		export QUANTAS_HOSTNAME="$$(hostname -f 2>/dev/null || hostname)"; \
	fi; \
	if [ -z "$$QUANTAS_MACHINE_IP" ]; then \
		export QUANTAS_MACHINE_IP="$$(hostname -I 2>/dev/null | awk '{print $$1}')"; \
	fi; \
	if [ -z "$$QUANTAS_PROCESS_ROLE" ]; then \
		if [ "$(MODE)" = "concrete" ]; then \
			if [ -n "$(PORT)" ]; then \
				export QUANTAS_PROCESS_ROLE="leader"; \
				export QUANTAS_IS_LEADER="1"; \
			else \
				export QUANTAS_PROCESS_ROLE="follower"; \
				export QUANTAS_IS_LEADER="0"; \
			fi; \
		else \
			export QUANTAS_PROCESS_ROLE="abstract"; \
		fi; \
	fi; \
	if [ -z "$$QUANTAS_MACHINE_IP" ]; then export QUANTAS_MACHINE_IP="unknown"; fi; \
	if [ -n "$(PORT)" ]; then \
		echo running with input: $(INPUTFILE) on port $(PORT); \
		./$(EXE) $(INPUTFILE) $(PORT); \
	else \
		echo running with input: $(INPUTFILE); \
		./$(EXE) $(INPUTFILE); \
	fi; \
	exit_code=$$?; \
	if [ $$exit_code -ne 0 ]; then $(call check_failure); exit $$exit_code; fi

# Show commonly used routes when running `make`
help:
	@echo "QUANTAS make routes:"
	@echo "  make run INPUTFILE=<file>                  # Abstract mode"
	@echo ""
	@echo "Concrete mode:"
	@echo "  make run_distributed_concrete INPUTFILE=<file> LEADER=localhost FOLLOWERS=localhost"
	@echo "  make run_distributed_concrete INPUTFILE=<file> HOSTS_FILE=available_hosts.txt HOST_COUNT=5"
	@echo "  make stop_distributed_concrete HOSTS_FILE=available_hosts.txt HOST_COUNT=5"
	@echo "  make run_batch                             # Batch wrapper script"
	@echo ""
	@echo "Tests:"
	@echo "  make test                                  # Run test inputs without valgrind"
	@echo "  make test_with_memory                      # Run memory tests with valgrind"
	@echo ""
	@echo "Optional concrete vars:"
	@echo "  LEADER=<host> FOLLOWERS=<h1,h2,...> LEADER_INDEX=<n> PORT=<p> WORKDIR=<dir> ROOT_DIR=<dir>"

# Wrapper route for scripts/run_distributed_concrete.sh
# Examples:
# make run_distributed_concrete INPUTFILE=quantas/KademliaPeer/KademliaConcreteInput.json LEADER=localhost FOLLOWERS=localhost,localhost
# make run_distributed_concrete INPUTFILE=quantas/KademliaPeer/KademliaConcreteInput.json HOSTS_FILE=available_hosts.txt HOST_COUNT=5
# make run_distributed_concrete INPUTFILE=quantas/BitcoinPeer/BitcoinConcreteInput.json LEADER=eon1 FOLLOWERS=eon1,eon2,eon3
run_distributed_concrete:
	@set -e; \
	args="-i \"$(INPUTFILE)\""; \
	if [ -n "$(LEADER)" ]; then args="$$args -l \"$(LEADER)\""; fi; \
	if [ -n "$(FOLLOWERS)" ]; then args="$$args -f \"$(FOLLOWERS)\""; fi; \
	if [ -n "$(HOSTS_FILE)" ]; then args="$$args --hosts-file \"$(HOSTS_FILE)\""; fi; \
	if [ -n "$(HOST_COUNT)" ]; then args="$$args --count \"$(HOST_COUNT)\""; fi; \
	if [ -n "$(LEADER_INDEX)" ]; then args="$$args --leader-index \"$(LEADER_INDEX)\""; fi; \
	if [ -n "$(PORT)" ]; then args="$$args -p \"$(PORT)\""; fi; \
	if [ -n "$(WORKDIR)" ]; then args="$$args -w \"$(WORKDIR)\""; fi; \
	if [ -n "$(ROOT_DIR)" ]; then args="$$args --root-dir \"$(ROOT_DIR)\""; fi; \
	echo "launching distributed concrete run with input: $(INPUTFILE)"; \
	eval "env MAKEFLAGS= QUANTAS_RUN_VIA_MAKE=1 bash ./scripts/run_distributed_concrete.sh $$args"

# Wrapper route for scripts/stop_distributed_concrete.sh
# Examples:
# make stop_distributed_concrete HOSTS_FILE=available_hosts.txt HOST_COUNT=5
# make stop_distributed_concrete HOSTS=eon1,eon2,eon3
stop_distributed_concrete:
	@set -e; \
	args=""; \
	if [ -n "$(HOSTS)" ]; then args="$$args -H \"$(HOSTS)\""; fi; \
	if [ -n "$(HOSTS_FILE)" ]; then args="$$args --hosts-file \"$(HOSTS_FILE)\""; fi; \
	if [ -n "$(HOST_COUNT)" ]; then args="$$args --count \"$(HOST_COUNT)\""; fi; \
	if [ -n "$(WORKDIR)" ]; then args="$$args -w \"$(WORKDIR)\""; fi; \
	eval "env MAKEFLAGS= QUANTAS_RUN_VIA_MAKE=1 bash ./scripts/stop_distributed_concrete.sh $$args"

run_batch:
	echo "running batch"; \
	eval "env MAKEFLAGS= QUANTAS_RUN_VIA_MAKE=1 bash ./scripts/run_batch.sh"

############################### Debugging ###############################

# runs the program with full Valgrind to trace memory leaks
run_memory:
	@command -v valgrind >/dev/null 2>&1 || { echo "valgrind is required for run_memory. Install it with: sudo apt-get install -y valgrind"; exit 1; }
	+@$(MAKE) --no-print-directory clean
	+@$(MAKE) --no-print-directory debug MODE="$(MODE)" INPUTFILE="$(INPUTFILE)"
	@if [ -n "$(RUN_DIR)" ]; then \
		export QUANTAS_RUN_DIR="$(RUN_DIR)"; \
	elif [ -z "$$QUANTAS_RUN_DIR" ]; then \
		export QUANTAS_RUN_DIR="$(CURDIR)/experiments/$$(date +%Y%m%d_%H%M%S)"; \
	fi; \
	mkdir -p "$$QUANTAS_RUN_DIR"; \
	if [ -z "$$QUANTAS_HOSTNAME" ]; then \
		export QUANTAS_HOSTNAME="$$(hostname -f 2>/dev/null || hostname)"; \
	fi; \
	if [ -z "$$QUANTAS_MACHINE_IP" ]; then \
		export QUANTAS_MACHINE_IP="$$(hostname -I 2>/dev/null | awk '{print $$1}')"; \
	fi; \
	if [ -z "$$QUANTAS_PROCESS_ROLE" ]; then \
		if [ "$(MODE)" = "concrete" ]; then \
			if [ -n "$(PORT)" ]; then \
				export QUANTAS_PROCESS_ROLE="leader"; \
				export QUANTAS_IS_LEADER="1"; \
			else \
				export QUANTAS_PROCESS_ROLE="follower"; \
				export QUANTAS_IS_LEADER="0"; \
			fi; \
		else \
			export QUANTAS_PROCESS_ROLE="abstract"; \
		fi; \
	fi; \
	if [ -z "$$QUANTAS_MACHINE_IP" ]; then export QUANTAS_MACHINE_IP="unknown"; fi; \
	echo running: $(INPUTFILE) with valgrind; \
	valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
		 ./$(EXE) $(INPUTFILE)

# runs the program with Valgrind to see if there are any memory leaks
run_simple_memory:
	@command -v valgrind >/dev/null 2>&1 || { echo "valgrind is required for run_simple_memory/make test. Install it with: sudo apt-get install -y valgrind"; exit 1; }
	+@$(MAKE) --no-print-directory clean
	+@$(MAKE) --no-print-directory debug MODE="$(MODE)" INPUTFILE="$(INPUTFILE)"
	@if [ -n "$(RUN_DIR)" ]; then \
		export QUANTAS_RUN_DIR="$(RUN_DIR)"; \
	elif [ -z "$$QUANTAS_RUN_DIR" ]; then \
		export QUANTAS_RUN_DIR="$(CURDIR)/experiments/$$(date +%Y%m%d_%H%M%S)"; \
	fi; \
	mkdir -p "$$QUANTAS_RUN_DIR"; \
	if [ -z "$$QUANTAS_HOSTNAME" ]; then \
		export QUANTAS_HOSTNAME="$$(hostname -f 2>/dev/null || hostname)"; \
	fi; \
	if [ -z "$$QUANTAS_MACHINE_IP" ]; then \
		export QUANTAS_MACHINE_IP="$$(hostname -I 2>/dev/null | awk '{print $$1}')"; \
	fi; \
	if [ -z "$$QUANTAS_PROCESS_ROLE" ]; then \
		if [ "$(MODE)" = "concrete" ]; then \
			if [ -n "$(PORT)" ]; then \
				export QUANTAS_PROCESS_ROLE="leader"; \
				export QUANTAS_IS_LEADER="1"; \
			else \
				export QUANTAS_PROCESS_ROLE="follower"; \
				export QUANTAS_IS_LEADER="0"; \
			fi; \
		else \
			export QUANTAS_PROCESS_ROLE="abstract"; \
		fi; \
	fi; \
	if [ -z "$$QUANTAS_MACHINE_IP" ]; then export QUANTAS_MACHINE_IP="unknown"; fi; \
	echo ""; \
	echo running: $(INPUTFILE) with valgrind; \
	valgrind --leak-check=full ./$(EXE) $(INPUTFILE) 2>&1 \
		| grep -E "HEAP SUMMARY|in use|LEAK SUMMARY|definitely lost: |indirectly lost: |possibly lost: |still reachable: |ERROR SUMMARY"; \
	echo ""

# runs the program with GDB for more advanced error viewing
run_debug:
	@command -v gdb >/dev/null 2>&1 || { echo "gdb is required for run_debug. Install it with: sudo apt-get install -y gdb"; exit 1; }
	+@$(MAKE) --no-print-directory clean
	+@$(MAKE) --no-print-directory debug MODE="$(MODE)" INPUTFILE="$(INPUTFILE)"
	@if [ -n "$(PORT)" ]; then \
		echo debugging with input: $(INPUTFILE) on port $(PORT); \
		gdb -q -nx \
			-iex "set pagination off" \
			--ex "set pagination off" \
			--ex "set height 0" \
			--ex "set debuginfod enabled off" \
			--ex "set print thread-events off" \
			--ex run \
			--ex backtrace \
			--args ./$(EXE) $(INPUTFILE) $(PORT); \
	else \
		echo debugging with input: $(INPUTFILE); \
		gdb -q -nx \
			-iex "set pagination off" \
			--ex "set pagination off" \
			--ex "set height 0" \
			--ex "set debuginfod enabled off" \
			--ex "set print thread-events off" \
			--ex run \
			--ex backtrace \
			--args ./$(EXE) $(INPUTFILE); \
	fi; \
	exit_code=$$?; \
	if [ $$exit_code -ne 0 ]; then $(call check_failure); exit $$exit_code; fi

getIP:
	@hostname -I


kill:
	@USER=$$(whoami); \
	PIDS=$$(pgrep -u $$USER -x $(EXE) 2>/dev/null || true); \
	if [ -n "$$PIDS" ]; then \
		echo "Killing $$(echo "$$PIDS" | wc -w) process(es) of $(EXE) started by $$USER..."; \
		kill $$PIDS 2>/dev/null || true; \
		sleep 0.2; \
		kill -9 $$PIDS 2>/dev/null || true; \
	else \
		echo "No running $(EXE) processes found for user $$USER."; \
	fi

status:
	@USER=$$(whoami); \
	PIDS=$$(ps -eo user=,pid=,comm= | awk -v user="$$USER" '$$1 == user && $$3 == "$(EXE)" {print $$3}'); \
	COUNT=$$(echo "$$PIDS" | wc -w); \
	if [ -n "$$PIDS" ]; then \
		echo "$$COUNT process(es) of $(EXE) started by $$USER:"; \
		echo "$$PIDS"; \
	else \
		echo "No running $(EXE) processes found for user $$USER."; \
	fi

statusAll:
	@USER=$$(whoami); \
	PIDS=$$(ps -eo user=,pid=,comm= | awk -v user="$$USER" '$$1 == user'); \
	COUNT=$$(echo "$$PIDS" | wc -l); \
	if [ -n "$$PIDS" ]; then \
		echo "$$COUNT process(es) of started by $$USER:"; \
		echo "$$PIDS"; \
	else \
		echo "No running processes found for user $$USER."; \
	fi

############################### Tests ###############################

# Test thread based random number generation
rand_test: quantas/Tests/randtest.cpp
	@echo "Testing thread based random number generation..."
	+@$(MAKE) --no-print-directory clean
	@$(CXX) $(CXXFLAGS) $^ -o $@.exe
	@./$@.exe
	@echo ""
	
# in the future this could be generalized to go through every file in a Tests
# folder such that the input files need not be listed here
TEST_INPUTS := quantas/ExamplePeer/ExampleInput.json quantas/AltBitPeer/AltBitInput.json quantas/AltBitPeer/AltBitFaultToleranceInput.json quantas/PBFTPeer/PBFTInput.json quantas/BitcoinPeer/BitcoinPeerInput.json quantas/EthereumPeer/EthereumPeerInput.json quantas/ChordPeer/ChordPeerInput.json quantas/KademliaPeer/KademliaPeerInput.json quantas/RaftPeer/RaftInput.json quantas/StableDataLinkPeer/StableDataLinkInput.json quantas/StableDataLinkPeer/StableDataLinkFaultToleranceInput.json

test: check-version rand_test
	+@$(MAKE) --no-print-directory clean
	@echo "Running tests on all test inputs (no valgrind)..."
	@echo ""
	@set -e; \
	for file in $(TEST_INPUTS); do \
		$(MAKE) --no-print-directory run INPUTFILE="$$file"; \
	done

test_with_memory: check-version rand_test
	+@$(MAKE) --no-print-directory clean
	@echo "Running memory tests on all test inputs..."
	@echo ""
	@set -e; \
	for file in $(TEST_INPUTS); do \
		$(MAKE) --no-print-directory run_simple_memory INPUTFILE="$$file"; \
	done

############################### Helpers ###############################

# Define a helper function to check dmesg for errors
define check_failure
    echo "Make target '$@' failed! Checking kernel logs..."; \
    { \
      if command -v journalctl >/dev/null 2>&1; then \
        journalctl -k -n 200 --no-pager 2>/dev/null; \
      else \
        dmesg 2>/dev/null | tail -200; \
      fi; \
    } | grep -iE 'oom|killed|segfault|error' || echo "No relevant logs found."
endef

%.o: %.cpp $(BUILD_SIGNATURE_FILE)
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# check the version of the GCC compiler being used is above a threshold
check-version:
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then echo "Default version of g++ must be higher than 8."; fi
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then echo "To change the default version visit: https://linuxconfig.org/how-to-switch-between-multiple-gcc-and-g-compiler-versions-on-ubuntu-20-04-lts-focal-fossa"; fi
	@if [ "$(GCC_VERSION)" -lt "$(GCC_MIN_VERSION)" ]; then exit 1; fi

$(BUILD_SIGNATURE_FILE): FORCE
	@mkdir -p .build
	@new_sig='$(BUILD_SIGNATURE)'; \
	old_sig=""; \
	if [ -f "$(BUILD_SIGNATURE_FILE)" ]; then \
		old_sig="$$(cat "$(BUILD_SIGNATURE_FILE)")"; \
	fi; \
	if [ "$$old_sig" != "$$new_sig" ]; then \
		if [ -n "$$old_sig" ]; then echo "Build configuration changed; rebuilding objects..."; fi; \
		printf '%s\n' "$$new_sig" > "$(BUILD_SIGNATURE_FILE)"; \
	fi

FORCE:

$(EXE): $(ALG_OBJS) $(SIM_OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $(EXE)

############################### Cleanup ###############################

# clean generated build artifacts
clean:
	@find . -type f \( -name '*.out' -o -name '*.o' -o -name '*.d' -o -name '*.dSYM' -o -name '*.gch' -o -name '*.tmp' -o -name '*.exe' \) -delete

# clean run output folders
clean_logs:
	@find . -type d -name experiments -exec sh -c 'rm -rf "$$1"/*' _ {} \;

# -include $(OBJS:.o=.d)

############################### PHONY ###############################

# All make commands found in this file
.PHONY: help clean run release debug $(EXE) %.o clang run_memory run_simple_memory run_debug check-version rand_test test test_with_memory clean_txt run_distributed_concrete stop_distributed_concrete run_batch FORCE
