SHELL=/bin/bash

CPPFLAGS += -std=c++11 -W -Wall  -g
CPPFLAGS += -O3
CPPFLAGS += -I include

CPPFLAGS += -I opencl_sdk/include/

TBB_DIR = $(shell pwd)/tbb42_20140601oss
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

lib/libpuzzler.a : provider/*.cpp provider/*.hpp
	cd provider && $(MAKE) all

bin/% : src/%.cpp lib/libpuzzler.a
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) -o $@ $^ $(XLINKER) $(LDFLAGS) $(LDLIBS) -Llib -lpuzzler

all : clean bin/execute_puzzle bin/create_puzzle_input bin/run_puzzle bin/compare_puzzle_output

clean : 
	rm -rf bin/*
	rm -f lib/libpuzzler.a
