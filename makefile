CC = /usr/bin/gcc-7
CFLAGS = -Wall -g -O2 -Werror -std=gnu99 -Wno-unused-function

TARGET_HARDWARE = ./bin/test_hardware

SRC_DIR = ./src

# debug
COMMON = $(SRC_DIR)/common/print.c $(SRC_DIR)/common/convert.c

# hardware
CPU =$(SRC_DIR)/hardware/cpu/mmu.c $(SRC_DIR)/hardware/cpu/isa.c
MEMORY = $(SRC_DIR)/hardware/memory/dram.c

# main
TEST_HARDWARE = $(SRC_DIR)/tests/test_hardware.c

.PHONY:hardware
hardware:
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(COMMON) $(CPU) $(MEMORY) $(TEST_HARDWARE) -o $(TARGET_HARDWARE)
	./$(TARGET_HARDWARE)

clean:
	rm -f *.o *~ $(TARGET_HARDWARE)