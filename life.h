
#ifndef __LIFE_H__
#define __LIFE_H__
namespace Life
{
	//Rules
	//live cells that have less than two live neighbors will die
	//live cells that have two or three live neighbors stays alive
	//live cells that have greater than three live neighbors will die
	//dead cells that have three live neighbors will come to life

	#define SEED_ROW 50
	#define SEED_COL 50

	struct GridBase : map<string,int>
	{
		GridBase() : births(0) {}
		virtual void operator()(const unsigned long color,Pixmap&  bitmap,const int x,const int y, const int CW,const int CH) = 0;
		virtual int operator()(const int x,const int y) = 0;
		virtual bool limits(const int x,const int y) = 0;
		friend ostream& operator<<(ostream&,GridBase&);
		virtual ostream& operator<<(ostream& o) { for (iterator it=begin();it!=end();it++) o<<it->first<<":"<<setw(8)<<it->second<<" "; return o;}
		unsigned long births;
	};
	inline ostream& operator<<(ostream& o,GridBase& b){return b.operator<<(o);}


	struct Cell 
	{
		Cell(GridBase& _grid,const int _x,const int _y,bool _dead=true) 
			: grid(_grid), X(_x), Y(_y),neighbors(0),
				dead(_dead),dying(false),grayscale(0X3F),age(0) , remove(false) { }
		GridBase& grid;
		const int X,Y;
		int neighbors;
		bool dead,dying,remove;
		unsigned long grayscale,age;
		void seed(){}
		bool update()
		{
			age++;
			if (dying) if (!dead) grid["about to die"]++;
			if (dying) if (!dead) grayscale=0XF0;
			if (dying) dead=true;
			neighbors=grid(X,Y);
			if (neighbors<2) dying=true;
			if (neighbors>3) dying=true;
			if (neighbors==3) dying=false;
			if (!dead) grid["live"]++; 
			if (dead) grid["dead"]++; 
			if (dying) grid["dying"]++; 
			return remove;
		}
		virtual void operator()(Pixmap& bitmap,const int CW,const int CH) 
		{
			unsigned long color((grayscale+0XFF)<<8);
			if (dying) if (!dead) color=(grayscale<<8)|(grayscale);
			if (!dying) if (grayscale>(0XFF>>1)) color=(grayscale<<16)|(grayscale<<8)|(grayscale);
			if (dying) {if (grayscale>4) grayscale-=4; else grayscale=0;}
			else {if (grayscale<250) grayscale+=4; else grayscale=255;}
			if (age>100) color=((grayscale<<16)|(grayscale<<8));
			if (age<10) if (dying) color=((grayscale<<2)<<16);
			if (dead) color=0X3333;
			grid(color,bitmap,X,Y,CW,CH);	
			if (dead) remove=true;
		}
		bool Alive() 
		{ 
			if ((dead) or (dying)) return false;
			return true;
		}
	};

	struct Column : map<int,Cell>
	{
		Column(GridBase& _grid,const int _position,const int _height) : grid(_grid),X(_position),height(_height) {}
		void Birth(const int x,const int y)
		{
			if (find(y)!=end()) return;
			if (grid.limits(x,y)) return;
			grid.births++;
			insert(make_pair<int,Cell>(y,Cell(grid,x,y,false)));
			iterator it(find(y));
			if (it==end()) throw runtime_error("Cannot create column");
		}
		bool Alive(Point& p)
		{
			iterator it(find(p.second));
			if (it==end()) return false;
			return it->second.Alive();
		}
		bool update()
		{
			if (empty()) return true;
			for (iterator it=begin();it!=end();it++) 
				if (it->second.update()) erase(it);
			if (empty()) return true;
			return false;
		}
		virtual void operator()(Pixmap& bitmap,const int CW,const int CH) 
			{ for (iterator it=begin();it!=end();it++) it->second(bitmap,CW,CH); }
		private:
		GridBase& grid;
		const int X,height;
	};

	struct Row : map<int,Column>
	{
		Row(GridBase& _grid,const int _width,const int _height) : grid(_grid), width(_width), height(_height) {}
		void update()
		{
			grid.clear();
			grid["about to die"]=0;
			grid["live"]=0;
			grid["dead"]=0;
			grid["dying"]=0;
			if (empty()) seed();
			for (iterator it=begin();it!=end();it++) 
				if (it->second.update()) erase(it);
		}
		virtual void operator()(Pixmap& bitmap,const int CW,const int CH) 
			{ for (iterator it=begin();it!=end();it++) it->second(bitmap,CW,CH); }
		bool Alive(Point& p)
		{
			iterator it(find(p.first));
			if (it==end()) return false;
			return it->second.Alive(p);
		}
		protected:
		void Birth(const int x,const int y)
		{
			if (x<0) return;
			if (y<0) return;
			if (x>width) return;
			if (y>height) return;
			if (find(x)==end()) insert(make_pair<int,Column>(x,Column(grid,x,height)));
			iterator it(find(x));
			if (it==end()) throw runtime_error("Cannot create row");
			it->second.Birth(x,y);
		}

		virtual void seed()
		{
			srand(time(0));
			for (int y=0;y<4;y++)
			{
				const int cx((width/8)+(rand()%width/4));
				const int cy((height/8)+(rand()%width/4));
				if (false)
				{
					Birth(cx,cy);
					Birth(cx+1,cy);
					Birth(cx+1,cy+1);
					Birth(cx,cy+1);
					return;
				}
				for (int j=0;j<((rand()%3)+1);j++)
					for (int i=0;i<((rand()%3)+1);i++)
						Birth((rand()%10)+cx,(rand()%10)+cy);
			}
		}
		private:
		GridBase& grid;
		protected:
		const int width, height;
	};

	struct Grid : Canvas, Row, GridBase
	{
		Grid(Display* _display,GC& _gc,const int _ScreenWidth, const int _ScreenHeight)
			: Canvas(_display,_gc,_ScreenWidth,_ScreenHeight), Row(*this,_ScreenWidth/10,_ScreenWidth/10), CW(_ScreenWidth/100), CH(_ScreenHeight/100),
				updaterate(40),updateloop(0),birthrate(0),endoflife(10000), populationcontrol(0)  {}
		protected:
		virtual void seed()
		{
			invalid.insert(Rect(0,0,ScreenWidth,ScreenHeight));	
			updateloop=1;
			births=birthrate=0;
			Row::seed();
		}
		struct BirthingPool : map<Point,int> { } birthingpool;
		virtual void update() 
		{
			if (births) birthrate=((double)births/(double)updateloop);
			if ((++updateloop)%updaterate) return;
			if (updateloop>2000) if (birthrate<0.1) Row::clear(); // this rule will reset the game if the birth rate is too low
			birthingpool.clear();
			Row::update();
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
					if (populationcontrol<0.5) Row::clear();
					if (birthrate>populationcontrol) return;
				}
			}
			for (BirthingPool::iterator bit=birthingpool.begin();bit!=birthingpool.end();bit++)
				if (bit->second==3) {Birth(bit->first.first,bit->first.second);}
		}
		virtual void operator()(Pixmap& bitmap) 
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
			invalid.insert(Rect(10,100,ScreenWidth-160,140));	
			Row::operator()(bitmap,CW,CH); 
		}
		const int CW,CH,updaterate;
		unsigned long updateloop;
		double birthrate;
		Rect paint;
		const double endoflife;
		double populationcontrol;
		virtual void operator()(const unsigned long color,Pixmap&  bitmap,const int x,const int y, const int CW,const int CH) 
		{
			XPoint points[4]; 
			const int X(x*CW);
			const int Y(y*CH);
			points[0].x=X+1;
			points[0].y=Y+1;
			points[1].x=X+CW-2;
			points[1].y=Y+1;
			points[2].x=X+CW-2;
			points[2].y=Y+CH-2;
			points[3].x=X+1;
			points[3].y=Y+CH-2;
			XSetForeground(display,gc,color);
			XFillPolygon(display,bitmap,  gc,points, 4, Complex, CoordModeOrigin);
			invalid.insert(Rect(X+1,Y+1,X+CW-2,Y+CH-2));	
		}
		private:
		virtual int operator()(const int x,const int y) 
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
		virtual bool limits(const int x,const int y) 
		{
			const int margin(200);
			const int X(x*CW);
			const int Y(y*CH);
			if (X<margin) return true;
			if (Y<margin) return true;
			if (X>(ScreenWidth-margin)) return true;
			if (Y>(ScreenHeight-margin)) return true;	
			return false;
		}
	};
} // Life
#endif  //__LIFE_H__


