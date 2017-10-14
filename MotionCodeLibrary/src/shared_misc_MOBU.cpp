
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_misc_MOBU.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "shared_misc_MOBU.h"



void RenderModel(FBModel *pModel, bool setupMaterial, bool bindColor, const bool recursive)
{
	FBModelVertexData* lModelVertexData = pModel->ModelVertexData;
	if (lModelVertexData && lModelVertexData->IsDrawable() )
	{

		FBMatrix m;
		pModel->GetMatrix(m);

		glPushMatrix();
		glMultMatrixd(m);

		//Get number of region mapped by different materials.
		const int lSubRegionCount = lModelVertexData->GetSubRegionCount();
		if (lSubRegionCount)
		{
			//Set up vertex buffer object (VBO) or vertex array
			lModelVertexData->EnableOGLVertexData();
			
			if (bindColor)
				lModelVertexData->EnableOGLUVSet();

			for (int lSubRegionIndex = 0; lSubRegionIndex < lSubRegionCount; lSubRegionIndex++)
			{
				// Setup material, texture, shader, parameters here.
				/* 
				FBMaterial* lMaterial = lModelVertexData->GetSubRegionMaterial(lSubRegionIndex);
				*/

				if (setupMaterial)
				{
					FBMaterial* lMaterial = lModelVertexData->GetSubRegionMaterial(lSubRegionIndex);
					lMaterial->OGLInit();
				}
				/*
				if (bindColor)
				{
					FBMaterial* lMaterial = lModelVertexData->GetSubRegionMaterial(lSubRegionIndex);
					
					if (lMaterial)
					{
						FBTexture *pTexture = lMaterial->GetTexture();
						if (pTexture)
						{
							GLuint id = pTexture->TextureOGLId;
							if (0 == id)
							{
								pTexture->OGLInit();
								id = pTexture->TextureOGLId;
							}

							glBindTexture(GL_TEXTURE_2D, id);
						}
					}
				}
				*/
				lModelVertexData->DrawSubRegion(lSubRegionIndex);

				//Cleanup material, texture, shader, parameters here
			}

			if (bindColor)
				lModelVertexData->DisableOGLUVSet();
			lModelVertexData->DisableOGLVertexData();
		}

		glPopMatrix();
	}

	if (recursive)
	{
		int count = pModel->Children.GetCount();
		for (int i=0; i<count; ++i)
			RenderModel( pModel->Children[i], setupMaterial, bindColor, recursive );
	}
}

void DrawSceneModel(FBModel *pModel)
{

	for (int i=0; i<pModel->Children.GetCount(); ++i)
		DrawSceneModel(pModel->Children[i]);

	if (pModel->IsVisible() == false) return;

	//printf( pModel->Name );

	//FBModel* lModel = pShaderModelInfo->GetFBModel();
	FBModelVertexData* lModelVertexData = pModel->ModelVertexData;
	
	if (!lModelVertexData) return;
	if (!lModelVertexData->IsDrawable() ) return;

	FBMatrix pMatrix;
	pModel->GetMatrix(pMatrix);

	glPushMatrix();
	glMultMatrixd( pMatrix );

	const int lSubRegionCount = lModelVertexData->GetSubRegionCount();

	glEnable( GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	lModelVertexData->EnableOGLVertexData();  //Bind Vertex Array or Vertex Buffer Object.

	for(int lSubRegionIdx = 0; lSubRegionIdx < lSubRegionCount; ++lSubRegionIdx)
	{
		if (pModel->Materials.GetCount() > lSubRegionIdx)
		{
			/*
			FBMaterial* lMaterial = pModel->Materials[lSubRegionIdx];
			lMaterial->OGLInit(); // Setup OpenGL fixed pipeline's ambient/diffuse/emissive/specular/shininess materials parameters.
			FBTexture* lDiffuseTex = lMaterial->GetTexture( kFBMaterialTextureDiffuse );
			if (lDiffuseTex)
			{
				lDiffuseTex->OGLInit();	//Bind color texture and it's associated matrix.
				lModelVertexData->EnableOGLUVSet(lDiffuseTex->Mapping); //Bind the UVSet. 
				glEnable( GL_TEXTURE_2D );
			}
			*/
			lModelVertexData->DrawSubRegion(lSubRegionIdx); // draw all the sub patches inside this sub regions.
			/*
			if (lDiffuseTex)
			{
				lModelVertexData->DisableOGLUVSet(); //Unbind the UVSet.
				glDisable(GL_TEXTURE_2D);
			}
			*/
		}
        else
        {
            lModelVertexData->DrawSubRegion(lSubRegionIdx); // draw all the sub patches inside this sub regions.
        }
	}
	
	lModelVertexData->DisableOGLVertexData(); //Unbind Vertex Array or Vertex Buffer Object.
	glDisable(GL_LIGHTING);

	glPopMatrix();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////


void PropertiesClearAll(FBBrowsingProperty *pProperty)
{
	FBArrayTemplate<FBPlug*>	plugs;
	plugs.SetCount(pProperty->ObjectGetCount() );

	for (int i=0; i<pProperty->ObjectGetCount(); ++i)
		plugs[i] = pProperty->ObjectGet(i);

	for (int i=0; i<plugs.GetCount(); ++i)
		pProperty->RemoveObject(plugs[i]);
}

void PropertiesAddExclusive(FBBrowsingProperty *pProperty, FBPlug *pPlug)
{
	if (pProperty && pPlug)
	{
		PropertiesClearAll(pProperty);
		pProperty->AddObject(pPlug);
	}
}

void PropertiesAdd(FBBrowsingProperty *pProperty, FBPlug *pPlug)
{
	if (pProperty && pPlug)
	{
		//PropertiesClearAll(pProperty);
		pProperty->AddObject(pPlug);
	}
}

void PropertyCopy( FBPropertyAnimatableVector4d &to, FBPropertyAnimatableVector4d &from )
{
	FBVector4d pColor;
	from.GetData( &pColor, sizeof(FBVector4d) );
	to.SetData( pColor );
}

void PropertyCopy( FBPropertyAnimatableColor &to, FBPropertyAnimatableColor &from )
{
	FBColor pColor;
	from.GetData( &pColor, sizeof(pColor) );
	to.SetData( pColor );
}

void PropertyCopy( FBPropertyAnimatableColorAndAlpha &to, FBPropertyAnimatableColorAndAlpha &from )
{
	FBColorAndAlpha pColor;
	from.GetData( &pColor, sizeof(pColor) );
	to.SetData( pColor );
}

void PropertyCopy( FBPropertyAnimatableDouble &to, FBPropertyAnimatableDouble &from)
{
	double dValue;
	from.GetData( &dValue, sizeof(double) );
	to.SetData( &dValue );
}

void PropertyCopy( FBPropertyAnimatableBool &to, FBPropertyAnimatableBool &from)
{
	bool bValue;
	from.GetData( &bValue, sizeof(bool) );
	to.SetData( &bValue );
}

void PropertyCopy( FBPropertyBool &to, FBPropertyBool &from )
{
	to = (bool) from;
}

void PropertyCopy( FBPropertyInt &to, FBPropertyInt &from )
{
	to = (int) from;
}

void PropertyCopy( FBPropertyDouble &to, FBPropertyDouble &from )
{
	to = (double) from;
}