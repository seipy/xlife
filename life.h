
#ifndef __LIFE_H__
#define __LIFE_H__
namespace Life
{
	//Rules
	//live cells that have less than two live neighbors will die
	//live cells that have two or three live neighbors stays alive
	//live cells that have greater than three live neighbors will die
	//dead cells that have three live neighbors will come to life

	struct LifeGrid;
	struct LifeRow;
	struct LifeColumn;
	struct LifeCell;
	struct TestStructure : X11Grid::DefaultStructure
	{
			typedef LifeGrid GridType;
			typedef LifeRow RowType;
			typedef LifeColumn ColumnType;
			typedef LifeCell CellType;
	};

	struct LifeCell : X11Grid::Cell
	{
			LifeCell(X11Grid::GridBase& _grid,const int _x,const int _y,bool _dead=true)
				: X11Grid::Cell(_grid,_x,_y),X(_x), Y(_y),
					neighbors(0), dead(_dead),dying(false),grayscale(0X3),age(1) , remove(false) { }
			virtual bool update(const unsigned long updateloop);
			virtual void operator()(Pixmap& bitmap);
			virtual bool Alive();
			private:
			const int X,Y;
			int neighbors;
			bool dead,dying,remove;
			unsigned long grayscale,age;
	};

	struct LifeColumn : X11Grid::Column<TestStructure>
	{
			LifeColumn(X11Grid::GridBase& _grid,const int _position) : X11Grid::Column<TestStructure>(_grid,_position) {}
			void Birth(const int x,const int y);
			virtual bool Alive(Point& p)
			{
				iterator it(find(p.second));
				if (it==end()) return false;
				return it->second.Alive();
			}
			virtual bool update(const unsigned long updateloop)
			{
				if (empty()) return true;
				for (iterator it=begin();it!=end();it++) 
					if (it->second.update(updateloop)) erase(it);
				if (empty()) return true;
				return false;
			}
			virtual void operator()(Pixmap& bitmap)
				{ for (iterator it=begin();it!=end();it++) it->second(bitmap); }
	};

	struct LifeRow : X11Grid::Row<TestStructure>
	{
			LifeRow(X11Grid::GridBase& _grid) : X11Grid::Row<TestStructure>(_grid) {}
			virtual void update(const unsigned long updateloop) ;
			virtual void operator()(Pixmap& bitmap)
			{ 
				for (LifeRow::iterator it=this->begin();it!=this->end();it++) it->second(bitmap);
			}
			bool Alive(Point& p)
			{
				iterator it(find(p.first));
				if (it==end()) return false;
				return it->second.Alive(p);
			}
			protected:
			void Birth(const int x,const int y)
			{
				//if (x<0) return;
				//if (y<0) return;
				//if (x>width) return;
				//if (y>height) return;
				if (find(x)==end()) insert(make_pair<int,LifeColumn>(x,LifeColumn(grid,x)));
				iterator it(find(x));
				if (it==end()) throw runtime_error("Cannot create row");
				it->second.Birth(x,y);
			}

			virtual void seed()
			{
				srand(time(0));
				for (int y=0;y<10;y++)
				{
					const int cx((rand()%10)+10);
					const int cy((rand()%10)+30);
					if (true)
					{
						Birth(cx,cy);
						Birth(cx+1,cy);
						Birth(cx+1,cy+1);
						Birth(cx,cy+1);
						Birth(cx,cy+2);
						Birth(cx+2,cy+1);
						//return;
					}
					for (int j=0;j<((rand()%3)+1);j++)
						for (int i=0;i<((rand()%3)+1);i++)
							Birth((rand()%10)+20,(rand()%10)+20);
				}
			}
	};


	struct LifeGrid : X11Grid::Grid<TestStructure>
	{
		LifeGrid(Display* _display,GC& _gc,const int _ScreenWidth, const int _ScreenHeight)
			: X11Grid::Grid<TestStructure>(_display,_gc,_ScreenWidth,_ScreenHeight),
					updaterate(40),updateloop(0),birthrate(0),endoflife(10000), populationcontrol(0),births(0)  {}
		int births;
		private:
		const int updaterate;
		unsigned long updateloop;
		double birthrate;
		Rect paint;
		const double endoflife;
		double populationcontrol;
		struct BirthingPool : map<Point,int> { } birthingpool;
		virtual void seed()
		{
			invalid.insert(Rect(0,0,ScreenWidth,ScreenHeight));	
			updateloop=1;
			births=birthrate=0;
			LifeRow::seed();
		}
		void operator()(Pixmap& bitmap) 
		{ 
			paint.clear();
			stringstream ss;
			stringstream ssupdates; ssupdates<<"Update:"<<updateloop;
			stringstream ssbirthrate; ssbirthrate<<"Birth Rate:"<<setprecision(2)<<birthrate;
			stringstream sspc; sspc<<"Population Control:"<<setprecision(2)<<populationcontrol;
			ss<<setw(20)<<left<<ssupdates.str();
			ss<<setw(20)<<left<<ssbirthrate.str();
			ss<<setw(30)<<left<<sspc.str();
			ss<<(*this);
			XSetForeground(display,gc,0X2222);
			XFillRectangle(display,bitmap,gc,10,100,ScreenWidth-160,40);
			XSetForeground(display,gc,0X7F7F7F);
			XDrawString(display,bitmap,gc,20,120,ss.str().c_str(),ss.str().size());
			X11Grid::Grid<TestStructure>::operator()(bitmap);
		}
		virtual void update()
		{
			TestStructure::RowType& grid(*this);
			if (births) birthrate=((double)births/(double)updateloop);
			++updateloop;
			if (updateloop>2000) if (birthrate<0.1) LifeRow::clear(); // this rule will reset the game if the birth rate is too low
			if ((updateloop%updaterate)) return;
			birthingpool.clear();
			LifeRow::update(updateloop);
			if (endoflife>0)	// these additional rules will eventually cause births to cease and the game will start over
			{
				GridBase& me(*this);
				double whence((double)updateloop);
				populationcontrol=((100*whence)/endoflife);
				if (populationcontrol>0)
				{
					populationcontrol=populationcontrol/((double)100);
					populationcontrol=1-populationcontrol;
					if (populationcontrol<0) populationcontrol=0;
					if (populationcontrol<0.5) LifeRow::clear();
					if (birthrate>populationcontrol) return;
				}
			}
			for (BirthingPool::iterator bit=birthingpool.begin();bit!=birthingpool.end();bit++)
			{
				if (bit->second==3) 
					{ Birth(bit->first.first,bit->first.second); }
			}
		}
		private:
		unsigned long color;
		double r,c;
		int cx,cy;
		deque<Point> tests;
		void operator()(const unsigned long color,Pixmap&  bitmap,const int _x,const int _y)
		{
			const int x(_x*10);
			const int y(_y*10);
			Rect r(x-4,y-4,x+4,y+4);	
			XPoint& points(r);
			XSetForeground(display,gc,color);
			XFillPolygon(display,bitmap,  gc,&points, 4, Complex, CoordModeOrigin);
			invalid.insert(r);
		}
		public:
			int operator()(const int x,const int y) 
			{
				int n(0);
				vector<Point> p;
				p.push_back(Point(x-1,y));
				p.push_back(Point(x-1,y+1));
				p.push_back(Point(x-1,y-1));
				p.push_back(Point(x+1,y));
				p.push_back(Point(x+1,y+1));
				p.push_back(Point(x+1,y-1));
				p.push_back(Point(x,y+1));
				p.push_back(Point(x,y-1));
				for (vector<Point>::iterator it=p.begin();it!=p.end();it++)
				{
					Point& P(*it);
					birthingpool[P]++;
					if (Alive(P)) n++;
				}
				return n;
			}
	};


		bool LifeCell::update(const unsigned long updateloop)
		{
			if (dying) {if (age>0) age--;} else if (age<0XFF) age++;
			map<string,int>& metrics(static_cast<map<string,int>&>(grid));	
			X11Grid::GridBase& gridbase(static_cast<X11Grid::GridBase&>(grid));	
			LifeGrid& lifegrid(static_cast<LifeGrid&>(grid));	
			if (dying) if (!dead) metrics["about to die"]++;
			if (dying) dead=true;
			neighbors=lifegrid(X,Y);
			if (neighbors<2) dying=true;
			if (neighbors>3) dying=true;
			if (neighbors==3) dying=false;
			if (!dead) metrics["live"]++; 
			if (dead) metrics["dead"]++; 
			if (dying) metrics["dying"]++; 
			return remove;
		}

		void LifeCell::operator()(Pixmap& bitmap)
		{
			unsigned long color(((grayscale*age)<<8) | (grayscale*age) );
			if (dead) color=0X3333;
			grid(color,bitmap,X,Y);
			if (dead) remove=true;
		}
		bool LifeCell::Alive() 
		{ 
			if ((dead) or (dying)) return false;
			return true;
		}
		void LifeColumn::Birth(const int x,const int y)
		{
			LifeGrid& lifegrid(static_cast<LifeGrid&>(grid));	
			if (find(y)!=end()) return;
			lifegrid.births++;
			insert(make_pair<int,LifeCell>(y,LifeCell(grid,x,y,false)));
			iterator it(find(y));
			if (it==end()) throw runtime_error("Cannot create column");
		}

			void LifeRow::update(const unsigned long updateloop) 
			{
				map<string,int>& metrics(static_cast<map<string,int>&>(grid));	
				//metrics.clear();
				metrics["about to die"]=0;
				metrics["live"]=0;
				metrics["dead"]=0;
				metrics["dying"]=0;
				if (empty()) seed();
				for (iterator it=begin();it!=end();it++) 
					if (it->second.update(updateloop)) erase(it);
			}
} // Life
#endif  //__LIFE_H__


