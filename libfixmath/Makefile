#Project settings
PROJECT = libfixmath
LIB =
SRC = .
INC =

#Compiler settings
CPP = masp430-elf-gcc
CC = msp430-elf-gcc
AS = msp430-elf-gcc
LD = msp430-elf-gcc
AR = msp430-elf-ar
CPP_FLAGS = -O2 $(INC) -Wall -Wextra
CC_FLAGS += -O2 $(INC) -Wall -Wextra
AS_FLAGS = $(CC_FLAGS) -D_ASSEMBLER_
LD_FLAGS = -Wall

# Find all source files
SRC_CPP = $(foreach dir, $(SRC), $(wildcard $(dir)/*.cpp))
SRC_C   = $(foreach dir, $(SRC), $(wildcard $(dir)/*.c))
SRC_S   = $(foreach dir, $(SRC), $(wildcard $(dir)/*.S))
OBJ_CPP = $(patsubst %.cpp, %.o, $(SRC_CPP))
OBJ_C   = $(patsubst %.c, %.o, $(SRC_C))
OBJ_S   = $(patsubst %.S, %.o, $(SRC_S))
OBJ     = $(OBJ_CPP) $(OBJ_C) $(OBJ_S)

# Compile rules.
.PHONY : all
all: $(PROJECT).a

$(PROJECT).a: $(OBJ)
	$(AR) rcs $(PROJECT).a $(OBJ)

$(OBJ_CPP) : %.o : %.cpp
	$(CPP) $(CPP_FLAGS) -c -o $@ $<

$(OBJ_C) : %.o : %.c
	$(CC) $(CC_FLAGS) -c -o $@ $<

$(OBJ_S) : %.o : %.S
	$(AS) $(AS_FLAGS) -o $@ $<



# Clean rules
.PHONY : clean
clean:
	rm -f $(PROJECT).a $(OBJ)
