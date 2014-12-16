.PHONY: all clean opt 
  
TARGET := j80

CC  := clang
CXX := clang++

# using brew flex/bison which are up to date compared to OSX versions
BISON := /usr/local/opt/bison/bin/bison
FLEX := /usr/local/opt/flex/bin/flex

# added to use newest flex/bison
INCLUDE = -I/usr/local/opt/flex/include

CFLAGS = $(INCLUDE) -std=c++11 -stdlib=libc++ -Wno-deprecated-register
#opt: CFLAGS = $(INCLUDE) -O3 -ffast-math -pipe -std=c++11 -stdlib=libc++

CXXFLAGS = $(CFLAGS)

# added to use newest flex/bison
LDFLAGS = -L/usr/local/opt/bison/lib -L/usr/local/opt/flex/lib

# Find all source files
BUILD = ./build

BASESRC = ./src
ASSEMBLER = $(BASESRC)/assembler



SOURCE = $(BASESRC) $(ASSEMBLER)
VPATH = ./src

INCLUDE += -I./src

SRC_CPP = $(patsubst $(BASESRC)/%, %, $(foreach dir, $(SOURCE), $(wildcard $(dir)/*.cpp))) assembler/j80parser.cpp assembler/j80lexer.cpp # nanoc.tab.cpp
#SRC_C   = $(foreach dir, $(SOURCE), $(wildcard $(dir)/*.c))  # lex.nanocyy.cpp
OBJ_CPP = $(patsubst %.cpp, %.o, $(SRC_CPP))
#OBJ_C   = $(patsubst $(BASESRC)%, $(BUILD)%, $(patsubst %.c, %.o, $(SRC_C)))
OBJS    =  $(OBJ_CPP) #$(OBJ_C)


all: $(TARGET)
#flex --prefix j80yy --header-file=lex.yy.c j80.l
#bison -pj80yy -d -v j80.ypp

$(ASSEMBLER)/lex.nanocyy.cpp: $(ASSEMBLER)/nanoc.l
	$(FLEX) -c++ --outfile=$@ $<

$(ASSEMBLER)/j80lexer.cpp: $(ASSEMBLER)/j80.l
	$(FLEX) -c++ --outfile=$@ $<

nanoc.tab.cpp: nanoc.ypp lex.nanocyy.cpp
	$(BISON) -d -v -p nc $<

$(ASSEMBLER)/j80parser.cpp: $(ASSEMBLER)/j80.ypp $(ASSEMBLER)/j80lexer.cpp
	$(BISON) --output-file=$(ASSEMBLER)/j80parser.cpp -v $<

$(TARGET) : $(addprefix $(BUILD)/, $(OBJS))
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD)/%.o: $(BASESRC)/%.c | $(ASSEMBLER)/j80parser.cpp
	$(CC) -x c++ $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(BASESRC)/%.cpp | $(ASSEMBLER)/j80parser.cpp
	@test -d $(@D) || mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET)
	rm -rf $(BUILD)
	rm -f $(ASSEMBLER)/j80parser.* $(ASSEMBLER)/j80lexer.cpp 