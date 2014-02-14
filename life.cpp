
#include "x11grid.h"
using namespace X11Methods;
#include <math.h>
#include <life.h>

int main(int argc,char** argv)
{
	KeyMap keys;
	return X11Grid::x11main<Life::TestStructure>(argc,argv,keys);
}

