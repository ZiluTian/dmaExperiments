# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Target executable
TARGET = econSim

# Source files and object files
SRCS = src/main.cpp
OBJS = $(SRCS:.cpp=.o)

# Test files and object files
TEST_SRCS = $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# Header files directory
INCLUDES = -Iinclude

# Test-specific include directories
TEST_INCLUDES = -Itest

# Compile production code
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Compile each source file into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile test files
test: $(TARGET) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_INCLUDES) -o test_runner $(TEST_OBJS)
	./test_runner

# Clean compiled files
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) test_runner
