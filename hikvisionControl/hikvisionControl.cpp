#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include "Windows.h"
#include "HCNetSDK.h"
#include <time.h>
#include <conio.h>
#include "cameraSet.h"
#include <shared_mutex>
#include <thread>
#include <atomic>
#pragma comment(lib, "HCNetSDK.lib")
#pragma warning(disable:4996)

using namespace std;

bool isRecording = false;

void logError(const string& message) {
	ofstream logFile("error.log", ios::app);
	logFile << "[" << time(nullptr) << "] " << message << endl;
}

//초기화 함수
int init()
{
	if (!NET_DVR_Init()) {
		cerr << "NET_DVR_Init error!" << endl;
		logError("NET_DVR_Init error!");
		return false;
	}
	else cout << "NET_DVR_Init success!" << endl;
	return true;
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
		logError("CreateWindowEx error!");
		NET_DVR_Cleanup();
		return nullptr;
	}

	return hwnd;
}

struct Config {
	string ip;
	string username;
	string password;
	int port;
};

Config readConfig(const string& filename) {
	Config config;
	ifstream configFile(filename);

	if (!configFile.is_open()) {
		cerr << "Failed to open config file!" << endl;
		logError("Failed to open config file!");
		exit(EXIT_FAILURE);
	}

	configFile >> config.ip >> config.username >> config.password >> config.port;
	configFile.close();
	return config;
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

void checkError() {
	if (NET_DVR_GetLastError() != 0) {
		switch (NET_DVR_GetLastError())
		{
		case 1:
			cerr << "Error: 사용자 이름 또는 비밀번호 오류 (NET_DVR_PASSWORD_ERROR)" << endl;
			break;
		case 2:
			cerr << "Error: 권한이 부족합니다. (NET_DVR_NOENOUGHPRI)" << endl;
			break;
		case 3:
			cerr << "Error: SDK가 초기화되지 않았습니다. (NET_DVR_NOINIT)" << endl;
			break;
		case 4:
			cerr << "Error: 채널 번호 오류입니다. (NET_DVR_CHANNEL_ERROR)" << endl;
			break;
		case 5:
			cerr << "Error: 최대 연결 수를 초과했습니다. (NET_DVR_OVER_MAXLINK)" << endl;
			break;
		case 6:
			cerr << "Error: 버전이 맞지 않습니다. (NET_DVR_VERSIONNOMATCH)" << endl;
			break;
		case 7:
			cerr << "Error: 네트워크 연결에 실패했습니다. (NET_DVR_NETWORK_FAIL_CONNECT)" << endl;
			break;
		case 10:
			cerr << "Error: 네트워크 데이터 송신 오류입니다. (NET_DVR_NETWORK_SEND_ERROR)" << endl;
			break;
		case 11:
			cerr << "Error: 네트워크 데이터 수신 오류입니다. (NET_DVR_NETWORK_RECV_ERROR)" << endl;
			break;
		case 12:
			cerr << "Error: 네트워크 연결이 시간 초과되었습니다. (NET_DVR_NETWORK_TIMEOUT)" << endl;
			break;
		case 20:
			cerr << "Error: 파일 생성 실패 (NET_DVR_FILE_CREATE_ERROR)" << endl;
			break;
		case 21:
			cerr << "Error: 파일 열기 실패 (NET_DVR_FILE_OPEN_ERROR)" << endl;
			break;
		case 22:
			cerr << "Error: 파일 쓰기 실패 (NET_DVR_FILE_WRITE_ERROR)" << endl;
			break;
		case 30:
			cerr << "Error: 매개변수 오류 (NET_DVR_PARAMETER_ERROR)" << endl;
			break;
		case 31:
			cerr << "Error: 데이터 복호화 실패 (NET_DVR_DECODER_ERROR)" << endl;
			break;
		case 32:
			cerr << "Error: 데이터 형식 오류 (NET_DVR_FORMAT_ERROR)" << endl;
			break;
		case 40:
			cerr << "Error: 장치에서 오류가 발생했습니다. (NET_DVR_DEVICE_ERROR)" << endl;
			break;
		case 41:
			cerr << "Error: 장치 메모리가 부족합니다. (NET_DVR_MEMORY_ALLOC_ERROR)" << endl;
			break;
		case 42:
			cerr << "Error: 장치가 바쁘거나 요청을 처리할 수 없습니다. (NET_DVR_DEVICE_BUSY)" << endl;
			break;
		case 90:
			cerr << "Error: 알 수 없는 오류 (NET_DVR_UNKNOWNERROR)" << endl;
			break;
		case 91:
			cerr << "Error: 사용자 정의 실패 (NET_DVR_CUSTOM_ERROR)" << endl;
			break;
		default:
			cerr << "Error: 정의되지 않은 오류 코드입니다." << NET_DVR_GetLastError() << endl;
			break;
		}
	}
}

int main()
{
	if (!init()) return -1;
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	cameraSet cam = logIn("192.168.0.64", "admin", "insung1025", 8000);

	HWND hwnd = createWindow();
	if (!hwnd)
	{
		cerr << "CreateWindowEx error!" << endl;
		NET_DVR_Cleanup();
		return -1;
	}

	NET_DVR_PREVIEWINFO previewInfo = setPreviewInfo(1, 0, 0, NULL);
	previewInfo.bBlocked = 1;

	checkError();

	cout << cam.getIUserID() << endl;

	LONG playHandle = NET_DVR_RealPlay_V40(cam.getIUserID(), &previewInfo, nullptr, nullptr);
	if (playHandle < 0) {
		checkError();
		NET_DVR_Cleanup();
		return -1;
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