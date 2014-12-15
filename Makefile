.PHONY: all clean opt 
  
TARGET = j80

CC  := clang
CXX := clang++

INCLUDE = 

CFLAGS = $(INCLUDE) -std=c++11 -stdlib=libc++ -Wno-deprecated-register
#opt: CFLAGS = $(INCLUDE) -O3 -ffast-math -pipe -std=c++11 -stdlib=libc++

CXXFLAGS = $(CFLAGS)

LDFLAGS = 

# Find all source files
SOURCE = .
SRC_CPP = $(foreach dir, $(SOURCE), $(wildcard $(dir)/*.cpp)) j80.tab.cpp
SRC_C   = $(foreach dir, $(SOURCE), $(wildcard $(dir)/*.c)) lex.j80yy.c
OBJ_CPP = $(patsubst %.cpp, %.o, $(SRC_CPP))
OBJ_C   = $(patsubst %.c, %.o, $(SRC_C))
OBJS    = $(OBJ_CPP) $(OBJ_C)

all: $(TARGET)
#flex --prefix j80yy --header-file=lex.yy.c j80.l
#bison -pj80yy -d -v j80.ypp


lex.j80yy.c: j80.l
	flex --header-file=lex.j80yy.h --outfile=lex.j80yy.c -P j80 $<

j80.tab.cpp: j80.ypp lex.j80yy.c
	bison -d -v -p j80 $<

$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -x c++ $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) lex.yy.* j80.tab.* lex.j80yy.*