#pragma once
#include "olcPixelGameEngine.h"


struct copyBuffer
{
	olc::vi2d pos;
	int w, h;
	std::vector<int> data;
};


class dfile
{
private:
	std::vector<int> _dfile;

	void doPlot(int, int, int);
	bool handleSpecial(int cha);

	olc::vi2d _selStart;
	olc::vi2d _selEnd;

public:
	olc::Sprite* _charSet;

public:
	dfile(olc::Sprite* charSet);

	void draw(olc::PixelGameEngine*);

	void cls();
		
	void poke(int x, int y, int c);
	int peek(int x, int y);

	void plot(int x, int y);
	void unplot(int x, int y);

	void setSelectRect(olc::vi2d start, olc::vi2d end) {
		_selStart = start;
		_selEnd = end;
	}

	void getSelectRect(olc::vi2d& start, olc::vi2d& end) {
		start = _selStart;
		end = _selEnd;
	}

	void getSelectRectNormal(olc::vi2d& start, olc::vi2d& end) {
		start.x = std::min(_selStart.x, _selEnd.x);
		start.y = std::min(_selStart.y, _selEnd.y);
		end.x = std::max(_selStart.x, _selEnd.x);
		end.y = std::max(_selStart.y, _selEnd.y);
	}

	static int ascii2zeddy(int c);

	void fill(int c);

	void load(const std::string& filename);
	void save(const std::string& filename);

	void cursorRight();
	void cursorDown();
	void cursorUp();
	void cursorLeft();
	void insert(bool);
	void del(bool);

	void rst8a(int);
	void rst88a(int);

	copyBuffer copy();
	void paste(copyBuffer&);
	void draw(olc::PixelGameEngine* pge, copyBuffer& cb);
};
