CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic
TARGET := wordle_plus
SOURCE := Wordle\ Plus.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE) ALL.TXT SOLUTION.TXT
	$(CXX) $(CXXFLAGS) "Wordle Plus.cpp" -o $(TARGET)

clean:
	rm -f $(TARGET)
