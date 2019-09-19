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
	TPOSE_STATUS_SUCCESS = 0,//的
	TPOSE_VRIK_NOT_INITIATED = -1,//
	TPOSE_VRIK_HEAD_NOT_DATA = -2,//
	//TPOSE_TEST = 3,//
	//TPOSE_STATUS_DOINING = 0xffffffd,//
	TPOSE_SERVICE_NORESPONSE = 0xffff,//
	TPOSE_STATUS_NOT_SEND_TPOSE = 0xfffffff//
};
enum ERegStatus
{
	REG_STATUS_OK = 0,//注册成功
	REG_STATUS_REGCODE_INVALID_FORMAT = -1,//无效注册码格式
	REG_STATUS_WRONG_REGCODE = -2,//错误注册码
	REG_STATUS_ALREADY_USED = -3,//注册码已使用
	REG_STATUS_COMMUNICATION_ERROR = -4,//网络连接失败
	REG_STATUS_NOT_REG = -10,//未注册
	REG_STATUS_INVALID_REG = -11,//无效注册信息
	//STATUS_SERVICE_NORESPONSE = 0xffff,
	//REG_STATUS_SERVER_NOT_START = 0xffffff
};

enum ConnectReturnValue
{
	CRV_NORMAL_ONLINE = 0,//正常
	CRV_RECORDING = 1,//录数模式
	CRV_REPLAY = 2,//回放模式
	CRV_SERVER_INTERNAL_ERROR = -8,//IK算法初始化失败
	CRV_INVALID_REG = -11,//未注册
	CRV_SERVICE_NORESPONSE = 0xffff//服务无回应
};

struct MacArmData
{
	int id;//1表示左手数据, 2表示右手数据，0表示无效数据
	float angle[7];//7个轴的角度值，从内向外
	float speed[7];//7个轴的速度值，与上面对应
	unsigned int timeStamps;//时间戳
	float extension[5];//5个扩展值
};


class StepVR_UDP;
typedef void(*FnBtnCallBack)(int iBtnEvent, int iLR);//（按键类型）iBtnEvent:1,Click;2,Hold;3,Release. （左右）iLR:0,Left;1,Right
class STEPIK_DLL_API StepIK_Client
{
	StepVR_UDP * m_UDPdev;
public:
	StepIK_Client();
	~StepIK_Client();
	int Connect(char * szServerIP, int serverPort);//port: 9516..  返回值：忽略其值
	void DisConnect();
	bool IsConnected();

	int startData();//大于0表示成功
	int StopData();//大于0表示成功
	int TPose();//返回值：参考TPoseStatus
	
	int GetServerStatus();//返回值：参考ConnectReturnValue
	int GetTPoseStatus();//返回值：参考TPoseStatus
	int GetRegStatus();//返回值：参考RegisterStatus
	
	void GetLossyScale(V3 * scale);
	void getData(transform * data);//22个骨骼值
	
								   //获得手指各关节姿态的方法 1. GloEnable(true);GloSetDir(i_dir);  2. 在主循环调用GetGloveData
	void GetGloveM(V4 * data);//30个4元数（前15个为左手的，后15个是右手的，手指顺序和GetGloveData的相同）
	void GetGloveData(V4 * data); //32个4元数（前16个为：左手手掌，左手拇指从内向外三个关节，食指三个关节，中指三关节，无名指三关节，小指三关节。后16个为右手的，顺序和左手相同）
	void GloEnable(bool enable);//开启、关闭手套数据
	void GloSetDir(int i_dir);//设置航向角
	void GloCaliFive(int iLR);//五指校准（0左1右）
	void GloCaliStatic(int iLR);//静态校准
	void GloCaliDynamic(int iLR);//动态校准
	void GloSetBtnCallBack(FnBtnCallBack pFn);//按键回调函数，见上方函数声明
	
	//deprecated.
	void GloSetRotType(int iType);//已失效，只输出Global姿态。//设置输出的姿态（0 Local，1 Global）默认是0
	
	void GetHandPos(V3 * data);//在纯手模式下用于获得两只手的位置

	void GetFaceData(float* fFaceData, int & iLen);

	void GetMacArmData(MacArmData * data);//七轴机械臂专用,返回两个MacArmData

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