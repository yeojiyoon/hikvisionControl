#include "atheleteInfo.h"
#include <iostream>

using namespace std;

class atheleteInfo
{
private:
	int atheleteNum;
	string atheleteName;
public:
	atheleteInfo(int num, string name)
	{
		this->atheleteNum = num;
		this->atheleteName = name;
	}
	void setAtheleteNum(int num) 
	{ 
		this->atheleteNum = num; 
	}
	void setAtheleteName(string name) 
	{ 
		this->atheleteName = name; 
	}
	int getAtheleteNum() 
	{ 
		return atheleteNum; 
	}
	string getAtheleteName() 
	{ 
		return atheleteName; 
	}
};