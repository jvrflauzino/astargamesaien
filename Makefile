CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -Isrc

all: fruitbox_solver

fruitbox_solver: src/main.cpp src/board.cpp src/search.cpp src/board.hpp src/search.hpp
	$(CXX) $(CXXFLAGS) src/main.cpp src/board.cpp src/search.cpp -o fruitbox_solver.exe


clean:
	del /f /q fruitbox_solver.exe *.o 2>nul || rm -f fruitbox_solver.exe  *.o
