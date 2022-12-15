# to build the assignment:
#   $ cd assignment_dir
#   $ make
# executable binaries should be put in assignment_dir/bin
# to run the game, you need to cd bin, and type ./sfmlgame
# but it's annoying to change directory each time
#
# to compile and run in one command type:
# make run

# define which compiler to use
CXX    := g++
OUTPUT := sfmlgame

# if you need to manually specify your SFML install dir, do so here
SFML_DIR  := .

# compiler and linker flags
CXX_FLAGS := -O3 -std=c++17
INCLUDES  := -I./src -I$(SFML_DIR)/include
LDFLAGS   := -O3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -L$(SFML_DIR)/lib

# the source files for the ecs game engine
SRC_FILES := $(wildcard src/*.cpp) 
OBJ_FILES := $(SRC_FILES:.cpp=.o)

# all of these targets will be made if you just type make
all:$(OUTPUT)

# define the main executable requirements / command
$(OUTPUT):$(OBJ_FILES) Makefile
	$(CXX) $(OBJ_FILES) $(LDFLAGS) -o ./bin/$@ 

# specifies how the object files are compiled from cpp files
.cpp.o:
	$(CXX) -c $(CXX_FLAGS) $(INCLUDES) $< -o $@

# typing 'make clean' will remove all intermediate build files
clean:
	rm -f $(OBJ_SFMLGAME) ./bin/sfmlgame
    
# typing 'make run' will compile and run the program
run: $(OUTPUT)
	cd bin && ./sfmlgame && cd ..