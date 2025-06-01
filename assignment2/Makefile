CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
LDFLAGS = -pthread

# Targets
TARGETS = proc1 proc2 proc3

# Default target
all: $(TARGETS)

# Process 1: Reads data and writes to shared memory
proc1: proc1.cpp student.h
	$(CXX) $(CXXFLAGS) -o proc1 proc1.cpp $(LDFLAGS)

# Process 2: Calculates GPA and sorts students
proc2: proc2.cpp student.h
	$(CXX) $(CXXFLAGS) -o proc2 proc2.cpp $(LDFLAGS)

# Process 3: Finds students with most grades > 90
proc3: proc3.cpp student.h
	$(CXX) $(CXXFLAGS) -o proc3 proc3.cpp $(LDFLAGS)

# Run all processes
run: all
	@echo "Starting all processes..."
	@echo "\nProcess 1: Reading data and writing to shared memory..."
	./proc1 &
	@sleep 2
	@echo "\nProcess 2: Calculating GPA and sorting..."
	./proc2 &
	@sleep 2
	@echo "\nProcess 3: Finding best students..."
	./proc3
	@echo "\nAll processes completed!"

# Clean up
clean:
	rm -f $(TARGETS)
	@echo "Cleaned up executables and data files\n"

# Clean shared memory (in case of interrupted execution)
clean-shm:
	@echo "\nCleaning up shared memory objects..."
	@ipcs -m | grep $(USER) | awk '{print $$2}' | xargs -r ipcrm -m
	@echo "Shared memory cleaned"

.PHONY: all run clean clean-shm