#include "olcPixelGameEngine.h"
#include "dfile.h"
#include <regex>
#include <iomanip>

dfile::dfile(olc::Sprite* charSet) :
    _charSet(charSet)
{
    _dfile.resize(32 * 24);
    _selStart = olc::vi2d(0, 0);
    _selEnd = olc::vi2d(31, 23);
}

int dfile::ascii2zeddy(int c) {
    const char* zeddycs = " !!!!!!!!!!\"£$:?()><=+-*/;,.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    auto idx = strrchr(zeddycs, toupper(c));
    if (idx == nullptr || *idx == '!') {
        return -1;
    }
    return idx - zeddycs;
}

void dfile::draw(olc::PixelGameEngine* pge) {
    int xo = 4;
    int yo = 4;

	for (int x = 0; x < 32; x++)
		for (int y = 0; y < 24; y++) {
            auto c = _dfile[x + 32 * y];
            if (c >= 128) {
                c -= 64;
            }
			if (c == 0)
				pge->FillRect(x * 8 + xo, y * 8 + yo, 8, 8, (x & 1) ^ (y & 1) ? olc::Pixel(0xFFF0F0F0) : olc::Pixel(0xFFE0E0E0));
			else
				pge->DrawPartialSprite(x * 8 + xo, y * 8 + yo, _charSet, 0, c * 8, 8, 8);
		}
}

void dfile::poke(int x, int y, int c) {
	_dfile[x + 32 * y] = c;
}

int dfile::peek(int x, int y) {
	return _dfile[x + 32 * y];
}

void dfile::doPlot(int x, int y, int mode) {
    auto b = (y & 1) == 0 ? 1 : 4;
    if ((x & 1) != 0) {
        b *= 2;
    }

    auto i = x / 2 + y / 2 * 32;
    auto c = _dfile[i];

    if ((c & 127) > 7) {
        c = 0;
    }
    if (c > 127) {
        c ^= 0x8f;
    }

    if (mode == 0) {
        c = ~b & c;
    }
    else if (mode == 1) {
        c |= b;
    }
    else {
        c = c ^ b;
    }

    if (c > 7) {
        c ^= 0x8f;
    }

    _dfile[i] = c;
}

void dfile::unplot(int x, int y) {
    doPlot(x, y, 0);
}

void dfile::plot(int x, int y) {
    doPlot(x, y, 1);
}

void dfile::cls() {
    std::fill(_dfile.begin(), _dfile.end(), 0);
}


void dfile::fill(int c) {

    olc::vi2d start, end;
    getSelectRectNormal(start, end);

    for (int row = start.y; row <= end.y; ++row) {
        for (int x = start.x; x <= end.x; ++x) {
            _dfile[row * 32 + x] = c;
        }
    }
}


void dfile::load(const std::string& filename) {

    auto n = 0;
    std::string end = filename.substr(filename.rfind("."));

    // bin, raw are dumps of the dfile without newlines.
    // dfile is the whole dfile including newlines.
    // because we just filter out newlines here we can treat them all equally
    //
    if (end == ".bin" || end == ".raw" || end == ".dfile") {
        std::ifstream binFile(filename, std::ios::binary);
        if (binFile.is_open()) {

            unsigned char m;
            while (!binFile.eof()) {
                binFile.read((char*)&m, 1);
                if (m != 0x76 && n < 768)
                    _dfile[n++] = m;
            }
        }
    }
    else if (end == ".txt" || end == ".asm") {
        std::ifstream txtFile(filename);
        if (txtFile.is_open()) {

            std::string line;
            std::regex reg((const char*)"\\$[0-9A-Fa-f]{2}");

            while (getline(txtFile, line)) {
                for (std::sregex_iterator it = std::sregex_iterator(line.begin(), line.end(), reg); it != std::sregex_iterator(); ++it) {
                    std::smatch match = *it;
                    auto mc = match.str().substr(1, 2);
                    auto m = std::strtol(mc.c_str(), nullptr, 16);
                    if (m != 0x76 && n < 768) {
                        _dfile[n++] = m;
                    }
                }
            }
            txtFile.close();
        }
    }
}


void dfile::save(const std::string& filename) {

    std::string end = filename.substr(filename.rfind("."));

    if (end == ".asm" || end == ".txt") {

        std::ofstream dfilefile(filename);

        if (dfilefile.is_open())
        {
            for (int line = 0; line < 24; ++line) {
                dfilefile << std::setw(8) << std::setfill('0') << "\t.byte\t$76";
                for (int x = 0; x < 32; ++x) {
                    dfilefile << ",$" << std::hex << std::setw(2) << std::setfill('0') << _dfile[x + line * 32];
                }
                dfilefile << std::endl;
            }
            dfilefile << "\t.byte\t$76";
            dfilefile.close();
        }
    }
    else if (end == ".bin" || end == ".raw" || end == ".dfile") {

        // dump of characters only, no newlines
        std::ofstream dfilefile(filename, std::ios::binary);

        if (dfilefile.is_open())
        {
            if (end == ".dfile") {
                dfilefile << (byte)0x76;
            }
            for (int line = 0; line < 24; ++line) {
                for (int x = 0; x < 32; ++x) {
                    dfilefile << (byte)_dfile[x + line * 32];
                }
                if (end == ".dfile") {
                    dfilefile << (byte)0x76;
                }
            }
            dfilefile.close();
        }
    }
}


void dfile::cursorUp() {
    _selStart.y = (_selStart.y + 24 - 1) % 24;
    _selEnd = _selStart;
}

void dfile::cursorDown() {
    _selStart.y = (_selStart.y + 1) % 24;
    _selEnd = _selStart;
}

void dfile::cursorRight() {
    _selStart.x = (_selStart.x + 1) % 32;
    _selEnd = _selStart;
}

void dfile::cursorLeft() {
    _selStart.x = (_selStart.x + 32 - 1) % 32;
    _selEnd = _selStart;
}


void dfile::insert(bool gMode) {

    auto row = _selStart.y * 32;
    auto numToShift = 31 - _selStart.x;

    for (int i = 0; i < numToShift; ++i) {
        _dfile[row + 31 - i] = _dfile[row + 30 - i];
    }

    _dfile[_selStart.x + row] = gMode ? 128 : 0;
}


void dfile::del(bool gMode) {

    auto row = _selStart.y * 32;
    auto numToShift = 31 - _selStart.x;

    for (int i = 0; i < numToShift; ++i) {
        _dfile[row + _selStart.x + i] = _dfile[row + _selStart.x + 1 + i];
    }
    _dfile[row + 31] = gMode ? 128 : 0;
}


bool dfile::handleSpecial(int cha) {

    if (8 == cha) {
        cursorLeft();
        _dfile[_selStart.x + 32 * _selStart.y] = 0;
        return true;
    }
    return false;
}


void dfile::rst8a(int cha) {

    if (handleSpecial(cha))
        return;

    auto c = ascii2zeddy(cha);
    if (c != -1) {
        _dfile[_selStart.x + 32 * _selStart.y] = c;
        cursorRight();
    }
}


void dfile::rst88a(int cha) {

    if (handleSpecial(cha))
        return;

    auto c = ascii2zeddy(cha);
    if (c != -1) {
        _dfile[_selStart.x + 32 * _selStart.y] = c + 128;
        cursorRight();
    }
}


copyBuffer dfile::copy() {

    olc::vi2d end;
    copyBuffer cb;

    getSelectRectNormal(cb.pos, end);

    cb.w = end.x - cb.pos.x + 1;
    cb.h = end.y - cb.pos.y + 1;
    cb.data.resize(cb.w * cb.h);
    for (int y = 0; y < cb.h; ++y) {
        for (int x = 0; x < cb.w; ++x) {
            cb.data[x + y * cb.w] = peek(cb.pos.x + x, cb.pos.y + y);
        }
    }
    return cb;
}

void dfile::paste(copyBuffer& cb) {

    int sxo = 0;
    int syo = 0;

    auto pos = cb.pos;

    int xc = std::min(cb.w, 32 - pos.x);
    int yc = std::min(cb.h, 24 - pos.y);

    if (pos.x < 0) {
        sxo = -pos.x;
        xc += pos.x;
        pos.x = 0;
    }
    if (pos.y < 0) {
        syo = -pos.y;
        yc += pos.y;
        pos.y = 0;
    }

    for (int x = 0; x < xc; x++)
        for (int y = 0; y < yc; y++) {
            _dfile[pos.x + x + (pos.y + y) * 32] = cb.data[x + sxo + (y + syo) * cb.w];
        }
}


void dfile::draw(olc::PixelGameEngine* pge, copyBuffer& cb) {

    int xo = 4;
    int yo = 4;
    int sxo = 0;
    int syo = 0;

    auto pos = cb.pos;

    int xc = std::min(cb.w, 32 - pos.x);
    int yc = std::min(cb.h, 24 - pos.y);

    if (pos.x < 0) {
        sxo = -pos.x;
        xc += pos.x;
        pos.x = 0;
    }
    if (pos.y < 0) {
        syo = -pos.y;
        yc += pos.y;
        pos.y = 0;
    }

    for (int x = 0; x < xc; x++)
        for (int y = 0; y < yc; y++) {
            auto c = cb.data[x + sxo + (syo + y) * cb.w];
            if (c >= 128) {
                c -= 64;
            }
            pge->DrawPartialSprite((x + pos.x) * 8 + xo, (y + pos.y) * 8 + yo, _charSet, 0, c * 8, 8, 8);
        }

    pge->DrawRect(pos.x * 8 + xo, pos.y * 8 + yo, xc * 8, yc * 8, olc::RED);
}