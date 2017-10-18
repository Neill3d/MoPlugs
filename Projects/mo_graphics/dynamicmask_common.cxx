
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_common.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "dynamicmask_common.h"
#include <GL\glew.h>
#include "shared_misc.h"


// global variables

bool				gAutoKey = false;

ObjectMask			*gCurrentMask = nullptr;
ObjectChannel		*gCurrentChannel = nullptr;
ObjectShape			*gCurrentShape = nullptr;


HGLRC		gMainContext;

void SetMainContext(HGLRC context)
{
	gMainContext = context;
}

HGLRC GetMainContext()
{
	return gMainContext;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace DYNAMIC_MASK_OPERATIONS
{

void SetAutoKey(const bool mode)
{
	gAutoKey = mode;
}

bool IsAutoKey()
{
	return gAutoKey;
}

ObjectMask	*GetCurrentMaskPtr()
{
	return gCurrentMask;
}

ObjectChannel *GetCurrentChannelPtr()
{
	return gCurrentChannel;
}

const int GetCurrentChannelIndex()
{
	if (gCurrentMask != nullptr && gCurrentChannel != nullptr)
	{
		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
			if (gCurrentMask->Components[i] == gCurrentChannel)
				return i;
	}

	return 0;
}

ObjectShape *GetCurrentShapePtr()
{
	return gCurrentShape;
}

void ClearCurrentMaskAssignment()
{
	gCurrentMask = nullptr;
	gCurrentChannel = nullptr;
	gCurrentShape = nullptr;
}

bool MakeSomeMaskCurrent(void)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0; i<pScene->UserObjects.GetCount(); ++i)
		if (pScene->UserObjects[i]->Is( ObjectMask::TypeInfo ) )
		{
			ChooseMask( (ObjectMask*) pScene->UserObjects[i]);
			return true;
		}
	return false;
}

void ChooseMaskByName(const char *name)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0; i<pScene->UserObjects.GetCount(); ++i)
		if ( FBIS(pScene->UserObjects[i], ObjectMask) && strcmp(pScene->UserObjects[i]->Name, name) == 0 )
		{
			ChooseMask( (ObjectMask*) pScene->UserObjects[i]);
			break;
		}
}

void ChooseMask(ObjectMask *pNewMask)
{
	gCurrentMask = pNewMask;
	ChooseChannel(0);

}

void ChooseChannel(const int index)
{
	gCurrentChannel = nullptr;

	if (gCurrentMask)
	{
		if (gCurrentMask->Components.GetCount() && index < gCurrentMask->Components.GetCount() && index >= 0 )
			gCurrentChannel = (ObjectChannel*) gCurrentMask->Components[index];
	}
	ChooseShape(-1);
}

void ChooseShape(const int index)
{
	gCurrentShape = nullptr;

	if (gCurrentChannel)
	{
		if (gCurrentChannel->Components.GetCount() && index < gCurrentChannel->Components.GetCount() && index >= 0 )
			gCurrentShape = (ObjectShape*) gCurrentChannel->Components[index];
	}
}

void AddMaskResources(ObjectMask *newMask)
{
	FBFolder *pFolderTextures = nullptr;
	FBFolder *pFolderVideos = nullptr;

	FBScene *pScene = FBSystem::TheOne().Scene;
	const int numberOfFolders = pScene->Folders.GetCount();
	for (int i=0; i<numberOfFolders; ++i)
	{
		if (strcmp(pScene->Folders[i]->Name, "Dynamic Textures") == 0)
			pFolderTextures = pScene->Folders[i];
		else
		if (strcmp(pScene->Folders[i]->Name, "Dynamic Videos") == 0)
			pFolderVideos = pScene->Folders[i];

		//
		if (pFolderTextures && pFolderVideos)
			break;
	}
		
	const char *name = newMask->Name;
		
	FBTexture *pTexture = new FBTexture( name );
	FBVideoMemory *pVideo = new FBVideoMemory( name );

	if (pFolderTextures == nullptr)
		pFolderTextures = new FBFolder( "Dynamic Textures", pTexture );
	else
		pFolderTextures->Items.Add( pTexture );

	if (pFolderVideos == nullptr)
		pFolderVideos = new FBFolder( "Dynamic Videos", pVideo );
	else
		pFolderVideos->Items.Add(pVideo);

	pVideo->TextureOGLId = 0;
	pVideo->SetObjectImageSize( 512, 512 );

	pTexture->Video = pVideo;
	newMask->Video.Clear();
	newMask->Video.Add( pVideo );
}

ObjectMask *AddMask(const char *name)
{
	ObjectMask *newMask = (ObjectMask*) FBCreateObject( "FbxStorable/User", 
																"ObjectMask", 
																name ) ;
	if (newMask) {

		// add channels to this mask
		AddChannel(newMask, FBString(name, "_R") );
		AddChannel(newMask, FBString(name, "_G") );
		AddChannel(newMask, FBString(name, "_B") );
		AddChannel(newMask, FBString(name, "_A") );
		
		// add a new texture and memory video clip and connect them

		AddMaskResources(newMask);
	}

	return newMask;
}

ObjectChannel *AddChannel(ObjectMask *mask, const char *name)
{
	ObjectChannel *newChannel = (ObjectChannel*) FBCreateObject( "FbxStorable/User", 
																"ObjectChannel", 
																name ) ;

	if (newChannel)
	{
		mask->Components.Add( newChannel );
		newChannel->Parents.Add(mask);
	}
	return newChannel;
}

ObjectShape *AddShape(ObjectChannel *channel, const char *name)
{
	if (channel)
	{
		ObjectShape *newShape = (ObjectShape*) FBCreateObject( "FbxStorable/User", 
																	"ObjectShape", 
																	name ) ;
		if (newShape)
		{
			channel->Components.Add( newShape );
			newShape->Parents.Add(channel);
		}
		return newShape;
	}
	else
	{
		return nullptr;
	}
}

ObjectKnot *AddKnot(ObjectShape *shape, FBVector3d position, const ObjectKnotType &type, const char *name)
{
	if (shape)
	{
		ObjectKnot *newKnot = (ObjectKnot*) FBCreateObject( "FbxStorable/User", 
																	"ObjectKnot", 
																	name ) ;

		if (newKnot)
		{
			shape->Components.Add( newKnot );
			newKnot->Parents.Add(shape);
			newKnot->Position.SetData(position);
			newKnot->Type = type;
		}
		return newKnot;
	}
	else
	{
		return nullptr;
	}
}

ObjectKnot *InsertKnot(ObjectShape *shape, ObjectKnot *knot, FBVector3d position, const char *name)
{
	if (shape)
	{
		ObjectKnot *newKnot = (ObjectKnot*) FBCreateObject( "FbxStorable/User", 
																	"ObjectKnot", 
																	name ) ;

		if (newKnot)
		{
			
			std::vector <ObjectKnot*> knots;
			knots.resize(shape->Components.GetCount() );
			for (int i=0; i<shape->Components.GetCount(); ++i)
				knots[i] = (ObjectKnot*) shape->Components[i];

			shape->Components.RemoveAll();
			for (int i=0; i<knots.size(); ++i)
			{
				shape->Components.Add(knots[i]);
				if (knots[i] == knot) shape->Components.Add(newKnot);
			}

			newKnot->Parents.Add(shape);
			newKnot->Position.SetData(position);
		}
		return newKnot;
	}
	else
	{
		return nullptr;
	}
}

void RemoveKnots(ObjectShape *shape)
{
	int count = shape->Components.GetCount();
	for (int i=count-1; i>=0; i--)
	{
		FBComponent *pComp = shape->Components[i];
		pComp->FBDelete();
		pComp = nullptr;
	}
}

void RemoveShapes(ObjectChannel *channel)
{
	for (int i=channel->Components.GetCount()-1; i>=0; i--)
	{
		FBComponent *pComp = channel->Components[i];

		// remove shapes for this channel
		RemoveKnots( (ObjectShape*) pComp);

		pComp->FBDelete();
		pComp = nullptr;
	}
}

void RemoveChannels(ObjectMask *mask)
{
	for (int i=mask->Components.GetCount()-1; i>=0; i--)
	{
		FBComponent *pComp = mask->Components[i];

		// remove shapes for this channel
		RemoveShapes( (ObjectChannel*) pComp);

		pComp->FBDelete();
		pComp = nullptr;
	}
}

void RemoveMaskResources(ObjectMask *mask)
{
	if (mask->Video.GetCount() > 0)
	{
		FBVideo *pVideo = (FBVideo*) mask->Video.GetAt(0);

		int count = pVideo->GetDstCount();
		for (int i=count-1; i>=0; --i)
			if ( FBIS(pVideo->GetDst(i), FBTexture) )
			{
				FBTexture *pTexture = (FBTexture*) pVideo->GetDst(i);
				pTexture->FBDelete();
			}

		pVideo->FBDelete();
		mask->Video.Clear();
	}
}

void RemoveMask(ObjectMask *mask)
{
	if (mask)
	{
		// remove mask components (channels, shapes, knots)
		RemoveChannels(mask);

		if (mask == gCurrentMask) ClearCurrentMaskAssignment();

		// remove dynamic texture and video if exist
		RemoveMaskResources(mask);	

		mask->FBDelete();
		mask = nullptr;

		MakeSomeMaskCurrent();
	}
}

void RemoveAllMasks()
{
	std::vector<ObjectMask*>		masks;

	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0; i<pScene->UserObjects.GetCount(); ++i)
		if ( FBIS(pScene->UserObjects[i], ObjectMask) )
		{
			masks.push_back( (ObjectMask*) pScene->UserObjects[i] );
		}

	//
	for (int i=0; i<masks.size(); ++i)
	{
		RemoveMask(masks[i]);
	}
}

// TODO: this function doesn't work (texture dst connections if FBScene and FBFolder classes, there is no any material or model... don't know why)
bool IsMaskInUse(ObjectMask *pMask)
{
	bool inUse = false;

	if (pMask->Video.GetCount() > 0)
	{
		FBVideo *pVideo = (FBVideo*) pMask->Video.GetAt(0);

		int count = pVideo->GetDstCount();
		for (int i=0; i<count; ++i)
			if ( FBIS(pVideo->GetDst(i), FBTexture) )
			{
				FBTexture *pTexture = (FBTexture*) pVideo->GetDst(i);
				
				int texCount = pTexture->GetDstCount();
				for (int j=0; j<texCount; ++j)
				{
					FBString name (pTexture->GetDst(j)->ClassName() );
					if (FBIS(pTexture->GetDst(j), FBMaterial) || FBIS(pTexture->GetDst(j), FBModel) )
					{
						inUse = true;
						break;
					}
				}
			}

	}

	return inUse;
}

void RemoveUnusedMasks()
{
	std::vector<ObjectMask*>		masks;

	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0; i<pScene->UserObjects.GetCount(); ++i)
	{
		FBUserObject *pUserObject = pScene->UserObjects[i];
		if ( FBIS(pUserObject, ObjectMask) && IsMaskInUse( (ObjectMask*) pUserObject ) == false )
		{
			masks.push_back( (ObjectMask*) pUserObject );
		}
	}

	/*
	//
	for (int i=0; i<masks.GetCount(); ++i)
	{
		RemoveMask(masks[i]);
	}
	*/
}

ObjectMask *DublicateMask(ObjectMask *mask)
{
	return nullptr;
}

void SelectComponents(FBComponent *pComp)
{
	pComp->Selected = true;

	for (int i=0; i<pComp->Components.GetCount(); ++i)
		SelectComponents(pComp->Components[i]);
}

void SelectMask(ObjectMask *mask)
{
	SelectComponents(mask);
}

void SelectMaskResources(ObjectMask *mask)
{
	if (mask->Video.GetCount() > 0)
	{
		FBVideo *pVideo = (FBVideo*) mask->Video.GetAt(0);

		int count = pVideo->GetDstCount();
		for (int i=0; i<count; ++i)
		{
			if (FBIS(pVideo->GetDst(i), FBTexture))
			{
				FBTexture *pTexture = (FBTexture*) pVideo->GetDst(i);
				pTexture->Selected = true;
			}
		}
	}
}

void RegenerateMask(ObjectMask *mask)
{
	RemoveMaskResources(mask);
	AddMaskResources(mask);
}

void RenameMask(ObjectMask *mask, const char *newname)
{
	if (mask && (newname != nullptr) && (strlen(newname) > 0) )
	{
		mask->Name = newname;
		for (int i=0; i<mask->Components.GetCount(); ++i)
		{
			switch(i)
			{
			case 0: mask->Components[i]->Name = FBString(newname, "_R");
				break;
			case 1: mask->Components[i]->Name = FBString(newname, "_G");
				break;
			case 2: mask->Components[i]->Name = FBString(newname, "_B");
				break;
			case 3: mask->Components[i]->Name = FBString(newname, "_A");
				break;
			}
		}
	}
}


void SplitEdge()
{
	if (gCurrentChannel)
	{
		for (int i=0; i<gCurrentChannel->Components.GetCount(); ++i)
		{
			ObjectShape *shape = (ObjectShape*) gCurrentChannel->Components[i];

			FBArrayTemplate<ObjectKnot*> knots;
			knots.SetCount(shape->Components.GetCount());
			for (int j=0; j<shape->Components.GetCount(); ++j)
				knots[j] = (ObjectKnot*) shape->Components[j];

			for (int j=0; j<knots.GetCount(); ++j)
			{
				ObjectKnot *pKnot = knots[j];
				ObjectKnot *pNextKnot = nullptr;
				
				if (j == knots.GetCount()-1) pNextKnot = knots[0];
				else pNextKnot = knots[j+1];

				FBVector4d v1, v2;
				FBVector3d pos;
				FBMatrix m;

				pKnot->Position.GetData(v1, sizeof(double) * 3);
				pNextKnot->Position.GetData(v2, sizeof(double) * 3);
				m = shape->GetMatrix();

				FBVectorMatrixMult( v1, m, v1 );
				FBVectorMatrixMult( v2, m, v2 );

				VectorCenter(v1, v2, pos);

				FBMatrixInverse(m, m);
				v1 = FBVector4d(pos[0], pos[1], pos[2], 1.0);
				FBVectorMatrixMult(v1, m, v1);
				pos = FBVector3d(v1[0], v1[1], v1[2]);

				if (pKnot->Selected && pNextKnot->Selected)
				{
					// calculate center position between two knots

					InsertKnot(shape, pKnot, pos);
				}

			}
		}
	}
}

// TODO: check if vertices is less or equal to 2, then remove shape
void RemoveVertices(bool &needUpdateUI)
{
	needUpdateUI = false;

	if (gCurrentChannel)
	{
		for (int i=0; i<gCurrentChannel->Components.GetCount(); ++i)
		{
			ObjectShape *shape = (ObjectShape*) gCurrentChannel->Components[i];

			FBArrayTemplate<ObjectKnot*> knots;
			knots.SetCount(shape->Components.GetCount());
			for (int j=0; j<shape->Components.GetCount(); ++j)
				knots[j] = (ObjectKnot*) shape->Components[j];


			for (int j=0; j<knots.GetCount(); ++j)
			{
				if (knots[j]->Selected)
				{
					shape->Components.Remove(knots[j]);
					knots[j]->FBDelete();
				}
			}

			if (shape->Components.GetCount() <= 2)
			{
				gCurrentChannel->Components.Remove(shape);
				shape->FBDelete();

				needUpdateUI = true;
			}
		}
	}
}

void RemoveShape(ObjectShape *shape)
{
	if (shape == nullptr) return;

	int parentsCount = shape->Parents.GetCount();
	for (int i=0; i<parentsCount; ++i)
		shape->Parents[i]->Components.Remove(shape);

	RemoveKnots(shape);
			
	shape->FBDelete();
}

bool IsAnyKnotSelected(ObjectChannel *pChannel)
{
	if (pChannel == nullptr)
		return false;

	for (int i=0; i<pChannel->Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) pChannel->Components[i];

		for (int j=0; j<shape->Components.GetCount(); ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];
			if (pKnot->Selected)
				return true;
		}
	}	

	return false;
}

void KeyAll(ObjectChannel *pChannel)
{
	if (pChannel == nullptr)
		return;

	for (int i=0; i<pChannel->Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) pChannel->Components[i];

		for (int j=0; j<shape->Components.GetCount(); ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];
			pKnot->Position.Key();
		}
	}
}

void KeyAll()
{
	if (gCurrentChannel)
	{
		KeyAll(gCurrentChannel);	
	}
	else if (gCurrentMask)
	{
		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
			KeyAll( dynamic_cast<ObjectChannel*> (gCurrentMask->Components[i]) );
	}
}

void KeySelected(ObjectChannel *pChannel)
{
	if (pChannel == nullptr)
		return;

	for (int i=0; i<pChannel->Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) pChannel->Components[i];

		for (int j=0; j<shape->Components.GetCount(); ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];
			if (pKnot->Selected)
				pKnot->Position.Key();
		}
	}
}

void KeySelected()
{
	if (gCurrentChannel)
	{
		KeySelected(gCurrentChannel);	
	}
	else if (gCurrentMask)
	{
		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
			KeySelected( dynamic_cast<ObjectChannel*> (gCurrentMask->Components[i]) );
	}
}

void ChangeInterpolationType( ObjectChannel *pChannel, const int type )
{
	if (pChannel == nullptr)
		return;

	for (int i=0; i<pChannel->Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) pChannel->Components[i];

		for (int j=0; j<shape->Components.GetCount(); ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];
			if (pKnot->Selected)
				pKnot->Type = (ObjectKnotType) type;
		}
	}
}

void ChangeInterpolationType( const int type )
{
	if (gCurrentChannel)
	{
		ChangeInterpolationType(gCurrentChannel, type);	
	}
	else if (gCurrentMask)
	{
		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
			ChangeInterpolationType( dynamic_cast<ObjectChannel*> (gCurrentMask->Components[i]), type );
	}
}

bool GoToNextKey(ObjectChannel *pChannel, const bool anyKnotSelected, const FBTime &currTime, FBTime &nextTime)
{
	if (pChannel == nullptr)
		return false;

	for (int i=0; i<pChannel->Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) pChannel->Components[i];

		for (int j=0; j<shape->Components.GetCount(); ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];
				
			if (anyKnotSelected == false || (anyKnotSelected==true && pKnot->Selected) )
			{
				FBAnimationNode *pNode = pKnot->Position.GetAnimationNode();
				if (pNode)
				{
					for (int j=0; j<pNode->Nodes.GetCount(); ++j)
					{
						FBFCurve *pCurve = pNode->Nodes[j]->FCurve;

						if (pCurve)
						{
							FBTime lTime;
							for (int i=0; i<pCurve->Keys.GetCount(); ++i)
							{
								lTime = pCurve->Keys[i].Time;
								if (lTime > currTime && lTime < nextTime)
								{
									nextTime = lTime;
									break;
								}
							}
						}
					}
				}
			}
				
		}
	}

	return true;
}

bool GoToPrevKey(ObjectChannel *pChannel, const bool anyKnotSelected, const FBTime &currTime, FBTime &prevTime)
{
	if (pChannel == nullptr)
		return false;

	for (int i=0; i<pChannel->Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) pChannel->Components[i];

		for (int j=0; j<shape->Components.GetCount(); ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];
				
			if (anyKnotSelected == false || (anyKnotSelected==true && pKnot->Selected) )
			{
				FBAnimationNode *pNode = pKnot->Position.GetAnimationNode();
				if (pNode)
				{
					for (int j=0; j<pNode->Nodes.GetCount(); ++j)
					{
						FBFCurve *pCurve = pNode->Nodes[j]->FCurve;

						if (pCurve)
						{
							FBTime lTime;
							const int numberOfKeys = pCurve->Keys.GetCount();
							for (int i=numberOfKeys-1; i>=0; --i)
							{
								lTime = pCurve->Keys[i].Time;
								if (lTime < currTime && lTime > prevTime)
								{
									prevTime = lTime;
									break;
								}
							}
						}
					}
				}
			}
				
		}
	}

	return true;
}

bool GoToNextKey()
{
	FBTime currTime = FBSystem::TheOne().LocalTime;
	FBTime nextTime = FBTime::Infinity;

	if (gCurrentChannel)
	{
		const bool anyKnotSelected = IsAnyKnotSelected(gCurrentChannel);
		GoToNextKey(gCurrentChannel, anyKnotSelected, currTime, nextTime);
	}
	else if (gCurrentMask)
	{
		bool anyKnotSelected = false;

		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
			anyKnotSelected |= IsAnyKnotSelected( (ObjectChannel*) gCurrentMask->Components[i] );

		FBTime ltime = FBTime::Infinity;
		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
		{
			GoToNextKey( (ObjectChannel*)gCurrentMask->Components[i], anyKnotSelected, currTime, ltime );
			if (ltime < nextTime)
				nextTime = ltime;
		}
	}

	if (nextTime != FBTime::Infinity)
	{
		FBPlayerControl::TheOne().Goto( nextTime );
		return true;
	}

	return false;
}

bool GoToPrevKey()
{
	FBTime currTime = FBSystem::TheOne().LocalTime;
	FBTime prevTime = FBTime::MinusInfinity;

	if (gCurrentChannel)
	{
		const bool anyKnotSelected = IsAnyKnotSelected(gCurrentChannel);
		GoToPrevKey(gCurrentChannel, anyKnotSelected, currTime, prevTime);
	}
	else if (gCurrentMask)
	{
		bool anyKnotSelected = false;

		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
			anyKnotSelected |= IsAnyKnotSelected( (ObjectChannel*) gCurrentMask->Components[i] );

		FBTime ltime = FBTime::MinusInfinity;
		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
		{
			GoToPrevKey( (ObjectChannel*)gCurrentMask->Components[i], anyKnotSelected, currTime, ltime );
			if (ltime > prevTime)
				prevTime = ltime;
		}
	}

	if (prevTime != FBTime::MinusInfinity)
	{
		FBPlayerControl::TheOne().Goto( prevTime );
		return true;
	}

	return false;
}

void KeyRemove(ObjectChannel *pChannel, const bool anyKnotSelected, const FBTime &currTime)
{
	if (pChannel == nullptr)
		return;

	for (int i=0; i<pChannel->Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) pChannel->Components[i];

		for (int j=0; j<shape->Components.GetCount(); ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];
				
			if (anyKnotSelected == false || (anyKnotSelected==true && pKnot->Selected) )
					pKnot->Position.KeyRemoveAt(currTime);
		}
	}
}

void KeyRemove()
{
	FBTime currTime = FBSystem::TheOne().LocalTime;

	if (gCurrentChannel)
	{
		const bool anyKnotSelected = IsAnyKnotSelected(gCurrentChannel);
		KeyRemove(gCurrentChannel, anyKnotSelected, currTime);
	}
	else if (gCurrentMask)
	{
		bool anyKnotSelected = false;

		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
			anyKnotSelected |= IsAnyKnotSelected( (ObjectChannel*) gCurrentMask->Components[i] );

		FBTime ltime = FBTime::MinusInfinity;
		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
		{
			KeyRemove( (ObjectChannel*)gCurrentMask->Components[i], anyKnotSelected, currTime );
		}
	}
}

void ClearAnimation(ObjectChannel *pChannel, const bool anyKnotSelected)
{
	if (pChannel == nullptr)
		return;

	for (int i=0; i<pChannel->Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) pChannel->Components[i];

		for (int j=0; j<shape->Components.GetCount(); ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];

			if (anyKnotSelected == false || (anyKnotSelected==true && pKnot->Selected) )
					pKnot->Position.SetAnimated(false);
		}
	}
}

void ClearAnimation()
{
	if (gCurrentChannel)
	{
		const bool anyKnotSelected = IsAnyKnotSelected(gCurrentChannel);
		ClearAnimation(gCurrentChannel, anyKnotSelected);
	}
	else if (gCurrentMask)
	{
		bool anyKnotSelected = false;

		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
			anyKnotSelected |= IsAnyKnotSelected( (ObjectChannel*) gCurrentMask->Components[i] );

		FBTime ltime = FBTime::MinusInfinity;
		for (int i=0; i<gCurrentMask->Components.GetCount(); ++i)
		{
			ClearAnimation( (ObjectChannel*)gCurrentMask->Components[i], anyKnotSelected );
		}
	}
}

}