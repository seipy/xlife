
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <vector>
#include <stdexcept>
#include <iomanip.h>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <iostream>
#include <utility>
using namespace std;

#include "keystrokes.h"
#include "x11methods.h"
#include "life.h"


int main(int argc,char** argv)
{
	XSizeHints displayarea;
	Display *display(XOpenDisplay(""));
	int myscreen(DefaultScreen(display));
	const unsigned long mybackground(0X3333);
	const unsigned long myforeground(0X3333);

	displayarea.x = 000;
	displayarea.y = 000;
	displayarea.width = 1280;
	displayarea.height = 1024;
	displayarea.flags = PPosition | PSize;

	Window window(XCreateSimpleWindow(display,DefaultRootWindow(display), displayarea.x,displayarea.y,displayarea.width,displayarea.height,5,myforeground,mybackground));
	GC mygc(XCreateGC(display,window,0,0));
	XSetBackground(display,mygc,mybackground);
	XSetForeground(display,mygc,myforeground);
	XMapRaised(display,window);
	XSelectInput(display,window,ButtonPressMask|KeyPressMask|ExposureMask);

	stringstream except;
	try
	{
		Life::Grid canvas(display,mygc,displayarea.width, displayarea.height);
		KeyMap keys;
		Program life(myscreen,display,window,mygc,NULL,canvas,keys,displayarea.width,displayarea.height);
		life(argc,argv);
	}
	catch(runtime_error& e){except<<"runtime error:"<<e.what();}
	catch(...){except<<"unknown error";}
	if (!except.str().empty()) cout<<except.str()<<endl;

	XFreeGC(display,mygc);
	XDestroyWindow(display,window);
	XCloseDisplay(display);
	return 0;
}


