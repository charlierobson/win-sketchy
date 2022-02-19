#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "sketchyIf.h"

#include "buttons.h"
#include "regions.h"
#include "dfile.h"

#include "3dm.h"

class sketchy : public olc::PixelGameEngine, sketchyIf
{
private:
	std::vector<region*> _regions;

	std::map<std::string, std::pair<buttonRegion*, button*>> _clickables;

	std::vector<dfile*> _dfiles;
	int _dfilePage = 0;

	int _mode = 0;
	int _curChar = 0;

	copyBuffer _copyBuffer;

	olc::Pixel _selectColour = olc::DARK_YELLOW;

	dfile* _dfile;

private:
	void DoFileOp(std::function<void(LPOPENFILENAME)> action) {
		OPENFILENAME ofn;
		char filename[MAX_PATH];

		ZeroMemory(&ofn, sizeof(ofn));
		ZeroMemory(&filename, sizeof(filename));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = "Text Files\0*.txt\0Any File\0*.*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = "S81ect a File";
		ofn.Flags = OFN_DONTADDTORECENT;

		action(&ofn);
	}

	void DoCopy() {
		_copyBuffer = _dfile->copy();
	}

public:
	sketchy()
	{
		sAppName = "Sketchy ZX81 screen editor V1.31";
	}

	void setMode(int mode) {
		_mode = mode;
	}

	int getMode() {
		return _mode;
	}

	void setCurChar(int curr) {
		_curChar = curr;
	}

	int getCurChar() {
		return _curChar;
	}

	void setPage(int p) {
		_dfilePage = p;
		_dfile = _dfiles[_dfilePage];
	}

	dfile* getDFile() {
		return _dfile;
	}

	copyBuffer& getCopyBuffer() {
		return _copyBuffer;
	}

	void ClickButton(std::string buttonName) override {
		std::pair<buttonRegion*, button*> regionalWineLady = _clickables[buttonName];
		if (regionalWineLady.first != nullptr)
			regionalWineLady.first->select(regionalWineLady.second);
	}


public:
	void OnDropFile(const std::string& filename) override
	{
		_dfile->load(filename);
	}

	bool OnUserCreate() override
	{
		auto sprite = new olc::Sprite("zx81font.png");

		for (int i = 0; i < 4; ++i) {
			_dfiles.push_back(new dfile(sprite));
		}

		_dfile = _dfiles[0];

		int basex = 32 * 8 + 16;

		auto modeButtons = new buttonRegion();
		auto charButton = new textButton(basex, 192 - 20, _dfile, "CHAR", [this]() {setMode(0); });
		modeButtons->add(charButton);
		_clickables["char"] = std::pair<buttonRegion*, button*>(modeButtons, charButton);
		modeButtons->select(charButton);
		modeButtons->add(new textButton(basex + 40, 192 - 20, _dfile, "BLOCK", [this]() {setMode(1); }));
		auto selectButton = new textButton(basex, 192 - 8, _dfile, "SELECT", [this]() {setMode(2); });
		modeButtons->add(selectButton);
		_clickables["select"] = std::pair<buttonRegion*, button*>(modeButtons, selectButton);
		modeButtons->add(new lgModeButton(basex + 56, 192 - 8, _dfile, 0xB1, [this](int c)
			{
				setMode(c == 0xB1 ? 3 : 4);
				olc::vi2d tl, br;
				_dfile->getSelectRect(tl, br);
				_dfile->setSelectRect(tl, tl);
			}));

		auto pasteButton = new textButton(basex, 192 + 4, _dfile, "PASTE", [this]() {
			setMode(5);
			_copyBuffer.pos = olc::vi2d(0, 0);
			});
		modeButtons->add(pasteButton);
		_clickables["paste"] = std::pair<buttonRegion*, button*>(modeButtons, pasteButton);

		_regions.push_back(modeButtons);

		auto characterButtons = new buttonRegion();
		for (int i = 0; i < 128; ++i) {
			int dx = (i & 7) * 10;
			int dy = (i / 8) * 10;

			auto b = new charsetButton(basex + dx, 8 + dy, _dfile, i,
				[this](int c) {
					if (getMode() == 1) {
						ClickButton("char");
					}
					setCurChar(c >= 0x40 ? c + 64 : c);
				});

			characterButtons->add(b);

			if (i == 64) {
				characterButtons->select(b);
			}
		}
		_regions.push_back(characterButtons);

		_regions.push_back(new dfileRegion(this));

		auto workButtons = new buttonRegion();
		workButtons->add(new textButton(8, 204, _dfile, "CLS", [this]() {
				_dfile->cls();
			}, false));

		workButtons->add(new textButton(8 + 4 * 8, 204, _dfile, "FILL", [this]() {
				_dfile->fill(getCurChar());
			}, false));

		workButtons->add(new textButton(8 + 9 * 8, 204, _dfile, "LOAD", [this]() {
				DoFileOp([this](LPOPENFILENAMEA ofn) {
					if (GetOpenFileNameA(ofn)) {
						_dfile->load(ofn->lpstrFile);
					}
				});
			}, false));

		workButtons->add(new textButton(8 + 14 * 8, 204, _dfile, "SAVE", [this]() {
				DoFileOp([this](LPOPENFILENAMEA ofn) {
					if (GetSaveFileNameA(ofn)) {
						_dfile->save(ofn->lpstrFile);
					}
				} );
			}, false));

		workButtons->add(new textButton(8 + 19 * 8, 204, _dfile, "COPY", [this]() {
				DoCopy();
			}, false));

		_regions.push_back(workButtons);

		auto pageButtons = new buttonRegion();
		for (int i = 0; i < 4; ++i) {
			auto b = new pageButton(8 + (21 + i) * 10, 204, _dfile, 0x1d + i,
				[this](int c) {
					setPage(c - 0x1d);
				},
				[this](int c)->bool {
					return _dfilePage == c - 0x1d;
				} );
			pageButtons->add(b);

			if (i == 0) {
				pageButtons->select(b);
			}
		};

		_regions.push_back(pageButtons);

		_copyBuffer.pos = olc::vi2d(0, 0);
		_copyBuffer.w = 32;
		_copyBuffer.h = 24;
		_copyBuffer.data.assign(std::begin(threedeeemem), std::end(threedeeemem));

		return true;
	}

	struct olc::Pixel getSelectColour() {
		return _selectColour;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		static float totalTime;
		totalTime += fElapsedTime;

		_selectColour = ((int)(totalTime * 2) & 1) == 0 ? olc::Pixel(0xFF2266FF) : olc::Pixel(0xFF0011CC);

		Clear(olc::Pixel(0xFF808080));

		olc::vi2d mousePos(GetMouseX(), GetMouseY());
		olc::HWButton lButton = GetMouse(0);
		olc::HWButton rButton = GetMouse(1);

		if (GetKey(olc::Key::UP).bPressed) _dfile->cursorUp();
		if (GetKey(olc::Key::DOWN).bPressed) _dfile->cursorDown();
		if (GetKey(olc::Key::LEFT).bPressed) _dfile->cursorLeft();
		if (GetKey(olc::Key::RIGHT).bPressed) _dfile->cursorRight();
		if (GetKey(olc::Key::INS).bPressed) _dfile->insert(getMode() == 4);
		if (GetKey(olc::Key::DEL).bPressed) _dfile->del(getMode() == 4);
		if (GetKey(olc::Key::F1).bPressed) setPage(0);
		if (GetKey(olc::Key::F2).bPressed) setPage(1);
		if (GetKey(olc::Key::F3).bPressed) setPage(2);
		if (GetKey(olc::Key::F4).bPressed) setPage(3);

		for (auto region : _regions) {
			if (region->isWithin(mousePos.x, mousePos.y)) {
				region->update(this);
			}
			region->draw(this);
		}

		return true;
	}

	void OnKeyDown(int key) override
	{
 		if (getMode() == 3) {
			_dfile->rst8a(key);
		}
		else if (getMode() == 4) {
			_dfile->rst88a(key);
		}
	}
};


int main()
{
	sketchy demo;
	if (demo.Construct(364, 224, 2, 2))
		demo.Start();

	return 0;
};
