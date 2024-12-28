# Compiler
CC = mpicc

# Executable name
EXEC = maze_race

# Source and include directories
SRC_DIR = src
INC_DIR = include

# Source files
SRC = $(SRC_DIR)/main.c $(SRC_DIR)/maze.c $(SRC_DIR)/maze_corrector.c

# Include directories
INCLUDES = -I$(INC_DIR)

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
	$(CC) $(INCLUDES) -o $(EXEC) $(SRC)

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
