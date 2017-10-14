
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MoBuOpticalUtils.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "MoBuOpticalUtils.h"


void OpticalDone()
{
	FBModelList pList;

	FBFindModelsOfType( pList, FBModelOptical::TypeInfo );

	FBModelOptical	*pOptical = (pList.GetCount() > 0) ? (FBModelOptical*)pList[0] : nullptr;
	if (nullptr == pOptical) return;

	pOptical->ExportSetup();

	FBTime startTime = pOptical->SamplingStart;
	FBTime stopTime = pOptical->SamplingStop;
	FBTime period = pOptical->SamplingPeriod;

	for (int i=0; i<pOptical->Children.GetCount(); ++i)
	{
		if (pOptical->Children[i]->Is( FBModelMarkerOptical::TypeInfo ) )
		{
			FBModelMarkerOptical *pMarker = (FBModelMarkerOptical*) pOptical->Children[i];

			int samples = pMarker->ExportBegin();
			FBVector4d v;
			FBTime currTime = startTime;

			for (int i=0; i<samples; ++i)
			{
				pMarker->ExportKey(&v[0], &v[1], &v[2], &v[3]);

				pMarker->Translation.GetAnimationNode()->KeyAdd(currTime, &v[0]);

				currTime += period;
			}
			pMarker->ExportEnd();

		}
	}

}

void OpticalMarkerSegmentCutOnCurrentTime()
{
	FBModelList pList;

	FBFindModelsOfType( pList, FBModelOptical::TypeInfo );

	FBModelOptical	*pOptical = (pList.GetCount() > 0) ? (FBModelOptical*)pList[0] : nullptr;
	if (nullptr == pOptical) return;

	FBTime currTime;
	currTime = FBSystem().LocalTime;

	for (int i=0; i<pOptical->Children.GetCount(); ++i)
	{
		if (pOptical->Children[i]->Is( FBModelMarkerOptical::TypeInfo ) )
		{	
			FBModelMarkerOptical *pMarker = (FBModelMarkerOptical*) pOptical->Children[i];
			
			if (pMarker->Selected)
			{
				//Getting the Segments, because you have to get the segments to get the gaps count to renew, (BUG)
				pMarker->SetModelOptical(pOptical); 
				
				int index = pMarker->Segments.GetIndexAt(currTime);
				if (index >= 0) pMarker->Segments[index].Cut(currTime);
			}

			//FBTrace("Marker Segment Count is %d\n", pMarker->Segments.GetCount());
			//FBTrace("Marker Gap Count is %d\n", pMarker->Gaps.GetCount()); //now the gap count doesn't return 0 but the actual value...
		}
	}
}

void OpticalMarkerSegmentRemoveOnCurrentTime()
{
	FBModelList pList;

	FBFindModelsOfType( pList, FBModelOptical::TypeInfo );

	FBModelOptical	*pOptical = (pList.GetCount() > 0) ? (FBModelOptical*)pList[0] : nullptr;
	if (nullptr == pOptical) return;

	FBTime currTime;
	currTime = FBSystem().LocalTime;

	for (int i=0; i<pOptical->Children.GetCount(); ++i)
	{
		if (pOptical->Children[i]->Is( FBModelMarkerOptical::TypeInfo ) )
		{	
			FBModelMarkerOptical *pMarker = (FBModelMarkerOptical*) pOptical->Children[i];
			
			if (pMarker->Selected)
			{
				//Getting the Segments, because you have to get the segments to get the gaps count to renew, (BUG)
				pMarker->SetModelOptical(pOptical); 
				
				int index = pMarker->Segments.GetIndexAt(currTime);
				if (index >= 0) pMarker->Segments.RemoveAt(index);
			}

			//FBTrace("Marker Segment Count is %d\n", pMarker->Segments.GetCount());
			//FBTrace("Marker Gap Count is %d\n", pMarker->Gaps.GetCount()); //now the gap count doesn't return 0 but the actual value...
		}
	}
}

void OpticalMarkerGapChangeInterpolation(int newMode)
{
	FBModelList pList;

	FBFindModelsOfType( pList, FBModelOptical::TypeInfo );

	FBModelOptical	*pOptical = (pList.GetCount() > 0) ? (FBModelOptical*)pList[0] : nullptr;
	if (nullptr == pOptical) return;

	FBTime currTime;
	currTime = FBSystem().LocalTime;

	for (int i=0; i<pOptical->Children.GetCount(); ++i)
	{
		if (pOptical->Children[i]->Is( FBModelMarkerOptical::TypeInfo ) )
		{	
			FBModelMarkerOptical *pMarker = (FBModelMarkerOptical*) pOptical->Children[i];
			
			if (pMarker->Selected)
			{
				//Getting the Segments, because you have to get the segments to get the gaps count to renew, (BUG)
				pMarker->SetModelOptical(pOptical); 
				
				for (int j=0; j<pMarker->Gaps.GetCount(); ++j)
				{
					FBTimeSpan timespan = pMarker->Gaps[j].TimeSpan;
					if (timespan & currTime)
					{
						pMarker->Gaps[j].Interpolation = (FBGapMode) newMode;
					}
				}
			}
		}
	}
}