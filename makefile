CC = /usr/bin/gcc-7
CFLAGS = -Wall -g -O2 -Werror -std=gnu99 -Wno-unused-function

EXE_HARDWARE = exe_hardware

SRC_DIR = ./src

# debug
COMMON = $(SRC_DIR)/common/print.c $(SRC_DIR)/common/convert.c

# hardware
CPU =$(SRC_DIR)/hardware/cpu/mmu.c $(SRC_DIR)/hardware/cpu/isa.c
MEMORY = $(SRC_DIR)/hardware/memory/dram.c

# main
MAIN_HARDWARE = $(SRC_DIR)/main_hardware.c

.PHONY:hardware
hardware:
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(COMMON) $(CPU) $(MEMORY) $(DISK) $(MAIN_HARDWARE) -o $(EXE_HARDWARE)
	./$(EXE_HARDWARE)

clean:
	rm -f *.o *~ $(EXE_HARDWARE)