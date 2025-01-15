#include "cameraSet.h"
#include "HCNetSDK.h"
#include "Windows.h"
#include <atomic>
#include <conio.h>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <shared_mutex>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <time.h>

#define START	0
#define STOP	1

#pragma comment(lib, "HCNetSDK.lib")
#pragma warning(disable:4996)

using namespace std;
bool isRecording = false;

const int CONNECT_TIMEOUT = 2000;
const int RECONNECT_INTERVAL = 10000;
char LOG_FILE_PATH[] = "./SdkLog/";

time_t timer;
struct tm* t;

void logError(const string& message);
void setTimer();
void checkError();

int init();
HWND createWindow();
NET_DVR_PREVIEWINFO setPreviewInfo(int lChannel, int dwStreamType, int dwLinkMode, HWND hwnd);

cameraSet logIn(const char* ip, const char* id, const char* pw, int port);
LONG startLiveStream(cameraSet cam, HWND hwnd);
void StartRecording(LONG realPlayHandle, char* filePath);
void StopRecording(LONG realPlayHandle);

void showWindow();
char* setFileName();
void checkKey(LONG playHandle);
void handleMessageLoop(cameraSet cam, LONG playHandle);
void PTZControl(cameraSet cam);

struct Config
{
	string server_ip;
	int server_port;
	string username;
	string password;
};

int main()
{
	setTimer();

	if (!init()) return -1;

	NET_DVR_SetLogToFile(TRUE, LOG_FILE_PATH, 0);
	NET_DVR_SetConnectTime(CONNECT_TIMEOUT, 1);
	NET_DVR_SetReconnect(RECONNECT_INTERVAL, true);

	cameraSet cam = logIn("192.168.0.64", "admin", "password132!", 8000);

	HWND hwnd = createWindow();
	
	cout << cam.getLoginInfo().sDeviceAddress << endl;

	NET_DVR_PREVIEWINFO previewInfo = setPreviewInfo(1, 0, 0, hwnd);
	previewInfo.bBlocked = FALSE;

	checkError();

	LONG playHandle = startLiveStream(cam, hwnd);

	handleMessageLoop(cam, playHandle);

	NET_DVR_StopRealPlay(playHandle); //라이브 뷰 종료
	NET_DVR_Logout(cam.getIUserID()); //로그아웃
	NET_DVR_Cleanup(); //SDK 종료

	return 0;
}

void logError(const string& message) {
	ofstream logFile("error.log", ios::app);
	logFile << "[" << time(nullptr) << "] " << message << endl;
}

void setTimer() {
	timer = time(NULL);
	t = localtime(&timer);
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
	if (!hwnd)
	{
		cerr << "CreateWindowEx error!" << endl;
		NET_DVR_Cleanup();
		exit(1);
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
		setTimer();
		cout << "Recording started. Saving to: " << filePath << " " << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << endl;
		isRecording = true;
	}
}

//녹화 종료 함수
void StopRecording(LONG realPlayHandle) {
	if (!NET_DVR_StopSaveRealData(realPlayHandle)) {
		cout << "Failed to stop saving data, error: " << NET_DVR_GetLastError() << endl;
	}
	else {
		setTimer();
		cout << "Recording stopped." << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << endl;
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

char* setFileName()
{
	char fileName[100];
	char tmp[100];
	setTimer();
	sprintf(tmp, "C:\\Recordings\\output_%d-%d-%d_%d-%d-%d.mp4", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	strcpy(fileName, tmp);
	return fileName;
}
//키 입력 확인 함수
void checkKey(LONG playHandle)
{
	char saveFilePath[100];

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
					strcpy(saveFilePath, setFileName());
					StartRecording(playHandle, saveFilePath);
				}
			}
			else if (ch == 'Q' || ch == 'q')
			{
				exit(1);
			}
		}
		//Sleep(100); // CPU 사용률 절약
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
		case 34:
			cerr << "Error: 데이터 저장 오류 (NET_DVR_CREATEFILE_ERROR)" << endl;
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

LONG startLiveStream(cameraSet cam, HWND hwnd)
{
	NET_DVR_PREVIEWINFO previewInfo = setPreviewInfo(1, 0, 0, hwnd);
	previewInfo.bBlocked = FALSE;
	checkError();

	LONG playHandle = NET_DVR_RealPlay_V40(cam.getIUserID(), &previewInfo, NULL, NULL);
	if (playHandle < 0) {
		checkError();
		NET_DVR_Cleanup();
		return -1;
	}
	else {
		cout << "라이브 뷰 시작 성공" << endl;
	}
	return playHandle;
}

void handleMessageLoop(cameraSet cam, LONG playHandle) {
	char saveFilePath[100] = { 0 };
	strcpy(saveFilePath, setFileName());

	thread windowThread(showWindow);
	thread keyThread(checkKey, playHandle);
	thread PTZControlThread(PTZControl, cam);

	windowThread.join();
	keyThread.join();
	PTZControlThread.join();
}

void PTZControl(cameraSet cam)
{
	int ptzCommand = -1;
	bool isStart = false;
	
	printf("press 1 to ZOOM_IN_START\n");
	printf("press 2 to ZOOM_IN_STOP\n");
	printf("press 3 to ZOOM_OUT_START\n");
	printf("press 4 to ZOOM_OUT_STOP\n");

	while (true)
	{
		if (_kbhit())
		{
			char ch = _getch();
			if (ch == '1')
			{
				if (!NET_DVR_PTZControl(cam.getIUserID(), ZOOM_IN, START))
				{
					cerr << "PTZ 제어 실패, 에러 코드: " << NET_DVR_GetLastError() << endl;
				}
				else
				{
					cout << "PTZ 제어 성공!" << endl;
				}
			}
			else if (ch == '2')
			{
				if (!NET_DVR_PTZControl(cam.getIUserID(), ZOOM_IN, STOP))
				{
					cerr << "PTZ 제어 실패, 에러 코드: " << NET_DVR_GetLastError() << endl;
				}
				else
				{
					cout << "PTZ 제어 성공!" << endl;
				}
			}
			else if (ch == '3')
			{
				if (!NET_DVR_PTZControl(cam.getIUserID(), ZOOM_OUT, START))
				{
					cerr << "PTZ 제어 실패, 에러 코드: " << NET_DVR_GetLastError() << endl;
				}
				else
				{
					cout << "PTZ 제어 성공!" << endl;
				}
			}
			else if (ch == '4')
			{
				if (!NET_DVR_PTZControl(cam.getIUserID(), ZOOM_OUT, STOP))
				{
					cerr << "PTZ 제어 실패, 에러 코드: " << NET_DVR_GetLastError() << endl;
				}
				else
				{
					cout << "PTZ 제어 성공!" << endl;
				}
			}
			else if (ch == 'Q' || ch == 'q')
			{
				exit(1);
			}
		}
		//Sleep(100); // CPU 사용률 절약
	}
}