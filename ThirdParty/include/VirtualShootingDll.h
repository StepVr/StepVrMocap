#pragma once
#ifdef VIRTUALSHOOTINGDLL_EXPORTS
#define VIRTUALSHOOTINGDLL_API __declspec(dllexport)
#else
#define VIRTUALSHOOTINGDLL_API __declspec(dllimport)
#endif

#include <map>
#include <string>
#include <mutex>
#include <vector>
#define ALLLOCATION 255 // 限制最大255
#define TIMESTAMP 12

#define MOTION_SIZE 22
#define GLOVE_SIZE 32
#define FACE_SIZE 100

typedef unsigned int uint32;
typedef unsigned char uchar;



struct TransformData {
	float position[3];
	float orientation[4];
};
struct TrackerInfo {
	int tracker_id;
	TransformData data;
};

struct MFGKey
{
	int nPort;
	std::string strIP;
	bool bFlag;

	MFGKey() :nPort(0), strIP(""), bFlag(true)
	{}

	bool operator<(const MFGKey &stMFGConnectInfo) const
	{
		bool bRet = false;
		if (!bFlag && !stMFGConnectInfo.bFlag)
		{
			bRet = nPort < stMFGConnectInfo.nPort;
		}
		else
		{
			bRet = strIP < stMFGConnectInfo.strIP;
		}
		return bRet;
	}
};

struct FaceInfo
{
	int nValidNum; // 定义长度是FACE_SIZE，但是以这个有效长度为准
	float GFaceData[FACE_SIZE]; // 面捕
};

struct MFGValue
{
	::transform StepVrMocapData[MOTION_SIZE]; // 动捕
	V4 GRotators[GLOVE_SIZE]; // 手套
	FaceInfo stFaceInf;
	//float GFaceData[FACE_SIZE]; // 面捕
};

struct MultiTracker
{
	int timestamp;
	std::vector<TrackerInfo> vtracker;
	std::map<MFGKey, MFGValue> mapMFG;
};


struct VideoData
{
	uint32 * pPicContent;
	int nPicSize;
	std::string strTimeStamp;
};


struct CameraDataWithDepthField
{
	int nNodeID;
	TransformData stTransData;
	float fDepthField;
};

struct Position
{
	float x;
	float y;
	float z;
};
struct Rotation
{
	float w;
	float x;
	float y;
	float z;
};

struct PositionRotation
{
	Position P;
	Rotation R;
	std::string strTimeStamp;
};

struct FocusPannel
{
	float fSensorWidth;
	float fSensorHeight;
	float sPannelDistance;
};


struct CameraParam_EX
{
	float fSensorWidth; // sensor宽
	float fSensorHigh; // senson高
	float fPanelDistance; // 面板距离
	float fFocalDistance; // 焦距
};

struct VideoData_EX
{
	uint32 * pPicContent;
	int nPicSize;
	char szTimeStamp[TIMESTAMP];
	char szTimeCode[TIMESTAMP];
};

struct Refresh_EX // 留着以后扩展
{
	char szTimeCode[TIMESTAMP];
};

class FrameData
{
public:
	FrameData();
	~FrameData();

	bool SetMemoryData(MultiTracker stMultiTracker, VideoData_EX stVideoData, CameraParam_EX stCameraParam, CameraDataWithDepthField stCameraDataWithDepthField);
	bool GetMemoryData(MultiTracker &stMultiTracker, VideoData_EX &stVideoData, CameraParam_EX &stCameraParam, CameraDataWithDepthField &stCameraDataWithDepthField);

	bool GetMultiTracker(MultiTracker &stMultiTracker);
	bool GetVideoData(VideoData_EX &stVideoData);
	bool GetCameraParam(CameraParam_EX &stCameraParam);
	bool GetCameraDataWithDepthField(CameraDataWithDepthField &stCameraDataWithDepthField);
public:
	std::mutex m_mtxLock;
	MultiTracker m_stMultiTracker;  // 除相机以外的定位件的定位数
	VideoData_EX m_stVideoData;     // 视频的相关数据
	CameraParam_EX m_stCameraParam; // 相机参数 
	CameraDataWithDepthField m_stCameraDataWithDepthField;	// 相机景深相关
};

class VIRTUALSHOOTINGDLL_API CVirtualShootingDll
{
public:
	CVirtualShootingDll();

	bool GetTimeCode(Refresh_EX &stRefreshData);

	bool GetFrameData();

	bool GetMultiTracker(MultiTracker &stMultiTracker);
	bool GetVideoData(VideoData_EX &stVideoData);
	bool GetCameraParam(CameraParam_EX &stCameraParam);
	bool GetCameraDataWithDepthField(CameraDataWithDepthField &stCameraDataWithDepthField);

	void ReleaseFrameData();
protected:
	static FrameData m_pFrameData;
private:
	bool m_bInit;
	std::mutex m_mtxLock;
};