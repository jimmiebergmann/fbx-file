# examples
examples: example1

example1: fbx-file obj/example1.o
	$(CXX) -o bin/example1 obj/miniz.o obj/fbx.o obj/example1.o

obj/example1.o: examples/example1.cpp
	$(CXX) -std=c++11 -c examples/example1.cpp -o obj/example1.o

# test
test: fbx-file obj/test.o
	$(CXX) -o bin/test obj/miniz.o obj/fbx.o obj/test.o -s test/googletest/googletest/make/gtest_main.a -lpthread

obj/test.o: test/test.cpp
	$(CXX) -std=c++11 -Itest/googletest/googletest/include -c test/test.cpp -o obj/test.o

# fbx-file
fbx-file: folders obj/miniz.o obj/fbx.o

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
