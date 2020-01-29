DESIGN_NAME := Metabolite_Network

EXECUTABLE := bin/$(DESIGN_NAME).x

SYSTEMC := $$SYSTEMC_HOME

PLATFORM := $(shell uname -s)
ARCH := $(shell getconf LONG_BIT)

ifeq ($(PLATFORM),Darwin)
SYSC_LIBDIR := $(SYSTEMC)/lib-macosx
endif
ifeq ($(PLATFORM),Linux)
SYSC_LIBDIR := $(SYSTEMC)/lib-linux
endif

ifeq ($(ARCH),64)
SYSC_LIBDIR := $(SYSC_LIBDIR)64
endif

LIBDIR := $(SYSC_LIBDIR)
LIBS := $(SYSC_LIBDIR)/libsystemc.a -lpthread

INCDIR := -I./include/ -I$(SYSTEMC)/include/ 
#-I./include/base/

CXX := g++
CFLAGS := -c
#-pedantic -Wall -Wextra -Weffc++ https://stackoverflow.com/questions/5088460/flags-to-enable-thorough-and-verbose-g-warnings

SRCDIR := src
OBJDIR := obj

# To speed up parallelizing on all cores
MAKEFLAGS += "-j $(shell nproc)"

# Get Structure of Subdirectory
STRUCTURE := $(shell find $(SRCDIR) -type d)
CODEFILES := $(addsuffix /*,$(STRUCTURE))
CODEFILES := $(wildcard $(CODEFILES))

# Filter Only Specific Files								
SRCDIRFILES := $(filter %.cc,$(CODEFILES))
OBJDIRFILES := $(subst $(SRCDIR),$(OBJDIR),$(SRCDIRFILES:%.cc=%.o))

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJDIRFILES)
	@echo Building $@
	$(CXX) -g $(INCDIR) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	@echo Building $@
	$(CXX) -g $(CFLAGS) $(INCDIR) -o $@ $<
	
clean:
	@echo Cleaning
	@rm -f $(EXECUTABLE)
	@rm -f obj/*.o *~

clean_all:
	@echo Cleaning all generated files
	@rm -f $(EXECUTABLE)
	@rm -f obj/*.o *~
	@rm -f src/*.cc *~
	@rm -f include/*.h* *~
	@rm -f ./met_concentrations.txt *~
	@rm -f ../met_concentrations.txt *~
	@rm -f ./metabolite_network.vcd *~
	@rm -f ./met_concentrations_Rplot.png *~

.PHONY: all clean clean_all
