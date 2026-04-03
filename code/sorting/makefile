CXX := g++
CXXFLAGS := -O2 -std=gnu++17 -Wall -Wextra -Wshadow

SRC_DIR := .
ALG_DIR := algorithms
INCLUDES := -I$(SRC_DIR) -I$(ALG_DIR)

INPUT_DIR := data/array_input
MEASURE_DIR := data/measurements

ALGS := mergesort quicksort stdsort

MERGE_OBJ := $(ALG_DIR)/mergesort.o
QUICK_OBJ := $(ALG_DIR)/quicksort.o
STD_OBJ   := $(ALG_DIR)/sort.o

all: sorting

sorting: sorting.cpp $(MERGE_OBJ) $(QUICK_OBJ) $(STD_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@
	
$(ALG_DIR)/%.o: $(ALG_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: sorting
	@./sorting

run_cadauno: sorting
	@set -e; \
	if ls -1 $(INPUT_DIR)/*.txt >/dev/null 2>&1; then \
	  for ALG in $(ALGS); do \
	    echo "==> Ejecutando modo aislado para $$ALG"; \
	    for F in $(INPUT_DIR)/*.txt; do \
	      ./sorting --only $$ALG --file "$$F"; \
	    done; \
	  done; \
	  echo "Run aislado completado (memoria por archivo registrada en $(MEASURE_DIR))."; \
	else \
	  echo "No se encontraron .txt en $(INPUT_DIR)"; \
	fi

plots:
	@python3 scripts/plot_generator.py
	@echo "graficos hechos"

clean:
	rm -f sorting
	rm -f $(ALG_DIR)/*.o
	@echo "Limpieza completada."

.PHONY: all run run_cadauno clean plots