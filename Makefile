
FLAGS := `pkg-config --cflags opencv4` -std=c++17
LIBS := `pkg-config --libs opencv4`

main : main.cc camera.o pose.o
	g++ main.cc camera.o pose.o -o main $(FLAGS) $(LIBS)

camera.o : camera.cc camera.h defs.h
	g++ -c camera.cc -o camera.o $(FLAGS)

pose.o : pose.cc pose.h defs.h
	g++ -c pose.cc -o pose.o $(FLAGS)
