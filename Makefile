SRC_DIR := src
SRCS := $(shell find $(SRC_DIR) -name "*.c")
OBJS := $(subst .c,.o, $(SRCS))
EXE := polyxb

ISL_PREFIX := /home/hanjh/local/isl
PET_PREFIX := /home/hanjh/local/pet
LLVM_PREFIX := /home/hanjh/local/llvm-9.x
INC_FLAGS := -I$(ISL_PREFIX)/include -I$(PET_PREFIX)/include
LD_FLAGS := -L$(ISL_PREFIX)/lib -L$(PET_PREFIX)/lib -L$(LLVM_PREFIX)/lib \
			-lm -lclang-cpp -lisl -lpet

all: $(EXE)

$(EXE): $(OBJS)
	gcc -o $@ $(OBJS) $(LD_FLAGS) $(LD_FLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	gcc -g $(INC_FLAGS) -o $@ -c $<

clean:
	rm -f $(SRC_DIR)/*.o $(EXE)

.PHONY: all clean
