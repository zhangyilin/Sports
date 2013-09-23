# Copyright (c) 2007 Zhuo Li
#
# Written by Zhuo Li
#
# This Makefile will compile all C (*.c) and C++ (*.C) sources in 
# this directory and link them into an executable specified by variable TARGET.
# In order not to have to model the exact dependencies on header files this
# Makefile assumes that all C/C++ files depend on all header files.
# Bison and flex input files, if any, are handled as expected.
#
# make debug   - compile and link to produce debuggable executable
# make opt     - compile and link to produce optimized executable
# make clean   - remove all generated files
#

TARGET = logic_simulate

#
# Tools used
#
CC    = gcc
CXX   = g++
FLEX  = flex
BISON = bison
COMPARE_RESULT = diff -bB

#
# Extra libraries
#
LIBS = -lm

#
# Define LD_RUN_PATH such that ld.so will find the shared objcets
#
#export LD_RUN_PATH=$(HOME)/lib

#
# Bison, Flex input if any 
# (Only one grammar/scanner spec at this point)
#
Y_FILE := $(wildcard *.y)
Y_SRCS := $(subst .y,.tab.c,$(Y_FILE))
Y_HDRS := $(subst .y,.tab.h,$(Y_FILE))
L_FILE := $(wildcard *.l)
L_SRCS := $(subst .l,.yy.c,$(L_FILE))

#
# Assemble sources, objects, and headers
#
C_SRCS  := $(wildcard *.c)
C_SRCS  := $(subst $(L_SRCS),,$(C_SRCS))
C_SRCS  := $(subst $(Y_SRCS),,$(C_SRCS))
C_SRCS  += $(L_SRCS)
C_DOBJS := $(C_SRCS:.c=.o)
C_SRCS  += $(Y_SRCS)
C_OBJS  := $(C_SRCS:.c=.o)

CXX_SRCS  := $(wildcard *.C)
CXX_OBJS  := $(CXX_SRCS:.C=.o)

HDRS   := $(wildcard *.h)
HDRS   := $(subst $(Y_HDRS),,$(HDRS))
HDRS   += $(Y_HDRS)

# Choose suitable commandline flags 
#

#DEF = -D
OPT = -O3

ifeq "$(MAKECMDGOALS)" "opt"
CFLAGS   = $(OPT) $(DEF)
CXXFLAGS = $(OPT) $(DEF)
else
PG = -pg
CFLAGS   = -g -W -Wall -pedantic -Wno-unused-parameter -DYYDEBUG $(PG)
CXXFLAGS = -g -W -Wall -pedantic $(PG)
endif

CFLAGS   +=  -I. -I$(HOME)/include
CXXFLAGS +=  -I. -I$(HOME)/include

.PHONY:	clean debug opt

.SECONDARY: $(Y_SRCS) $(L_SRCS)

debug opt pg: $(TARGET)

$(TARGET):  $(C_OBJS) $(CXX_OBJS) 
	$(CXX) -o $(TARGET) $(C_OBJS) $(CXX_OBJS) $(LIBS) $(PG)


%.yy.c:%.l
	$(FLEX) -o$(notdir $*).yy.c $<

%.tab.c %.tab.h:%.y
	$(BISON) -d --output=$*.tab.c $<

%.o:%.c
	$(CC) -c $(CFLAGS) $<

%.o:%.C
	$(CXX) -c $(CXXFLAGS) $<

$(C_DOBJS) $(CXX_OBJS): $(HDRS)

#
# Flex generates code that causes harmless warnings; suppress those
#
$(L_OBJS): $(L_SRCS) $(HDRS)
	$(CC) -c $(CFLAGS) -Wno-unused-function -Wno-unused-label \
                 -Wno-implicit-function-declaration $<

clean : 
	rm -f *.o $(TARGET) $(Y_HDRS) $(Y_SRCS) $(L_SRCS) *.gpl

