
# hostname -I


EXE := test.exe

CXX := g++
CXXFLAGS := -pthread -std=c++17

CONFIGFILE := config.json

OBJS := socketExample.o ipUtil.o

$(EXE): $(OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $(EXE)

%.o: %.cpp
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

getIP:
	@hostname -I

release: CXXFLAGS += -O3 -s
release: $(EXE)

run: kill release
	# @echo running with input: $(CONFIGFILE)
	@./$(EXE) config0.json &
	@./$(EXE) config1.json 5001 &
	@./$(EXE) config1.json 5002 &
	@./$(EXE) config1.json 5003 &
	@./$(EXE) config1.json 5004 &

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

status:
	@USER=$$(whoami); \
	PIDS=$$(ps -eo user,pid,command | awk -v user="$$USER" '$$1 == user && $$3 ~ /^\.\/$(EXE)$$/ {print $$2}'); \
	COUNT=$$(echo "$$PIDS" | wc -w); \
	if [ -n "$$PIDS" ]; then \
		echo "$$COUNT process(es) of $(EXE) started by $$USER..."; \
	else \
		echo "No running $(EXE) processes found for user $$USER."; \
	fi
