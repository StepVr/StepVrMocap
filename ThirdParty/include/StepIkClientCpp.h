#pragma once
#ifdef _STEP_DLL_EXPORT
#define STEPIK_DLL_API __declspec(dllexport)
#else
#define STEPIK_DLL_API __declspec(dllimport)
#endif // _STEP_DLL_EXPORT

#include <string>

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
typedef void(*FnBtnCallBack)(int iBtnEvent, int iLR);//���������ͣ�iBtnEvent:1,Click;2,Hold;3,Release. �����ң�iLR:0,Left;1,Right
class STEPIK_DLL_API StepIK_Client
{
public:
	StepIK_Client();
	~StepIK_Client();
	int Connect(char * szServerIP, int serverPort);//port: 9516
	int startData();
	int TPose();
	void getData(transform * data);
	int GetServerStatus();
	int GetTPoseStatus();
	int GetRegStatus();
	int StopData();
	StepVR_UDP * m_UDPdev;

	//�����ָ���ؽ���̬�ķ��� 1. GloEnable(true);GloSetDir(i_dir);  2. ����ѭ������GetGloveData
	void GetGloveData(V4 * data); //32��4Ԫ����ǰ16��Ϊ���������ƣ�����Ĵָ�������������ؽڣ�ʳָ�����ؽڣ���ָ���ؽڣ�����ָ���ؽڣ�Сָ���ؽڡ���16��Ϊ���ֵģ�˳���������ͬ��
	void GloEnable(bool enable);//�������ر���������
	void GloSetDir(int i_dir);//���ú����
	void GloCaliFive(int iLR);//��ָУ׼��0��1�ң�
	void GloCaliStatic(int iLR);//��̬У׼
	void GloCaliDynamic(int iLR);//��̬У׼
	void GloSetBtnCallBack(FnBtnCallBack pFn);//�����ص����������Ϸ���������
	void GloSetRotType(int iType);//�����������̬��0 Local��1 Global��Ĭ����0

	void GetFaceData(float* fFaceData, int & iLen);

	bool HasBodyData();
	bool HasGloveData();
	bool HasFaceData();
};

extern "C" {
	STEPIK_DLL_API int dllInit(char * szServerIP, int serverPort);
	STEPIK_DLL_API int startData();
	STEPIK_DLL_API int TPose();
	STEPIK_DLL_API void getData(transform * data);
	
	STEPIK_DLL_API int GetTPoseStatus();
	STEPIK_DLL_API int GetRegStatus();
	STEPIK_DLL_API int Stop();
	
	STEPIK_DLL_API void GetGloveData(V4 * data);
	STEPIK_DLL_API void GloEnable(bool enable);
	STEPIK_DLL_API void GloSetDir(int i_dir);
	STEPIK_DLL_API void GloCaliFive(int iLR);
	STEPIK_DLL_API void GloCaliStatic(int iLR);
	STEPIK_DLL_API void GloCaliDynamic(int iLR);
	STEPIK_DLL_API void GloSetBtnCallBack(FnBtnCallBack pFn);
	STEPIK_DLL_API void GloSetRotType(int iType);

	STEPIK_DLL_API void GetFaceData(float* fFaceData, int & iLen);

	STEPIK_DLL_API bool HasBodyData();
	STEPIK_DLL_API bool HasGloveData();
	STEPIK_DLL_API bool HasFaceData();
	
}