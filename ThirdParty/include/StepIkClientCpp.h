#pragma once
#ifdef _STEP_DLL_EXPORT
#define STEPIK_DLL_API __declspec(dllexport)
#else
#define STEPIK_DLL_API __declspec(dllimport)
#endif // _STEP_DLL_EXPORT

#include <string>


#if WITH_STEPMAGIC
#include "VirtualShootingDll.h"
#else
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
#endif

enum ServerStatus
{
	STATUS_NORMAL_ONLINE = 0,//runStatus
	STATUS_RECORDING = 1,//runStatus
	STATUS_REPLAY = 2,//runStatus
	FILE_NOT_FIND = -1,
	FILE_OPEN_ERROR = -2,
	STATUS_NOT_IN_NORMAL_MODE = -3,
	FILE_WRONG_FORMAT = -4,
	FILE_TRACKER_COUNT_ERROR = -5,
	FILE_ALREADY_EXIST = -6,
	STATUS_NOT_SUPPORT_TYPE = -7,
	STATUS_SERVER_INTERNAL_ERROR = -8,//runStatus
	STATUS_CONNECT_MMAP_ERROR = -9,
	STATUS_NOT_REG = -10,
	STATUS_INVALID_REG = -11,//runStatus
	STATUS_SERVICE_NORESPONSE = 0xffff,//runStatus
	STATUS_NOTSEND_UNKNOW = 0xffffff
};
enum TPoseStatus
{
	TPOSE_STATUS_SUCCESS = 0,//��
	TPOSE_VRIK_NOT_INITIATED = -1,//
	TPOSE_VRIK_HEAD_NOT_DATA = -2,//
	//TPOSE_TEST = 3,//
	//TPOSE_STATUS_DOINING = 0xffffffd,//
	TPOSE_SERVICE_NORESPONSE = 0xffff,//
	TPOSE_STATUS_NOT_SEND_TPOSE = 0xfffffff//
};
enum ERegStatus
{
	REG_STATUS_OK = 0,//ע��ɹ�
	REG_STATUS_REGCODE_INVALID_FORMAT = -1,//��Чע�����ʽ
	REG_STATUS_WRONG_REGCODE = -2,//����ע����
	REG_STATUS_ALREADY_USED = -3,//ע������ʹ��
	REG_STATUS_COMMUNICATION_ERROR = -4,//��������ʧ��
	REG_STATUS_NOT_REG = -10,//δע��
	REG_STATUS_INVALID_REG = -11,//��Чע����Ϣ
	//STATUS_SERVICE_NORESPONSE = 0xffff,
	//REG_STATUS_SERVER_NOT_START = 0xffffff
};

enum ConnectReturnValue
{
	CRV_NORMAL_ONLINE = 0,//����
	CRV_RECORDING = 1,//¼��ģʽ
	CRV_REPLAY = 2,//�ط�ģʽ
	CRV_SERVER_INTERNAL_ERROR = -8,//IK�㷨��ʼ��ʧ��
	CRV_INVALID_REG = -11,//δע��
	CRV_SERVICE_NORESPONSE = 0xffff//�����޻�Ӧ
};

struct MacArmData
{
	int id;//1��ʾ��������, 2��ʾ�������ݣ�0��ʾ��Ч����
	float angle[7];//7����ĽǶ�ֵ����������
	float speed[7];//7������ٶ�ֵ���������Ӧ
	unsigned int timeStamps;//ʱ���
	float extension[5];//5����չֵ
};


class StepVR_UDP;
typedef void(*FnBtnCallBack)(int iBtnEvent, int iLR);//���������ͣ�iBtnEvent:1,Click;2,Hold;3,Release. �����ң�iLR:0,Left;1,Right
class STEPIK_DLL_API StepIK_Client
{
	StepVR_UDP * m_UDPdev;
public:
	StepIK_Client();
	~StepIK_Client();
	int Connect(char * szServerIP, int serverPort);//port: 9516..  ����ֵ��������ֵ
	void DisConnect();
	bool IsConnected();

	int startData();//����0��ʾ�ɹ�
	int StopData();//����0��ʾ�ɹ�
	int TPose();//����ֵ���ο�TPoseStatus
	
	int GetServerStatus();//����ֵ���ο�ConnectReturnValue
	int GetTPoseStatus();//����ֵ���ο�TPoseStatus
	int GetRegStatus();//����ֵ���ο�RegisterStatus
	
	void GetLossyScale(V3 * scale);
	void getData(transform * data);//22������ֵ
	
								   //�����ָ���ؽ���̬�ķ��� 1. GloEnable(true);GloSetDir(i_dir);  2. ����ѭ������GetGloveData
	void GetGloveM(V4 * data);//30��4Ԫ����ǰ15��Ϊ���ֵģ���15�������ֵģ���ָ˳���GetGloveData����ͬ��
	void GetGloveData(V4 * data); //32��4Ԫ����ǰ16��Ϊ���������ƣ�����Ĵָ�������������ؽڣ�ʳָ�����ؽڣ���ָ���ؽڣ�����ָ���ؽڣ�Сָ���ؽڡ���16��Ϊ���ֵģ�˳���������ͬ��
	void GloEnable(bool enable);//�������ر���������
	void GloSetDir(int i_dir);//���ú����
	void GloCaliFive(int iLR);//��ָУ׼��0��1�ң�
	void GloCaliStatic(int iLR);//��̬У׼
	void GloCaliDynamic(int iLR);//��̬У׼
	void GloSetBtnCallBack(FnBtnCallBack pFn);//�����ص����������Ϸ���������
	
	//deprecated.
	void GloSetRotType(int iType);//��ʧЧ��ֻ���Global��̬��//�����������̬��0 Local��1 Global��Ĭ����0
	
	void GetHandPos(V3 * data);//�ڴ���ģʽ�����ڻ����ֻ�ֵ�λ��

	void GetFaceData(float* fFaceData, int & iLen);

	void GetMacArmData(MacArmData * data);//�����е��ר��,��������MacArmData

	bool HasBodyData();
	bool HasGloveData();
	bool HasFaceData();
};


extern "C" {
	STEPIK_DLL_API int dllInit(char * szServerIP, int serverPort);
	STEPIK_DLL_API void DisConnect();
	STEPIK_DLL_API int startData();
	STEPIK_DLL_API int StopData();
	STEPIK_DLL_API int TPose();
		
	STEPIK_DLL_API int GetServerStatus();
	STEPIK_DLL_API int GetTPoseStatus();
	STEPIK_DLL_API int GetRegStatus();

	STEPIK_DLL_API void getData(transform * data);
	
	STEPIK_DLL_API void GetLossyScale(V3 * scale);
	STEPIK_DLL_API void GetGloveM(V4 * data);
	STEPIK_DLL_API void GetGloveData(V4 * data);
	STEPIK_DLL_API void GloEnable(bool enable);
	STEPIK_DLL_API void GloSetDir(int i_dir);
	STEPIK_DLL_API void GloCaliFive(int iLR);
	STEPIK_DLL_API void GloCaliStatic(int iLR);
	STEPIK_DLL_API void GloCaliDynamic(int iLR);
	STEPIK_DLL_API void GloSetBtnCallBack(FnBtnCallBack pFn);
	STEPIK_DLL_API void GloSetRotType(int iType);

	STEPIK_DLL_API void GetHandPos(V3 * data);

	STEPIK_DLL_API void GetFaceData(float* fFaceData, int & iLen);

	STEPIK_DLL_API void GetMacArmData(MacArmData * data);

	STEPIK_DLL_API bool HasBodyData();
	STEPIK_DLL_API bool HasGloveData();
	STEPIK_DLL_API bool HasFaceData();
	
}