

EXE := test.exe

CXX := g++
CXXFLAGS := -pthread -std=c++17

CONFIGFILE := config.json

OBJS := main.o

$(EXE): $(OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $(EXE)

%.o: %.cpp
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

release: CXXFLAGS += -O3 -s
release: $(EXE)

run: release
	@echo running with input: $(CONFIGFILE)
	@./$(EXE) $(CONFIGFILE) 3 &
	@./$(EXE) $(CONFIGFILE) 4 &

kill:
	@USER=$$(whoami); \
	PIDS=$$(ps -eo user,pid,command | awk -v user="$$USER" '$$1 == user && $$3 ~ /^\.\/$(EXE)$$/ {print $$2}'); \
	COUNT=$$(echo "$$PIDS" | wc -w); \
	if [ -n "$$PIDS" ]; then \
		echo "Killing $$COUNT process(es) of $(EXE) started by $$USER..."; \
		kill $$PIDS; \
	else \
		echo "No running $(EXE) processes found for user $$USER."; \
	fi
