CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
all:
	$(CXX) $(CXXFLAGS) main.cpp -o mayday404
run: all
	./mayday404
clean:
	rm -f mayday404
