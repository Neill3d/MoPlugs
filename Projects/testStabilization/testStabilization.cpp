
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: testStabilization.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <conio.h>

#include <vector>
#include <string>

#include <glm\mat4x4.hpp>
#include <glm\matrix.hpp>
#include <glm\gtc\matrix_transform.hpp>

//
// input data - one point per frame

std::vector< glm::vec2 > sourcePositions;

const double gBackProjDepth = 0.1;

glm::mat4	proj = glm::mat4(	2.0542292324120672, 0.0, 0.0, 0.0,
								0.0, 1.1503684335345870, 0.0, 0.0,
								0.0, 0.0, -1.0202020406723022, -1.0,
								0.0, 0.0, -0.20202021281484017, 0.0 );

glm::mat4	modelview = glm::mat4(	1.0, -2.2287413956779756e-019, 5.6012709962688496e-017, 0.0,
									4.9967558551113714e-018, 0.99636018908108082, -0.085243026778223374, 0.0,
									-5.5789835823120698e-017, 0.085243026778223374, 0.99636018908108082, 0.0,
									0.59764808356762211, -157.78639603465336, -14.230821058390440, 1.0 );

// output position - base point from the first frame

glm::vec3	goalPoint = glm::vec3(-0.59101995394759921, 156.00587841062574, 27.409460608297607);

/////////////////////////////////////////////////////////////////////////////////////////////////////
// unproject 2d point in camera view into 3d point in world space

void UnProjectPoints(const double depth, const glm::mat4 &invMVP, glm::vec3 &outPoint)
{
	glm::vec4 p(	0.01 * outPoint[0] * 2.0 - 1.0,
					0.01 * outPoint[1] * 2.0 - 1.0,
					depth,
					1.0 );

	p = invMVP * p;
	
	if (p[3] != 0.0)
	{
		p[0] /= p[3];
		p[1] /= p[3];
		p[2] /= p[3];
		p[3] = 1.0;
	}
	
	outPoint.x = p.x;
	outPoint.y = p.y;
	outPoint.z = p.z;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// read source positions

bool ReadPositions( const char *filename, std::vector<glm::vec2> &outPositions )
{
	FILE *fp = fopen( filename, "r" );
	if (fp == nullptr)
		return false;

	char *data = nullptr;

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data = new char[len+1];
	memset(data, 0, sizeof(char) * len+1);

	size_t readed = fread_s( data, sizeof(char) * len, sizeof(char), len, fp );

	fclose(fp);

	//

	int numberOfFrames = atoi(data);
	sourcePositions.reserve(numberOfFrames);

	char *ptr = strchr( data, '\n' );

	int idx = 0;
	while (ptr != nullptr && idx < numberOfFrames)
	{
		double valx = atof(ptr);
		ptr = strchr( ptr, ' ' );
		double valy = atof(ptr);

		sourcePositions.push_back( glm::vec2(valx, valy) );

		ptr = strchr( ptr, '\n' );
		idx = idx + 1;
	}

	// free mem

	if (data != nullptr)
	{
		delete [] data;
		data = nullptr;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// computation of the mean error between output positions

void TestPoint( const int frameId, const bool test, const glm::vec3 &point )
{
	double len = glm::length( point - goalPoint );

	if (test)
		printf( " > AFTER absolute error - %.5lf\n", len );
	else
		printf( "frame %d, absolute error - %.5lf\n", frameId, len );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN



int _tmain(int argc, _TCHAR* argv[])
{

	std::string aux(argv[0]);

    // get '/' or '\\' depending on unix/mac or windows.
#if defined(_WIN32) || defined(WIN32)
    int pos = aux.rfind('\\');
#else
    int pos = aux.rfind('/');
#endif

    // Get the path and the name
    std::string path = aux.substr(0,pos+1);

	// read positions
	std::string filename = path + "..\\data\\positions.txt"; 
	if (false == ReadPositions( filename.c_str(), sourcePositions ) )
	{
		printf ( "failed to read positions from a data file\n" );
		getch();
		return 0;
	}

	// for each points (for all frames)

	int frameId = 0;

	for (auto iter=sourcePositions.begin(); iter!=sourcePositions.end(); ++iter)
	{

		glm::vec3 point(iter->x, iter->y, 0.0);
		
		

		//
		// compute new modelview matrix here

		glm::mat4 stab = glm::mat4(1.0);

		glm::mat4 m = modelview;

		
		glm::mat4 mvp = proj * m;
		glm::mat4	invMVP = glm::inverse(mvp);

		//
		// 2d -> 3d

		UnProjectPoints( gBackProjDepth, invMVP, point );

		//
		// compare to goal point

		if (frameId == 0)
			goalPoint = point;
		
		TestPoint( frameId, false, point );

		if (frameId > 0)
		{
		
			stab = glm::translate(glm::mat4(1.0), point - goalPoint);
		
			m = modelview * stab;
			mvp = proj * m;
			invMVP = glm::inverse(mvp);

			point = glm::vec3(iter->x, iter->y, 0.0);
			UnProjectPoints( gBackProjDepth, invMVP, point );


			TestPoint( frameId, true, point );
		}

		//
		frameId = frameId + 1;
	}

	getch();
	return 0;
}

