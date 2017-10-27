
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_common.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "dynamicmask_object.h"

#define DYNAMIC_MASK_SHADER_FRAGMENT		"\\GLSL\\dynamicMask.fsh"
#define DYNAMIC_MASK_SHADER_VERTEX			"\\GLSL\\simple.vsh"

#define DYNAMIC_MASK_BLUR_FRAGMENT			"\\GLSL\\blur2.fsh"
#define DYNAMIC_MASK_BLUR_VERTEX			"\\GLSL\\simple.vsh"

///////////////////////////////////////////

namespace DYNAMIC_MASK_OPERATIONS
{

	void				ChooseMask(ObjectMask *pNewMask);
	void				ChooseMaskByName(const char *name);
	bool				MakeSomeMaskCurrent(void);
	void				ChooseChannel(const int index); // index of a channel in mask components
	void				ChooseShape(const int shape);	// index of a shape in a channel components

	ObjectMask			*GetCurrentMaskPtr();
	ObjectChannel		*GetCurrentChannelPtr();
	const int			GetCurrentChannelIndex();
	ObjectShape			*GetCurrentShapePtr();

	ObjectMask			*AddMask(const char *name="mask");
	ObjectChannel		*AddChannel(ObjectMask *mask, const char *name="channel");
	ObjectShape			*AddShape(ObjectChannel *channel, const char *name="shape");
	ObjectKnot			*AddKnot(ObjectShape *shape, FBVector3d position, const ObjectKnotType &type, const char *name="knot");
	// insert a new knot from the specified knot
	ObjectKnot			*InsertKnot(ObjectShape *shape, ObjectKnot *knot, FBVector3d position, const char *name="knot");
	void				RemoveMask(ObjectMask *mask);
	void				RemoveAllMasks();
	void				RemoveUnusedMasks();
	ObjectMask			*DublicateMask(ObjectMask *mask);
	void				SelectMask(ObjectMask *mask);
	void				SelectMaskResources(ObjectMask *mask);
	void				RegenerateMask(ObjectMask *mask);
	void				RenameMask(ObjectMask *mask, const char *newname);

	void				SplitEdge();
	void				RemoveVertices(bool &needUpdateUI);
	void				RemoveShape(ObjectShape *shape);
	void				KeyAll();
	void				KeySelected();
	bool				GoToNextKey();
	bool				GoToPrevKey();
	void				KeyRemove();
	void				ClearAnimation();

	void				ChangeInterpolationType( const int type );

	void		SetAutoKey(const bool mode);
	bool		IsAutoKey();

};