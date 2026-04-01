CXX = g++
CXXFLAGS = -std=c++20 -Iinclude
TARGET = build/ndep

all: $(TARGET) build deploy

$(TARGET): src/main.cpp | build
	$(CXX) $(CXXFLAGS) src/main.cpp -o $(TARGET)

deploy:
	sudo cp build/ndep /usr/local/bin/
	sudo chown root:root /usr/local/bin/ndep
	sudo chmod u+s /usr/local/bin/ndep

build:
	mkdir -p build

clean:
	rm -rf build

setup:
	mkdir -p .ndeploy/backups
	touch .ndeploy/watched_files.nd

.PHONY: all deploy clean build setup