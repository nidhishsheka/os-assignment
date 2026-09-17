CXX = clang++
CXXFLAGS = -std=c++17 -O2 -Xpreprocessor -fopenmp
OMP_INC = -I/opt/homebrew/opt/libomp/include
OMP_LIB = -L/opt/homebrew/opt/libomp/lib -lomp

TARGET = matrix_multi
SOURCES = main.cpp src/matrix.cpp src/benchmark.cpp

all:
	$(CXX) $(CXXFLAGS) $(OMP_INC) $(SOURCES) $(OMP_LIB) -o $(TARGET)

clean:
	rm -f $(TARGET) matrix_data.json
