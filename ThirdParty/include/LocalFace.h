#pragma once
#include <iostream>
#ifdef FACE_EXPORT
#define FACE_API __declspec(dllexport)
#else
#define FACE_API __declspec(dllimport)
#endif

using namespace std;

namespace GCWT
{


	FACE_API class LocalFace
	{
	public:
		FACE_API LocalFace();
		FACE_API ~LocalFace();
		FACE_API string GetFaceStr(string ip);

	private:
		bool isListen;
	};


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
	};


	FACE_API class LocalGlove
	{
	public:
		FACE_API LocalGlove();
		FACE_API ~LocalGlove();
		FACE_API void GetGloveData(transform* data,string ip);

	private:
		bool isListen;

	};
}
