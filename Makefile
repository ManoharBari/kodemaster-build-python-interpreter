CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g

# Find all source files
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

TARGET = your_program

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
