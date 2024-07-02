# Directories
TARGET  := bin
EXE     := gpitool
BUILD   := obj
SOURCES := src
GENASM  := genasm
# Find source files
export CFILES       := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export FULLCFILES   := $(addprefix $(SOURCES)/,$(CFILES))
export CPPFILES     := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export FULLCPPFILES := $(addprefix $(SOURCES)/,$(CPPFILES))
export HEADERS      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.h)))
export FULLHEADERS  := $(addprefix $(SOURCES)/,$(HEADERS))
export GENASMFILES  := $(addprefix $(GENASM)/,$(CFILES:.c=.asm) $(CPPFILES:.cpp=.asm))
export OFILESC      := $(CFILES:.c=.o)
export OFILESCPP    := $(CPPFILES:.cpp=.o)
export OFILES       := $(OFILESC) $(OFILESCPP)
export FULLOFILES   := $(addprefix $(BUILD)/,$(OFILES))

# Hacky overrides for my own use, they don't interfere with 'sane' building
ifdef MHB_SYSTEM_INCLUDE
MHB_SYSTEM_INCLUDE := -isystem$(MHB_SYSTEM_INCLUDE)
endif

ifdef MHB_SYSTEM_LIBS
MHB_SYSTEM_LIBS := -L$(MHB_SYSTEM_LIBS)
endif

# Flags and libraries
export CC               := gcc
export CXX              := g++
export CFLAGSBASE       := -fopenmp $(MHB_SYSTEM_INCLUDE) `pkg-config gtkmm-3.0 --cflags`
export CFLAGSDEBUG      := $(CFLAGSBASE) -Og
export CFLAGSRELEASE    := $(CFLAGSBASE) -O3 -flto=auto -fno-trapping-math -fno-math-errno -ffp-contract=fast -ffinite-math-only -fno-signed-zeros -freciprocal-math
export CFLAGS           := $(CFLAGSBASE)
export CXXFLAGS         := -DGLIBMM_DISABLE_DEPRECATED -DGIOMM_DISABLE_DEPRECATED -DGTKMM_DISABLE_DEPRECATED -DGDKMM_DISABLE_DEPRECATED
export LINKFLAGSBASE    := $(MHB_SYSTEM_LIBS)
export LINKFLAGSDEBUG   := $(LINKFLAGSBASE)
export LINKFLAGSRELEASE := $(LINKFLAGSBASE) -Wl,-s
export LINKFLAGS        := $(LINKFLAGSBASE)
export LINKLIBS         := -lpng -ljpeg -llz4 -lm `pkg-config gtkmm-3.0 --libs`

# Define some variable overrides
default : CFLAGS = $(CFLAGSRELEASE)
build-linux32 : CFLAGS = $(CFLAGSRELEASE)
build-linux64 : CFLAGS = $(CFLAGSRELEASE)
build-win32 : CFLAGS = $(CFLAGSRELEASE)
build-win64 : CFLAGS = $(CFLAGSRELEASE)
native : CFLAGS = $(CFLAGSRELEASE) -march=native
debug : CFLAGS = $(CFLAGSDEBUG)

default : LINKFLAGS = $(LINKFLAGSRELEASE)
build-linux32 : LINKFLAGS = $(LINKFLAGSRELEASE)
build-linux64 : LINKFLAGS = $(LINKFLAGSRELEASE)
build-win32 : LINKFLAGS = $(LINKFLAGSRELEASE)
build-win64 : LINKFLAGS = $(LINKFLAGSRELEASE)
native : LINKFLAGS = $(LINKFLAGSRELEASE)
debug : LINKFLAGS = $(LINKFLAGSDEBUG)

build-linux32 : CC = i686-linux-gnu-gcc
build-linux64 : CC = x86_64-linux-gnu-gcc
build-win32 : CC = i686-w64-mingw32-gcc
build-win64 : CC = x86_64-w64-mingw32-gcc

build-linux32 : CXX = i686-linux-gnu-g++
build-linux64 : CXX = x86_64-linux-gnu-g++
build-win32 : CXX = i686-w64-mingw32-g++
build-win64 : CXX = x86_64-w64-mingw32-g++

.PHONY : default native debug showasm clean cleanasm install-linux

all : default

#Builds for the current host OS, but assumes a basic variant of the current processor architecture
default : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Builds for Linux on x86-32
build-linux32 : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Builds for Linux on x86-64
build-linux64 : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Builds for Windows on x86-32
build-win32 : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Builds for Windows on x86-64
build-win64 : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Builds specifically for this machine
native : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Build a debug-friendly version
debug : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Compiles the source files to assembly language for curiosity's sake
showasm : $(GENASM) $(GENASMFILES)

#Cleans the build directory
clean : $(BUILD) $(TARGET)
	rm -rf $(BUILD)
	rm -rf $(TARGET)
	mkdir -p $(BUILD)
	mkdir -p $(TARGET)

#Removes the generated assembly files
cleanasm : $(GENASM)
	rm -rf $(GENASM)

#Copies output executable to /usr/local/bin/ (with confirmation)
install-linux : $(TARGET)/$(EXE)
	sudo cp $^ /usr/local/bin/

$(BUILD) :
	mkdir -p $(BUILD)

$(TARGET) :
	mkdir -p $(TARGET)

$(GENASM) :
	mkdir -p $(GENASM)

$(TARGET)/$(EXE) : $(FULLOFILES)
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(LINKFLAGS) -o $@ $^ $(LINKLIBS)

$(FULLOFILES) : $(FULLCFILES) $(FULLCPPFILES) $(FULLHEADERS)

$(BUILD)/%.o : $(SOURCES)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD)/%.o : $(SOURCES)/%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(GENASM)/%.asm	: $(SOURCES)/%.c
	$(CC) -c -O3 -S -o $@ $<

$(GENASM)/%.asm	: $(SOURCES)/%.cpp
	$(CXX) -c -O3 $(CXXFLAGS) -S -o $@ $<
