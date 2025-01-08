#ifndef ATHELETEINFO_H
#define ATHELETEINFO_H

using namespace std;

class atheleteInfo
{
private:
	int atheleteNum;
	string atheleteName;
public:
	atheleteInfo(int num, string name) : atheleteNum(num), atheleteName(name) {}
	void setAtheleteNum(int num) { atheleteNum = num; }
	void setAtheleteName(string name) { atheleteName = name; }
	int getAtheleteNum() { return atheleteNum; }
	string getAtheleteName() { return atheleteName; }
};

#endif 