SHELL=/bin/bash

CPPFLAGS += -std=c++11 -W -Wall  -g
CPPFLAGS += -O3
CPPFLAGS += -I ../include

CPPFLAGS += -isystem ../opencl_sdk/include/

TBB_DIR = $(shell pwd)/../tbb42_20140601oss
TBB_INC_DIR = $(TBB_DIR)/include
TBB_LIB_DIR = $(TBB_DIR)/lib

PLATFORM = $(shell uname -s)
ifeq ($(PLATFORM), Linux)
	TBB_LIB_DIR = $(TBB_DIR)/lib/intel64/gcc4.4
	LDLIBS += -lOpenCL
endif

ifeq ($(PLATFORM), Darwin)
	LDLIBS += -framework OpenCL
endif

CPPFLAGS += -I $(TBB_INC_DIR)
LDFLAGS += -L $(TBB_LIB_DIR)

XLINKER += -Xlinker -rpath -Xlinker $(TBB_LIB_DIR)
LDLIBS += -ltbb

puzzles.o : *.hpp

../lib/libpuzzler.a : puzzles.o
	-mkdir -p ../lib
	ar cr ../lib/libpuzzler.a puzzles.o

all : ../lib/libpuzzler.a
