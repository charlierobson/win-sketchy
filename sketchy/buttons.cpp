#include "olcPixelGameEngine.h"

#include "buttons.h"
#include "dfile.h"

button::button(int x, int y, int w, int h, dfile* dfile) :
	_dfile(dfile)
{
	_topLeft = olc::vi2d(x, y);
	_bottomRight = olc::vi2d(x + w, y + h);
	_width = w;
	_height = h;
	_highlighted = false;
	_selected = false;
}

bool button::checkSelect(olc::vi2d& selectionPos, olc::HWButton& selector) {
	bool retval = false;
	if (selectionPos.x >= _topLeft.x &&
		selectionPos.x < _bottomRight.x &&
		selectionPos.y >= _topLeft.y &&
		selectionPos.y < _bottomRight.y) {

		if (!_highlighted && !_selected) {
			_highlighted = true;
		}

		if (selector.bPressed) {
			retval = true;
		}
	}
	else {
		_highlighted = false;
	}

	return retval;
}

void button::select(bool selected) {
	_selected = selected;
}

void button::highlight(olc::PixelGameEngine* pge) {
	if (!_selected && !_highlighted) return;
	pge->DrawRect(_topLeft.x - 1, _topLeft.y - 1, _width + 1, _height + 1, _selected ? olc::Pixel(0xFF00D000) : olc::Pixel(0xFF00FF00));
}

void button::draw(olc::PixelGameEngine* pge) {
	highlight(pge);
}




charsetButton::charsetButton(int x, int y, dfile* dfile, int c, std::function<void(int)> onSelect) :
	button(x, y, 8, 8, dfile),
	_char(c),
	_onSelect(onSelect)
{
	_onSelect = onSelect;
}

void charsetButton::draw(olc::PixelGameEngine* pge) {
	auto c = _char;
	if (c > 128) {
		c -= 64;
	}

	pge->DrawPartialSprite(_topLeft, _dfile->_charSet, olc::vi2d(0, c * 8), olc::vi2d(8, 8));
	button::draw(pge);
}

void charsetButton::select(bool selected) {
	auto wasSelected = _selected;
	button::select(selected);
	if (selected && !wasSelected) {
		auto c = _char;
		if (c > 63) c += 64;

		if (_onSelect) {
			_onSelect(_char);
		}
	}
}



pageButton::pageButton(int x, int y, dfile* dfile, int c, std::function<void(int)> onSelect, std::function<bool(int)> getSelected) :
	charsetButton(x, y, dfile, c, onSelect),
	_getSelected(getSelected)
{
}

void pageButton::draw(olc::PixelGameEngine* pge) {
	_selected = _getSelected(_char);
	charsetButton::draw(pge);
}



lgModeButton::lgModeButton(int x, int y, dfile* dfile, int c, std::function<void(int)> onSelect) :
	charsetButton(x, y, dfile, c, onSelect) {
}

void lgModeButton::select(bool selected) {
	auto wasSelected = _selected;
	button::select(selected);
	if (selected && !wasSelected) {
		if (_onSelect) {
			_onSelect(_char);
		}
	}
	else if (selected && wasSelected) {
		if (_char == 0xB1) {
			_char = 0xAC;
		}
		else {
			_char = 0xB1;
		}
		_onSelect(_char);
	}
}



textButton::textButton(int x, int y, dfile* dfile, std::string text, std::function<void()> onSelect, bool shouldSelect) :
	button(x, y, text.size() * 8, 8, dfile),
	_text(text),
	_onSelect(onSelect),
	_shouldSelect(shouldSelect)
{
}

void textButton::draw(olc::PixelGameEngine* pge) {
	for (size_t i = 0; i < _text.size(); ++i) {
		pge->DrawPartialSprite(_topLeft.x + i * 8, _topLeft.y, _dfile->_charSet, 0, dfile::ascii2zeddy(_text.c_str()[i]) * 8, 8, 8);
	}
	button::draw(pge);
}

void textButton::select(bool selected) {
	if (_shouldSelect){
		button::select(selected);
	}
	if (selected) {
		if (_onSelect) {
			_onSelect();
		}
	}
}
