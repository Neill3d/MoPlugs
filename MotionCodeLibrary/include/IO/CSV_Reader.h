
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: CSV_Reader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <math.h>
//#include "Utils\TextUtils.h"

//////////////////////////////////////////////
//! naturalpoint tracking tools format reader
class csv_data 
{
public:
	//! number of samples
	int  framecount;
	//!	while tracked time range
	double	length;
	//! number of trackable bodies
	int	 rigidbodycount;
	//! number of markers
	int  markercount;

	/////////////////////////
	//! marker information for each frame
	struct marker_data 
	{
		//! frame
		int  frame;
		//! timestamp of this frame (real frame time)
		double timestamp;
		//! position in this frame
		FBVector3d	pos;
		//! is marker visible
		bool occulusion;

		//! a constructor
		marker_data() {
			frame = 0;
			timestamp = 0.0;
			occulusion = false;
		}
		//! a constructor with parameters
		/*!
			\param _frame - marker data frame
			\param _pos - setting this frame pos
		*/
		marker_data(int &_frame, double &_timestamp, FBVector3d &_pos)
			: frame(_frame)
			, timestamp(_timestamp)
			, pos(_pos)
		{ occulusion = false; }
		//! output frame info on console
		void print() {
			printf( "marker data - %f; %f; %f\n", pos[0], pos[1], pos[2] );
		}
	};

	/////////////////////////
	//! each marker struct
	struct marker 
	{
		//! first active frame
		int firstframe;
		double firsttime;	// timestamp of a first frame
		int observepos;	// last time when we get a frame (check for increasing)
		//! array of data for this marker
		FBArrayTemplate<marker_data>	data;

		//! a constructor
		marker() {
			firstframe=0;
			firsttime = 0.0;
			observepos = 0;
		}
		//! a constructor with parameters
		/*!
			\param _frame - setting first marker frame
		*/
		marker(int &_frame, double &timestamp)
			: firstframe(_frame)
			, firsttime(timestamp)
		{
			observepos = 0;
		}
		//! GetFramePos
		/*!
			\param frame - frame index
			\param pos - instance to pos vector class
			\return - is marker occoluded in this frame
		*/
		bool GetFramePos(int frame, FBVector3d &pos)
		{
			for (int i=observepos; i<data.GetCount(); i++)
				if (frame == data[i].frame)
				{
					observepos = i;
					pos = data[i].pos;
					return true;
				}
			for (int i=0; i<observepos; i++)
				if (frame == data[i].frame)
				{
					observepos = i;
					pos = data[i].pos;
					return true;
				}
			observepos = 0;
			return false;
		}
		bool near1(double v1, double v2, double f)
		{
			double v = v1-v2;
			return ( (v >= 0.0) && (v < f) );
		}
		bool GetFramePos(double time, FBVector3d &pos)
		{
			for (int i=observepos; i<data.GetCount(); i++)
				if ( near1(data[i].timestamp, time, 0.1) )
				{
					observepos = i;
					pos = data[i].pos;
					//assert( time >= firsttime );
					return true;
				}
			for (int i=0; i<observepos; i++)
			{
				if ( near1(data[i].timestamp, time, 0.1) )
				{
					observepos = i;
					pos = data[i].pos;
					//assert( time >= firsttime );
					return true;
				}
			}
			observepos = 0;
			return false;
		}
	};

	struct marker_filtered : public marker {
		bool  active;
		int activeFrame;

		marker_filtered()
		{
			firstframe = 0;
			active = false;
			activeFrame = 0;
		}
		bool test(int frame, double timestamp, FBVector3d pos)
		{
			bool result = false;
			if (firstframe > frame) return result;

			for(int i=0; i<data.GetCount(); i++)
				if (frame==data[i].frame)
				{
					double dist = sqrt( (data[i].pos[0] + pos[0]) * (data[i].pos[0] + pos[0]) +
										(data[i].pos[1] + pos[1]) * (data[i].pos[1] + pos[1]) +
										(data[i].pos[2] + pos[2]) * (data[i].pos[2] + pos[2]) );
					if (dist < 1.0) {
						result = true;
	/*
						if (!active) {
							active = true;
							activeFrame = frame;

							csv_data::marker_data	framedata(frame, pos);
							data.Add(framedata);
						}
						*/
						active = true;
						activeFrame = frame;
						csv_data::marker_data	framedata(frame, timestamp, pos);
						data.Add(framedata);
					}
				}
			return result;
		}
		void insert(int frame, double timestamp, FBVector3d pos)
		{
			firstframe = frame;
			csv_data::marker_data	framedata(frame, timestamp, pos);
			data.Add(framedata);
			active=true;
			activeFrame = frame;
		}
		void insertOcculuded(int frame, double timestamp)
		{
			if (!active) {
				FBVector3d pos;
				memset( &pos[0], 0, sizeof(double)*3);
				csv_data::marker_data	framedata(frame, timestamp, pos);
				framedata.occulusion = true;
				data.Add(framedata);
			}
		}
	};
	
	//! markers array
	FBArrayTemplate<marker*>  markers;
	//! filtered markers array
	FBArrayTemplate<marker_filtered>	 markers_filtered;


	/////////////////////////////////
	//! rigidbody data for each frame
	struct rigidbody_data {
		//! frame index
		int frame;
		//! position
		FBVector3d	pos;
		//! orientation in quat
		FBVector4d	quat; 
		//! euler angles
		FBVector3d	rot;  

		//! a constructor
		rigidbody_data() {
			frame = 0;
		}
		rigidbody_data(int &_frame, FBVector3d &_pos, FBVector4d &_quat, FBVector3d &_rot)
			: frame(_frame)
			, pos(_pos)
			, quat(_quat)
			, rot(_rot)
		{}
	};
	struct rigidbody {
		char	name[80];

		int markercount;
		FBArrayTemplate<marker_data>	marker_setup;

		int firstframe;
		FBArrayTemplate<rigidbody_data>		data;
		rigidbody() {
			firstframe = 0;
			markercount = 0;
		}
		rigidbody(int &_frame, int &_markercount)
			: firstframe(_frame)
			, markercount(_markercount)
		{}
		void print_marker_setup()
		{
			if (marker_setup.GetCount() != markercount)
				printf("!! > error: marker_setup count is not equal to variable!\n" );
			for(int i=0; i<markercount; i++)
				marker_setup[i].print();
		}
	};

	//! rigid bodies
	FBArrayTemplate<rigidbody*>	rigidbodies;

	//! a constructor
	csv_data() {
		framecount = 0;
		length = 0.0;
		rigidbodycount = 0;
		markercount = 0;
	}
	//! free all reader allocated data
	void FreeData()
	{
	}

	//! a destructor
	~csv_data() {
		FreeData();
	}

	//! read information from *.csv text file
	bool Read(const char *pFileName)
	{
		return false;
	}

private:
	//! import frame data from text line
	// TODO:
	//void ImportFrameData( CEUtils::StringTokens	&tokens );
	//! filter frame data (markers numerations)
	void FilterFrameData();
	//! create new optical rigid body form text line parsing
	void CreateOpticalRigidBody( char* line );
};