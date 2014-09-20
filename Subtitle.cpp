#include "stdafx.h"

#include "Subtitle.h"

Subtitle::Subtitle()
{
	startTime = 0;
	duration = 0;
	readed = false;
}

Subtitle::Subtitle(unsigned long time)
{
	startTime = time;
	duration = 0;
}

Subtitle::Subtitle(unsigned long time, unsigned long durat)
{
	startTime = time;
	duration = durat;
}



unsigned long Subtitle::getStartTime() const
{
	return startTime;
}

unsigned long Subtitle::getDuration() const
{
	return duration;
}

std::vector<std::wstring>& Subtitle::getLines() 
{
	return lines;
}

bool Subtitle::operator<(const Subtitle* sub) const
{
   return startTime < sub->startTime;
}

void Subtitle::addLine(std::wstring str)
{
	lines.push_back(str);
}

int Subtitle::getNumberLines() 
{
	return lines.size();
}


Subtitle::~Subtitle(void)
{
}