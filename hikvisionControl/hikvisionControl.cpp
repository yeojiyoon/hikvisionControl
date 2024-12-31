#include <stdio.h>
#include <iostream>
#include <cstring>
#include "Windows.h"
#include "HCNetSDK.h"
#include <time.h>
#pragma comment(lib, "HCNetSDK.lib")
#pragma warning(disable:4996)

using namespace std;

int main()
{
	//---------------------------------------

	if (!NET_DVR_Init())
	{
		cerr << "NET_DVR_Init error!" << endl;
		return -1;
	}
	cout << "NET_DVR_Init success!" << endl;

	//---------------------------------------
	//윈도우 생성
	HWND hwnd = CreateWindowEx(
		0, 
		L"STATIC", 
		L"Hikvision Live View", 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 
		100, 
		800, 
		600, 
		nullptr, 
		nullptr, 
		GetModuleHandle(nullptr), 
		nullptr
	);
	if (!hwnd)
	{
		cerr << "CreateWindowEx error!" << endl;
		NET_DVR_Cleanup();
		return -1;
	}

	//---------------------------------------
	//로그인 정보 설정

	NET_DVR_USER_LOGIN_INFO loginInfo = { 0 };
	NET_DVR_DEVICEINFO_V40 deviceInfo = { 0 };

	strncpy(loginInfo.sDeviceAddress, "192.168.1.100", sizeof(loginInfo.sDeviceAddress) - 1); //디바이스 IP
	loginInfo.wPort = 8000; //포트 번호
	strncpy(loginInfo.sUserName, "admin", sizeof(loginInfo.sUserName) - 1); //사용자 이름
	strncpy(loginInfo.sPassword, "12345", sizeof(loginInfo.sPassword) - 1); //비밀번호

	LONG lUserID = NET_DVR_Login_V40(&loginInfo, &deviceInfo);
	if (lUserID < 0) {
		std::cerr << "로그인 실패, 오류 코드: " << NET_DVR_GetLastError() << std::endl;
		NET_DVR_Cleanup();
		return -1;
	}

	//---------------------------------------
	//영상 출력 설정

	NET_DVR_PREVIEWINFO previewInfo = { 0 };
	previewInfo.lChannel = 1;
	previewInfo.dwStreamType = 0;
	previewInfo.dwLinkMode = 0;
	previewInfo.hPlayWnd = hwnd; //영상 출력할 윈도우 핸들

	LONG playHandle = NET_DVR_RealPlay_V40(lUserID, &previewInfo, nullptr, nullptr);
	if (playHandle < 0) {
		std::cerr << "라이브 뷰 시작 실패, 오류 코드: " << NET_DVR_GetLastError() << std::endl;
	}
	else {
		std::cout << "라이브 뷰 시작 성공" << std::endl;
	}

	// 메시지 루프 (라이브 스트리밍 유지)
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//---------------------------------------
	// 라이브 스트리밍 종료
	NET_DVR_StopRealPlay(playHandle);
	NET_DVR_Logout(lUserID);
	NET_DVR_Cleanup();

	return 0;
}