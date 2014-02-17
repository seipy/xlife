
#ifndef __LIFE_H__
#define __LIFE_H__
namespace Life
{
	//Rules
	//live cells that have less than two live neighbors will die
	//live cells that have two or three live neighbors stays alive
	//live cells that have greater than three live neighbors will die
	//dead cells that have three live neighbors will come to life


	#if 0
		struct Invalid : X11Methods::InvalidArea<Rect> 
			{ void insert(const int _x,const int _y,Rect r) {set<Rect>::insert(r); } };
	#else
		struct Invalid : X11Grid::InvalidGrid { };
	#endif

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

	struct ColorCurve
	{
		ColorCurve(X11Grid::GridBase& _grid,double _sx,double _sy) : grid(_grid),sx(_sx), sy(_sy),t(0),c(0X33),alive(true) {}
		void operator()()
		{
			if (alive) if (c>=0XFF) {c=0XFF;return;}
			if (!alive) if (t<150) t=150;
			t+=0.1; 
			double T((t*1000)/300);
			if (T>1000) return;
			if (T<500) c=(log(T)*50);
			if (T>500) c=255-((log(T-500)*50));
			if (c>255) c=255;
			if (c<0) c=0;
			double y=-c;
			double range(0XFF-0X33);
			c=((c/0XFF)*range)+0X33;
			if (c<0X55) c=0X33;
		}
		operator const unsigned long (){return floor(c);}
		void operator = (bool b){alive=b;}
		private:
		X11Grid::GridBase& grid;
		const double sx,sy;
		double t,c;
		bool alive;
	};

	struct LifeCell : X11Grid::Cell
	{
			LifeCell(X11Grid::GridBase& _grid,const int _x,const int _y,bool _dead=true)
				: X11Grid::Cell(_grid,_x,_y),X(_x), Y(_y),
					neighbors(0), dead(_dead),dying(false),remove(false),curve(grid,200,600) { }
			virtual bool update(const unsigned long updateloop,const unsigned long updaterate);
			virtual void operator()(Pixmap& bitmap);
			virtual bool Alive();
			private:
			const int X,Y;
			int neighbors;
			bool dead,dying,remove;
			ColorCurve curve;
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
			virtual bool update(const unsigned long updateloop,const unsigned long updaterate)
			{
				if (empty()) return true;
				for (iterator it=begin();it!=end();it++) 
					if (it->second.update(updateloop,updaterate)) erase(it);
				if (empty()) return true;
				return false;
			}
			virtual void operator()(Pixmap& bitmap)
				{ for (iterator it=begin();it!=end();it++) it->second(bitmap); }
	};

	struct LifeRow : X11Grid::Row<TestStructure>
	{
			LifeRow(X11Grid::GridBase& _grid) : X11Grid::Row<TestStructure>(_grid) {}
			virtual void update(const unsigned long updateloop,const unsigned long updaterate) ;
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
			void Birth(const int x,const int y,const int ScreenWidth,const int ScreenHeight,const int CW,const int CH)
			{
				int W(ScreenWidth-CW);
				int H(ScreenHeight-CH);
				int accross(W/CW);
				int down(H/CH);
				if (x<1) return;
				if (y<1) return;
				if (x>accross) return;
				if (y>down) return;
				if (find(x)==end()) insert(make_pair<int,LifeColumn>(x,LifeColumn(grid,x)));
				iterator it(find(x));
				if (it==end()) throw runtime_error("Cannot create row");
				it->second.Birth(x,y);
			}

			virtual void seed(const int ScreenWidth,const int ScreenHeight,const int CW,const int CH)
			{
				srand(time(0));
				int W(ScreenWidth-CW);
				int H(ScreenHeight-CH);
				int accross(W/CW);
				int down(H/CH);
				for (int y=0;y<10;y++)
				{
					const int cx(accross/2);
					const int cy(down/2);
					if (false)
					{
						Birth(cx,cy,ScreenWidth,ScreenHeight,CW,CH);
						Birth(cx+1,cy,ScreenWidth,ScreenHeight,CW,CH);
						Birth(cx+1,cy+1,ScreenWidth,ScreenHeight,CW,CH);
						Birth(cx,cy+1,ScreenWidth,ScreenHeight,CW,CH);
						Birth(cx+(rand()%2)+1,cy+1+(rand()%2),ScreenWidth,ScreenHeight,CW,CH);
						Birth(cx+(rand()%2)+1,cy+1+(rand()%2),ScreenWidth,ScreenHeight,CW,CH);
						Birth(cx+(rand()%2)+1,cy+1+(rand()%2),ScreenWidth,ScreenHeight,CW,CH);
						return;
					}
					for (int j=0;j<((rand()%20)+10);j++)
						Birth((rand()%accross),(rand()%down),ScreenWidth,ScreenHeight,CW,CH);
				}
			}
	};



	struct LifeGrid : X11Grid::Grid<TestStructure>
	{
		LifeGrid(Display* _display,GC& _gc,const int _ScreenWidth, const int _ScreenHeight)
			: X11Grid::Grid<TestStructure>(_display,_gc,_ScreenWidth,_ScreenHeight),
					CW(20),CH(20),
					updaterate(10),updateloop(0),birthrate(0),endoflife(2000), populationcontrol(0),births(0)  {}
		virtual operator InvalidBase& () {return invalid;}
		int births;
		private:
		Invalid invalid;
		const int CW,CH;
		const int updaterate;
		unsigned long updateloop;
		double birthrate;
		const double endoflife;
		double populationcontrol;
		struct BirthingPool : map<Point,int> { } birthingpool;
		virtual void seed()
		{
			updateloop=1;
			births=birthrate=0;
			LifeRow::seed(ScreenWidth,ScreenHeight,CW,CH);
		}
		void operator()(Pixmap& bitmap) 
		{ 
			#if 0
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
			#endif
			XSetForeground(display,gc,0X3333);
			XFillRectangle(display,bitmap,gc,0,00,ScreenWidth,ScreenHeight);
			X11Grid::Grid<TestStructure>::operator()(bitmap);
		}
		virtual void update()
		{
			if (LifeRow::empty()) seed();
			TestStructure::RowType& grid(*this);
			if (births) birthrate=((double)births/(double)updateloop);
			++updateloop;
			if (updateloop>200) if (birthrate<0.1) LifeRow::clear(); // this rule will reset the game if the birth rate is too low
			//if ((updateloop%updaterate)) return;
			birthingpool.clear();
			LifeRow::update(updateloop,updaterate);
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
					{ Birth(bit->first.first,bit->first.second,ScreenWidth,ScreenHeight,CW,CH); }
			}
		}
		private:
		void operator()(const unsigned long color,Pixmap&  bitmap,const int _x,const int _y)
		{
			InvalidBase& _invalidbase(*this);
			Invalid& _invalid(static_cast<Invalid&>(_invalidbase));
			const int x(_x*CW);
			const int y(_y*CW);
			if (color!=0X3333)
			{
				const int border(2);
				X11Methods::Rect r((x-(CW/2))+border,(y-(CH/2))+border,(x+(CW/2))-border,(y+(CH/2))-border);	
				X11Grid::ProximityRectangle i(_x,_y,(x-(CW/2)),(y-(CH/2)),(x+(CW/2)),(y+(CH/2)));	
				XPoint& points(r);
				XPoint& bpoints(i);
				XSetForeground(display,gc,0X3333);
				XFillPolygon(display,bitmap,  gc,&bpoints, 4, Complex, CoordModeOrigin);
				XSetForeground(display,gc,color);
				XFillPolygon(display,bitmap,  gc,&points, 4, Complex, CoordModeOrigin);
				_invalid.insert(_x,_y,i);
			} else {
				const int border(0);
				X11Grid::ProximityRectangle r(_x,_y,(x-(CW/2))+border,(y-(CH/2))+border,(x+(CW/2))-border,(y+(CH/2))-border);	
				XPoint& points(r);
				XSetForeground(display,gc,color);
				XFillPolygon(display,bitmap,  gc,&points, 4, Complex, CoordModeOrigin);
				_invalid.insert(_x,_y,r);
			}
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


		bool LifeCell::update(const unsigned long updateloop,const unsigned long updaterate)
		{
			curve();
			if (updateloop%updaterate) return false;
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
			if (dying) curve=false;
			return remove;
		}

		void LifeCell::operator()(Pixmap& bitmap)
		{
			const unsigned long  c(curve);
			unsigned long color((c<<8)|c);
			grid(color,bitmap,X,Y);
			if (dead) if (c==0X33) remove=true;
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

		void LifeRow::update(const unsigned long updateloop,const unsigned long updaterate) 
		{
			map<string,int>& metrics(static_cast<map<string,int>&>(grid));	
			metrics["about to die"]=0;
			metrics["live"]=0;
			metrics["dead"]=0;
			metrics["dying"]=0;
			for (iterator it=begin();it!=end();it++) 
				if (it->second.update(updateloop,updaterate)) erase(it);
		}
} // Life
#endif  //__LIFE_H__


