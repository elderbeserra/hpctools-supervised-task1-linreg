# ============================================================================== 
# Makefile for HPC Tools - Deliverable 1: Sequential Linear Regression 
# ============================================================================== 

CC ?= gcc 
CFLAGS ?= -O2 -Wall -std=c99 
LDLIBS = -lm 
TARGET = linreg 
OBJS = linreg.o gaussian.o

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: all clean test_config1 test_config2 test_config3

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

linreg.o: linreg.c gaussian.h
		$(CC) $(CFLAGS) -c linreg.c

gaussian.o: gaussian.c gaussian.h
		$(CC) $(CFLAGS) -c gaussian.c

clean:
		rm -f $(OBJS) $(TARGET)

# ------------------------------------------------------------------------------
# Benchmark Workload Configurations
# ------------------------------------------------------------------------------

test_config1: $(TARGET)
		./$(TARGET) --N 20000 --p 50

test_config2: $(TARGET)
		./$(TARGET) --N 50000 --p 300

test_config3: $(TARGET)
		./$(TARGET) --N 2000 --p 2000