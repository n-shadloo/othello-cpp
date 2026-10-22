CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

SRC := $(wildcard src/*.cpp)
GUI_SRC := $(wildcard src/gui/*.cpp) $(filter-out src/main.cpp, $(SRC))

# pkg-config on Linux, Homebrew's prefix on macOS
RAYLIB_FLAGS ?= $(shell pkg-config --cflags --libs raylib 2>/dev/null || \
	echo "-I$$(brew --prefix raylib)/include -L$$(brew --prefix raylib)/lib -lraylib")

othello: $(SRC) $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@

othello-gui: $(GUI_SRC) $(wildcard src/*.h src/gui/*.h)
	$(CXX) $(CXXFLAGS) $(GUI_SRC) $(RAYLIB_FLAGS) -o $@

run: othello
	./othello

gui: othello-gui
	./othello-gui

clean:
	rm -f othello othello-gui

.PHONY: run gui clean
