
LIB=-L/usr/local/lib -L/usr/X11R6/lib -lX11 
INC=-I. -I /usr/X11R6/include -I /usr/local/include 

life: life.cpp life.h x11methods.h 
	g++ -I. life.cpp -o life $(LIB) $(INC) -w

clean:
	rm life
	rm *.o
