
#include "x11grid.h"
using namespace X11Methods;
#include <life.h>

int main(int argc,char** argv)
{
	KeyMap keys;
	return X11Grid::x11main<Life::TestStructure>(argc,argv,keys,0X3333);
}

