
LIB=-L/usr/local/lib -L/usr/X11R6/lib -lX11 ../x11grid/x11grid.a
INC=-I. -I /usr/X11R6/include -I /usr/local/include -I ../x11grid

life: life.cpp life.h 
	g++ -I. life.cpp -lpthread -o life $(LIB) $(INC) -w

clean:
	rm life
	rm *.o
