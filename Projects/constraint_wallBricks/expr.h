
#pragma once


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: expr.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class CExpression
{
public:
	// a constructor
	CExpression();

	// a destructor
	~CExpression();

public:

	void ExpressionInit();

	void ExpressionParse(const char *szposx, const char *szposy, const char *szposz, 
		const char *szrotx, const char *szroty, const char *szrotz, 
		const char *szscalex, const char *szscaley, const char *szscalez);

	// n - element unique index
	// count - total elements count
	// r is a random value for this element, [0; 1] can be used to add some movement variantions to others
	// f - animation factor, [0; 1]
	// distToCamera - distance between element pivot and camera position in world space

	void ExpressionValue(const int n, const int count, const double r, const double f, const double distToCamera, double &posx, double &posy, double &posz,
		double &rotx, double &roty, double &rotz,
		double &sclx, double &scly, double &sclz );

	const size_t GetErrorCount() const;
	const char *GetLastError() const;

private:

	void	*impl;

};