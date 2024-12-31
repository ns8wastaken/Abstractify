CPP = g++ -std=c++20 -g -Wall -pedantic -Wextra src/main.cpp
DEPS_RAYLIB = -L C:/raylib/lib -lraylib -lgdi32 -lwinmm

all: compile finish open

compile:
	$(CPP) -I C:/raylib/include -o main.exe $(DEPS_RAYLIB)

finish:
	@echo -e "\033[0;32m== $(shell date) ==\e[0m"

open:
	@echo -e "\033[0;94mRunning exe...\e[0m"
	@main.exe
