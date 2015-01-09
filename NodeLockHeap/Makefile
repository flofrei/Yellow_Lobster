#	CPQ, version 1.0  
#	(c) 2014 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich

# === Compiler ===

CXX = g++-4.9
CFLAGS += -fdiagnostics-color=always

ifeq ($(shell uname -s),Linux)
	CFLAGS += -march=native -g 
endif
 
# === Sources ===
EXE = 	testsuite_concurrent 	\
		testsuite_serial 		\
		benchmark_CPQ			\
		benchmark_Intel			\
		benchmark_STL

# === Compiler Flags ===
WARNINGS = -Wall -DNDEBUG
CFLAGS 	+= -O2 -std=c++11 $(WARNINGS) -fopenmp
LDFLAGS	 = -ltbb

# === Compilation ===
.PHONY: all
all: $(EXE)

benchmark_CPQ$(EXTENSION) : benchmark.cpp
	$(CXX) -o $@ -D_CPQ $(CFLAGS) $^ $(LDFLAGS)

benchmark_Intel$(EXTENSION) : benchmark.cpp
	$(CXX) -o $@ -D_Intel $(CFLAGS) $^ $(LDFLAGS)

benchmark_STL$(EXTENSION) : benchmark.cpp
	$(CXX) -o $@ -D_STL $(CFLAGS) $^ $(LDFLAGS)

% :: %.cpp
	$(CXX) -o $@ $(CFLAGS) $^ $(LDFLAGS)

# === Cleaning ===
.PHONY: clean
clean:
	 rm -f $(EXE)
