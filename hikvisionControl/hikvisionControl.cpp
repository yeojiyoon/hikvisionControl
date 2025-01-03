#include <stdio.h>
#include <iostream>
#include <cstring>
#include "Windows.h"
#include "HCNetSDK.h"
#include <time.h>
#include <conio.h>
#include "cameraSet.h"
#include <thread>
#pragma comment(lib, "HCNetSDK.lib")
#pragma warning(disable:4996)

using namespace std;

bool isRecording = false;

//초기화 함수
int init()
{
	if (!NET_DVR_Init())
	{
		cerr << "NET_DVR_Init error!" << endl;
		return -1;
	}
	cout << "NET_DVR_Init success!" << endl;
}

//윈도우 생성 함수
HWND createWindow()
{
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
		return nullptr;
	}

	return hwnd;
}

//로그인 함수
cameraSet logIn(const char* ip, const char* id, const char* pw, int port)
{
	NET_DVR_USER_LOGIN_INFO loginInfo = { 0 };
	NET_DVR_DEVICEINFO_V40 deviceInfo = { 0 };
	LONG lUserID = { 0 };

	cameraSet cam = cameraSet(&loginInfo, &deviceInfo, &lUserID);

	cam.setLoginInfo(ip, id, pw, port);
	cam.setIUserID();

	return cam;
}

//라이브 뷰 설정 함수
NET_DVR_PREVIEWINFO setPreviewInfo(int lChannel, int dwStreamType, int dwLinkMode, HWND hwnd) //클래스?
{
	NET_DVR_PREVIEWINFO previewInfo = { 0 };
	previewInfo.lChannel = lChannel;
	previewInfo.dwStreamType = dwStreamType;
	previewInfo.dwLinkMode = dwLinkMode;
	previewInfo.hPlayWnd = hwnd; //영상 출력할 윈도우 핸들
	return previewInfo;
}

//녹화 시작 함수
void StartRecording(LONG realPlayHandle, char* filePath) {
	if (!NET_DVR_SaveRealData(realPlayHandle, filePath)) {
		cout << "Failed to start saving data, error: " << NET_DVR_GetLastError() << endl;
	}
	else {
		cout << "Recording started. Saving to: " << filePath << endl;
		isRecording = true;
	}
}

//녹화 종료 함수
void StopRecording(LONG realPlayHandle) {
	if (!NET_DVR_StopSaveRealData(realPlayHandle)) {
		cout << "Failed to stop saving data, error: " << NET_DVR_GetLastError() << endl;
	}
	else {
		cout << "Recording stopped." << endl;
		isRecording = false;
	}
}

//윈도우 메시지 루프 함수
void showWindow()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

//키 입력 확인 함수
void checkKey(LONG playHandle, char* saveFilePath)
{
	while (true)
	{
		if (_kbhit())
		{
			char ch = _getch();
			if (ch == 'R' || ch == 'r')
			{
				if (isRecording)
				{
					StopRecording(playHandle);
				}
				else
				{
					StartRecording(playHandle, saveFilePath);
				}
			}
			else if (ch == 'Q' || ch == 'q')
			{
				break;
			}
		}
		Sleep(100); // CPU 사용률 절약
	}
}

int main()
{
	if (!init()) return -1;

	cameraSet cam = logIn("192.168.1.100", "admin", "12345", 8000);

	HWND hwnd = createWindow();
	if (!hwnd)
	{
		cerr << "CreateWindowEx error!" << endl;
		NET_DVR_Cleanup();
		return -1;
	}

	NET_DVR_PREVIEWINFO previewInfo = setPreviewInfo(1, 0, 0, hwnd);

	LONG playHandle = NET_DVR_RealPlay_V40(cam.getIUserID(), &previewInfo, nullptr, nullptr);
	if (playHandle < 0) {
		cerr << "라이브 뷰 시작 실패, 오류 코드: " << NET_DVR_GetLastError() << endl;
		//NET_DVR_Cleanup();
		//return -1;
	}
	else {
		cout << "라이브 뷰 시작 성공" << endl;
	}
	
	//---------------------------------------
	//메시지 루프 (라이브 스트리밍 유지)

	char saveFilePath[100] = "C:\\Recordings\\output.mp4";  // 저장할 파일 경로

	thread windowThread(showWindow);
	thread keyThread(checkKey, playHandle, saveFilePath);

	windowThread.join();
	keyThread.join();

	//---------------------------------------
	//라이브 스트리밍 종료

	NET_DVR_StopRealPlay(playHandle); //라이브 뷰 종료
	NET_DVR_Logout(cam.getIUserID()); //로그아웃
	NET_DVR_Cleanup(); //SDK 종료

	return 0;
}