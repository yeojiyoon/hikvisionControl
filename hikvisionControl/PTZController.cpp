#include <iostream>
#include <conio.h> 
#include <cstring> 
#include "HCNetSDK.h"  
#include "cameraSet.h"
#include <thread>

#define START 0  
#define STOP  1

#pragma once

using namespace std;

enum PTZCommand
{
    ZOOM_IN_START = 1,
    ZOOM_IN_STOP,
    ZOOM_OUT_START,
    ZOOM_OUT_STOP
};

class PTZController
{
private:
    cameraSet cam;
    LONG playHandle;
    bool ZIStart, ZIStop;
    bool ZOStart, ZOStop;
    bool isRecording;

    thread controlThread;
    atomic<bool> running;

    void handleError(const string& action)
    {
        cerr << action << " [failed] error code: " << NET_DVR_GetLastError() << endl;
    }

    bool handlePTZControl(PTZCommand cmd, bool& startFlag, bool& stopFlag)
    {
        LONG userID = cam.getIUserID();
        DWORD command = (cmd == ZOOM_IN_START || cmd == ZOOM_IN_STOP) ? ZOOM_IN : ZOOM_OUT;
        DWORD control = (cmd == ZOOM_IN_START || cmd == ZOOM_OUT_START) ? START : STOP;

        if (!NET_DVR_PTZControl(userID, command, control))
        {
            handleError("PTZ 제어");
            return false;
        }

        cout << "PTZ control succeed." << endl;
        startFlag = (cmd == ZOOM_IN_START || cmd == ZOOM_OUT_START);
        stopFlag = !startFlag;
        return true;
    }

    char* setFileName()
    {
        char fileName[100];
        time_t now = time(0);
        tm* ltm = localtime(&now);

        sprintf(fileName, "record_%d-%02d-%02d_%02d-%02d-%02d.mp4",
            1900 + ltm->tm_year, ltm->tm_mon + 1, ltm->tm_mday,
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

        return fileName;
    }

    void StartRecording()
    {
        char* saveFilePath = setFileName();
        if (!NET_DVR_SaveRealData(playHandle, saveFilePath))
        {
            cout << "Failed to start saving data, error: " << NET_DVR_GetLastError() << endl;
        }
        else {
            cout << "Recording started. Saving to: " << saveFilePath << endl;
            isRecording = true;
        }
    }

    void StopRecording()
    {
        if (!NET_DVR_StopSaveRealData(playHandle))
        {
            cout << "Failed to stop saving data, error: " << NET_DVR_GetLastError() << endl;
        }
        else
        {
            handleError("Recording stopped.");
        }
    }

public:
    PTZController(cameraSet camera, LONG handle)
        :
        cam(camera),
        playHandle(handle),
        ZIStart(false),
        ZIStop(false),
        ZOStart(false),
        ZOStop(false),
        isRecording(false)
    {

    }

    ~PTZController()
    {
        stop();
    }

    void printMenu()
    {
        cout << "=============================" << endl;
        cout << "1: ZOOM_IN_START" << endl;
        cout << "2: ZOOM_IN_STOP" << endl;
        cout << "3: ZOOM_OUT_START" << endl;
        cout << "4: ZOOM_OUT_STOP" << endl;
        cout << "R: 녹화 시작/중지" << endl;
        cout << "Q: 종료" << endl;
        cout << "=============================" << endl;
    }

    void toggleRecording()
    {
        if (isRecording)
        {
            StopRecording();
        }
        else
        {
            StartRecording();
        }
    }

    void run()
    {
        printMenu();

        while (running)
        {
            if (_kbhit())
            {
                char ch = _getch();

                switch (ch)
                {
                case '1':
                    if (!ZIStart) handlePTZControl(ZOOM_IN_START, ZIStart, ZIStop);
                    break;
                case '2':
                    if (!ZIStop) handlePTZControl(ZOOM_IN_STOP, ZIStart, ZIStop);
                    break;
                case '3':
                    if (!ZOStart) handlePTZControl(ZOOM_OUT_START, ZOStart, ZOStop);
                    break;
                case '4':
                    if (!ZOStop) handlePTZControl(ZOOM_OUT_STOP, ZOStart, ZOStop);
                    break;
                case 'R': case 'r':
                    toggleRecording();
                    break;
                case 'Q': case 'q':
                    cout << "프로그램을 종료합니다." << endl;
                    return;
                default:
                    cout << "잘못된 입력입니다. 다시 입력하세요." << endl;
                    break;
                }
            }
        }
    }

    void start()
    {
        if (!running)
        {
            running = true;
            controlThread = thread(&PTZController::run, this);
        }
    }

    void stop()
    {
        if (running)
        {
            running = false;
            if (controlThread.joinable())
            {
                controlThread.join();
            }
        }
    }
};