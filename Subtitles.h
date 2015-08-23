#pragma once

class Window;

class Subtitles
{
public:
	Subtitles(Window& wnd);
	~Subtitles(void);
	void start(unsigned long offset = 0);
	Subtitle* getCurrentSubtitle(DWORD el = 0);
	Subtitle* getNextSubtitle(DWORD el = 0);
	Subtitle* getPreviousSubtitle(DWORD el = 0);
	void goTo(Subtitle* sub);
	void addDelay(int ms);
	void timerCallback();
	void pause();
	void nextSubtitle();
	bool load(std::wistream& stream);
	bool isPaused();
	void adjustWindow();

private:
	static VOID CALLBACK timerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
	void startTimer(DWORD time);
	void stopTimer();
	std::set<Subtitle*, SubtitleComp> subtitles;
	DWORD startTime;
	std::wstring largestLine;
	int maxHeight;
	HANDLE timer;
	Window& window;
	Subtitle defaultSubtitle;
	DWORD pauseTimer;
	int delay;
	bool justStarted;
};

