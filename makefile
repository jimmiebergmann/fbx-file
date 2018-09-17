examples: example1

example1: folders obj/miniz.o obj/fbx.o obj/example1.o
	$(CXX) -o bin/example1 obj/miniz.o obj/fbx.o obj/example1.o

obj/example1.o: examples/example1.cpp
	$(CXX) -std=c++11 -c examples/example1.cpp -o obj/example1.o

obj/fbx.o: fbx.cpp
	$(CXX) -std=c++11 -c fbx.cpp -o obj/fbx.o

obj/miniz.o: miniz.c
	$(CXX) -std=c++11 -c miniz.c -o obj/miniz.o

folders:
	mkdir -p bin
	mkdir -p obj

.PHONY: clean
clean:
	rm -r obj
