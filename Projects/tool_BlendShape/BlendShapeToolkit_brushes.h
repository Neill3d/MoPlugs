
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_brushes.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- OR SDK include
#include <fbsdk/fbsdk.h>
#include "BlendShapeToolkit_brushesBase.h"
#include "IO\FileUtils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushMove

class BrushDrag : public BaseBrush
{
public:
	//! a constructor
	BrushDrag( FBComponent *pMaster );

	~BrushDrag();

	// information
	char	*GetCaption() { return "Drag"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\Grab.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\Grab.png"; 
	}

	bool	WantToReacalcWeights() { return false; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushPush

class BrushPush : public BaseBrush
{
public:
	//! a constructor
	BrushPush( FBComponent *pMaster );

	~BrushPush();

	// information
	char	*GetCaption() { return "Push"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\Bulge.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\Bulge.png"; 
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushPush

class BrushFreeze : public BaseBrush
{
public:
	//! a constructor
	BrushFreeze( FBComponent *pMaster );

	~BrushFreeze();

	// information
	char	*GetCaption() { return "Freeze"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\Freeze.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\Freeze.png";
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushSmooth

class BrushSmooth : public BaseBrush
{
public:
	//! a constructor
	BrushSmooth( FBComponent *pMaster );

	~BrushSmooth();

	// information
	char	*GetCaption() { return "Smooth"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\Smooth.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\Smooth.png";
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushErase

class BrushErase : public BaseBrush
{
public:
	//! a constructor
	BrushErase( FBComponent *pMaster );

	~BrushErase();

	// information
	char	*GetCaption() { return "Erase"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\Erase.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\Erase.png";
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// BrushPaint

class BrushPaint : public BaseBrush
{
public:
	//! a constructor
	BrushPaint( FBComponent *pMaster );

	~BrushPaint();

	// information
	char	*GetCaption() { return "Paint"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\Paint.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\Paint.png";
	}

	bool	WantToReacalcWeights() { return true; }

	// update mesh vertices according to this brush algorithm
	void	Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer );
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FalloffLinear

class FalloffLinear : public BaseFalloff
{
public:
	//! a constructor
	FalloffLinear()
		: BaseFalloff()
	{}

	~FalloffLinear()
	{}

	char	*GetCaption() { return "Linear"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\falloffLinear.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\falloffLinear.png";
	}

	double	Calculate(double t) { return t; }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// FalloffHard

class FalloffHard: public BaseFalloff
{
public:
	//! a constructor
	FalloffHard()
		: BaseFalloff()
	{}

	~FalloffHard()
	{}

	char	*GetCaption() { return "Hard"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\falloffTop.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\falloffTop.png";
	}

	double	Calculate(double t);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FalloffPoint

class FalloffPoint: public BaseFalloff
{
public:
	//! a constructor
	FalloffPoint()
		: BaseFalloff()
	{}

	~FalloffPoint()
	{}

	char	*GetCaption() { return "Point"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\falloffPoint.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\falloffPoint.png";
	}

	double	Calculate(double t);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FalloffSmooth

class FalloffSmooth: public BaseFalloff
{
public:
	//! a constructor
	FalloffSmooth()
		: BaseFalloff()
	{}

	~FalloffSmooth()
	{}

	char	*GetCaption() { return "Smooth"; }
	char	*GetPicturePath() { 
		FBString out_path;
		if (FindEffectLocation( "\\System\\BlendShapeToolkit\\falloffSmooth.png", out_path, FullPath ) == true)
			return FullPath;
		
		return "\\BlendShapeToolkit\\falloffSmooth.png";
	}

	double	Calculate(double t);
};