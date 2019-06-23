
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_view.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <Windows.h>
#include <math.h>

#include "dynamicmask_view.h"
#include "dynamicmask_viewTools.h"
#include "dynamicmask_common.h"
#include "shared_misc_mobu.h"
#include "algorithm\math3d_mobu.h"
#include "graphics\OGL_Utils.h"
#include "graphics\checkGLError_mobu.h"

using namespace DYNAMICMASK;

FBClassImplementation( View2d );

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);


////////////////////////////////////////////////////////////////////////
//


void BaseGraphManipulator::DrawChannel(ObjectChannel *channel)
{
	if (channel == nullptr)
		return;

	// draw each shape
	for (int i=0; i<channel->Components.GetCount(); ++i)
	{
		FBComponent *pComp = channel->Components[i];
		FBVector2d	pos;

		ObjectShape *shape = (ObjectShape*) pComp;

		if (pComp->Components.GetCount() )
		{
			FBVector4d shapeSpacePos;
			shapeSpacePos[0] = worldPos[0];
			shapeSpacePos[1] = worldPos[1];
			shapeSpacePos[2] = worldPos[2];

			FBMatrix m = shape->GetMatrix();

			glPushMatrix();
			glMultMatrixd( m );

			FBMatrixInverse(m, m);
			FBVectorMatrixMult( shapeSpacePos, m, shapeSpacePos );

			//
			// draw unselected first

			glPointSize(2.0f);

			glBegin(GL_POINTS);

			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
				pKnot->Position.GetData(pos, sizeof(double)*2);

				if (false == pKnot->Selected)
				{
					//glPointSize(2.0f);
					glColor3d(0.5, 0.5, 0.5);
					glVertex2dv(pos);
				}

				// draw tangents on bezier mode
				if (pKnot->Type == eObjectKnotType_BEZIER)
				{
					if (false == pKnot->SelectedLeftTangent)
					{
						glColor3d(0.8, 0.8, 0.0);
						glVertex2dv(pKnot->incoming);
					}
					
					if (false == pKnot->SelectedRightTangent)
					{
						glColor3d(0.8, 0.8, 0.0);
						glVertex2dv(pKnot->outgoing);
					}
				}
			}

			glEnd();

			// draw selected points then

			glPointSize(5.0f);

			glBegin(GL_POINTS);

			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
				pKnot->Position.GetData(pos, sizeof(double)*2);

				if (pKnot->Selected)
				{
					glColor3d(0.2, 0.5, 0.2);
					glVertex2dv(pos);
				}
				
				// draw tangents on bezier mode
				if (pKnot->Type == eObjectKnotType_BEZIER)
				{	
					if (pKnot->SelectedLeftTangent)
					{
						glColor3d(0.2, 0.5, 0.2);	
						glVertex2dv(pKnot->incoming);
					}
					
					if (pKnot->SelectedRightTangent)
					{
						glColor3d(0.2, 0.5, 0.2);	
						glVertex2dv(pKnot->outgoing);
					}
				}
			}

			glEnd();

			// draw tangents connections
			glLineStipple(1, 0x3F07);
			glEnable(GL_LINE_STIPPLE);

			glColor3d(1.0, 1.0, 0.0);
			glBegin(GL_LINES);

			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
			
				// draw tangents on bezier mode
				if (pKnot->Type == eObjectKnotType_BEZIER)
				{
					pKnot->Position.GetData(pos, sizeof(double)*2);

					glVertex2dv(pos);
					glVertex2dv(pKnot->incoming);

					glVertex2dv(pos);
					glVertex2dv(pKnot->outgoing);
				}
			}

			glEnd();
			glDisable(GL_LINE_STIPPLE);

			glPopMatrix();
		}
	}
}

bool IsInside(double *pos, double *a, double *b)
{
	FBVector2d min, max;

	min[0] = (a[0] < b[0]) ? a[0] : b[0];
	min[1] = (a[1] < b[1]) ? a[1] : b[1];

	max[0] = (a[0] > b[0]) ? a[0] : b[0];
	max[1] = (a[1] > b[1]) ? a[1] : b[1];

	if (pos[0] >= min[0] && pos[0] <= max[0] && pos[1] >= min[1] && pos[1] <= max[1])
		return true;

	return false;
}

void BaseGraphManipulator::SelectChannel(ObjectChannel *channel, const bool selectPoints, const bool selectTangents, const bool singleClick, const double limit, const FBVector3d &startWorldPos)
{
	if (channel == nullptr)
		return;
	
	// draw each shape
	for (int i=0; i<channel->Components.GetCount(); ++i)
	{
		FBComponent *pComp = channel->Components[i];
		FBVector3d	pos;

		ObjectShape *shape = (ObjectShape*) pComp;

		if (pComp->Components.GetCount() )
		{
			FBVector4d worldSpacePos, worldSpaceLeft, worldSpaceRight;
			FBMatrix m = shape->GetMatrix();
						
			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
				pKnot->Position.GetData(pos, sizeof(double)*3);
				pKnot->Selected = false;
				pKnot->SelectedLeftTangent = false;
				pKnot->SelectedRightTangent = false;

				if (selectPoints)
				{
					worldSpacePos[0] = pos[0];
					worldSpacePos[1] = pos[1];
					worldSpacePos[2] = pos[2];
					FBVectorMatrixMult( worldSpacePos, m, worldSpacePos );

					if (singleClick)
					{
						if (VectorDistance(worldSpacePos, worldPos) < limit)
						{
							pKnot->Selected = true;
						}
					}
					else if (IsInside(worldSpacePos, worldPos, startWorldPos) )
					{
						pKnot->Selected = true;
					}
				}

				//
				//

				if (selectTangents)
				{
					worldSpaceLeft = FBVector4d(pKnot->incoming[0], pKnot->incoming[1], 0.0, 1.0);
					worldSpaceRight = FBVector4d(pKnot->outgoing[0], pKnot->outgoing[1], 0.0, 1.0);

					FBVectorMatrixMult( worldSpaceLeft, m, worldSpaceLeft );
					FBVectorMatrixMult( worldSpaceRight, m, worldSpaceRight );

					if (singleClick)
					{
						if (VectorDistance(worldSpaceLeft, worldPos) < limit)
						{
							pKnot->SelectedLeftTangent = true;
						}
						if (VectorDistance(worldSpaceRight, worldPos) < limit)
						{
							pKnot->SelectedRightTangent = true;
						}
					}
					else 
					{
						if (IsInside(worldSpaceLeft, worldPos, startWorldPos) )
						{
							pKnot->SelectedLeftTangent = true;
						}
						if (IsInside(worldSpaceRight, worldPos, startWorldPos) )
						{
							pKnot->SelectedRightTangent = true;
						}
					}
				}

				//
				//

				if (pKnot->Selected || pKnot->SelectedLeftTangent || pKnot->SelectedRightTangent)
				{
					// show properties of a selected element
					PropertiesAdd( GetGraphPtr()->mBrowsingProperty, pKnot );
				}
			}

		}
	}
}

void BaseGraphManipulator::DragChannelDown(ObjectChannel *channel, const bool affectPoints, const bool affectTangents, const double limit)
{
	if (channel == nullptr)
		return;
		
	initialWorldPos = worldPos;

	// draw each shape
	for (int i=0; i<channel->Components.GetCount(); ++i)
	{
		FBComponent *pComp = channel->Components[i];
		FBVector3d	pos, posLeft, posRight;

		ObjectShape *shape = (ObjectShape*) pComp;

		if (pComp->Components.GetCount() )
		{
			FBVector4d shapeSpacePos;
			shapeSpacePos[0] = worldPos[0];
			shapeSpacePos[1] = worldPos[1];
			shapeSpacePos[2] = worldPos[2];

			FBMatrix m = shape->GetMatrix();
			FBMatrixInverse(m, m);
			FBVectorMatrixMult( shapeSpacePos, m, shapeSpacePos );

			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
				pKnot->Position.GetData(pos, sizeof(double)*3);

				pKnot->DragWeight = 0.0;
				pKnot->DragWeightLeft = 0.0;
				pKnot->DragWeightRight = 0.0;

				double len = VectorDistance(shapeSpacePos, pos);
				if (affectPoints && len < limit)
				{
					pKnot->Drag = true;
					pKnot->DragWeight = len / limit;
					pKnot->DragWeight = 1.0 - pKnot->DragWeight * pKnot->DragWeight;

					// show properties of a selected element
					//PropertiesAddExclusive( GetGraphPtr()->mBrowsingProperty, pKnot );
				}		

				len = VectorDistance(shapeSpacePos, FBVector3d(pKnot->incoming[0], pKnot->incoming[1], 0.0) );
				if (affectTangents && len < limit)
				{
					pKnot->Drag = true;
					pKnot->DragWeightLeft = len / limit;
					pKnot->DragWeightLeft = 1.0 - pKnot->DragWeightLeft * pKnot->DragWeightLeft;
				}

				len = VectorDistance(shapeSpacePos, FBVector3d(pKnot->outgoing[0], pKnot->outgoing[1], 0.0) );
				if (affectTangents && len < limit)
				{
					pKnot->Drag = true;
					pKnot->DragWeightRight = len / limit;
					pKnot->DragWeightRight = 1.0 - pKnot->DragWeightRight * pKnot->DragWeightRight;
				}
			}

		}
	}
}

void BaseGraphManipulator::DragChannelMotion(ObjectChannel *channel)
{
	if (channel == nullptr)
		return;

	// draw each shape
	for (int i=0; i<channel->Components.GetCount(); ++i)
	{
		FBComponent *pComp = channel->Components[i];
		ObjectShape *shape = (ObjectShape*) pComp;

		if (pComp->Components.GetCount() )	
		{
			FBVector2d pos, posLeft, posRight;

			FBVector4d shapeSpacePos, initialPos;
			shapeSpacePos[0] = worldPos[0];
			shapeSpacePos[1] = worldPos[1];
			shapeSpacePos[2] = worldPos[2];

			initialPos[0] = initialWorldPos[0];
			initialPos[1] = initialWorldPos[1];
			initialPos[2] = initialWorldPos[2];

			FBMatrix m = shape->GetMatrix();
			FBMatrixInverse(m, m);
			FBVectorMatrixMult( shapeSpacePos, m, shapeSpacePos );
			FBVectorMatrixMult( initialPos, m, initialPos );

			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
					
				if (pKnot->Drag == false)
					continue;

				if (pKnot->DragWeight > 0.0)
				{
					pKnot->Position.GetData(pos, sizeof(double)*2);
					
					pos[0] = pos[0] + (shapeSpacePos[0] - initialPos[0]) * pKnot->DragWeight;
					pos[1] = pos[1] + (shapeSpacePos[1] - initialPos[1]) * pKnot->DragWeight;

					pKnot->Position.SetData(pos);

					if (DYNAMIC_MASK_OPERATIONS::IsAutoKey() )
						pKnot->Position.Key();
				}		

				if (pKnot->LockTangents)
				{
					if (pKnot->DragWeightLeft > 0.0 || pKnot->DragWeightRight > 0.0)
					{
						if (pKnot->DragWeightLeft > pKnot->DragWeightRight)
						{
							pKnot->DragWeightRight = 0.0;
						}
						else
						{
							pKnot->DragWeightLeft = 0.0;
						}
					}
					/*
					if (pKnot->DragWeightLeft > 0.0 || pKnot->DragWeightRight > 0.0)
					{
						if (pKnot->DragWeightLeft > pKnot->DragWeightRight)
						{
							pos = pKnot->incoming; 
							pos[0] = pos[0] + (shapeSpacePos[0] - initialPos[0]) * pKnot->DragWeightLeft;
							pos[1] = pos[1] + (shapeSpacePos[1] - initialPos[1]) * pKnot->DragWeightLeft;
							pKnot->incoming = pos;

							pKnot->Position.GetData(pos, sizeof(double)*2);
							posLeft = FBVector2d(pKnot->incoming[0]-pos[0], pKnot->incoming[1]-pos[1]);
							
							pKnot->LeftTangent = posLeft;
							pKnot->RightTangent = FBVector2d(-posLeft[0], -posLeft[1]);

							pKnot->outgoing = FBVector2d(pos[0]-posLeft[0], pos[1]-posLeft[1]);
						}
						else
						{
							pos = pKnot->outgoing; 
							pos[0] = pos[0] + (shapeSpacePos[0] - initialPos[0]) * pKnot->DragWeightRight;
							pos[1] = pos[1] + (shapeSpacePos[1] - initialPos[1]) * pKnot->DragWeightRight;
							pKnot->outgoing = pos;

							pKnot->Position.GetData(pos, sizeof(double)*2);
							posRight = FBVector2d(pKnot->outgoing[0]-pos[0], pKnot->outgoing[1]-pos[1]);
							
							pKnot->RightTangent = posRight;
							pKnot->LeftTangent = FBVector2d(-posRight[0], -posRight[1]);

							pKnot->incoming = FBVector2d(pos[0]-posRight[0], pos[1]-posRight[1]);
						}
					}
					*/
				}
				
				if (pKnot->DragWeightLeft > 0.0)
				{
					pos = pKnot->incoming; 
					pos[0] = pos[0] + (shapeSpacePos[0] - initialPos[0]) * pKnot->DragWeightLeft;
					pos[1] = pos[1] + (shapeSpacePos[1] - initialPos[1]) * pKnot->DragWeightLeft;
					pKnot->incoming = pos;

					pKnot->Position.GetData(pos, sizeof(double)*2);
					posLeft = FBVector2d(pKnot->incoming[0]-pos[0], pKnot->incoming[1]-pos[1]);
					pKnot->LeftTangent = posLeft;

					if (pKnot->LockTangents)
					{
						posRight = FBVector2d(-posLeft[0], -posLeft[1]);
						pKnot->RightTangent = posRight;
						pKnot->outgoing = FBVector2d(pos[0]+posRight[0], pos[1]+posRight[1]);
					}
				}
				if (pKnot->DragWeightRight > 0.0)
				{
					pos = pKnot->outgoing; 
					pos[0] = pos[0] + (shapeSpacePos[0] - initialPos[0]) * pKnot->DragWeightRight;
					pos[1] = pos[1] + (shapeSpacePos[1] - initialPos[1]) * pKnot->DragWeightRight;
					pKnot->outgoing = pos;

					pKnot->Position.GetData(pos, sizeof(double)*2);
					posRight = FBVector2d(pKnot->outgoing[0]-pos[0], pKnot->outgoing[1]-pos[1]);
					pKnot->RightTangent = posRight;

					if (pKnot->LockTangents)
					{
						posLeft = FBVector2d(-posRight[0], -posRight[1]);
						pKnot->LeftTangent = posLeft;
						pKnot->incoming = FBVector2d(pos[0]+posLeft[0], pos[1]+posLeft[1]);
					}
				}
				
			}
		}
	}

	initialWorldPos = worldPos;
}

void BaseGraphManipulator::DragChannelUp(ObjectChannel *channel)
{
	if (channel == nullptr)
		return;

	// draw each shape
	for (int i=0; i<channel->Components.GetCount(); ++i)
	{
		FBComponent *pComp = channel->Components[i];
			
		if (pComp->Components.GetCount() )
			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
				pKnot->Drag = false;	
				pKnot->DragWeight = 0.0;
				pKnot->DragWeightLeft = 0.0;
				pKnot->DragWeightRight = 0.0;
			}
			
	}
	
}

void BaseGraphManipulator::MoveChannelDown(ObjectChannel *channel, bool &hasSelection)
{
	if (channel == nullptr)
		return;

	// process each shape
	for (int i=0; i<channel->Components.GetCount(); ++i)
	{
		FBComponent *pComp = channel->Components[i];
		//ObjectShape *shape = (ObjectShape*) pComp;

		if (pComp->Components.GetCount() )	
		{
				
			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
				if (pKnot->Selected || pKnot->SelectedLeftTangent || pKnot->SelectedRightTangent)
				{
					hasSelection = true;
					break;
				}		
			}
		}
	}
}

void BaseGraphManipulator::MoveChannelMotion(ObjectChannel *channel, FBVector4d &oldSpacePos, FBVector4d &newSpacePos, const bool hasSelection)
{
	if (channel == nullptr)
		return;

	
	// process each shape
	for (int i=0; i<channel->Components.GetCount(); ++i)
	{
		FBComponent *pComp = channel->Components[i];
		ObjectShape *shape = (ObjectShape*) pComp;

		if (pComp->Components.GetCount() )	
		{
				
			FBMatrix m = shape->GetMatrix();
			FBMatrixInverse(m, m);

			FBVectorMatrixMult( oldSpacePos, m, oldSpacePos );
			FBVectorMatrixMult( newSpacePos, m, newSpacePos );

			for (int j=0; j<pComp->Components.GetCount(); ++j)
			{
				ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
					
				if (pKnot->Selected || (hasSelection == false) )
				{
					double pos[3];
					pKnot->Position.GetData(pos, sizeof(double)*2);
	
					pos[0] += newSpacePos[0] - oldSpacePos[0];
					pos[1] += newSpacePos[1] - oldSpacePos[1];

					pKnot->Position.SetData(pos);

					if (DYNAMIC_MASK_OPERATIONS::IsAutoKey() )
						pKnot->Position.Key();
				}		
				else 
				{
					if (pKnot->SelectedLeftTangent)
					{
						pKnot->incoming[0] += newSpacePos[0] - oldSpacePos[0];
						pKnot->incoming[1] += newSpacePos[1] - oldSpacePos[1];

						double pos[3], tangent[2];
						pKnot->Position.GetData(pos, sizeof(double)*2);

						tangent[0] = pKnot->incoming[0] - pos[0];
						tangent[1] = pKnot->incoming[1] - pos[1];
						pKnot->LeftTangent.SetData(tangent);

						if (pKnot->LockTangents)
						{
							tangent[0] = -tangent[0];
							tangent[1] = -tangent[1];
							pKnot->RightTangent.SetData(tangent);
							pKnot->outgoing = FBVector2d(pos[0]+tangent[0], pos[1]+tangent[1]);
						}
					}

					if (pKnot->SelectedRightTangent)
					{
						pKnot->outgoing[0] += newSpacePos[0] - oldSpacePos[0];
						pKnot->outgoing[1] += newSpacePos[1] - oldSpacePos[1];

						double pos[3], tangent[2];
						pKnot->Position.GetData(pos, sizeof(double)*2);

						tangent[0] = pKnot->outgoing[0] - pos[0];
						tangent[1] = pKnot->outgoing[1] - pos[1];
						pKnot->RightTangent.SetData(tangent);

						if (pKnot->LockTangents)
						{
							tangent[0] = -tangent[0];
							tangent[1] = -tangent[1];
							pKnot->LeftTangent.SetData(tangent);
							pKnot->incoming = FBVector2d(pos[0]+tangent[0], pos[1]+tangent[1]);
						}
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////
// GraphManager

//! a constructor
GraphManager::GraphManager(View2d	*Graph)
	: pGraph(Graph)
{
	mManip = NULL;
	mCurrentManipulator = 0;
	mCurrentTransformMode = 0;

	AddGlobal();
	AddCommand();

	SetManipulator( MANIPULATOR_GRAPH_SELECT );
}
//! a destructor
GraphManager::~GraphManager()
{
	ClearManipulator();
	ClearGlobalManip();
	ClearCommands();
}
//! add new global manipulator to a graph
void GraphManager::AddGlobal()
{
	mGlobalManip.SetCount(8);

	mGlobalManip.SetAt(0, ManipulatorPtr(new ManipGraphPan(pGraph)) );
	mGlobalManip.SetAt(1, ManipulatorPtr(new ManipGraphZoom(pGraph)) );
	mGlobalManip.SetAt(2, ManipulatorPtr(new ManipGraphSelect(pGraph)) );
	mGlobalManip.SetAt(3, ManipulatorPtr(new ManipGraphDrag(pGraph)) );
	mGlobalManip.SetAt(4, ManipulatorPtr(new ManipGraphMove(pGraph)) );
	mGlobalManip.SetAt(5, ManipulatorPtr(new ManipGraphShapeRect(pGraph)) );
	mGlobalManip.SetAt(6, ManipulatorPtr(new ManipGraphShapeCircle(pGraph)) );
	mGlobalManip.SetAt(7, ManipulatorPtr(new ManipGraphShapeSpline(pGraph)) );
	
}
//! clear all global manips from a graph
void GraphManager::ClearGlobalManip()
{
	mGlobalManip.Clear();
}
//! set current local manipulator as current
void GraphManager::SetManipulator(const char *szManipName)
{
	ClearManipulator();

	// toggle off all other manipulators

	for (int i=0; i<mGlobalManip.GetCount(); ++i)
		if (mGlobalManip[i].get() )
			if (strcmp( mGlobalManip[i]->GetCaption(), szManipName ) == 0 )
			{
				mManip = mGlobalManip[i].get();
				mGlobalManip[i]->SetActive(true);
				mCurrentManipulator = i;
			}
			else
			{
				mGlobalManip[i]->SetActive(false);
			}
}

void GraphManager::SetManipulatorActive(const char *szManipName, bool active )
{
	
	// toggle off all other manipulators

	for (int i=0; i<mGlobalManip.GetCount(); ++i)
		if (mGlobalManip[i].get() && strcmp( mGlobalManip[i]->GetCaption(), szManipName ) == 0 )
			{
				mGlobalManip[i]->SetActive(active);
			}
}

//! clear local manipulator
void GraphManager::ClearManipulator()
{
	mManip = nullptr;
}

void GraphManager::AddCommand()
{
	BaseGraphCommand	*command = new GraphFitCommand();

	mCommands.SetCount(1);

	if (command)
		mCommands.SetAt(0, CommandPtr(command) );
}

void GraphManager::ClearCommands()
{
	mCommands.Clear();
}

//! mouse down processing
void		GraphManager::MouseDown(int x, int y, int pButtonKey, int modifier)
{
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i].get() && mGlobalManip[i]->IsActive() )
		{
			mGlobalManip[i]->MouseDown(x,y, pButtonKey, modifier);
		}
	}
}
//! mouse move processing
void		GraphManager::MouseMove(int x, int y, int pButtonKey,  int modifier)
{
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i].get() && mGlobalManip[i]->IsActive() )
		{
			mGlobalManip[i]->MouseMove(x,y, pButtonKey, modifier);
		}
	}
}
//! mouse up processing
void		GraphManager::MouseUp(int x, int y,int pButtonKey,  int modifier)
{
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i].get() && mGlobalManip[i]->IsActive() )
		{
			mGlobalManip[i]->MouseUp(x,y, pButtonKey, modifier);
		}
	}
}

void		GraphManager::UserInput(int x, int y, int key, int modifier)
{
	for (int i=0; i<mCommands.GetCount(); i++)
	{
		if (mCommands[i].get() )
			if (mCommands[i]->CheckInput(x,y,key,modifier) )
				mCommands[i]->DoIt(pGraph);
	}
}

//! output 2d drawing
void		GraphManager::PreDraw()
{
	// process all global manipulators
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i].get() && mGlobalManip[i]->IsActive() )
			mGlobalManip[i]->PreDraw();
	}
	// process current local manipulator
	if (mManip) {
		mManip->PreDraw();
	}
}

void		GraphManager::PostDraw()
{
	// process all global manipulators
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i].get() && mGlobalManip[i]->IsActive() )
			mGlobalManip[i]->PostDraw();
	}
	// process current local manipulator
	if (mManip) {
		mManip->PostDraw();
	}
}


//////////////////////////////////////////////////////////////////////////////////////
// View2d


View2d::View2d()
	: FBView()
	, GraphManager(this)
{

	

	mDown=false;

	mW = 320;
	mH = 240;

	mClearColor = FBVector4d(0.5, 0.5, 0.5, 1.0);

	// startup time range (X)
	mXSpan = FBVector2d( 0.0, 512.0 );
	// startup value range (Y)
	mYSpan = FBVector2d( 0.0, 512.0 );

	mCanvasWidth = 512.0;
	mCanvasHeight = 512.0;

	// min & max
	mXRange = mXSpan;
	mYRange = mYSpan;

	mGrid = true;
	mGridDensity = FBVector2d(32.0, 32.0);

	mStatusLine = "Status here";

	mBrowsingProperty = nullptr;

}
//! Refresh callback.
void View2d::Refresh(bool pNow)
{
	FBView::Refresh(pNow);
}

void View2d::ViewReSize(int w, int h)
{
	mW = w;
	mH = h;
}

int View2d::CalcWidth()
{
	return (int) (mXSpan[1] - mXSpan[0]);
}
int View2d::CalcHeight()
{
	return (int) (mYSpan[1] - mYSpan[0]);
}
/*
double	GraphView::CalcXSens() {
	return mTimeSpan.GetDuration().GetSecondDouble() / mW;
}
double	GraphView::CalcYSens() {
	return (mValueSpan[1] - mValueSpan[0]) / mH;
}

// transfer from cursor pos to world graph pos
void GraphView::ScreenToWorld( int x, int y, double &wx, double &wy )
{
	wx = mTimeSpan.GetStart().GetSecondDouble() + mTimeSpan.GetDuration().GetSecondDouble() * x / mW;
	wy = mValueSpan[0] + (mValueSpan[1] - mValueSpan[0]) * y / mH;
}
*/
//! Expose callback.
void View2d::BeginViewExpose(bool predraw)
{	
	ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();

	if (pMask)
	{
		mCanvasWidth = pMask->CanvasWidth;
		mCanvasHeight = pMask->CanvasHeight;
	}
}

void DrawUVLayout(FBModel *pModel, double w, double h, bool showTexture, bool showWireframe)
{
	FBModelVertexData *lModelVertexData = pModel->ModelVertexData;
	if ( nullptr == lModelVertexData || false == lModelVertexData->IsDrawable() )
		return;

	FBStringList uvsets;
	FBMesh *pMesh = pModel->TessellatedMesh;
	uvsets = pMesh->GetUVSets();

	if (uvsets.GetCount() == 0 ) {
		return;
	}
	FBString uvset_name = uvsets[0];
	
	const int lSubPatchCount = lModelVertexData->GetSubPatchCount();

	lModelVertexData->VertexArrayMappingRequest();
	{
		int *indexArray = lModelVertexData->GetIndexArray();

		auto arrayElementType = lModelVertexData->GetUVSetArrayFormat( kFBTextureMappingUV, uvset_name );
		if (arrayElementType == kFBGeometryArrayElementType_Float2)
		{
			int uvstride = 2;
			float *uvArray = (float*) lModelVertexData->GetUVSetArray(kFBTextureMappingUV, uvset_name );
			

			for(int lSubPatchIdx = 0; lSubPatchIdx < lSubPatchCount; ++lSubPatchIdx)
			{
				
				FBMaterial	*pMaterial = lModelVertexData->GetSubPatchMaterial(lSubPatchIdx);
				if (pMaterial && showTexture)
				{
					FBTexture *pTexture = pMaterial->GetTexture();
					if (pTexture)
					{
						glActiveTexture(GL_TEXTURE0);
						pTexture->OGLInit();

						glEnable(GL_BLEND);
						glBlendFunc(GL_ONE, GL_ONE);

						glBegin(GL_QUADS);

							glTexCoord2d(0.0, 0.0);
							glVertex2d(0.0, 0.0);

							glTexCoord2d(1.0, 0.0);
							glVertex2d(w, 0.0);

							glTexCoord2d(1.0, 1.0);
							glVertex2d(w, h);

							glTexCoord2d(0.0, 1.0);
							glVertex2d(0.0, h);

						glEnd();

						//
						glMatrixMode(GL_TEXTURE);
						glLoadIdentity();
						glMatrixMode(GL_MODELVIEW);

						glDisable(GL_BLEND);
						glDisable(GL_TEXTURE_2D);
					}
				}

				if (showWireframe && lModelVertexData->GetSubPatchPrimitiveType(lSubPatchIdx) == kFBGeometry_TRIANGLES)
				{
					int lOffset = lModelVertexData->GetSubPatchIndexOffset(lSubPatchIdx);
					int lSize = lModelVertexData->GetSubPatchIndexSize(lSubPatchIdx);

					int *patchIndexArray = indexArray + lOffset;
					
					int TriCount = lSize / 3;

					glBegin(GL_LINES);

					for (int i=0; i<TriCount; ++i)
					{
						int idx1 = patchIndexArray[i * 3];
						int idx2 = patchIndexArray[i * 3 + 1];
						int idx3 = patchIndexArray[i * 3 + 2];

						FBUV uv1( &uvArray[idx1 * uvstride] );
						FBUV uv2( &uvArray[idx2 * uvstride] );
						FBUV uv3( &uvArray[idx3 * uvstride] );

						glVertex2f( w * uv1[0], h * uv1[1] );
						glVertex2f( w * uv2[0], h * uv2[1] );

						glVertex2f( w * uv2[0], h * uv2[1] );
						glVertex2f( w * uv3[0], h * uv3[1] );

						glVertex2f( w * uv3[0], h * uv3[1] );
						glVertex2f( w * uv1[0], h * uv1[1] );
					}
					
					glEnd();
				}
			}
		}
	}
	
	lModelVertexData->VertexArrayMappingRelease();
}

void View2d::EndViewExpose()
{
	CHECK_GL_ERROR_MOBU();

	glLineWidth(1.0);
	//glDisable(GL_DEPTH_TEST);
	
	glClearColor( mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3] );
	glClear(GL_COLOR_BUFFER_BIT);

	Set2dViewport(mW, mH);

	double sx = (mXSpan[1] - mXSpan[0]) / (mXRange[1] - mXRange[0]);
	double sy = (mYSpan[1] - mYSpan[0]) / (mYRange[1] - mYRange[0]);

	glTranslatef( (float) mXSpan[0], (float) mYSpan[0], 0.0f);
	glScalef( (float) sx, (float) sy, 0.0f );
	
	// grab matrix state
	glGetDoublev( GL_PROJECTION_MATRIX, &mProjection[0] );
	glGetIntegerv( GL_VIEWPORT, &mViewport[0] );
	glGetDoublev( GL_MODELVIEW_MATRIX, &mModelView[0] );

	// call graph manager proc
	//if (predraw)	PreDraw();

	// draw canvas area
	/*
	glColor3d(0.0, 0.0, 0.0);

	glBegin(GL_QUADS);
		glVertex2d(mXRange[0], mYRange[0]);
		glVertex2d(mXRange[1], mYRange[0]);
		glVertex2d(mXRange[1], mYRange[1]);
		glVertex2d(mXRange[0], mYRange[1]);
	glEnd();
	*/

	
	// trigger mipmaps generation explicitly
	// NOTE: If GL_GENERATE_MIPMAP is set to GL_TRUE, then glCopyTexSubImage2D()
	// triggers mipmap generation automatically. However, the texture attached
	// onto a FBO should generate mipmaps manually via glGenerateMipmap().
	
	ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
	ObjectChannel *pChannel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();

	// draw background if needed
	if (pMask && pChannel)
	{
		FBTexture *pTexture = (pChannel->ShowBackgroundTexture && pChannel->BackgroundTexture.GetCount() > 0) ? (FBTexture*)pChannel->BackgroundTexture[0] : nullptr;
		
		// try with a mask properties
		if (nullptr == pTexture)
			pTexture = (pMask->ShowBackgroundTexture && pMask->BackgroundTexture.GetCount() > 0) ? (FBTexture*)pMask->BackgroundTexture[0] : nullptr;

		if (pTexture)
		{
			glActiveTexture(GL_TEXTURE0);
			
			GLuint texId = pTexture->TextureOGLId;
			if (0 == texId)
			{
				pTexture->OGLInit();
				texId = pTexture->TextureOGLId;
			}

			// draw background
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texId);

			glBegin(GL_QUADS);

				glTexCoord2d(0.0, 0.0);
				glVertex2d(0.0, 0.0);

				glTexCoord2d(1.0, 0.0);
				glVertex2d(mCanvasWidth, 0.0);

				glTexCoord2d(1.0, 1.0);
				glVertex2d(mCanvasWidth, mCanvasHeight);

				glTexCoord2d(0.0, 1.0);
				glVertex2d(0.0, mCanvasHeight);

			glEnd();

			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}
	}


	if (pMask)
	{
		CHECK_GL_ERROR_MOBU();

		GLuint color_buffer_id = pMask->getColorTextureId();
		
		FBColor color = pMask->ShapeColor;
		double transparency = pMask->ShapeTransparency;
		if (pChannel && pChannel->OverrideDisplay)
		{
			color = pChannel->ShapeColor;
			transparency = pChannel->ShapeTransparency;
		}
		transparency = 1.0 - 0.01 * transparency;

		if (color_buffer_id)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, color_buffer_id); // color_buffer_id
			//glGenerateMipmap(GL_TEXTURE_2D);
	
			if (/*pMask->RenderFromCamera == false &&*/ pChannel)
			{
				//color_buffer_id = pChannel->getColorTextureId();
			
				// DONE: swizzle according channel id

				GLenum mode = GL_RED;
				if (pMask->Components.GetCount() == 4)
				{
					if (pMask->Components[0] == pChannel)
						mode = GL_RED;
					else if (pMask->Components[1] == pChannel)
						mode = GL_GREEN;
					else if (pMask->Components[2] == pChannel)
						mode = GL_BLUE;
					else if (pMask->Components[3] == pChannel)
						mode = GL_ALPHA;
				}


				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, mode );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, mode );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, mode );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE );
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}
			
			if (transparency < 1.0)
			{
				glEnable(GL_BLEND);
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			}

			//glBlendColor(0.0, 0.0, 0.0, transparency);
			glColor4d(color[0], color[1], color[2], transparency);
			glBegin(GL_QUADS);

				glTexCoord2d(0.0, 0.0);
				glVertex2d(0.0, 0.0);

				glTexCoord2d(1.0, 0.0);
				glVertex2d(mCanvasWidth, 0.0);

				glTexCoord2d(1.0, 1.0);
				glVertex2d(mCanvasWidth, mCanvasHeight);

				glTexCoord2d(0.0, 1.0);
				glVertex2d(0.0, mCanvasHeight);

			glEnd();

			// display alpha channel
			if (/*pMask->RenderFromCamera == false &&*/ pChannel == nullptr)
			{
				glEnable(GL_BLEND);
				glBlendFunc( GL_SRC_ALPHA, GL_ONE );

				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ALPHA );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ALPHA );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ALPHA );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA );

				glColor4d(color[0], color[1], color[2], 1.0);
				glBegin(GL_QUADS);

					glTexCoord2d(0.0, 0.0);
					glVertex2d(0.0, 0.0);

					glTexCoord2d(1.0, 0.0);
					glVertex2d(mCanvasWidth, 0.0);

					glTexCoord2d(1.0, 1.0);
					glVertex2d(mCanvasWidth, mCanvasHeight);

					glTexCoord2d(0.0, 1.0);
					glVertex2d(0.0, mCanvasHeight);

				glEnd();
				glDisable(GL_BLEND);
			}

			// !? return back the swizzle
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA );

			glBindTexture(GL_TEXTURE_2D, 0);

			if (transparency < 1.0)
				glDisable(GL_BLEND);
		}
	}

	//
	// draw UV layout from models
	//
	
	FBModel *pModelUV = nullptr;
	bool showTexture = false;
	bool showWire = false;

	if (pMask)
	{
		if (pMask->UVLayout.GetCount() > 0)
			pModelUV = (FBModel*) pMask->UVLayout.GetAt(0);
		showTexture = pMask->ShowTexture;
		showWire = pMask->ShowWire;

		if (pChannel && pChannel->UVLayout.GetCount() > 0)
		{
			pModelUV = (FBModel*) pChannel->UVLayout.GetAt(0);
			showTexture = pChannel->ShowTexture;
			showWire = pChannel->ShowWire;
		}
	}

	
	glColor3d( 0.3, 0.3, 0.3 );

	// draw each shape
	if (pModelUV)
		DrawUVLayout(pModelUV, mCanvasWidth, mCanvasHeight, showTexture, showWire);

	bool showGrid = false;
	
	if (pMask)
		showGrid = pMask->ShowGrid;
	if (pChannel && pChannel->OverrideDisplay) 
		showGrid = pChannel->ShowGrid;

	// draw grid
	if (mGrid && showGrid)
	{
		glColor3d( 0.3, 0.3, 0.3 );

		int xsteps = 512.0 / mGridDensity[0] * (mCanvasWidth / 512.0);
		int ysteps = 512.0 / mGridDensity[1] * (mCanvasHeight / 512.0);

		glBegin(GL_LINES);
		//glColor3d( 0.3, 0.0, 0.0 );
		double currx = 0.0;
		for (int i=0; i<xsteps; ++i)
		{
			glVertex2d( currx, 0.0 );
			glVertex2d( currx, mCanvasHeight );

			currx += mGridDensity[0];
		}
		//glColor3d( 0.0, 3.0, 0.0 );
		double curry = 0.0;
		for (int i=0; i<ysteps; ++i)
		{
			glVertex2d( 0.0, curry );
			glVertex2d( mCanvasWidth, curry );

			curry += mGridDensity[1];
		}

		glEnd();

		const int buffer_size = 128;
		char buffer[buffer_size];
		memset(buffer, 0, sizeof(char)*buffer_size);
		
		glColor3d( 0.0, 0.0, 0.0 );
		DrawString( "0", -5, -5 );
		
		glColor3d( 0.3, 0.0, 0.0 );
		sprintf_s(buffer, buffer_size, "%d", (int) mCanvasWidth );
		DrawString( buffer, mCanvasWidth, -5 );

		glColor3d( 0.0, 0.3, 0.0 );
		sprintf_s(buffer, buffer_size, "%d", (int) mCanvasHeight );
		DrawString( buffer, -5, mCanvasHeight );
	}

	PostDraw();

	// draw current manipulator title
	if ( GetCurrent() )
	{
		Set2dViewport(mW, mH);
		glColor3d( 0.0, 1.0, 0.0 );
		DrawString( GetCurrent()->GetCaption(), mW - 120.0, 25.0 );
	}
	//Pop2dViewport();
}



FBVector3d LinearInterpolation( FBVector3d &v1, FBVector3d &v2, float t )
{
  FBVector3d result;
  result = FBVector3d( (v2[0] - v1[0]) * t + v1[0],
              (v2[1] - v1[1]) * t + v1[1],
              (v2[2] - v1[2]) * t + v1[2] );
  return result;
}

FBVector3d HermiteInterpolation( FBVector3d v1, FBVector3d v2, FBVector3d incoming, FBVector3d outgoing, double t )
{
  FBVector3d result;
  double t2, t3, H1, H2, H3, H4;

  //-- processing vectors

  // -- T sqaured && T cubed
  t2 = t*t;
  t3 = t2*t;

  //-- hermite basis function coefficients
  H2 = -(t3 + t3) + t2*3.0f;
  H1 = 1.0f - H2;
  H4 = t3 - t2;
  H3 = H4 - t2 + t;

  result = FBVector3d( v1[0]*H1, v1[1]*H1, v1[2]*H1 );
  result = FBVector3d( result[0] + v2[0] * H2, result[1] + v2[1] * H2, result[2] + v2[2] * H2 );

  result = FBVector3d( result[0] + incoming[0] * H3, result[1] + incoming[1] * H3, result[2] + incoming[2] * H3 );
  result = FBVector3d( result[0] + outgoing[0] * H4, result[1] + outgoing[1] * H4, result[2] + outgoing[2] * H4 );

  //result = v1 * H1;
  //result = result + v2 * H2;
  //result = result + k1.incoming * H3;
  //result = result + k2.outgoing * H4;

  return result;
}

double ComputeTime(int index, int count)
{
	double result = (double) index;
	return (result / count);
}

void HermiteRecompute( ObjectShape *shape )
{
	/*
	const bool IsLooped = true;

	int i, index0, index1, index2;
	double time0, time1, time2, n0, n1, n0n1;
	FBVector3d v0, v1, v2, Slope;

	int count = shape->Components.GetCount();
	for( i=0; i<count; ++i) {

		ObjectKnot *pKnot = (ObjectKnot*) shape->Components[i];

		index0 = i-1;
		index1 = i;
		index2 = i+1;

		time1 = ComputeTime(index1, count);
		if (index1 == 0) {
		  if ( !IsLooped ) {
			index0 = 0;
			time0 = ComputeTime(index0, count);
		  } else {
			index0 = count-2;
			time0 = time1 - ( ComputeTime(index0+1, count) - ComputeTime(index0, count) );
		  }
		} else time0 = ComputeTime(index0, count);

		if (index2 == count ) {
		  if ( !IsLooped ) {
			index2 = count-1;
			time2 = ComputeTime(index2, count);
		  } else {
			index2 = 1;
			time2 = time1 + (ComputeTime(1, count) - ComputeTime(0, count));
		  }
		} else time2 = ComputeTime(index2, count);

		((ObjectKnot*)shape->Components[index0])->Position.GetData(v0, sizeof(double)*3);
		((ObjectKnot*)shape->Components[index1])->Position.GetData(v0, sizeof(double)*3);
		((ObjectKnot*)shape->Components[index2])->Position.GetData(v0, sizeof(double)*3);

		n0 = time1 - time0;
		n1 = time2 - time1;
		n0n1 = n0 + n1;

		if ( !IsLooped && index1 == 0) {
		  pKnot->incoming = FBVector3d(v2[0]-v1[0], v2[1]-v1[1], v2[2]-v1[2]);
		  pKnot->outgoing = pKnot->incoming;
		}
		else if ( !IsLooped && (index1 == (count-1)) ) {
		  pKnot->incoming = FBVector3d(v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2]);
		  pKnot->outgoing = pKnot->incoming;
		}
		else {
		  Slope = FBVector3d(v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2]);
		  pKnot->incoming = FBVector3d(Slope[0] * (n1 / n0n1), Slope[1] * (n1 / n0n1), Slope[2] * (n1 / n0n1));
		  pKnot->outgoing = FBVector3d(Slope[0] * (n0 / n0n1), Slope[1] * (n0 / n0n1), Slope[2] * (n0 / n0n1));
		}
	}
	*/
}

#define GL_CLOSE_PATH_NV                                    0x00
#define GL_MOVE_TO_NV                                       0x02
#define GL_LINE_TO_NV                                       0x04
#define GL_CUBIC_CURVE_TO_NV                                0x0C
#define GL_SMOOTH_QUADRATIC_CURVE_TO_NV                     0x0E

void View2d::ViewExpose()
{
	static bool firsttime = true;

	CHECK_GL_ERROR_MOBU();

	if (firsttime)
	{
		firsttime = false;
		/*
		HFBVideoMemory pVideoMemory = new FBVideoMemory("DynamicMask");
		pVideoMemory->SetObjectImageSize(512, 512);
		pVideoMemory->TextureOGLId = getColorTextureId();

		HFBTexture pTexture = new FBTexture("DynamicTexture");
		pTexture->Video = pVideoMemory;
		*/

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	}

//	const int density = 10;

	BeginViewExpose();
	
	// draw view content here
	ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
	if (channel)
	{
		
	}
 
	EndViewExpose();

	CHECK_GL_ERROR_MOBU();
}
//! input callback.
void View2d::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
	char buffer[256];
	memset(buffer, 0, sizeof(char)*256);

	switch(pAction)
	{
	case kFBKeyPress:
		{
			UserInput(pMouseX, pMouseY, pButtonKey, pModifier);
		}
		break;

	case kFBButtonPress:
		{
		// register mouse down in viewport
		mDown = true;
		mLastX = pMouseX;
		mLastY = pMouseY;

		if (pModifier == kFBKeyAlt)
			if (pButtonKey == 2)	// middle button click
				SetManipulatorActive( MANIPULATOR_GRAPH_PAN, true );
			else if (pButtonKey == 3)	// right button click
				SetManipulatorActive( MANIPULATOR_GRAPH_ZOOM, true );


		// do some action below
		MouseDown(pMouseX, pMouseY, pButtonKey, pModifier);
		} break;
	case kFBButtonRelease:
		// do some action below
		MouseUp(pMouseX, pMouseY, pButtonKey, pModifier);
		// release button
		mDown = false;

		SetManipulatorActive( MANIPULATOR_GRAPH_PAN, false );
		SetManipulatorActive( MANIPULATOR_GRAPH_ZOOM, false );
		break;
	case kFBMotionNotify:
		// do some action
		MouseMove(pMouseX, pMouseY, pButtonKey, pModifier);
		
		// update status information
		double wx, wy;
		ScreenToWorld(pMouseX, pMouseY, &wx, &wy);
		sprintf_s(buffer, 256, "Canvas size: %d, %d. Mouse position: %d, %d . World position: %.2lf, %.2lf\0", (int) mXRange[1], (int) mYRange[1], pMouseX, pMouseY, wx, wy );
		mStatusLine = buffer;

		// lastx, lasty
		mLastX = pMouseX;
		mLastY = pMouseY;
		break;
	}
}

void View2d::ForceUpdateUI(int type)
{
	OnUpdateUI(type);
}