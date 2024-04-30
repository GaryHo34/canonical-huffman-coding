# Compiler and assembler invocation.
SRC_DIR = src

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(SRC_DIR)/%.o, $(SRC_FILES))

DEFINES = -D_NON_PINTOS
WARNINGS = -Wall -W -Wstrict-prototypes -Wmissing-prototypes -Wsystem-headers
CFLAGS = -g -O2 -fno-omit-frame-pointer -ffreestanding

all: huffc testhuffc

testhuffc: $(SRC_DIR)/testhuffc.o
	$(CC) $(CFLAGS) $^ -o $@

huffc: $(SRC_DIR)/huffc.o $(SRC_DIR)/heap.o $(SRC_DIR)/codeword.o $(SRC_DIR)/treepool.o
	$(CC) $(CFLAGS) $^ -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(WARNINGS) -c $< -o $@

clean: 
	rm -f $(OBJ_FILES) *.cmp *.ucmp testhuffc huffc