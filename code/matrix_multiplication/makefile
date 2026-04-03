CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall

# Ejecutable
BIN = matrix_multiplication

# Fuentes
SRCS = matrix_multiplication.cpp \
       algorithms/naive.cpp \
       algorithms/strassen.cpp
OBJS = $(SRCS:.cpp=.o)

# Script de Python para plots
PLOT_SCRIPT = scripts/plot_generator.py

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(BIN)
	@./$(BIN)

# plots solo ejecuta el script, sin correr nada más
plots:
	@echo "=== Generando gráficos ==="
	@python3 $(PLOT_SCRIPT)
	@echo "Gráficos listos"

clean:
	rm -f $(OBJS) $(BIN)