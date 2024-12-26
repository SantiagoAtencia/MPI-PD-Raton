# Compiler
CC = mpicc

# Executable name
EXEC = main

# Source files
SRC = $(EXEC).c

# Number of processes
NP = 4

# Maze dimensions:
ROWS = 22
COLS = 8

# Default target
.PHONY: all
all: $(EXEC)

# Build target
$(EXEC): $(SRC)
	$(CC) -o $(EXEC) $(SRC)

# Install target
.PHONY: install
install:
	@if ! command -v mpirun > /dev/null; then \
        echo "Open MPI not found. Installing..."; \
        sudo apt-get update && sudo apt-get install -y openmpi-bin openmpi-common libopenmpi-dev; \
    else \
        echo "Open MPI is already installed."; \
    fi

# Run target
.PHONY: run
run: $(EXEC)
	mpirun --oversubscribe -np $(NP) ./$(EXEC) $(ROWS) $(COLS)

# Clean target
.PHONY: clean
clean:
	rm -f $(EXEC)