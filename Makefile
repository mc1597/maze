all: sample3D

sample3D: maze_3D.cpp glad.c
	g++ -o sample3D maze_3D.cpp glad.c -lGL -lglfw -lftgl -ldl -lSOIL -lGLEW -I/usr/local/include -I/usr/local/include/freetype2 -L/usr/local/lib

clean:
	rm sample3D
