# ============================================================================== 
# Makefile for HPC Tools - Deliverable 1: Sequential Linear Regression 
# ============================================================================== 

CC ?= gcc 
CFLAGS ?= -O2 -Wall -std=c99 -D_POSIX_C_SOURCE=200809L
LDLIBS = -lm 
TARGET = linreg 
OBJS = linreg.o gaussian.o rng.o

all: $(TARGET)

.PHONY: all clean test_config1 test_config2 test_config3

$(TARGET): $(OBJS)
		$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

linreg.o: linreg.c gaussian.h rng.h timer.h
		$(CC) $(CFLAGS) -c linreg.c

gaussian.o: gaussian.c gaussian.h
		$(CC) $(CFLAGS) -c gaussian.c

rng.o: rng.c rng.h
		$(CC) $(CFLAGS) -c rng.c

clean:
		rm -f $(OBJS) $(TARGET)

# ------------------------------------------------------------------------------
# Benchmark Workload Configurations
# ------------------------------------------------------------------------------

test_config1: $(TARGET)
		./$(TARGET) 20000 50

test_config2: $(TARGET)
		./$(TARGET) 50000 300

test_config3: $(TARGET)
		./$(TARGET) 2000 2000
