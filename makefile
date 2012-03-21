program_C_SRCS := $(wildcard *.c)
program_NAMES := viewbits
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)
program_INCLUDE_DIRS := .
program_LIBRARY_DIRS :=
program_LIBRARIES :=

CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
CPPFLAGS += -O3
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))

.PHONY: clean distclean

all: $(program_NAMES)

viewbits: viewbits.cpp 
	$(LINK.cc) -o viewbits viewbits.cpp -lglut -lGLU -lGL

clean:
	@- $(RM) $(program_NAMES)
	@- $(RM) $(program_OBJS)
install: all
	cp viewbits ~/bin/
distclean: clean
