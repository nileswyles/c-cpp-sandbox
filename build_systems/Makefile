ifndef CXX_COMPILER
CXX_COMPILER=g++
endif
CXX_WARN=-Wall
CXX_INCLUDE=-iquote $(LIB_ROOT)

OUT=./out
OBJS_OUT=./out/objs
LIB_ROOT=../lib

OBJECTS := $(addprefix $(OBJS_OUT)/, main.o array.o)

all: $(OUT) $(OBJS_OUT) $(OUT)/test
.PHONY: clean # because it doesn't actually create a target file.
# TODO: think about how useful this is for "hiding" intermediate object files if at all possible.
clean :
	rm -rf $(OUT)
$(OUT):
	mkdir $(OUT)
$(OBJS_OUT):
	mkdir $(OBJS_OUT)


# rule syntax (section 4.2)
# 	targets : prerequisites
#        recipe
#        …
#
# targets : prerequisites ; recipe
#        recipe
#        …
# $^ - list of all pre requisites
# $@ - target
$(OUT)/test : $(OBJECTS)
	$(CXX_COMPILER) $^ -o $@

# g++ -iquote ../lib -Wall -c main.cpp
$(OBJS_OUT)/main.o : main.cpp
	$(CXX_COMPILER) $(CXX_WARN) $(CXX_INCLUDE) -c $^ -o $@
	
# to be moved to it's own Makefile for reusability? see section 3.3
$(OBJS_OUT)/array.o : $(LIB_ROOT)/datastructures/array.cpp
	$(CXX_COMPILER) $(CXX_WARN) $(CXX_INCLUDE) -c $^ -o $@
















# Configuration variables to compile and link against the Google Cloud Storage
# # C++ client library.
# GCS_DEPS := google_cloud_cpp_storage
# GCS_CXXFLAGS   := $(shell pkg-config $(GCS_DEPS) --cflags)
# GCS_CXXLDFLAGS := $(shell pkg-config $(GCS_DEPS) --libs-only-L)
# GCS_LIBS       := $(shell pkg-config $(GCS_DEPS) --libs-only-l)
# A target using the Google Cloud Storage C++ client library.
# $(OUT)/bin: main.o \
# 	$(CXX_COMPILER) $(CXXFLAGS) $(GCS_CXXFLAGS) $(GCS_CXXLDFLAGS) -o $@ $^ $(GCS_LIBS)
