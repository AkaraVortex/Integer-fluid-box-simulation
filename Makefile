SOURCE_FILES := $(wildcard bin/*.cc) $(wildcard bin/*.c) $(wildcard bin/*.cpp)
OBJECT_FILES := $(patsubst %.cc, %.o, $(SOURCE_FILES))



all: compile

compile: $(OBJECT_FILES)
	g++ -o compiled.exe $^ 

%.o: %.c
	g++ -c -o $@ $<

%.o: %.cc
	g++ -c -o $@ $<

%.o: %.cpp
	g++ -c -o $@ $<