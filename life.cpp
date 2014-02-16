
#include "x11grid.h"
using namespace X11Methods;
#include <math.h>
#include <life.h>
#include <pthread.h>
#include <string>
#include <sstream>
using namespace std;


	inline string runpipe(string what)
	{
		stringstream ssret;
		string ret; 
		char * line = NULL;	
		size_t len = 0;	ssize_t bread;
		FILE* fp = popen((char*)what.c_str(),"r");
		if (fp == NULL) {return "cannot open pipe";}
		while (1)
		{
			char buf[514];
			memset(buf,0,513);
			int l = fread(buf,1,512,fp);
			if (l > 0) ret+=buf;
			else break;
		}
		pclose(fp);
		return ret;
	}

void* ssup(void*) 
{
	while (true)
	{
		runpipe("xset dpms force on");
		runpipe("xset s off");
		runpipe("xset s noblank");
		sleep(300);
	}
}

int main(int argc,char** argv)
{
	pthread_t t; 
	pthread_create(&t,0,ssup,0);
	KeyMap keys;
	return X11Grid::x11main<Life::TestStructure>(argc,argv,keys);
}

