CXX = g++
CXXFLAGS = -std=c++20
TARGET = build/ndep

all: $(TARGET) install

$(TARGET): src/main.cpp
	$(CXX) $(CXXFLAGS) src/main.cpp -o $(TARGET)

install:
	sudo cp $(TARGET) /usr/local/bin/

clean:
	rm -f $(TARGET)

.PHONY: all install clean