
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Common.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Common.h"

#include <vector>
#include <algorithm>
#include <random>

#include "orcommand_pack_command.h"
#include "perlin.h"
#include "algorithm\math3d_mobu.h"

std::vector<FBCommand*>		gCommands;

FBColor	gCommandColors[] = {
	FBColor(1.0, 1.0, 1.0),
	FBColor(0.0, 0.0, 0.0),
	FBColor(1.0, 0.0, 0.0),
	FBColor(0.0, 1.0, 0.0),
	FBColor(0.0, 0.0, 1.0),
	FBColor(1.0, 1.0, 0.0),
	FBColor(0.0, 1.0, 1.0),
	FBColor(1.0, 0.0, 1.0)
};

std::random_device		rd;
std::mt19937			rng(rd());	// random-number engine used

char	gFileName[256] = "";

//////////////////////////////////////////////////////////////

void	OpenNextFile(const char *filename)
{
	memset(gFileName, 0, sizeof(char) * 256);
	sprintf_s( gFileName, sizeof(char)*256, "%s", filename );
}

const char *GetNextFile()
{
	return gFileName;
}

void	NextFileOpened()
{
	memset(gFileName, 0, sizeof(char) * 256);
}

void InitCommon()
{

}

void RegisterCommand(FBCommand *pcommand)
{
	gCommands.push_back(pcommand);
}

void UnRegisterCommand(FBCommand *pcommand)
{
	auto iter = gCommands.begin();

	while( iter!=gCommands.end() )
	{
		if (*iter == pcommand)
			break;

		++iter;
	}

	if (iter!=gCommands.end() )
		gCommands.erase(iter);
}

FBCommand *IsClipInUse(FBStoryClip *pclip)
{
	for (int i=0; i<gCommands.size(); ++i)
	{
		FBCommand *pcommand = gCommands[i];
		if (FBIS(pcommand, ORCommand_Color) )
		{
			ORCommand_Color *pcolor = (ORCommand_Color*) pcommand;
			if (pcolor->GetClipPtr() == pclip)
				return pcommand;
		}
	}

	return nullptr;
}

FBColor &GetCommandColor(const int mode)
{
	return gCommandColors[mode];
}

void CheckCommandReferences()
{
	for (int i=0; i<gCommands.size(); ++i)
	{
		FBCommand *pcommand = gCommands[i];
		
		if ( FBIS(pcommand, ORCommand_Base) )
		{
			ORCommand_Base *pbase = (ORCommand_Base*) pcommand;
			pbase->UpdateReferences();
		}
	}
}

///////////////////////////////////////////////

FBTime FindFirstCommandTime(const double key[3])
{
	FBTime minTime(FBTime::Infinity);

	for (int i=0; i<gCommands.size(); ++i)
	{
		FBCommand *pcommand = gCommands[i];
		FBTime starttime(pcommand->StartTime);
		if (FBIS(pcommand, ORCommand_Color) && starttime < minTime )
		{
			ORCommand_Color *pcolor = (ORCommand_Color*) pcommand;
			
			int mode = pcolor->ColorMode.AsInt();
			FBColor &value = gCommandColors[mode];

			if (value[0]==key[0] && value[1]==key[1] && value[2]==key[2])
			{
				minTime = starttime;
			}
		}
	}

	return minTime;
}

FBTime FindPrevCommandTime(const FBTime &localtime, const double key[3])
{
	FBTime minTime(FBTime::MinusInfinity);

	for (int i=0; i<gCommands.size(); ++i)
	{
		FBCommand *pcommand = gCommands[i];
		FBTime starttime(pcommand->StartTime);
		if (FBIS(pcommand, ORCommand_Color) && starttime > minTime && starttime < localtime )
		{
			ORCommand_Color *pcolor = (ORCommand_Color*) pcommand;
			
			int mode = pcolor->ColorMode.AsInt();
			FBColor &value = gCommandColors[mode];

			if (value[0]==key[0] && value[1]==key[1] && value[2]==key[2])
			{
				minTime = starttime;
			}
		}
	}

	return minTime;
}

FBTime FindAnyPrevCommandTime(const FBTime &localtime)
{
	FBTime minTime(FBTime::MinusInfinity);

	for (int i=0; i<gCommands.size(); ++i)
	{
		FBCommand *pcommand = gCommands[i];
		FBTime starttime(pcommand->StartTime);
		if (FBIS(pcommand, ORCommand_Color) && starttime > minTime && starttime < localtime )
		{
			minTime = starttime;	
		}
	}

	return minTime;
}

struct less_than_key
{
	inline bool operator() (const ORCommand_Color *color1, const ORCommand_Color *color2)
	{
		FBTime time1 = color1->StartTime;
		FBTime time2 = color2->StartTime;
		return (time1 < time2);
	}
};

FBTime FindNextCommandTime(const FBTime &localtime, const double key[3], const int step, const bool dorandom)
{
	
	// take a vector of all next colored markers
	// sort them if step is used

	FBTime	minTime(FBTime::Infinity);
	ORCommand_Color *pCurrCommand = nullptr;
	bool currSameColor = false;

	std::vector<ORCommand_Color*>	nextColors;

	for (int i=0; i<gCommands.size(); ++i)
	{
		FBCommand *pcommand = gCommands[i];
		FBTime starttime(pcommand->StartTime);
		if (FBIS(pcommand, ORCommand_Color) )
		{
			ORCommand_Color *pcolor = (ORCommand_Color*) pcommand;
			
			if (minTime > starttime && starttime > localtime)
			{
				pCurrCommand = pcolor;
				minTime = starttime;
			}

			int mode = pcolor->ColorMode.AsInt();
			FBColor &value = gCommandColors[mode];

			if (value[0]==key[0] && value[1]==key[1] && value[2]==key[2])
			{
				nextColors.push_back(pcolor);

				if (pCurrCommand == pcolor) currSameColor = true;
			}
			else if (pCurrCommand == pcolor) currSameColor = false;
		}
	}

	if (nextColors.size() == 0)
		return FBTime::Infinity;

	int localstep = step;

	if (nextColors.size() > 1 && dorandom == false)
	{
		std::sort(nextColors.begin(), nextColors.end(), less_than_key() );

		if (pCurrCommand != nullptr && currSameColor)
		{
			for (int i=0; i<nextColors.size(); ++i)
				if (nextColors[i] == pCurrCommand)
					localstep = i + step;
		}
	}
	
	if (localstep >= nextColors.size())
	{
		localstep = (int) nextColors.size() - 1;
	}
	
	if (dorandom == true)
	{
		std::uniform_int_distribution<int> uni(0, (int) nextColors.size()-1); // guaranteed unbiased
		int random = uni(rng);
		/*
		if (nextColors[random] == pCurrCommand)
			random += 1;
		if (random >= nextColors.size() )
			random = 0;
			*/
		return nextColors[random]->StartTime;
	}
	else
	{
		return nextColors[localstep]->StartTime;
	}

	return FBTime::Infinity;
}


//////////////////////////////////////////////

struct CameraShakeData
{
	double				amplitude;
	double				duration;
	double				frequency;

	double				lastTime;

	double				elapsed;

	bool				needReset;

	CameraShakeData()
	{
		amplitude = 0.0;
		duration = 0.0;
		frequency = 1.0;
		lastTime = 0.0;
		elapsed = 0.0;

		needReset = false;
	}
};

struct CameraZoomData
{
	double		lastTime;
	double		elapsed;
	double		duration;
	double		amplitude;
	bool		repeat;

	double		resetValue;
	bool		needReset;

	CameraZoomData()
	{
		lastTime = 0.0;
		elapsed = 0.0;
		duration = 0.0;
		amplitude = 40.0;
		repeat = false;

		resetValue = 40.0;
		needReset = false;
	}
};

CameraShakeData		gCameraShakeData;
CameraZoomData		gCameraZoomData;
Perlin				gPerlinNoise;

/*
private float amplitude = 10.0f;
private float adjustAmplitudePerSecond = -1.0f;
private float timeElapsed = 0.0f;

public void shake(float deltaTime) {
    timeElapsed += deltaTime;
    amplitude += adjustAmplitudePerSecond * deltaTime;
    yourCamera.y += + Math.sin(timeElapsed);
}

//////////////////////////

newCam = cam;
newCam.x += noise(cam.x/f,cam.y/f,cam.z/f)*amp;
newCam.y += noise(cam.x/f,cam.y/f+100,cam.z/f)*amp;
newCam.z += noise(cam.x/f,cam.y/f,cam.z/f+100)*amp;

*/



void shake(const double freq, const double adjAmp, const double deltaTime, double &amp, double &timeElapsed, double &value)
{
	timeElapsed += deltaTime;
	amp += adjAmp * deltaTime;
	value += sin(freq * timeElapsed);
}

float Clampf( const float value, const float minv, const float maxv )
{
	if (value < minv)
		return minv;
	if (value > maxv)
		return maxv;

	return value;
}

void InitCameraShake()
{
	gPerlinNoise.Init( 4, 2.0, 1.0, 12345 );
}

void RunCameraShake(const double amplitude, const double frequency, const double duration)
{
	gCameraShakeData.duration = duration;
	gCameraShakeData.elapsed = 0.0;
	gCameraShakeData.lastTime = 0.0;
	gCameraShakeData.amplitude = amplitude;
	gCameraShakeData.frequency = frequency;
}

void UpdateCameraShake(const double amplitude, const double frequency)
{
	gCameraShakeData.amplitude = amplitude;
	gCameraShakeData.frequency = frequency;
}

void	ContinueCameraShake()
{
	gCameraShakeData.elapsed = 0.0;
}

void RenderCameraShake(const double currTimeValue, FBCamera *pCamera)
{
	if (pCamera == nullptr)
		return;

	if (gCameraShakeData.elapsed < gCameraShakeData.duration)
	{
		if (gCameraShakeData.lastTime == 0.0)
			gCameraShakeData.lastTime = currTimeValue;

		double opticalX = pCamera->OpticalCenterX;
		double opticalY = pCamera->OpticalCenterY;

		gCameraShakeData.elapsed += (currTimeValue-gCameraShakeData.lastTime);

		float percentComplete = gCameraShakeData.elapsed / gCameraShakeData.duration;
		float damper = 1.0f - Clampf(4.0f * percentComplete - 3.0f, 0.0f, 1.0f);

		// map value to [-1, 1]
		float x = gPerlinNoise.Get(gCameraShakeData.frequency * currTimeValue + gCameraShakeData.frequency * 100.0);
		float y = gPerlinNoise.Get(gCameraShakeData.frequency * currTimeValue);

		x *= gCameraShakeData.amplitude * damper;
		y *= gCameraShakeData.amplitude * damper;

		pCamera->OpticalCenterX = opticalX + x;
		pCamera->OpticalCenterY = opticalY + y;

		gCameraShakeData.lastTime = currTimeValue;
		gCameraShakeData.needReset = true;
	}
	else if (gCameraShakeData.needReset)
	{
		pCamera->OpticalCenterX = 0.0;
		pCamera->OpticalCenterY = 0.0;

		gCameraShakeData.needReset = false;
	}
}

void RunCameraZoom(const double amplitude, const double duration, const bool repeat)
{
	gCameraZoomData.duration = duration;
	gCameraZoomData.amplitude = amplitude;
	gCameraZoomData.elapsed = 0.0;
	gCameraZoomData.lastTime = 0.0;
	gCameraZoomData.repeat = repeat;
}

void StopCameraZoom()
{
	gCameraZoomData.duration = 0.0;
	gCameraZoomData.repeat = false;
}

void UpdateCameraZoom(const double amplitude)
{
	gCameraZoomData.amplitude = amplitude;
}

void RenderCameraZoom(const double currTimeValue, FBCamera *pCamera)
{
	if (pCamera == nullptr)
		return;

	if (gCameraZoomData.repeat == true)
	{
		if (gCameraZoomData.lastTime == 0.0)
			gCameraZoomData.lastTime = currTimeValue;

		if (gCameraZoomData.needReset == false)
		{
			gCameraZoomData.resetValue = pCamera->FieldOfView;
		}

		pCamera->FieldOfView = gCameraZoomData.resetValue + gCameraZoomData.amplitude;

		gCameraZoomData.needReset = true;
	}
	else if (gCameraZoomData.elapsed < gCameraZoomData.duration)
	{
		if (gCameraZoomData.lastTime == 0.0)
			gCameraZoomData.lastTime = currTimeValue;

		if (gCameraZoomData.needReset == false)
		{
			gCameraZoomData.resetValue = pCamera->FieldOfView;
		}

		gCameraZoomData.elapsed += (currTimeValue-gCameraZoomData.lastTime);

		float percentComplete = gCameraZoomData.elapsed / gCameraZoomData.duration;
		//float damper = 1.0f - Clampf(4.0f * percentComplete - 3.0f, 0.0f, 1.0f);

		// map value to [-1, 1]
		double value = 0.0;

		if (percentComplete < 0.25)
		{
			value = smoothstep( 0.0, 0.25, percentComplete );
		}
		else
		{
			value = 1.0 - smoothstep(0.25, 1.0, percentComplete);
		}

		pCamera->FieldOfView = gCameraZoomData.resetValue + value * gCameraZoomData.amplitude;

		gCameraZoomData.lastTime = currTimeValue;
		gCameraZoomData.needReset = true;
	}
	else if (gCameraZoomData.needReset)
	{
		pCamera->FieldOfView = gCameraZoomData.resetValue;
		gCameraZoomData.needReset = false;
	}
}