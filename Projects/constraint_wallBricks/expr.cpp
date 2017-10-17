
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: expr.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "exprtk.hpp"
#include "expr.h"

class CExpressionImpl
{
protected:

	struct selectfunc_t : public exprtk::ifunction<double>
	{
	   selectfunc_t()
	   : exprtk::ifunction<double>(3)
	   {}

	   inline double operator()(const double &x, const double &v1, const double &v2)
	   {
		   double res = x-v1;
		   if (res < 0.0)
			   res = 0.0;
		   res = res * 1.0 / (v2-v1);
		   if (res > 1.0)
			   res = 1.0;

		  return  res;
	   }
	};


	// expressions
	typedef exprtk::symbol_table<double>	symbol_table_t;
	typedef exprtk::expression<double>		expression_t;
	typedef exprtk::parser<double>			parser_t;
	
	double				elementIndex;
	double				normElementIndex;
	double				randomF;
	double				animFactor;
	double				distToCamera;
	selectfunc_t		selectfunc;			
	symbol_table_t		symbol_table;

	expression_t		expr_posx, expr_posy, expr_posz;
	expression_t		expr_rotx, expr_roty, expr_rotz;
	expression_t		expr_scalex, expr_scaley, expr_scalez;

	size_t				lastErrorCount;
	std::string			lastError;

public:

	
	CExpressionImpl()
	{
		elementIndex = 0.0;
		normElementIndex = 0.0;
		randomF = 0.0;
		animFactor = 0.0;
		distToCamera = 0.0;
	}

	void ExpressionInit()
	{
		symbol_table.add_variable("n", elementIndex);
		symbol_table.add_variable("nN", normElementIndex);
		symbol_table.add_variable("rF", randomF);
		symbol_table.add_variable("f", animFactor);
		symbol_table.add_variable("distToCamera", distToCamera );
		symbol_table.add_function( "select", selectfunc );
		symbol_table.add_constants();

		expr_posx.register_symbol_table(symbol_table);
		expr_posy.register_symbol_table(symbol_table);
		expr_posz.register_symbol_table(symbol_table);

		expr_rotx.register_symbol_table(symbol_table);
		expr_roty.register_symbol_table(symbol_table);
		expr_rotz.register_symbol_table(symbol_table);

		expr_scalex.register_symbol_table(symbol_table);
		expr_scaley.register_symbol_table(symbol_table);
		expr_scalez.register_symbol_table(symbol_table);
	}

	bool ExpressionParse(const char *szposx, const char *szposy, const char *szposz, 
		const char *szrotx, const char *szroty, const char *szrotz, 
		const char *szscalex, const char *szscaley, const char *szscalez)
	{
		std::string expression_string_posx( szposx );
		std::string expression_string_posy( szposy );
		std::string expression_string_posz( szposz );

		std::string expression_string_rotx( szrotx );
		std::string expression_string_roty( szroty );
		std::string expression_string_rotz( szrotz );

		std::string expression_string_scalex( szscalex );
		std::string expression_string_scaley( szscaley );
		std::string expression_string_scalez( szscalez );

		parser_t parser;

		lastErrorCount = 0;
		lastError = "";

		try
		{
			if (false == parser.compile(expression_string_posx, expr_posx) )
				throw 0;
			if (false == parser.compile(expression_string_posy, expr_posy) )
				throw 0;
			if (false == parser.compile(expression_string_posz, expr_posz) )
				throw 0;
		
			if (false == parser.compile(expression_string_rotx, expr_rotx) )
				throw 0;
			if (false == parser.compile(expression_string_roty, expr_roty) )
				throw 0;
			if (false == parser.compile(expression_string_rotz, expr_rotz) )
				throw 0;

			if (false == parser.compile(expression_string_scalex, expr_scalex) )
				throw 0;
			if (false == parser.compile(expression_string_scaley, expr_scaley) )
				throw 0;
			if (false == parser.compile(expression_string_scalez, expr_scalez) )
				throw 0;
		}
		catch (int err)
		{
			lastErrorCount = parser.error_count();
			lastError = parser.error();
		}

		return true;
	}

	void ExpressionValue(const int n, const int count, const double r, const double f, const double _distToCamera, double &posx, double &posy, double &posz,
		double &rotx, double &roty, double &rotz,
		double &sclx, double &scly, double &sclz )
	{

		elementIndex = (double) n;
		if (count > 0)
			normElementIndex = 1.0 * (double) n / (double) count;
		else normElementIndex = 0.0;

		randomF = r;
		animFactor = f;
		distToCamera = _distToCamera;

		posx = expr_posx.value();
		posy = expr_posy.value();
		posz = expr_posz.value();

		rotx = expr_rotx.value();
		roty = expr_roty.value();
		rotz = expr_rotz.value();

		sclx = expr_scalex.value();
		scly = expr_scaley.value();
		sclz = expr_scalez.value();
	}

	const size_t GetErrorCount() const
	{
		return lastErrorCount;
	}

	const char *GetError() const
	{
		return lastError.c_str();
	}

};

//////////////////////////////////////////////////////////////////
//

CExpression::CExpression()
{
	CExpressionImpl *pimpl = new CExpressionImpl();
	impl = pimpl;
}

CExpression::~CExpression()
{
	CExpressionImpl *pimpl = (CExpressionImpl*) impl;
	if (pimpl)
	{
		delete pimpl;
		impl = nullptr;
	}
}

void CExpression::ExpressionInit()
{
	CExpressionImpl *pimpl = (CExpressionImpl*) impl;
	if (pimpl)
		pimpl->ExpressionInit();
}

void CExpression::ExpressionParse(const char *szposx, const char *szposy, const char *szposz, 
	const char *szrotx, const char *szroty, const char *szrotz, 
	const char *szscalex, const char *szscaley, const char *szscalez)
{
	CExpressionImpl *pimpl = (CExpressionImpl*) impl;
	if (pimpl)
		pimpl->ExpressionParse(szposx, szposy, szposz, szrotx, szroty, szrotz, szscalex, szscaley, szscalez);
}

void CExpression::ExpressionValue(const int n, const int count, const double r, const double f, const double distToCamera, double &posx, double &posy, double &posz,
	double &rotx, double &roty, double &rotz,
	double &sclx, double &scly, double &sclz )
{
	CExpressionImpl *pimpl = (CExpressionImpl*) impl;
	if (pimpl)
		pimpl->ExpressionValue(n, count, r, f, distToCamera, posx, posy, posz, rotx, roty, rotz, sclx, scly, sclz);
}

const size_t CExpression::GetErrorCount() const
{
	CExpressionImpl *pimpl = (CExpressionImpl*) impl;
	if (pimpl)
		return pimpl->GetErrorCount();

	return 0;
}

const char *CExpression::GetLastError() const
{
	CExpressionImpl *pimpl = (CExpressionImpl*) impl;
	if (pimpl)
		return pimpl->GetError();

	return nullptr;
}
