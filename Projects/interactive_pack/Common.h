
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Common.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

void InitCommon();

void RegisterCommand(FBCommand *pcommand);
void UnRegisterCommand(FBCommand *pcommand);

FBCommand *IsClipInUse(FBStoryClip *pclip);

void CheckCommandReferences();

FBColor &GetCommandColor(const int mode);

// return FBTime::Infinity when nothing found !
FBTime FindFirstCommandTime(const double key[3]);
FBTime FindPrevCommandTime(const FBTime &localtime, const double key[3]);
FBTime FindAnyPrevCommandTime(const FBTime &localtime);
FBTime FindNextCommandTime(const FBTime &localtime, const double key[3], const int step, const bool dorandom);

void	InitCameraShake();
void	RunCameraShake(const double amplitude, const double frequency, const double duration);
void	ContinueCameraShake();
void	UpdateCameraShake(const double amplitude, const double frequency);
void	RenderCameraShake(const double currTimeValue, FBCamera *pCamera);

void	RunCameraZoom(const double amplitude, const double duration, const bool repeat);
void	StopCameraZoom();
void	UpdateCameraZoom(const double amplitude);
void	RenderCameraZoom(const double currTimeValue, FBCamera *pCamera);

void	OpenNextFile(const char *filename);
const char *GetNextFile();
void	NextFileOpened();