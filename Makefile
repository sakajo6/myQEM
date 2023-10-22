SrcPath = ./src/
IncludePath = -lglut -lGLU -lGL -I /usr/include/eigen3

demo: ./src/main.cpp ./src/display.o ./src/models.o ./src/obj_reader.o
	g++ -o $@ $^ $(IncludePath)

./src/models.o: ./src/models.cpp
	g++ -std=c++17 -c $^ $(IncludePath) -o $@

./src/obj_reader.o: ./src/obj_reader.cpp
	g++ -std=c++17 -c $^ $(IncludePath) -o $@

./src/display.o: ./src/display.cpp
	g++ -std=c++17 -c $^ $(IncludePath) -o $@

clean:
	rm ./src/*.o demo