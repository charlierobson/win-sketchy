#pragma once
class sketchyIf {
public:
	virtual void setMode(int) = 0;
	virtual int getMode() = 0;

	virtual void setCurChar(int) = 0;
	virtual int getCurChar() = 0;

	virtual class dfile* getDFile() = 0;

	virtual struct copyBuffer& getCopyBuffer() = 0;

	virtual struct olc::Pixel getSelectColour() = 0;

	virtual void ClickButton(std::string buttonName) = 0;
};
