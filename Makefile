.PHONY: all clean opt 
  
TARGET := j80
TARGET_TEST := j80-test

CC  := clang
CXX := clang++
STRIP := strip

# using brew flex/bison which are up to date compared to OSX versions
BISON := /usr/local/opt/bison/bin/bison
FLEX := /usr/local/opt/flex/bin/flex

STD_LFAGS := -std=c++11 -stdlib=libc++ -Wno-deprecated-register
OPT_FLAGS := -O3 -ffast-math

# added to use newest flex/bison
INCLUDE = -I/usr/local/opt/flex/include

CXXFLAGS = $(INCLUDE) $(STD_LFAGS)
opt: CXXFLAGS +=  $(OPT_FLAGS)

# added to use newest flex/bison
LDFLAGS = -L/usr/local/opt/bison/lib -L/usr/local/opt/flex/lib -lncurses -lpanel

BUILD = ./build

BASESRC = ./src
ASSEMBLER := $(BASESRC)/assembler
COMPILER := $(BASESRC)/compiler



SOURCE := $(BASESRC) $(ASSEMBLER) $(COMPILER) $(BASESRC)/vm $(BASESRC)/support

INCLUDE += -I./src

# Find all source files
SRC_CPP := $(patsubst $(BASESRC)/%, %, $(foreach dir, $(SOURCE), $(wildcard $(dir)/*.cpp)))
SRC_CPP += assembler/j80parser.cpp assembler/j80lexer.cpp
SRC_CPP += compiler/nanocparser.cpp compiler/nanoclexer.cpp

ifeq ($(MAKECMDGOALS), test)
SRC_CPP := $(filter-out main.cpp, $(SRC_CPP))
SRC_CPP += support/tests.cpp
endif

#SRC_C   = $(foreach dir, $(SOURCE), $(wildcard $(dir)/*.c))  # lex.nanocyy.cpp
OBJ_CPP := $(patsubst %.cpp, %.o, $(SRC_CPP))
#OBJ_C   = $(patsubst $(BASESRC)%, $(BUILD)%, $(patsubst %.c, %.o, $(SRC_C)))
OBJS    :=  $(OBJ_CPP) #$(OBJ_C)



$(info $$SRC_CPP is [${SRC_CPP}])
	
all: $(TARGET)
opt: $(TARGET)
test: $(TARGET_TEST)

strip: opt
	$(STRIP) $(TARGET)

$(COMPILER)/nanoclexer.cpp: $(COMPILER)/nanoc.l
	$(FLEX) -c++ --outfile=$@ $<

$(ASSEMBLER)/j80lexer.cpp: $(ASSEMBLER)/j80.l
	$(FLEX) -c++ --outfile=$@ $<

$(COMPILER)/nanocparser.cpp: $(COMPILER)/nanoc.ypp $(COMPILER)/nanoclexer.cpp
	$(BISON) -v --output-file=$(COMPILER)/nanocparser.cpp $<

$(ASSEMBLER)/j80parser.cpp: $(ASSEMBLER)/j80.ypp $(ASSEMBLER)/j80lexer.cpp
	$(BISON) -v --output-file=$(ASSEMBLER)/j80parser.cpp $<

$(TARGET) : $(addprefix $(BUILD)/, $(OBJS))
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
		
$(BUILD)/%.o: $(BASESRC)/%.c | $(ASSEMBLER)/j80parser.cpp $(COMPILER)/nanocparser.cpp
	$(CC) -x c++ $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(BASESRC)/%.cpp | $(ASSEMBLER)/j80parser.cpp $(COMPILER)/nanocparser.cpp
	@test -d $(@D) || mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(TARGET_TEST) : $(addprefix $(BUILD)/, $(OBJS))
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	
.phony: all opt strip test

clean:
	rm -rf $(TARGET)
	rm -rf $(BUILD)
	rm -f $(ASSEMBLER)/j80parser.* $(ASSEMBLER)/j80lexer.cpp 
	rm -f $(COMPILER)/nanocparser.* $(COMPILER)/nanoclexer.cpp 