#pragma once
using namespace Gdiplus;

class TextFont : public Gdiplus::Font
{
public:
	TextFont(wchar_t* family, Gdiplus::REAL emSize, Gdiplus::FontStyle style) : Font(family, emSize, style) {}

	Gdiplus::RectF getTextBounds(HDC hdc, RECT clientRect, const WCHAR* text);
};

