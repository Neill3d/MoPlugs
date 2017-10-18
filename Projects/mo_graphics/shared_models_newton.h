
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_models_newton.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_models.h"
#include "..\Common_Physics\physics_common.h"

/////////////////////////////////////////////////////////////////////////////////////
// GPUCacheGeometry

class GPUCacheGeometry : public PHYSICS_INTERFACE::IQueryGeometry
{
public:

	//! a constructor
	GPUCacheGeometry();
	GPUCacheGeometry( CGPUModelRenderCached *pModel );

	//! a destructor
	virtual ~GPUCacheGeometry();

	//
	bool	Prep( CGPUModelRenderCached *pModel );

	//

	virtual const int		GetVertexCount() const;
	virtual const float		*GetVertexPosition(const int index) const;
	virtual const float		*GetVertexNormal(const int index) const;

	virtual const int		GetPolyCount() const;
	virtual const Poly		*GetPoly(const int index) const;
	virtual const int		GetPolyAttribute(const int index) const;

	virtual bool			HasVertexNormal() const;
	virtual bool			HasAttributes() const;

private:

	int							mNumberOfVertices;
	int							mNumberOfPolys;

	std::vector<vec4>			mVertices;
	std::vector<vec4>			mNormals;
	std::vector<Poly>			mPolys;
	std::vector<int>			mAttributes;	// attribute per face (store model or mesh index)

	void Allocate(int numberOfVerts, int numberOfPolys);
	void Free();
};