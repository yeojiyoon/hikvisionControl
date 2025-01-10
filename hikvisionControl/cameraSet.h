#include <stdio.h>
#include <iostream>
#include <cstring>
#include "Windows.h"
#include "HCNetSDK.h"
#include <time.h>
#pragma comment(lib, "HCNetSDK.lib")
#pragma warning(disable:4996)

using namespace std;

class cameraSet
{
private:
	NET_DVR_USER_LOGIN_INFO loginInfo;
	NET_DVR_DEVICEINFO_V40 deviceInfo;
	LONG lUserID;

public:
	cameraSet(NET_DVR_USER_LOGIN_INFO* loginInfo, NET_DVR_DEVICEINFO_V40* deviceInfo, LONG* lUserID)
	{
		memset(&this->loginInfo, 0, sizeof(this->loginInfo));
		memset(&this->deviceInfo, 0, sizeof(this->deviceInfo));
		this->loginInfo = *loginInfo;
		this->deviceInfo = *deviceInfo;
		this->lUserID = *lUserID;
	}

	void setLoginInfo(const char* ip, const char* id, const char* pw, int port)
	{
		memset(&loginInfo, 0, sizeof(loginInfo));
		strcpy(loginInfo.sDeviceAddress, ip);
		strcpy(loginInfo.sUserName, id);
		strcpy(loginInfo.sPassword, pw);
		loginInfo.wPort = port;
		loginInfo.bUseAsynLogin = 0;
	}

	void setIUserID()
	{
		cout << "add: " << loginInfo.sDeviceAddress << endl;
		cout << "name: " << loginInfo.sUserName << endl;
		cout << "pw: " << loginInfo.sPassword << endl;

		NET_DVR_Init();
		lUserID = NET_DVR_Login_V40(&loginInfo, &deviceInfo);

		cout << lUserID << endl;

		if (lUserID < 0)
		{
			cerr << "NET_DVR_Login_V40 error! Error code: " << NET_DVR_GetLastError() << endl;
			NET_DVR_Cleanup();
		}
		else
		{
			cout << "NET_DVR_Login_V40 success!" << endl;
		}
		cout << NET_DVR_GetLastError() << endl;
	}

	NET_DVR_USER_LOGIN_INFO getLoginInfo()
	{
		return loginInfo;
	}

	NET_DVR_DEVICEINFO_V40 getDeviceInfo()
	{
		return deviceInfo;
	}

	LONG getIUserID()
	{
		return lUserID;
	}
};