## 목표
hikvision사의 CCTV조작 프로그램 제작

## 개요
- 기간: 2024.12.24-2025.02.10
- IDE: Visual Studio 2022
- language: C/C++

---
- IVMS-4200
https://www.hikvision.com/korean/support/download/software/ivms4200-series/
- SADP
https://www.hikvision.com/korean/support/tools/hitools/clea8b3e4ea7da90a9/
두 가지 프로그램에서 IP / pw / port num 설정이 가능합니다.

- 녹화, ZOOM IN / OUT 구현

========================

- hikvisionControl.cpp: main
  - Config 구조체: 로그인 정보 저장(const * char 형태. 이후 로그인에 필요한 형태로는 cameraSet 클래스에서 저장)
  - logError(const string& message): 로그파일 저장
  - setTimer(): 타이머 세팅
  - init(): api 초기화
  - createWindow(): 녹화 화면 띄울 윈도우 생성
  - logIn(): IP카메라 로그인 (로그인 된 cameraSet 객체 cam 생성, 해당 객체 리턴)
  - showWindow(): 
  - checkError(): 에러 체크 함수. (chatGPT사용. sdk 파일 캡처해서 chat gpt에 집어넣은 후, 에러 내용들을 생성하게끔 요청했습니다.)
  - startLiveStream(cameraset cam, HWND hwnd): hwnd 윈도우에 라이브 영상 송출 
  - setConfig(): 로그인 input 받아 출력

- PTZController.h: 녹화, 및 PTZ 헤더. 
  - handleError(): 에러코드 출력(디버깅용 함수)
  - handlePTZControl(PTZCommand cmd, bool& startFlag, bool& stopFlag): ZOON IN / OUT 실행
  - setFileName(): 파일 이름 + 경로 함께 지정(상대경로 지정)
  - StartRecording(): 녹화 시작
  - StopRecording(): 녹화 종료
  - printMenu(): 메뉴 출력
  - toggleRecording(): 레코딩 여부 판별 및 제어
  - run(): PTZ 및 녹화 실행
  - start(): 스레드 추가(main에서 실행)
  - stop(): 스레드 프로세스 종료


- cameraSet.h: 각 const* char에 대해 sdk에서 로그인 시 요구하는 형태의 구조체로 저장 및 로그인
로그인을 위해 필요한 정보: NET_DVR_USER_LOGIN_INFO(SDK 로그인 api 호출 시 필요한 자료형), NET_DVR_DEVICEINFO_V40(SDK 로그인 api 호출 시 필요한 자료형)
LONG lUserID: 로그인 성공 실패 판단을 위한 변수
로그인 함수: NET_DVR_Login_V40(): 로그인 시 -1이상의 값을 return. 자료형은 LONG
  - cameraSet(): setLoginInfo에서 저장한 값들을 mapping
  - setLoginInfo(): loginInfo에 필요한 값들을 받아옴.
  - setIUserID(): 로그인 실행
