INCLUDES = ./Boundless D:/vulkanSDK/Include D:/msys2/mingw64/include D:/c++programs/assimp-5.3.1/include
INCLUDEDIRS := $(patsubst %,-I%, $(INCLUDES))
FLAGS = DEBUG
FLAGSDEF := $(patsubst %,-D%, $(FLAGS))
LIBRARIES = ./lib
LIBRARYDIR = $(patsubst %,-L%, $(LIBRARIES))
LIBS = glfw3dll vulkan-1 zlib
LIBARG = $(patsubst %,-l%, $(LIBS))
CXX = g++
CXXFLAGS = -g -std=c++20

SRCS := 
OBJS := $(patsubst %.cpp,output/%.o,$(SRCS))

SANDBOXSRC = main.cpp
SANDBOXOBJ = $(patsubst %.cpp,output/%.o,$(SANDBOXSRC))

TARGET := MAIN

.PHONY: all clean
all: $(TARGET)
	./bin/$(TARGET).exe

clean:
	$(RM) bin/$(TARGET) $(OBJS) $(SANDBOXOBJ)

$(TARGET):$(OBJS) $(SANDBOXOBJ)
	$(CXX) -o $(TARGET) $(OBJS) $(SANDBOXOBJ) $(LIBRARYDIR) $(LIBARG)

$(OBJS):output/%.o:src/%.cpp inc/%.hpp
	$(CXX) -c $< -o $@ $(INCLUDEDIRS) $(FLAGSDEF) $(CXXFLAGS)

$(SANDBOXOBJ):output/%.o:sandbox/%.cpp
	$(CXX) -c $< -o $@ $(INCLUDEDIRS) $(FLAGSDEF) $(CXXFLAGS)