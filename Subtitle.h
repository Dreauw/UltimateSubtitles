#pragma once

class Subtitle
{
public:
	Subtitle();
	Subtitle(unsigned long startTime);
	Subtitle(unsigned long startTime, unsigned long duration);
	~Subtitle(void);
	unsigned long getStartTime() const;
	std::vector<std::wstring>& getLines();
	unsigned long getDuration() const;
	bool operator<(const Subtitle* sub) const;
	void addLine(std::wstring str);
	int getNumberLines();
	bool readed;

private:
	unsigned long startTime;
	std::vector<std::wstring> lines;
	unsigned long duration;
};

struct SubtitleComp
{
  bool operator()(const Subtitle* a, const Subtitle* b) const  
  { 
	  return a->getStartTime() < b->getStartTime();
  }
};