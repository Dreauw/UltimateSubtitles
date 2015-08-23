#include "stdafx.h"

#include "TextFont.h"
#include "Subtitle.h"
#include "Subtitles.h"
#include "Window.h"
#include "SearchBox.h"

Subtitles::Subtitles(Window& wnd) :
	window(wnd)
{
	maxHeight = 0;
	pauseTimer = 0;
	delay = 0;
}

bool Subtitles::load(std::wistream& stream) 
{
	std::wstring line;
	maxHeight = 0;
	pauseTimer = 0;
	delay = 0;
	subtitles.clear();

	while (std::getline(stream, line))
	{
		// If it's a line with time
		if (line.find(L"-->") != std::string::npos) {
			unsigned sHour = 0, sMin = 0, sSec = 0, sMill = 0, eHour = 0, eMin = 0, eSec = 0, eMill = 0;    
			int res = swscanf_s(line.c_str(), L"%2u:%2u:%2u,%3u --> %2u:%2u:%2u,%3u", &sHour, &sMin, &sSec, &sMill, &eHour, &eMin, &eSec, &eMill);
			unsigned long startTime = ((sHour * 60 + sMin) * 60 + sSec) * 1000 + sMill;
			unsigned long endTime = ((eHour * 60 + eMin) * 60 + eSec) * 1000 + eMill;


			if (res != 8 || startTime >= endTime) {
				print("Unable to read this file (error when parsing timecodes)");
				return false;
			}

			Subtitle* startSubtitle = new Subtitle(startTime, endTime - startTime);

			// Add the lines
			while (std::getline(stream, line) && !line.empty()) {
				// Remove HTML tags
				size_t pos1 = std::wstring::npos;
				while ((pos1 = line.find_first_of(L'<')) != std::wstring::npos) {
					size_t pos2 = line.find_first_of(L'>');
					if (pos2 == std::wstring::npos) break;
					line.erase(pos1, pos2+1-pos1);
				}
				if (line.length() > largestLine.length()) largestLine = line;
				startSubtitle->addLine(line);
			}

			if (startSubtitle->getNumberLines() > maxHeight) maxHeight = startSubtitle->getNumberLines();

			subtitles.insert(startSubtitle);
		}
		
	}

	if (subtitles.size() == 0) {
		return false;
	}

	
	startTime = GetTickCount();
	pause();
	window.updateSubtitles(defaultSubtitle.getLines());
	return true;
}

void Subtitles::adjustWindow() 
{
	if (subtitles.size() <= 0) return;
	window.adjust((wchar_t*)largestLine.c_str(), maxHeight);
	BringWindowToTop(window.getWnd());
}

void Subtitles::start(unsigned long offset)
{
	if (subtitles.size() <= 0) return;

	// Stop the current timer
	stopTimer();

	// Recalculate the start time
	startTime = GetTickCount() - offset;

	Subtitle* sub = getCurrentSubtitle();
	if (sub->getDuration() == 0) sub = *subtitles.begin();

	justStarted = true;

	if (offset >= sub->getStartTime())
		timerCallback();
	else
		startTimer(sub->getStartTime() - offset);
	
}

void Subtitles::pause() 
{
	if (subtitles.size() <= 0) return;

	if (pauseTimer > 0) 
	{
		// Unpause
		window.setInformation(L"");
		start(pauseTimer);
		pauseTimer = 0;
		adjustWindow();
		return;
	}
	// Pause
	window.setInformation(L"(Paused)");
	stopTimer();
	pauseTimer = GetTickCount() - startTime;
	if (pauseTimer == 0) pauseTimer = 1;
}


Subtitle* Subtitles::getPreviousSubtitle(DWORD el)
{
	DWORD elapsed = el > 0 ? el : GetTickCount() - startTime;
	if (pauseTimer > 0) elapsed = pauseTimer;
	Subtitle cSub(elapsed);
	std::set<Subtitle*>::iterator it = subtitles.upper_bound(&cSub);
	if (it == subtitles.begin() || it == subtitles.end()) return &defaultSubtitle;
	it = --it;
	if (it == subtitles.begin()) return &defaultSubtitle;

	return (Subtitle*)*(--it);
}


Subtitle* Subtitles::getCurrentSubtitle(DWORD el)
{
	DWORD elapsed = el > 0 ? el : GetTickCount() - startTime;
	if (pauseTimer > 0) elapsed = pauseTimer;
	Subtitle cSub(elapsed);
	std::set<Subtitle*>::iterator it = subtitles.upper_bound(&cSub);
	if (it == subtitles.begin()) return &defaultSubtitle;

	return (Subtitle*)*(--it);
}

Subtitle* Subtitles::getNextSubtitle(DWORD el)
{
	DWORD elapsed = el > 0 ? el : GetTickCount() - startTime;
	if (pauseTimer > 0) elapsed = pauseTimer;
	Subtitle cSub(elapsed);
	std::set<Subtitle*>::iterator it = subtitles.upper_bound(&cSub);
	if (it == subtitles.end()) return &defaultSubtitle;

	return (Subtitle*)*(it);
}

void Subtitles::goTo(Subtitle* sub)
{
	if (subtitles.size() <= 0) return;

	if (pauseTimer > 0) {
		pauseTimer = sub->getStartTime();
		if (pauseTimer == 0) pauseTimer = 1;
		window.updateSubtitles(sub->getLines());
	} else {
		stopTimer();
		start(sub->getStartTime());
	}
}

void Subtitles::addDelay(int ms) 
{
	if (subtitles.size() <= 0) return;

	delay += ms;
	if (pauseTimer > 0) {
		window.setInformation(L"(Paused " + std::to_wstring(delay) + L"ms)");
		pauseTimer += ms;
	} else {
		window.setInformation(L"(" + std::to_wstring(delay) + L"ms)");
		start(GetTickCount() - startTime + ms);
	}
}

void Subtitles::startTimer(DWORD time) 
{
	CreateTimerQueueTimer(&timer, NULL, timerCallback, this, time, 0, WT_EXECUTEONLYONCE);
}


void Subtitles::stopTimer() 
{
	if (timer != NULL) 
	{
		DeleteTimerQueueTimer(NULL, timer, NULL);
		timer = NULL;
	}
}

bool Subtitles::isPaused()
{
	return pauseTimer > 0 || subtitles.size() <= 0;
}



Subtitles::~Subtitles(void)
{
	stopTimer();
	std::set<Subtitle*>::iterator it;
	for (it = subtitles.begin(); it != subtitles.end(); ++it)
	{
		delete *it;
	}
}

void Subtitles::timerCallback()
{
	if (justStarted) justStarted = false;
	else window.setInformation(L"");
	
	
	DWORD elapsed = GetTickCount() - startTime;
	Subtitle* sub = getCurrentSubtitle(elapsed);

	// If the timer is in the range of a subtitle
	if (elapsed >= sub->getStartTime() && elapsed < sub->getStartTime()+sub->getDuration()) {
		sub->readed = true;
		// Show the subtitles
		window.updateSubtitles(sub->getLines());
		startTimer(sub->getStartTime()+sub->getDuration() - elapsed);
	} else {
		// No subtitles
		window.updateSubtitles(defaultSubtitle.getLines());
		startTimer(getNextSubtitle(elapsed)->getStartTime() - elapsed);
	}
}

VOID CALLBACK Subtitles::timerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	((Subtitles*)lpParameter)->timerCallback();
}

