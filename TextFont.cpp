#include "stdafx.h"
#include "TextFont.h"


Gdiplus::RectF TextFont::getTextBounds(HDC hdc, RECT clientRect, const WCHAR* text) 
{
	Graphics graphics(hdc);

	StringFormat strformat;
	Gdiplus::RectF layoutRect(0, 0, (Gdiplus::REAL)(clientRect.right - clientRect.left), (Gdiplus::REAL)(clientRect.bottom - clientRect.top));

	Gdiplus::RectF boundRect;
	Gdiplus::CharacterRange charRanges[1] = { Gdiplus::CharacterRange(0, wcslen(text))};

	strformat.SetMeasurableCharacterRanges(1, charRanges);

    Gdiplus::Region regions[1];

	graphics.MeasureCharacterRanges(text, -1, (Font*)this, layoutRect, &strformat, 1, regions);

	regions[0].GetBounds(&boundRect, &graphics);

	return boundRect;
}
