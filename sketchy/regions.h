#pragma once
#include <vector>

class region
{
protected:
	olc::vi2d _topLeft;
	olc::vi2d _bottomRight;
	bool _wasWithin = false;

public:
	region() {}

	region(int tlx, int tly, int brx, int bry) {
		_topLeft.x = tlx;
		_topLeft.y = tly;
		_bottomRight.x = brx;
		_bottomRight.y = bry;
	}

	void extend(olc::vi2d topLeft, olc::vi2d bottomRight) {
		if (topLeft.x < _topLeft.x) _topLeft.x = topLeft.x;
		if (topLeft.y < _topLeft.y) _topLeft.y = topLeft.y;
		if (bottomRight.x > _bottomRight.x) _bottomRight.x = bottomRight.x;
		if (bottomRight.y > _bottomRight.y) _bottomRight.y = bottomRight.y;
	}

	bool isWithin(int x, int y) {
		auto within = x >= _topLeft.x &&
			x < _bottomRight.x &&
			y >= _topLeft.y &&
			y < _bottomRight.y;

		if (within && !_wasWithin) {
			onMouseEnter();
		}
		else if (!within && _wasWithin) {
			onMouseLeave();
		}
		_wasWithin = within;
		return within;
	}

	virtual void onMouseEnter() {}
	virtual void onMouseLeave() {}

	virtual void update(olc::PixelGameEngine* pge) {}
	virtual void draw(olc::PixelGameEngine* pge) {}
};


class buttonRegion : public region
{
private:
	class button* _currentSelection;

public:
	std::vector<class button*> _buttons;

public:
	buttonRegion();

	void add(class button* button);

	void update(olc::PixelGameEngine* pge) override;
	void draw(olc::PixelGameEngine* pge) override;

	void onMouseLeave() override;

	void select(class button* button);
};



class dfileRegion : public region
{
private:
	class sketchyIf* _core;

public:
	dfileRegion(class sketchyIf*);

	void update(olc::PixelGameEngine* pge) override;
	void draw(olc::PixelGameEngine* pge) override;

	void onMouseLeave() override;
};
