
main: main.o
	g++ main.cpp -lreadline -o main.o


clean :
	rm -f *.o
