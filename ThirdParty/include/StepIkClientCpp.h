#pragma once
#ifdef _STEP_DLL_EXPORT
#define STEPIK_DLL_API __declspec(dllexport)
#else
#define STEPIK_DLL_API __declspec(dllimport)
#endif // _STEP_DLL_EXPORT

struct V3
{
	float x;
	float y;
	float z;
};

struct V4
{
	float x;
	float y;
	float z;
	float w;

};

struct transform
{
	V3 Location;
	V4 Rotation;
	V3 Scale;
};

enum ServerStatus
{
	STATUS_NORMAL_ONLINE = 0,
	STATUS_RECORDING = 1,
	STATUS_REPLAY = 2,
	FILE_NOT_FIND = -1,
	FILE_OPEN_ERROR = -2,
	STATUS_NOT_IN_NORMAL_MODE = -3,
	FILE_WRONG_FORMAT = -4,
	FILE_TRACKER_COUNT_ERROR = -5,
	FILE_ALREADY_EXIST = -6,
	STATUS_NOT_SUPPORT_TYPE = -7,
	STATUS_SERVER_INTERNAL_ERROR = -8,
	STATUS_CONNECT_MMAP_ERROR = -9,
	STATUS_NOT_REG = -10,
	STATUS_INVALID_REG = -11,
	STATUS_SERVICE_NORESPONSE = 0xffff,
	STATUS_NOTSEND_UNKNOW = 0xffffff
};
enum TPoseStatus
{
	TPOSE_STATUS_SUCCESS = 0,
	TPOSE_VRIK_NOT_INITIATED = -1,
	TPOSE_VRIK_HEAD_NOT_DATA = -2,
	TPOSE_STATUS_DOINING = 1,
	TPOSE_STATUS_NOT_SEND_TPOSE = 0xffffff
};
enum RegisterStatus
{
	REG_STATUS_OK = 0,
	REG_STATUS_REGCODE_INVALID_FORMAT = -1,
	REG_STATUS_WRONG_REGCODE = -2,
	REG_STATUS_ALREADY_USED = -3,
	REG_STATUS_COMMUNICATION_ERROR = -4,
	REG_STATUS_NOT_REG = -10,
	REG_STATUS_INVALID_REG = -11,
	REG_STATUS_SERVER_NOT_START = 0xffffff
};
class StepVR_UDP;
class STEPIK_DLL_API StepIK_Client
{
public:
	StepIK_Client();
	~StepIK_Client();
	int Connect(char * szServerIP, int serverPort);
	int startData();
	int TPose();
	void getData(transform * data);
	int GetServerStatus();
	int GetTPoseStatus();
	int GetRegStatus();
	int StopData();
	StepVR_UDP * m_UDPdev;

};

extern "C" {
	STEPIK_DLL_API int dllInit(char * szServerIP, int serverPort);
	STEPIK_DLL_API int startData();
	STEPIK_DLL_API int TPose();
	STEPIK_DLL_API void getData(transform * data);
	STEPIK_DLL_API int GetTPoseStatus();
	STEPIK_DLL_API int GetRegStatus();
	STEPIK_DLL_API int Stop();
}