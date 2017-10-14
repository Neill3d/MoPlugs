
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: CSV_Reader.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "IO\CSV_Reader.h"

#define		CSV_BUFFER_SIZE		4096
#define		CSV_TOKEN_SIZE		256

using namespace CEUtils;

void csv_data::FreeData() {
	for(int i=0; i<rigidbodycount; i++) {
		if (rigidbodies[i]) {
			delete rigidbodies[i];
			rigidbodies[i] = NULL;
		}
	}
	rigidbodies.Clear();
	rigidbodycount = 0;
	framecount = 0;
	markercount = 0;

	for (int i=0; i<markercount; i++)
	{
		if (markers[i])
		{
			markers[i]->data.Clear();
			delete markers[i];
			markers[i] = NULL;
		}
	}
	markers.Clear();
	markers_filtered.Clear();
}

bool	csv_data::Read(const char *pFileName)
{
	//printf("importing file... %s\n", pFileName );

	FreeData();

	// if successful, return true
	FILE *f;
	if ( (f = fopen(pFileName, "r")) == NULL )
		return false;

	char buffer[CSV_BUFFER_SIZE];
	char ptr[CSV_TOKEN_SIZE];

	{while(!feof(f) ) {
		memset(buffer,0,sizeof(char)*CSV_BUFFER_SIZE);
		fgets(buffer, CSV_BUFFER_SIZE, f);

		// parsing each line
		StringTokens		tokens(buffer, ",");

		if (strstr(buffer, "info") == buffer) {
			// whole optical model info

			if (strstr(buffer, "framecount") != NULL) {
				// read framecount
				if (tokens.GetCount() > 1) {
					tokens.GetToken(2, ptr, CSV_TOKEN_SIZE);
					sscanf( ptr, "%d", &framecount );

					//: create optical model
					/*
					modelRoot = CreateOpticalModel(frameCount);
					if (modelRoot->ImportSetup() == false ) {
						printf("optical model is not ready...\n");
					}
					*/
				}
			}
			else {
				if (strstr(buffer, "trackablecount") != NULL) {
					//read rigidbodycount
					if (tokens.GetCount() > 1) {
						tokens.GetToken(2, ptr, CSV_TOKEN_SIZE);
						sscanf( ptr, "%d", &rigidbodycount );
						rigidbodies.SetCount(rigidbodycount);
						//printf("rigidbodies count %d", rigidbodycount);
					}
				}
			}

		}
		else
		if (strstr(buffer, "rigidbody") == buffer ) {
			//rigid body stats
			CreateOpticalRigidBody(buffer);
		}
		else
		if (strstr(buffer, "frame") == buffer ) {
			//reading each frame data
			ImportFrameData(tokens);
			//FilterFrameData();
		}
	}}

	fclose(f);

	/*
	// end parsing markers data
	printf("markers count %d\n", modelRoot->Markers.GetCount() );
	for (int i=0; i<modelRoot->Markers.GetCount(); i++)
		modelRoot->Markers[i]->ImportEnd();
		*/
	
	//CreateOpticalModel(framecount);
	//FreeData();

	return true;
}


void csv_data::ImportFrameData( StringTokens	&tokens )
{
	FBVector3d	lPos;
	FBVector4d	lQuat;
	FBVector3d	lRot;  // euler angles
	
	int frame=0;
	double timestamp=0.0;
	int rigidbodiescount=0;
	int markerscount=0;
	//int n=0;

	char p[CSV_TOKEN_SIZE];
	//char *p;
	//p = strtok( (char*)line, "," );
	//printf( "%s\n", p );
	for (int n=1; n<tokens.GetCount(); n++)
	{
		//p = strtok(NULL, "," );
		//n++;
		tokens.GetToken(n, p, CSV_TOKEN_SIZE);
		//if() {

			switch(n) {
			// frame ID (integer)
			case 1:
				sscanf(p, "%d", &frame);
				/*
				if (frame > 19)
				{
					printf( "local frame |%d|\n", frame );
					OutputDebugString(p);
				}
				*/
				break;
			// time stamp (double) in seconds
			case 2:
				sscanf(p, "%lf", &timestamp);
				if (timestamp > length) length = timestamp;		// increase global system time range
				//printf( "frame timestamp %lf\n", timestamp );
				break;
			case 3:
				// number of rigid bodies tracked in current frame
				sscanf(p, "%d", &rigidbodiescount);
				//printf( "number of frame rigid bodies |%d|\n", rigidbodiescount );
				n++;

				// rigid bodies information
				int i,j;
				for(i=0; i<rigidbodiescount; i++){
					int rigidbodyNdx = 0;
					// rigidbody ID
					//p = strtok(NULL, ",");
					tokens.GetToken(n, p, CSV_TOKEN_SIZE);
					n++;
					if (!p) break;
					sscanf(p, "%d", &rigidbodyNdx);
					//printf("rigidbody index %d\n", rigidbodyNdx);
					// pos
					for (j=0; j<3; j++) {
						//p = strtok(NULL, ",");
						tokens.GetToken(n, p, CSV_TOKEN_SIZE);
						n++;
						if (!p) break;

						float value;
						sscanf(p, "%f", &value);
						lPos[j] = value*100;
					}
					// quat
					for (j=0; j<4; j++) {
						//p = strtok(NULL, ",");
						tokens.GetToken(n, p, CSV_TOKEN_SIZE);
						n++;
						if (!p) break;

						float value;
						sscanf(p, "%f", &value);
						lQuat[j] = value;
					}
					// euler angles (heading, attitude, bank)
					
					for (j=0; j<3; j++) {
						//p = strtok(NULL, ",");
						tokens.GetToken(n, p, CSV_TOKEN_SIZE);
						n++;
						if (!p) break;

						float value;
						sscanf(p, "%f", &value);
						lRot[j] = value;
					}
					

					//csv_data::rigidbody_data	rigidbody_data(frame, lPos, lQuat, lRot);
					//rigidbodies[rigidbodyNdx-1]->data.Add(rigidbody_data);
				}

				//p = strtok(NULL, ",");
				tokens.GetToken(n, p, CSV_TOKEN_SIZE);
				//n++;
				//if (!p) break;

				sscanf(p, "%d", &markerscount);
				//printf( "number of frame markers |%d|\n", markerscount );

				if (markerscount > markercount) {

					for(int i=markercount; i<markerscount; i++){
						csv_data::marker		*marker = new csv_data::marker(frame, timestamp);
						markers.Add(marker);
					}

					markercount = markerscount;
				}

				break;
			default:
				if (markerscount>0) {
					int i;
					for (i=0; i<markerscount; i++)
					{
						for (int j=0; j<3; j++, n++) {
							// get value token
							tokens.GetToken(n, p, CSV_TOKEN_SIZE);

							float value;
							sscanf(p, "%f", &value);
							lPos[j] = value*100;
						}

						// read marker id
						int id;
						tokens.GetToken(n, p, CSV_TOKEN_SIZE);
						n++;
						sscanf(p, "%d", &id);

						csv_data::marker_data	framedata(frame, timestamp, lPos);

						//if ( (i > 27) && (frame == 20) )
						//	printf( "import marker key - [%f, %f, %f]\n", lPos[0], lPos[1], lPos[2] );
						markers[i]->data.Add(framedata);
					}
				}
				break;
			}
			//printf( "%s\n", p );
		//}

	} //while(p);
	
}

void csv_data::FilterFrameData()
{
	FBVector3d pos;	
	double timestamp = 0.0;
	bool inserted=false;

	{for(int frame=0; frame<framecount; frame++)
	{

		for (int i=0; i<markercount; i++)
		{
			if (markers[i]->firstframe > frame) continue;
			if (!markers[i]->GetFramePos(frame, pos) ) continue;

			inserted = false;
			for (int j=0; j<markers_filtered.GetCount(); j++)
			{
				inserted = markers_filtered[j].test(frame, timestamp, pos);
				if (inserted) break;
			}

			if (!inserted) {
				csv_data::marker_filtered	marker_filtered;
				marker_filtered.insert(frame, timestamp, pos);
				markers_filtered.Add(marker_filtered);
				
			}
		}

		for (int i=0; i<markers_filtered.GetCount(); i++)
			markers_filtered[i].insertOcculuded(frame, timestamp);
	}}
}

void csv_data::CreateOpticalRigidBody( char* line )
{
	FBVector3d  lPos;

	int count = 0;
	int n=0;
	char *p;
	char name[80];
	char markerName[80];

	memset(name, 0, sizeof(char)*80);
	memset(markerName, 0, sizeof(char)*80);

	csv_data::rigidbody		*rigidbody = new csv_data::rigidbody;

	p = strtok( line, "," );
	printf( "%s\n", p );
	do {
		p = strtok(NULL, "," );
		n++;

		if(p) {

			switch(n) {
			case 1:
				// name of the rigid body
				strncpy(name, p+1, strlen(p)-2);
				printf( "ribig body name |%s|\n", name );
				break;
			case 2:
				// rigidbody ID
				printf("scan rigidbody id\n" );
				sscanf(p, "%d", &count);
				printf( "rigidbody  ID %d\n", count );
				//rigidbody=csv.rigidbodies[count-1];
				strcpy(rigidbody->name, name);

				printf( "rigidbody  ID %d\n", count );
				break;
			case 3:
				// count of the markers in the rigid body
				sscanf(p, "%d", &rigidbody->markercount);
				rigidbody->marker_setup.SetCount(rigidbody->markercount);
				break;
			default:
				if (n>3) {
					int markerNdx;
					markerNdx = (n-4);
					markerNdx = markerNdx%3;
					float value;
					sscanf(p, "%f", &value);

					lPos[markerNdx] = value*100;

					if (markerNdx==2) {
						markerNdx = (n-4) / 3;
						memcpy(rigidbody->marker_setup[markerNdx].pos, lPos, sizeof(double)*3);

						printf("marker[%d].pos - %f; %f; %f\n", markerNdx, lPos[0], lPos[1], lPos[2] );
						rigidbody->marker_setup[markerNdx].print();
					}
				}
				break;
			}
			printf( "%s\n", p );
		}

	} while(p);

	rigidbodies[count-1]=rigidbody;
	rigidbodies[0]->print_marker_setup();

/*
	model->RigidBodies.Add( lList, name );
	model->RigidBodies[model->RigidBodies.GetCount() - 1].Snap();

	FBRigidBody rigidbody = model->RigidBodies[model->RigidBodies.GetCount()-1];

	printf("list count - %d\n", rigidbody.Markers.GetCount() );
	for(int i=0; i<rigidbody.Markers.GetCount(); i++)
	{
		HFBModel marker = rigidbody.Markers[i];
		marker->GetVector( lPos, kModelTranslation, true );

		printf("pos - %f; %f; %f\n", lPos[0], lPos[1], lPos[2] );
	}
*/
	//printf( "|%s| - |%s|\n", temp2, temp );
}