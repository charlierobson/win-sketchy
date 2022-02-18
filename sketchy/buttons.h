#pragma once

class button {
protected:
	int _width;
	int _height;

	class dfile* _dfile;

public:
	bool _highlighted;
	bool _selected;

	olc::vi2d _topLeft;
	olc::vi2d _bottomRight;

public:
	button(int x, int y, int w, int h, class dfile* dfile);

	virtual void select(bool selected);

	virtual bool checkSelect(olc::vi2d& selectionPos, olc::HWButton& selector);

	virtual void highlight(olc::PixelGameEngine* pge);

	virtual void draw(olc::PixelGameEngine* pge);
};


class charsetButton : public button {
protected:
	std::function<void(int)> _onSelect;

public:
	int _char;

public:
	charsetButton(int x, int y, class dfile* dfile, int c, std::function<void(int)> onSelect);

	void draw(olc::PixelGameEngine* pge) override;
	void select(bool selected) override;
};



class lgModeButton : public charsetButton {
public:
	lgModeButton(int x, int y, dfile* dfile, int c, std::function<void(int)> onSelect);

	void select(bool selected) override;
};


class pageButton : public charsetButton {
private:
	std::function<bool(int)> _getSelected;

public:
	pageButton(int x, int y, dfile* dfile, int c, std::function<void(int)> onSelect, std::function<bool(int)> getSelected);

	void draw(olc::PixelGameEngine* pge) override;
};



class textButton : public button {
private:
	std::string _text;
	std::function<void()> _onSelect;
	bool _shouldSelect;

public:
	textButton(int x, int y, class dfile* dfile, std::string text, std::function<void()> onSelect, bool shouldSelect = true);

	void draw(olc::PixelGameEngine* pge) override;
	void select(bool selected) override;
};
