
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: facialRetargeting_constraint.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declarations
#include "facialRetargeting_constraint.h"
#include "algorithm\math3d_mobu.h"

#include "IO\tinyxml.h"

//--- Registration defines
#define	ORCONSTRAINTFACIALRETARGET__CLASS		ORCONSTRAINTFACIALRETARGET__CLASSNAME
#define ORCONSTRAINTFACIALRETARGET__NAME		"Facial Retargeting"
#define ORCONSTRAINTFACIALRETARGET__LABEL		"Facial Retargeting"
#define ORCONSTRAINTFACIALRETARGET__DESC		"Facial Retargeting"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINTFACIALRETARGET__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTFACIALRETARGET__NAME,
								ORCONSTRAINTFACIALRETARGET__CLASS,
								ORCONSTRAINTFACIALRETARGET__LABEL,
								ORCONSTRAINTFACIALRETARGET__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

//////////////////////////////////////////////////

#define XML_ERROR_LOAD_FILE_FAILED			"failed to load xml file"
#define XML_ERROR_HEADER_NODE_NOT_EXIST		"header node is not exist"
#define XML_ERROR_WRONG_FILE_VERSION		"wrong file version"		
#define XML_ERROR_MAPPING_NODE_NOT_EXIST	"mapping node is not exist"
#define XML_ERROR_NULL_MAPPING_ELEMENT		"null mapping element"

const char *MappingNames[] = {

	// Mouth
	"MOUTH_LIP_OUTER_VEE_RIGHT",
	"MOUTH_LIP_OUTER_VEE_RIGHT_MOUTH_LIP_OUTER_CORNER_RIGHT_0_33333",
	"MOUTH_LIP_OUTER_VEE_RIGHT_MOUTH_LIP_OUTER_CORNER_RIGHT_0_66667",
	"MOUTH_LIP_OUTER_CORNER_RIGHT",
	"MOUTH_LIP_OUTER_CORNER_RIGHT_MOUTH_LIP_OUTER_BOTTOM_MIDDLE_0_33333",
	"MOUTH_LIP_OUTER_CORNER_RIGHT_MOUTH_LIP_OUTER_BOTTOM_MIDDLE_0_66667",
	"MOUTH_LIP_OUTER_BOTTOM_MIDDLE",
	"MOUTH_LIP_OUTER_BOTTOM_MIDDLE_MOUTH_LIP_OUTER_CORNER_LEFT_0_33333",
	"MOUTH_LIP_OUTER_BOTTOM_MIDDLE_MOUTH_LIP_OUTER_CORNER_LEFT_0_66667",
	"MOUTH_LIP_OUTER_CORNER_LEFT",
	"MOUTH_LIP_OUTER_CORNER_LEFT_MOUTH_LIP_OUTER_VEE_LEFT_0_33333",
	"MOUTH_LIP_OUTER_CORNER_LEFT_MOUTH_LIP_OUTER_VEE_LEFT_0_66667",
	"MOUTH_LIP_OUTER_VEE_LEFT",
	"MOUTH_LIP_OUTER_TOP_MIDDLE",

	"MOUTH_LIP_INNER_TOP_MIDDLE_MOUTH_LIP_INNER_CORNER_RIGHT_0_66667",
	"MOUTH_LIP_INNER_CORNER_RIGHT",
	"MOUTH_LIP_INNER_CORNER_RIGHT_MOUTH_LIP_INNER_BOTTOM_MIDDLE_0_33333",
	"MOUTH_LIP_INNER_CORNER_RIGHT_MOUTH_LIP_INNER_BOTTOM_MIDDLE_0_66667",
	"MOUTH_LIP_INNER_BOTTOM_MIDDLE",
	"MOUTH_LIP_INNER_BOTTOM_MIDDLE_MOUTH_LIP_INNER_CORNER_LEFT_0_33333",
	"MOUTH_LIP_INNER_BOTTOM_MIDDLE_MOUTH_LIP_INNER_CORNER_LEFT_0_66667",
	"MOUTH_LIP_INNER_CORNER_LEFT",
	"MOUTH_LIP_INNER_CORNER_LEFT_MOUTH_LIP_INNER_TOP_MIDDLE_0_33333",
	"MOUTH_LIP_INNER_CORNER_LEFT_MOUTH_LIP_INNER_TOP_MIDDLE_0_66667",
	"MOUTH_LIP_INNER_TOP_MIDDLE",
	"MOUTH_LIP_INNER_TOP_MIDDLE_MOUTH_LIP_INNER_CORNER_RIGHT_0_33333",

	// Nose
	"NOSTRIL_OUTER_RIGHT",
	"NOSE_LOWER_MIDDLE",
	"NOSTRIL_OUTER_LEFT",

	// right eye area

	"EYE_TOP_RIGHT",
	"EYE_CORNER_OUTER_RIGHT",
	"EYE_BOTTOM_RIGHT",
	"EYE_CORNER_INNER_RIGHT",

	"PUPIL_OUTER_RIGHT",
	"PUPIL_INNER_RIGHT",

	"BROW_INNER_RIGHT",
	"BROW_INNER_RIGHT_BROW_MIDDLE_RIGHT_0_5",
	"BROW_MIDDLE_RIGHT",
	"BROW_MIDDLE_RIGHT_BROW_OUTER_RIGHT_0_5",
	"BROW_OUTER_RIGHT",

	// left eye area

	"EYE_TOP_LEFT",
	"EYE_CORNER_INNER_LEFT",
	"EYE_BOTTOM_LEFT",
	"EYE_CORNER_OUTER_LEFT",

	"PUPIL_OUTER_LEFT",
	"PUPIL_INNER_LEFT",

	"BROW_OUTER_LEFT",
	"BROW_MIDDLE_LEFT_BROW_OUTER_LEFT_0_5",
	"BROW_MIDDLE_LEFT",
	"BROW_INNER_LEFT_BROW_MIDDLE_LEFT_0_5",
	"BROW_INNER_LEFT",

	"BASE COUNT",

	"JAW_RIGHT",
	"JAW_MIDDLE",
	"JAW_LEFT",
	"JAW_ROTATION",

	// Sticky lips (addon nodes)
	"MOUTH_LIP_INNER_STICKY_LEFT_UP",
	"MOUTH_LIP_INNER_STICKY_LEFT_DOWN",
	"MOUTH_LIP_INNER_STICKY_RIGHT_UP",
	"MOUTH_LIP_INNER_STICKY_RIGHT_DOWN",

	"Eyes_Level"
};


void AddPropertyViewForFacial(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(ORCONSTRAINTFACIALRETARGET__CLASSSTR, pPropertyName, pHierarchy);
}
 
void ConstraintFacialRetargeting::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForFacial( "Active", "" );
	AddPropertyViewForFacial( "Characterize", "" );
	AddPropertyViewForFacial( "Grab Positions", "" );

	//
	AddPropertyViewForFacial( "Camera Setup", "", true );
	AddPropertyViewForFacial( "Camera For Mouth", "Camera Setup" );
	AddPropertyViewForFacial( "Camera For Nose", "Camera Setup" );
	AddPropertyViewForFacial( "Camera For Left Eye", "Camera Setup" );
	AddPropertyViewForFacial( "Camera For Right Eye", "Camera Setup" );

	AddPropertyViewForFacial( "Camera FOV", "Camera Setup" );
	AddPropertyViewForFacial( "Camera Width", "Camera Setup" );
	AddPropertyViewForFacial( "Camera Height", "Camera Setup" );
	AddPropertyViewForFacial( "Use Values From Optical Root", "Camera Setup" );
	AddPropertyViewForFacial( "Apply Camera Settings", "Camera Setup" );

	//
	AddPropertyViewForFacial( "Backing Setup", "", true );
	AddPropertyViewForFacial( "Backing For Head Markers", "Backing Setup" );
	AddPropertyViewForFacial( "Use Nose Backing", "Backing Setup" );
	AddPropertyViewForFacial( "Backing For Nose Markers", "Backing Setup" );
	AddPropertyViewForFacial( "Use Eyes Backing", "Backing Setup" );
	AddPropertyViewForFacial( "Backing For Eyes Markers", "Backing Setup" );
	AddPropertyViewForFacial( "Use Eyes Level Backing", "Backing Setup" );
	AddPropertyViewForFacial( "Backing For Eyes Level", "Backing Setup" );
	AddPropertyViewForFacial( "Use Jaw Backing", "Backing Setup" );
	AddPropertyViewForFacial( "Backing For Jaw Markers", "Backing Setup" );

	//
	AddPropertyViewForFacial( "Collision Setup", "", true );
	AddPropertyViewForFacial( "Collision Thickness", "Collision Setup" );
	AddPropertyViewForFacial( "Use Left Eye Collision", "Collision Setup" );
	AddPropertyViewForFacial( "Left Eye Collision", "Collision Setup" );
	AddPropertyViewForFacial( "Use Right Eye Collision", "Collision Setup" );
	AddPropertyViewForFacial( "Right Eye Collision", "Collision Setup" );

	//
	// MOUTH
	//

	//
	AddPropertyViewForFacial( "Mapping Source", "", true );
	
	AddPropertyViewForFacial( "Mouth Outer", "Mapping Source", true );
	for (int i=eMouthLipOuterVeeRight; i<=eMouthLipOuterTopMiddle; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Source"), "Mapping Source.Mouth Outer" );
	
	AddPropertyViewForFacial( "Mouth Inner", "Mapping Source", true );
	for (int i=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_066; i<=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_033; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Source"), "Mapping Source.Mouth Inner" );
	
	AddPropertyViewForFacial( "Nose", "Mapping Source", true );
	for (int i=eNostrilOuterRight; i<=eNostrilOuterLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Source"), "Mapping Source.Nose" );

	AddPropertyViewForFacial( "Left Eye", "Mapping Source", true );
	for (int i=eEyeTopLeft; i<=ePupilInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Source"), "Mapping Source.Left Eye" );

	AddPropertyViewForFacial( "Right Eye", "Mapping Source", true );
	for (int i=eEyeTopRight; i<=ePupilInnerRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Source"), "Mapping Source.Right Eye" );

	AddPropertyViewForFacial( "Left Brow", "Mapping Source", true );
	for (int i=eBrowOuterLeft; i<=eBrowInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Source"), "Mapping Source.Left Brow" );

	AddPropertyViewForFacial( "Right Brow", "Mapping Source", true );
	for (int i=eBrowInnerRight; i<=eBrowOuterRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Source"), "Mapping Source.Right Brow" );

	AddPropertyViewForFacial( "Jaw", "Mapping Source", true );
	for (int i=eJaw1; i<=eJaw3; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Source"), "Mapping Source.Jaw" );

	//
	// NOSE
	

	//
	AddPropertyViewForFacial( "Mapping Destination", "", true );
	
	AddPropertyViewForFacial( "Mouth Outer", "Mapping Destination", true );
	for (int i=eMouthLipOuterVeeRight; i<=eMouthLipOuterTopMiddle; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Mouth Outer" );
	
	AddPropertyViewForFacial( "Mouth Inner", "Mapping Destination", true );
	for (int i=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_066; i<=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_033; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Mouth Inner" );
	
	AddPropertyViewForFacial( "Nose", "Mapping Destination", true );
	for (int i=eNostrilOuterRight; i<=eNostrilOuterLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Nose" );

	AddPropertyViewForFacial( "Left Eye", "Mapping Destination", true );
	for (int i=eEyeTopLeft; i<=ePupilInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Left Eye" );

	AddPropertyViewForFacial( "Right Eye", "Mapping Destination", true );
	for (int i=eEyeTopRight; i<=ePupilInnerRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Right Eye" );

	AddPropertyViewForFacial( "Left Brow", "Mapping Destination", true );
	for (int i=eBrowOuterLeft; i<=eBrowInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Left Brow" );

	AddPropertyViewForFacial( "Right Brow", "Mapping Destination", true );
	for (int i=eBrowInnerRight; i<=eBrowOuterRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Right Brow" );

	AddPropertyViewForFacial( "Jaw", "Mapping Destination", true );
	for (int i=eJaw1; i<=eJawRotation; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Jaw" );

	AddPropertyViewForFacial( "Sticky Lips", "Mapping Destination", true );
	for (int i=eStickyLipUpLeft; i<=eStickyLipDownRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Destination"), "Mapping Destination.Sticky Lips" );

	//
	AddPropertyViewForFacial( "Mapping Offset", "", true );
	
	AddPropertyViewForFacial( "Mouth Outer", "Mapping Offset", true );
	for (int i=eMouthLipOuterVeeRight; i<=eMouthLipOuterTopMiddle; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Offset"), "Mapping Offset.Mouth Outer" );
	
	AddPropertyViewForFacial( "Mouth Inner", "Mapping Offset", true );
	for (int i=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_066; i<=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_033; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Offset"), "Mapping Offset.Mouth Inner" );
	
	AddPropertyViewForFacial( "Nose", "Mapping Offset", true );
	for (int i=eNostrilOuterRight; i<=eNostrilOuterLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Offset"), "Mapping Offset.Nose" );

	AddPropertyViewForFacial( "Left Eye", "Mapping Offset", true );
	for (int i=eEyeTopLeft; i<=ePupilInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Offset"), "Mapping Offset.Left Eye" );

	AddPropertyViewForFacial( "Right Eye", "Mapping Offset", true );
	for (int i=eEyeTopRight; i<=ePupilInnerRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Offset"), "Mapping Offset.Right Eye" );

	AddPropertyViewForFacial( "Left Brow", "Mapping Offset", true );
	for (int i=eBrowOuterLeft; i<=eBrowInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Offset"), "Mapping Offset.Left Brow" );

	AddPropertyViewForFacial( "Right Brow", "Mapping Offset", true );
	for (int i=eBrowInnerRight; i<=eBrowOuterRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Offset"), "Mapping Offset.Right Brow" );

	AddPropertyViewForFacial( "Jaw", "Mapping Offset", true );
	for (int i=eJaw1; i<=eJawRotation; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Offset"), "Mapping Offset.Jaw" );

	//
	AddPropertyViewForFacial( "Mapping Pivot Points", "", true );
	
	AddPropertyViewForFacial( "Mouth Outer", "Mapping Pivot Points", true );
	for (int i=eMouthLipOuterVeeRight; i<=eMouthLipOuterTopMiddle; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Pivot"), "Mapping Pivot Points.Mouth Outer" );
	
	AddPropertyViewForFacial( "Mouth Inner", "Mapping Pivot Points", true );
	for (int i=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_066; i<=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_033; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Pivot"), "Mapping Pivot Points.Mouth Inner" );
	
	AddPropertyViewForFacial( "Nose", "Mapping Pivot Points", true );
	for (int i=eNostrilOuterRight; i<=eNostrilOuterLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Pivot"), "Mapping Pivot Points.Nose" );

	AddPropertyViewForFacial( "Left Eye", "Mapping Pivot Points", true );
	for (int i=eEyeTopLeft; i<=ePupilInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Pivot"), "Mapping Pivot Points.Left Eye" );

	AddPropertyViewForFacial( "Right Eye", "Mapping Pivot Points", true );
	for (int i=eEyeTopRight; i<=ePupilInnerRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Pivot"), "Mapping Pivot Points.Right Eye" );

	AddPropertyViewForFacial( "Left Brow", "Mapping Pivot Points", true );
	for (int i=eBrowOuterLeft; i<=eBrowInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Pivot"), "Mapping Pivot Points.Left Brow" );

	AddPropertyViewForFacial( "Right Brow", "Mapping Pivot Points", true );
	for (int i=eBrowInnerRight; i<=eBrowOuterRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Pivot"), "Mapping Pivot Points.Right Brow" );

	AddPropertyViewForFacial( "Jaw", "Mapping Pivot Points", true );
	for (int i=eJaw1; i<=eJawRotation; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Pivot"), "Mapping Pivot Points.Jaw" );

	//
	AddPropertyViewForFacial( "Mapping Scaling", "", true );
	
	AddPropertyViewForFacial( "Mouth Outer", "Mapping Scaling", true );
	for (int i=eMouthLipOuterVeeRight; i<=eMouthLipOuterTopMiddle; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Scaling"), "Mapping Scaling.Mouth Outer" );
	
	AddPropertyViewForFacial( "Mouth Inner", "Mapping Scaling", true );
	for (int i=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_066; i<=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_033; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Scaling"), "Mapping Scaling.Mouth Inner" );
	
	AddPropertyViewForFacial( "Nose", "Mapping Scaling", true );
	for (int i=eNostrilOuterRight; i<=eNostrilOuterLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Scaling"), "Mapping Scaling.Nose" );

	AddPropertyViewForFacial( "Left Eye", "Mapping Scaling", true );
	for (int i=eEyeTopLeft; i<=ePupilInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Scaling"), "Mapping Scaling.Left Eye" );

	AddPropertyViewForFacial( "Right Eye", "Mapping Scaling", true );
	for (int i=eEyeTopRight; i<=ePupilInnerRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Scaling"), "Mapping Scaling.Right Eye" );

	AddPropertyViewForFacial( "Left Brow", "Mapping Scaling", true );
	for (int i=eBrowOuterLeft; i<=eBrowInnerLeft; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Scaling"), "Mapping Scaling.Left Brow" );

	AddPropertyViewForFacial( "Right Brow", "Mapping Scaling", true );
	for (int i=eBrowInnerRight; i<=eBrowOuterRight; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Scaling"), "Mapping Scaling.Right Brow" );

	AddPropertyViewForFacial( "Jaw", "Mapping Scaling", true );
	for (int i=eJaw1; i<=eJawRotation; ++i)
		AddPropertyViewForFacial( FBString(MappingNames[i], " Scaling"), "Mapping Scaling.Jaw" );


	//
	AddPropertyViewForFacial( "Process Parametric Values", "" );
	AddPropertyViewForFacial( "Calibrate Parametric Values", "" );
	AddPropertyViewForFacial( "Parametric Variables", "", true );
	
	auto fn_addparametric = [] (const char *name) {

		AddPropertyViewForFacial( FBString(name, " Base"), "Parametric Variables", true );
		AddPropertyViewForFacial( FBString(name, " Dir"), "Parametric Variables", true );
		AddPropertyViewForFacial( FBString(name, " Distance"), "Parametric Variables", true );
		AddPropertyViewForFacial( FBString(name, " Low"), "Parametric Variables", true );
		AddPropertyViewForFacial( FBString(name, " High"), "Parametric Variables", true );
		AddPropertyViewForFacial( FBString(name, " Position"), "Parametric Variables", true );
	};

	fn_addparametric( "Left Eye" );
	fn_addparametric( "Right Eye" );
#ifdef MORE_PARAMETRIC_VALUES
	fn_addparametric( "Left Nostril" );
	fn_addparametric( "Right Nostril" );
	fn_addparametric( "Left Mouth Corner" );
	fn_addparametric( "Right Mouth Corner" );
#endif
	//
	AddPropertyViewForFacial( "Stabilization (under construction)", "", true );

	AddPropertyViewForFacial( "Stabilization", "Stabilization (under construction)" );
	AddPropertyViewForFacial( "Stabilization Goal", "Stabilization (under construction)" );
	AddPropertyViewForFacial( "Stabilization Mult", "Stabilization (under construction)" );
	AddPropertyViewForFacial( "Stabilization Vector", "Stabilization (under construction)" );
	AddPropertyViewForFacial( "Stabilization Error", "Stabilization (under construction)" );
	AddPropertyViewForFacial( "Stab Iterations", "Stabilization (under construction)" );
}

void ConstraintFacialRetargeting::GrabPositionsProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) pplug->DoGrabPositions();
}

void ConstraintFacialRetargeting::CameraApplyProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) pplug->DoApplyCameraSettings();
}

void ConstraintFacialRetargeting::MappingClearProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) pplug->DoMappingClear();
}

void ConstraintFacialRetargeting::MappingSaveProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) pplug->DoMappingSave();
}

void ConstraintFacialRetargeting::MappingLoadProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) pplug->DoMappingLoad();
}

void ConstraintFacialRetargeting::ZeroEyesOffsetsProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) pplug->DoZeroEyesOffsets();
}

void ConstraintFacialRetargeting::ZeroEyesLookAtOffsetsProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) pplug->DoZeroEyesLookAtOffsets();
}

void ConstraintFacialRetargeting::ZeroMouthOffsetsProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) pplug->DoZeroMouthOffsets();
}

void ConstraintFacialRetargeting::CalibrateParametricProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (pplug) {
		
		if (value == true)
			pplug->DoCalibrateParametric();

		pplug->CalibrateParametric.SetPropertyValue(value);
	}
}

void ConstraintFacialRetargeting::CharacterizeProc(HIObject pObject, bool value) {
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (pplug) {

		if (value)
			pplug->DoCharacterize(true);

		/*
		if (value && true == pplug->DoCharacterize(true) )
		{
			pplug->Characterize.SetPropertyValue(value);
		}
		else 
		{
			pplug->DoCharacterize(false);
			pplug->Characterize.SetPropertyValue(false);
		}
		*/
	}
}

void ConstraintFacialRetargeting::OnActionAboutClick(HIObject pObject, bool value) 
{
	ConstraintFacialRetargeting *pplug = FBCast<ConstraintFacialRetargeting>(pObject);
	if (value && pplug) 
		pplug->ShowAbout();
}


/************************************************
 *	Creation function.
 ************************************************/
bool ConstraintFacialRetargeting::FBCreate()
{
	Deformer = true;
	HasLayout = false;
	Description = "Facial Retargeting";

	//
	
	for (int i=0; i<eMappingCount; ++i)
	{
		mSrcMapping.Add( MappingNames[i] );
		mDstMapping.Add( MappingNames[i] );
	}

	//

	FBPropertyPublish( this, Characterize, "Characterize", nullptr, CharacterizeProc );
	FBPropertyPublish( this, GrabPositions, "Grab Positions", nullptr, GrabPositionsProc );

	FBPropertyPublish( this, MappingFixNames, "Fix Mapping Names", nullptr, nullptr );

	FBPropertyPublish( this, MappingClear, "Clear Mapping", nullptr, MappingClearProc );
	FBPropertyPublish( this, MappingSave, "Save Mapping", nullptr, MappingSaveProc );
	FBPropertyPublish( this, MappingLoad, "Load Mapping", nullptr, MappingLoadProc );

	FBPropertyPublish( this, ZeroEyesOffsets, "Zero Eyes Offsets", nullptr, ZeroEyesOffsetsProc );
	FBPropertyPublish( this, ZeroEyesLookAtOffsets, "Zero Eyes LookAt Offsets", nullptr, ZeroEyesLookAtOffsetsProc );
	FBPropertyPublish( this, ZeroMouthOffsets, "Zero Mouth Offsets", nullptr, ZeroMouthOffsetsProc );

	FBPropertyPublish( this, CameraMouth, "Camera For Mouth", nullptr, nullptr );
	FBPropertyPublish( this, CameraNose, "Camera For Nose", nullptr, nullptr );
	FBPropertyPublish( this, CameraEyeLeft, "Camera For Left Eye", nullptr, nullptr );
	FBPropertyPublish( this, CameraEyeRight, "Camera For Right Eye", nullptr, nullptr );
	FBPropertyPublish( this, CameraFOV, "Camera FOV", nullptr, nullptr );
	FBPropertyPublish( this, CameraWidth, "Camera Width", nullptr, nullptr );
	FBPropertyPublish( this, CameraHeight, "Camera Height", nullptr, nullptr );
	FBPropertyPublish( this, UseValuesFromOpticalRoot, "Use Values From Optical Root", nullptr, nullptr );
	FBPropertyPublish( this, CameraApply, "Apply Camera Settings", nullptr, CameraApplyProc );
	FBPropertyPublish( this, UnProjectDepth, "UnProject Depth", nullptr, nullptr );

	FBPropertyPublish( this, HeadBacking, "Backing For Head Markers", nullptr, nullptr );
	FBPropertyPublish( this, UseNoseBacking, "Use Nose Backing", nullptr, nullptr );
	FBPropertyPublish( this, NoseBacking, "Backing For Nose Markers", nullptr, nullptr );
	FBPropertyPublish( this, UseEyesBacking, "Use Eyes Backing", nullptr, nullptr );
	FBPropertyPublish( this, EyesBacking, "Backing For Eyes Markers", nullptr, nullptr );
	FBPropertyPublish( this, UseEyesLevelBacking, "Use Eyes Level Backing", nullptr, nullptr );
	FBPropertyPublish( this, EyesLevelBacking, "Backing For Eyes Level", nullptr, nullptr );
	FBPropertyPublish( this, UseJawBacking, "Use Jaw Backing", nullptr, nullptr );
	FBPropertyPublish( this, JawBacking, "Backing For Jaw Markers", nullptr, nullptr );

	FBPropertyPublish( this, UseLeftEyeCollision, "Use Left Eye Collision", nullptr, nullptr );
	FBPropertyPublish( this, LeftEyeCollision, "Left Eye Collision", nullptr, nullptr );
	FBPropertyPublish( this, UseRightEyeCollision, "Use Right Eye Collision", nullptr, nullptr );
	FBPropertyPublish( this, RightEyeCollision, "Right Eye Collision", nullptr, nullptr );
	FBPropertyPublish( this, CollisionThickness, "Collision Thickness", nullptr, nullptr );

	FBPropertyPublish( this, StaticEyeCorner, "Keep Static Eye Corners", nullptr, nullptr );

	FBPropertyPublish( this, JawUpVector, "Jaw Up Vector", nullptr, nullptr );
	FBPropertyPublish( this, JawRotationUp, "Jaw Up Object", nullptr, nullptr );

	FBPropertyPublish( this, StickyLips, "Sticky Lips Weight", nullptr, nullptr );
	FBPropertyPublish( this, StickyLeftLip, "Sticky Left Lip", nullptr, nullptr );
	FBPropertyPublish( this, StickyRightLip, "Sticky Right Lip", nullptr, nullptr );

	FBPropertyPublish( this, ProcessParametric, "Process Parametric Values", nullptr, nullptr );
	FBPropertyPublish( this, CalibrateParametric, "Calibrate Parametric Values", nullptr, CalibrateParametricProc );
	PublishParametricProperty3d( "Left Eye", LeftEyeParam, FBVector3d(1.0, 1.0, 1.0) );
	PublishParametricProperty3d( "Right Eye", RightEyeParam, FBVector3d(1.0, 1.0, 1.0) );
#ifdef MORE_PARAMETRIC_VALUES
	PublishParametricProperty3d( "Left Nostril", LeftNostrilParam, FBVector3d(0.1, 1.0, 0.1) );
	PublishParametricProperty3d( "Right Nostril", RightNostrilParam, FBVector3d(0.1, 1.0, 0.1) );
	PublishParametricProperty3d( "Left Mouth Corner", LeftMouthCornerParam, FBVector3d(1.0, 0.1, 0.1) );
	PublishParametricProperty3d( "Right Mouth Corner", RightMouthCornerParam, FBVector3d(1.0, 0.1, 0.1) );
#endif
	FBPropertyPublish( this, Stabilization, "Stabilization", nullptr, nullptr );
	FBPropertyPublish( this, StabilizationGoal, "Stabilization Goal", nullptr, nullptr );
	FBPropertyPublish( this, StabilizationMult, "Stabilization Mult", nullptr, nullptr );
	FBPropertyPublish( this, StabilizationVector, "Stabilization Vector", nullptr, nullptr );
	FBPropertyPublish( this, StabilizationError, "Stabilization Error", nullptr, nullptr );
	FBPropertyPublish( this, StabilizationIterations, "Stab Iterations", nullptr, nullptr );

	FBPropertyPublish( this, ActionAbout, "About", nullptr, OnActionAboutClick );

	ProcessParametric = false;
	CalibrateParametric = false;

	Stabilization = false;
	StabilizationIterations = 10;
	StabilizationMult = FBVector2d(1.0, 1.0);

	mStabilizeMatrix.Identity();

	mNeedGrabEyesDistance = false;

	//Active.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	Lock.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	Snap.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	Weight.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	//Characterize = false;
	MappingFixNames = true;

	StaticEyeCorner = true;

	StickyLips.SetMinMax(0.0, 100.0, true, true);
	StickyLips = 50.0;

	StickyLeftLip.SetMinMax(0.0, 100.0, true, true);
	StickyLeftLip = 0.0;
	StickyRightLip.SetMinMax(0.0, 100.0, true, true);
	StickyRightLip = 0.0;

	
	CameraMouth.SetFilter( FBCamera::GetInternalClassId() );
	CameraMouth.SetSingleConnect(true);
	CameraNose.SetFilter( FBCamera::GetInternalClassId() );
	CameraNose.SetSingleConnect(true);
	CameraEyeLeft.SetFilter( FBCamera::GetInternalClassId() );
	CameraEyeLeft.SetSingleConnect(true);
	CameraEyeRight.SetFilter( FBCamera::GetInternalClassId() );
	CameraEyeRight.SetSingleConnect(true);

	UseValuesFromOpticalRoot = true;

	CameraFOV = 82.0;
	CameraWidth = 1080;
	CameraHeight = 1920;

	UnProjectDepth.SetMinMax(0.0, 100.0, true, true);
	UnProjectDepth = 10.0;

	HeadBacking.SetFilter( FBModel::GetInternalClassId() );
	HeadBacking.SetSingleConnect(true);
	UseNoseBacking = false;
	NoseBacking.SetFilter( FBModel::GetInternalClassId() );
	NoseBacking.SetSingleConnect(true);
	UseEyesBacking = false;
	EyesBacking.SetFilter( FBModel::GetInternalClassId() );
	EyesBacking.SetSingleConnect(true);
	UseEyesLevelBacking = false;
	EyesLevelBacking.SetFilter( FBModel::GetInternalClassId() );
	EyesLevelBacking.SetSingleConnect(true);
	UseJawBacking = false;
	JawBacking.SetFilter( FBModel::GetInternalClassId() );
	JawBacking.SetSingleConnect(true);

	UseLeftEyeCollision = false;
	UseRightEyeCollision = false;

	RightEyeCollision.SetFilter( FBModel::GetInternalClassId() );
	RightEyeCollision.SetSingleConnect(true);
	LeftEyeCollision.SetFilter( FBModel::GetInternalClassId() );
	LeftEyeCollision.SetSingleConnect(true);

	CollisionThickness = 100.0;

	JawUpVector = FBVector3d(0.0, 1.0, 0.0);
	JawRotationUp.SetFilter( FBModel::GetInternalClassId() );
	JawRotationUp.SetSingleConnect(true);

	//
	//

	MappingItem	*pItem = &Items[0];

	for (int i=0; i<eMappingCount; ++i, ++pItem)
	{
		pItem->srcInUse = false;
		pItem->dstInUse = false;
		pItem->srcRefIndex = -1;
		pItem->dstRefIndex = -1;
		pItem->needComputeOffset = false;
	}

	pItem = &Items[0];
	for (int i=0; i<eBaseCount; ++i, ++pItem)
	{
		FBPropertyPublish( this, pItem->SrcObject, FBString(MappingNames[i], " Source"), nullptr, nullptr );
		FBPropertyPublish( this, pItem->DstObject, FBString(MappingNames[i], " Destination"), nullptr, nullptr );
		FBPropertyPublish( this, pItem->Offset, FBString(MappingNames[i], " Offset"), nullptr, nullptr );
		FBPropertyPublish( this, pItem->PivotPoint, FBString(MappingNames[i], " Pivot"), nullptr, nullptr );
		FBPropertyPublish( this, pItem->Scale, FBString(MappingNames[i], " Scaling"), nullptr, nullptr );

		pItem->SrcObject.SetFilter( FBModel::GetInternalClassId() );
		pItem->SrcObject.SetSingleConnect(true);

		pItem->DstObject.SetFilter( FBModel::GetInternalClassId() );
		pItem->DstObject.SetSingleConnect(true);

		//pItem->SrcNode = nullptr;
		//pItem->DstNode = nullptr;

		pItem->srcInUse = true;
		pItem->dstInUse = true;

		pItem->Scale = FBVector3d(1.0, 1.0, 1.0);
	}

	pItem = &Items[eJaw1];
	for (int i=eJaw1; i<=eJaw3; ++i, ++pItem)
	{
		FBPropertyPublish( this, pItem->SrcObject, FBString(MappingNames[i], " Source"), nullptr, nullptr );
		FBPropertyPublish( this, pItem->DstObject, FBString(MappingNames[i], " Destination"), nullptr, nullptr );
		FBPropertyPublish( this, pItem->Offset, FBString(MappingNames[i], " Offset"), nullptr, nullptr );
		FBPropertyPublish( this, pItem->PivotPoint, FBString(MappingNames[i], " Pivot"), nullptr, nullptr );
		FBPropertyPublish( this, pItem->Scale, FBString(MappingNames[i], " Scaling"), nullptr, nullptr );

		pItem->SrcObject.SetFilter( FBModel::GetInternalClassId() );
		pItem->SrcObject.SetSingleConnect(true);

		pItem->DstObject.SetFilter( FBModel::GetInternalClassId() );
		pItem->DstObject.SetSingleConnect(true);

		//pItem->SrcNode = nullptr;
		//pItem->DstNode = nullptr;

		pItem->srcInUse = true;
		pItem->dstInUse = true;

		pItem->Scale = FBVector3d(1.0, 1.0, 1.0);
	}

	// jaw rotation
	pItem = &Items[eJawRotation];

	FBPropertyPublish( this, pItem->DstObject, FBString(MappingNames[eJawRotation], " Destination"), nullptr, nullptr );
	FBPropertyPublish( this, pItem->Offset, FBString(MappingNames[eJawRotation], " Offset"), nullptr, nullptr );
	
	pItem->DstObject.SetFilter( FBModel::GetInternalClassId() );
	pItem->DstObject.SetSingleConnect(true);

	//pItem->SrcNode = nullptr;
	//pItem->DstNode = nullptr;

	pItem->srcInUse = false;
	pItem->dstInUse = true;

	// sticky lips
	pItem = &Items[eStickyLipUpLeft];
	for (int i=eStickyLipUpLeft; i<=eStickyLipDownRight; ++i, ++pItem)
	{
		FBPropertyPublish( this, pItem->DstObject, FBString(MappingNames[i], " Destination"), nullptr, nullptr );
		
		pItem->DstObject.SetFilter( FBModel::GetInternalClassId() );
		pItem->DstObject.SetSingleConnect(true);

		//pItem->SrcNode = nullptr;
		//pItem->DstNode = nullptr;

		pItem->srcInUse = false;
		pItem->dstInUse = true;
	}

	// eyes level
	pItem = &Items[eEyesLevel];

	FBPropertyPublish( this, pItem->DstObject, FBString(MappingNames[eEyesLevel], " Destination"), nullptr, nullptr );
	FBPropertyPublish( this, pItem->Offset, FBString(MappingNames[eEyesLevel], " Offset"), nullptr, nullptr );
	
	pItem->DstObject.SetFilter( FBModel::GetInternalClassId() );
	pItem->DstObject.SetSingleConnect(true);

	pItem->srcInUse = false;
	pItem->dstInUse = true;

	//
	// Create reference group

	mGroupSource	= ReferenceGroupAdd( "Source Objects",	MAX_NUMBER_OF_NODES );
	mGroupConstrain	= ReferenceGroupAdd( "Constrain",		MAX_NUMBER_OF_NODES );

	
	for (int i = 0; i < MAX_NUMBER_OF_NODES; ++i)
	{
		mSrcTranslation[i] = nullptr;
		//mBoneInTranslation[i] = nullptr;
		mBoneOutTranslation[i] = nullptr;
	}

	mProcessJawRotation = false;
	mLastUpdateTime = FBTime::Infinity;

	return true;
}


/************************************************
 *	Destruction function.
 ************************************************/
void ConstraintFacialRetargeting::FBDestroy()
{
}

bool ConstraintFacialRetargeting::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	
	if (pThis == &CameraMouth || pThis == &CameraNose || pThis == &CameraEyeLeft || pThis == &CameraEyeRight
		|| pThis == &HeadBacking || pThis == &NoseBacking || pThis == &EyesBacking || pThis == &EyesLevelBacking || pThis == &JawBacking
		|| pThis == &LeftEyeCollision || pThis == &RightEyeCollision)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void ConstraintFacialRetargeting::SetupAllAnimationNodes()
{
	int index = 0;

	if (ReferenceGetCount( mGroupSource ) > 0 )
	{
		for (int i=0; i<ReferenceGetCount(mGroupSource); ++i)
		{
			FBModel *pModel = ReferenceGet(mGroupSource, i);
			if (pModel != nullptr)
			{
				for (int j=0; j<eMappingCount; ++j)
				{
					MappingItem &item = Items[j];
					if (item.srcInUse && item.SrcObject.GetCount() > 0 && item.SrcObject.GetAt(0) == pModel)
						item.srcRefIndex = i;
				}

				mSrcTranslation[i]	= AnimationNodeOutCreate( index, pModel, ANIMATIONNODE_TYPE_LOCAL_TRANSLATION );
				index += 1;
			}
		}
	}

	// create nodes for head and eyes level

	FBModel *pHeadBacking = (HeadBacking.GetCount() > 0) ? (FBModel*) HeadBacking.GetAt(0) : nullptr;

	if (nullptr != pHeadBacking)
	{
		mHeadBackingNodes.tr = AnimationNodeOutCreate( index, pHeadBacking, ANIMATIONNODE_TYPE_TRANSLATION );
		mHeadBackingNodes.rot = AnimationNodeOutCreate( index+1, pHeadBacking, ANIMATIONNODE_TYPE_ROTATION );
		mHeadBackingNodes.scl = AnimationNodeOutCreate( index+2, pHeadBacking, ANIMATIONNODE_TYPE_SCALING );

		index += 3;
	}
	else
	{
		mHeadBackingNodes.tr = nullptr;
		mHeadBackingNodes.rot = nullptr;
		mHeadBackingNodes.scl = nullptr;
	}

	FBModel *pEyesLevelBacking = (EyesLevelBacking.GetCount() > 0) ? (FBModel*) EyesLevelBacking.GetAt(0) : nullptr;
	if (nullptr != pEyesLevelBacking)
	{
		mEyesLevelNodes.tr = AnimationNodeOutCreate( index, pEyesLevelBacking, ANIMATIONNODE_TYPE_TRANSLATION );
		mEyesLevelNodes.rot = AnimationNodeOutCreate( index+1, pEyesLevelBacking, ANIMATIONNODE_TYPE_ROTATION );
		mEyesLevelNodes.scl = AnimationNodeOutCreate( index+2, pEyesLevelBacking, ANIMATIONNODE_TYPE_SCALING );

		index += 3;
	}
	else
	{
		mEyesLevelNodes.tr = nullptr;
		mEyesLevelNodes.rot = nullptr;
		mEyesLevelNodes.scl = nullptr;
	}

	//
	if (ReferenceGetCount( mGroupConstrain ) > 0 )
	{
		const int jawRotIndex = Items[eJawRotation].dstRefIndex;

		for (int i=0; i<ReferenceGetCount(mGroupConstrain); ++i)
		{
			FBModel *pModel = ReferenceGet(mGroupConstrain, i);

			if (pModel != nullptr)
			{

				for (int j=0; j<eMappingCount; ++j)
				{
					MappingItem &item = Items[j];
					if (item.dstInUse && item.DstObject.GetCount() > 0 && item.DstObject.GetAt(0) == pModel)
						item.dstRefIndex = i;
				}

				if (i == jawRotIndex)
				{
					mBoneOutTranslation[i]	= AnimationNodeInCreate	( index, pModel, ANIMATIONNODE_TYPE_ROTATION );
					index += 1;
				}
				else
				{
					//mBoneInTranslation[i]	= AnimationNodeOutCreate( index, ReferenceGet( mGroupSource,	i ), ANIMATIONNODE_TYPE_TRANSLATION );
					//index += 1;
					mBoneOutTranslation[i]	= AnimationNodeInCreate	( index, pModel, ANIMATIONNODE_TYPE_TRANSLATION );
					index += 1;
				}
			}
		}
	}	

	//
	SetupParametricAnimationNode(index, LeftEyeParam);
	SetupParametricAnimationNode(index+1, RightEyeParam);
#ifdef MORE_PARAMETRIC_VALUES
	SetupParametricAnimationNode(index+2, LeftNostrilParam);
	SetupParametricAnimationNode(index+3, RightNostrilParam);
	SetupParametricAnimationNode(index+4, LeftMouthCornerParam);
	SetupParametricAnimationNode(index+5, RightMouthCornerParam);
#endif
}


bool ConstraintFacialRetargeting::DoCharacterize(const bool state)
{
	//Active = false;
	ReferenceRemoveAll();

	if (state == false)
		return true;

	// activate all mapped nodes

	int computeTR = FBMessageBox( "Facial Retargeting", "Do you want to compute TR?", "Yes", "No" );

	mProcessJawRotation = false;
	mNeedGrabEyesDistance = (computeTR == 1);

	for (int i=0; i<eMappingCount; ++i)
	{
		MappingItem *pItem = &Items[i];

		pItem->srcRefIndex = -1;
		pItem->dstRefIndex = -1;
		pItem->needComputeOffset = false;

		if (pItem->srcInUse && pItem->SrcObject.GetCount() > 0)
		{
			pItem->srcRefIndex = ReferenceGetCount(mGroupSource);
			ReferenceAdd( mGroupSource, (FBModel*) pItem->SrcObject.GetAt(0) );

			FBVector3d v;
			((FBModel*) pItem->SrcObject.GetAt(0))->GetVector(v, kModelTranslation, false);
			pItem->PivotPoint.SetData(v);
		}

		if (pItem->dstInUse && pItem->DstObject.GetCount() > 0)
		{
			FBModel *pModel = (FBModel*) pItem->DstObject.GetAt(0);

			pItem->dstRefIndex = ReferenceGetCount(mGroupConstrain);
			ReferenceAdd( mGroupConstrain, pModel );
			pItem->needComputeOffset = (computeTR == 1);

			FBVector3d globalPos;

			if (i == eJawRotation)
			{
				pModel->GetVector(globalPos, kModelRotation);
				mProcessJawRotation = true;
			}
			else
			{
				pModel->GetVector(globalPos);
			}
			memcpy(pItem->globalPosForOffset, globalPos, sizeof(double)*3);
		}
	}

	//Active = true;
	return true;
}

void ConstraintFacialRetargeting::AssignSource(FBModel *pModel)
{
	FBModel *pParent = FindParent(pModel);

	// use parent properties if exist
	FBProperty *prop = nullptr;

	prop = pParent->PropertyList.Find( "SourceWidth" );
	if (nullptr != prop)
		CameraWidth = prop->AsInt();
	prop = pParent->PropertyList.Find( "SourceHeight" );
	if (nullptr != prop)
		CameraHeight = prop->AsInt();

	// collect all source markers

	FBModelList *pList = FBCreateModelList();
	CollectModels(pList, pParent);

	int collected = 0;

	for (int i=0; i<eBaseCount; ++i)
	{
		FBModel *pFinded = FindModelInList(pList, MappingNames[i], MappingFixNames);

		if (pFinded)
		{
			Items[i].SrcObject.Clear();
			Items[i].SrcObject.Add(pFinded);

			collected++;
		}
	}

	for (int i=eJaw1; i<=eJaw3; ++i)
	{
		FBModel *pFinded = FindModelInList(pList, MappingNames[i], MappingFixNames);

		if (pFinded)
		{
			Items[i].SrcObject.Clear();
			Items[i].SrcObject.Add(pFinded);

			collected++;
		}
	}

	FBDestroyModelList(pList);

	if (collected >= eBaseCount)
		FBMessageBox("Facial Retargeting", "All nodes has beed found!", "Ok");
	else
		FBMessageBox("Facial Retargeting", "Warning: some nodes are missing!", "Ok");

}

void ConstraintFacialRetargeting::AssignDestination(FBModel *pModel)
{
	FBModel *pParent = FindParent(pModel);

	FBModelList *pList = FBCreateModelList();
	CollectModels(pList, pParent);

	int collected = 0;

	for (int i=0; i<eBaseCount; ++i)
	{
		FBModel *pFinded = FindModelInList(pList, MappingNames[i], MappingFixNames);

		if (pFinded)
		{
			Items[i].DstObject.Clear();
			Items[i].DstObject.Add(pFinded);

			collected++;
		}
	}

	for (int i=eJaw1; i<=eJawRotation; ++i)
	{
		FBModel *pFinded = FindModelInList(pList, MappingNames[i], MappingFixNames);

		if (pFinded)
		{
			Items[i].DstObject.Clear();
			Items[i].DstObject.Add(pFinded);

			collected++;
		}
	}

	for (int i=eStickyLipUpLeft; i<=eStickyLipDownRight; ++i)
	{
		FBModel *pFinded = FindModelInList(pList, MappingNames[i], MappingFixNames);

		if (pFinded)
		{
			Items[i].DstObject.Clear();
			Items[i].DstObject.Add(pFinded);

			collected++;
		}
	}

	FBDestroyModelList(pList);
	
	int baseCount = eBaseCount;
	baseCount -= 4; // exclude pupil points
	//baseCount -= 2; // exclude 2 jaw points
	baseCount -= 4; // exclude 4 brows points

	if (collected >= baseCount)
		FBMessageBox("Facial Retargeting", "All nodes has been found!", "Ok");
	else
		FBMessageBox("Facial Retargeting", "Warning: some nodes are missing!", "Ok");

}

void ConstraintFacialRetargeting::AssignHeadBacking(FBModel *pModel)
{
	HeadBacking.Clear();
	if (pModel != nullptr)
		HeadBacking.Add(pModel);
}

void ConstraintFacialRetargeting::AssignNoseBacking(FBModel *pModel)
{
	UseNoseBacking = false;
	NoseBacking.Clear();
	if (pModel != nullptr)
	{
		UseNoseBacking = true;
		NoseBacking.Add(pModel);
	}
}

void ConstraintFacialRetargeting::AssignEyesBacking(FBModel *pModel)
{
	UseEyesBacking = false;
	EyesBacking.Clear();
	if (pModel != nullptr)
	{
		UseEyesBacking = true;
		EyesBacking.Add(pModel);
	}
}

void ConstraintFacialRetargeting::AssignEyesLevelBacking(FBModel *pModel)
{
	UseEyesLevelBacking = false;
	EyesLevelBacking.Clear();
	if (pModel != nullptr)
	{
		UseEyesLevelBacking = true;
		EyesLevelBacking.Add(pModel);
	}
}

void ConstraintFacialRetargeting::AssignJawBacking(FBModel *pModel)
{
	UseJawBacking = false;
	JawBacking.Clear();
	if (pModel != nullptr)
	{
		UseJawBacking = true;
		JawBacking.Add(pModel);
	}
}

void ConstraintFacialRetargeting::AssignLeftEyeCollision(FBModel *pModel)
{
	UseLeftEyeCollision = false;
	LeftEyeCollision.Clear();
	if (pModel != nullptr)
	{
		UseLeftEyeCollision = true;
		LeftEyeCollision.Add(pModel);
	}
}

void ConstraintFacialRetargeting::AssignRightEyeCollision(FBModel *pModel)
{
	UseRightEyeCollision = false;
	RightEyeCollision.Clear();
	if (pModel != nullptr)
	{
		UseRightEyeCollision = true;
		RightEyeCollision.Add(pModel);
	}
}

bool ConstraintFacialRetargeting::AreCamerasMissing()
{
	return !(CameraMouth.GetCount() > 0 && CameraNose.GetCount() > 0 && CameraEyeLeft.GetCount() > 0 && CameraEyeRight.GetCount() > 0);
}

void ConstraintFacialRetargeting::CheckForOtherCameras(bool userPrompt)
{
	if (AreCamerasMissing() == false)
		return;

	FBCamera *pCamera = nullptr;

	if (pCamera == nullptr && CameraMouth.GetCount() > 0)
		pCamera = (FBCamera*) CameraMouth.GetAt(0);
	if (pCamera == nullptr && CameraNose.GetCount() > 0)
		pCamera = (FBCamera*) CameraNose.GetAt(0);
	if (pCamera == nullptr && CameraEyeLeft.GetCount() > 0)
		pCamera = (FBCamera*) CameraEyeLeft.GetAt(0);
	if (pCamera == nullptr && CameraEyeRight.GetCount() > 0)
		pCamera = (FBCamera*) CameraEyeRight.GetAt(0);

	if (pCamera == nullptr)
		return;

	FBModel *pRoot = pCamera->Parent;
	if (pRoot == nullptr)
		return;

	if (userPrompt && 1 != FBMessageBox("Facial Retargeting", "Do you want to assign other cameras ?", "Yes", "No" ) )
		return;


	for (int i=0; i<pRoot->Children.GetCount(); ++i)
	{
		if (FBIS(pRoot->Children.GetAt(i), FBCamera))
		{
			pCamera = (FBCamera*) pRoot->Children.GetAt(i);

			FBString name(pCamera->Name);

			if (CameraMouth.GetCount() == 0 && strstr( (char*) name, "CameraForMouth") != nullptr)
				CameraMouth.Add(pCamera);
			else if (CameraNose.GetCount() == 0 && strstr( (char*) name, "CameraForNose") != nullptr )
				CameraNose.Add(pCamera);
			else if (CameraEyeLeft.GetCount() == 0 && strstr( (char*) name, "CameraForLeftEye") != nullptr )
				CameraEyeLeft.Add(pCamera);
			else if (CameraEyeRight.GetCount() == 0 && strstr( (char*) name, "CameraForRightEye") != nullptr )
				CameraEyeRight.Add(pCamera);

		}
	}
}

void ConstraintFacialRetargeting::AssignCameraMouth(FBCamera *pCamera)
{
	CameraMouth.Clear();
	if (pCamera != nullptr)
		CameraMouth.Add(pCamera);

	CheckForOtherCameras(true);
}

void ConstraintFacialRetargeting::AssignCameraNose(FBCamera *pCamera)
{
	CameraNose.Clear();
	if (pCamera != nullptr)
		CameraNose.Add(pCamera);

	CheckForOtherCameras(true);
}

void ConstraintFacialRetargeting::AssignCameraEyeLeft(FBCamera *pCamera)
{
	CameraEyeLeft.Clear();
	if (pCamera != nullptr)
		CameraEyeLeft.Add(pCamera);

	CheckForOtherCameras(true);
}

void ConstraintFacialRetargeting::AssignCameraEyeRight(FBCamera *pCamera)
{
	CameraEyeRight.Clear();
	if (pCamera != nullptr)
		CameraEyeRight.Add(pCamera);

	CheckForOtherCameras(true);
}

void ConstraintFacialRetargeting::DoSelectCameraMouth()
{
	if (CameraMouth.GetCount() )
		CameraMouth.GetAt(0)->Selected = true;
}

void ConstraintFacialRetargeting::DoSelectCameraNose()
{
	if (CameraNose.GetCount() )
		CameraNose.GetAt(0)->Selected = true;
}

void ConstraintFacialRetargeting::DoSelectCameraEyeLeft()
{
	if (CameraEyeLeft.GetCount() )
		CameraEyeLeft.GetAt(0)->Selected = true;
}

void ConstraintFacialRetargeting::DoSelectCameraEyeRight()
{
	if (CameraEyeRight.GetCount() )
		CameraEyeRight.GetAt(0)->Selected = true;
}

void ConstraintFacialRetargeting::ShowBakingGeometry(const bool show)
{
	if (JawBacking.GetCount() )
		((FBModel*)JawBacking.GetAt(0))->Show = show;
	if (HeadBacking.GetCount() )
		((FBModel*)HeadBacking.GetAt(0))->Show = show;
	if (NoseBacking.GetCount() )
		((FBModel*)NoseBacking.GetAt(0))->Show = show;
}

void ConstraintFacialRetargeting::ToggleBakingGeometry()
{
	if (JawBacking.GetCount() )
		((FBModel*)JawBacking.GetAt(0))->Show = !((FBModel*)JawBacking.GetAt(0))->Show;
	if (HeadBacking.GetCount() )
		((FBModel*)HeadBacking.GetAt(0))->Show = !((FBModel*)HeadBacking.GetAt(0))->Show;
	if (NoseBacking.GetCount() )
		((FBModel*)NoseBacking.GetAt(0))->Show = !((FBModel*)NoseBacking.GetAt(0))->Show;
}

/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ConstraintFacialRetargeting::RemoveAllAnimationNodes()
{
	/*
	if (LeftEyeParam.NodeOut != nullptr)
	{
		AnimationNodeDestroy( LeftEyeParam.NodeOut );
		LeftEyeParam.NodeOut = nullptr;
	}

	if (RightEyeParam.NodeOut != nullptr)
	{
		AnimationNodeDestroy( RightEyeParam.NodeOut );
		RightEyeParam.NodeOut = nullptr;
	}
	*/
}


/************************************************
 *	FBX storage of constraint parameters.
 ************************************************/
bool ConstraintFacialRetargeting::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


/************************************************
 *	FBX retrieval of constraint parameters.
 ************************************************/
bool ConstraintFacialRetargeting::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


/************************************************
 *	Suggest a snap.
 ************************************************/
void ConstraintFacialRetargeting::SnapSuggested()
{
	FBConstraint::SnapSuggested();

	
}

/************************************************
 *	Suggest a freeze.
 ************************************************/
void ConstraintFacialRetargeting::FreezeSuggested()
{
	
	FBConstraint::FreezeSuggested();

	if( ReferenceGet( 0,0 ) )
	{
		FreezeSRT( (FBModel*)ReferenceGet( 0, 0), true, true, true );
	}
}


/************************************************
 *	Real-Time Engine Evaluation
 ************************************************/

void ConstraintFacialRetargeting::ActiveChanged()
{
	if (Active == false)
	{
		DoGrabPositions();
	}
}

bool ConstraintFacialRetargeting::ProcessAll(FBEvaluateInfo *pEvaluateInfo)
{

#ifdef PRINT_RAY_INTERSECT_INFO
	FBTrace ("\n == START NEW FACIAL EVALUATION == \n" );
#endif

	//
	FBModel *pHeadBacking = (HeadBacking.GetCount() > 0) ? (FBModel*) HeadBacking.GetAt(0) : nullptr;
	FBModel *pNoseBacking = pHeadBacking;
	FBModel *pJawBacking = pHeadBacking;
	FBModel *pEyesBacking = pHeadBacking;
	FBModel *pEyesLevelBacking = pHeadBacking;

	if (UseNoseBacking && NoseBacking.GetCount() > 0)
		pNoseBacking = (FBModel*) NoseBacking.GetAt(0);
	if (UseJawBacking && JawBacking.GetCount() > 0)
		pJawBacking = (FBModel*) JawBacking.GetAt(0);
	if (UseEyesBacking && EyesBacking.GetCount() > 0)
		pEyesBacking = (FBModel*) EyesBacking.GetAt(0);
	if (UseEyesLevelBacking && EyesLevelBacking.GetCount() > 0)
		pEyesLevelBacking = (FBModel*) EyesLevelBacking.GetAt(0);

	// read TM

	ReadModelMatrix(pEvaluateInfo, mHeadBackingNodes, mHeadBackingMatrix);
	
	mEyesLevelMatrix = mHeadBackingMatrix;
	ReadModelMatrix(pEvaluateInfo, mEyesLevelNodes, mEyesLevelMatrix);

	// try to stabilize by shifting a camera

	if (Stabilization)
	{
		FBVector2d stabv = StabilizationGoal;

		StabilizePositions( (CameraNose.GetCount() > 0) ? (FBCamera*) CameraNose.GetAt(0) : nullptr,
			pNoseBacking,
			mHeadBackingMatrix,
			eNoseLowerMiddle,
			stabv[0], stabv[1],
			pEvaluateInfo );
	}
	else
	{
		mStabilizeMatrix.Identity();
	}

	// first of all compute eyes level if assign
	ProcessEyesLevel(	(CameraEyeLeft.GetCount() > 0) ? (FBCamera*) CameraEyeLeft.GetAt(0) : nullptr,
						(CameraEyeRight.GetCount() > 0) ? (FBCamera*) CameraEyeRight.GetAt(0) : nullptr,
						pEyesLevelBacking,
						mEyesLevelMatrix,
						pEvaluateInfo );

	// DONE: compute jaw movement

	if (mProcessJawRotation)
	{
		ProcessJawAim(	(CameraMouth.GetCount() > 0) ? (FBCamera*) CameraMouth.GetAt(0) : nullptr,
						pJawBacking,
						mHeadBackingMatrix,
						pEvaluateInfo );
	}
	else
	{
		ProcessPositions(	(CameraMouth.GetCount() > 0) ? (FBCamera*) CameraMouth.GetAt(0) : nullptr,
							pJawBacking,
							mHeadBackingMatrix,
							nullptr,
							0.0,
							eJaw1,
							eJaw3,
							pEvaluateInfo );
	}

	//
	// 4 regions - need to unproject point and then connect it with 
	
	// mouth, depends on a jaw bone

	ProcessPositions(	(CameraMouth.GetCount() > 0) ? (FBCamera*) CameraMouth.GetAt(0) : nullptr,
						pHeadBacking,
						mHeadBackingMatrix,
						nullptr,
						0.0,
						eMouthLipOuterVeeRight,
						eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_033,
						pEvaluateInfo );

	// NOSE movement

	

	ProcessPositions(	(CameraNose.GetCount() > 0) ? (FBCamera*) CameraNose.GetAt(0) : nullptr,
						pNoseBacking,
						mHeadBackingMatrix,
						nullptr,
						0.0,
						eNostrilOuterRight,
						eNostrilOuterLeft,
						pEvaluateInfo );

	// LEFT EYE
	const double thickness = 0.01 * CollisionThickness;
	
	ProcessPositions(	(CameraEyeLeft.GetCount() > 0) ? (FBCamera*) CameraEyeLeft.GetAt(0) : nullptr,
						pEyesBacking,
						mHeadBackingMatrix,
						(UseLeftEyeCollision && LeftEyeCollision.GetCount() > 0) ? (FBModel*) LeftEyeCollision.GetAt(0) : nullptr,
						thickness,
						eEyeTopLeft,
						eBrowInnerLeft,
						pEvaluateInfo );

	// RIGHT EYE

	ProcessPositions(	(CameraEyeRight.GetCount() > 0) ? (FBCamera*) CameraEyeRight.GetAt(0) : nullptr,
						pEyesBacking,
						mHeadBackingMatrix,
						(UseRightEyeCollision && RightEyeCollision.GetCount() > 0) ? (FBModel*) RightEyeCollision.GetAt(0) : nullptr,
						thickness,
						eEyeTopRight,
						eBrowOuterRight,
						pEvaluateInfo );

	//
	//

	if (ProcessParametric)
	{

		bool calibrationMode = CalibrateParametric;

		ProcessParametricProperty3dTwoWays( eEyeTopLeft, eEyeBottomLeft, LeftEyeParam, pEvaluateInfo, calibrationMode );
		ProcessParametricProperty3dTwoWays( eEyeTopRight, eEyeBottomRight, RightEyeParam, pEvaluateInfo, calibrationMode );
#ifdef MORE_PARAMETRIC_VALUES
		ProcessParametricProperty3dOneWay( eNostrilOuterLeft, LeftNostrilParam, pEvaluateInfo );
		ProcessParametricProperty3dOneWay( eNostrilOuterRight, RightNostrilParam, pEvaluateInfo );
		ProcessParametricProperty3dOneWay( eMouthLipOuterCornerLeft, LeftMouthCornerParam, pEvaluateInfo );
		ProcessParametricProperty3dOneWay( eMouthLipOuterCornerRight, RightMouthCornerParam, pEvaluateInfo );
#endif
	}
	else
	{
		if (LeftEyeParam.NodeOut != nullptr)
			LeftEyeParam.NodeOut->DisableIfNotWritten(pEvaluateInfo);
		if (RightEyeParam.NodeOut != nullptr)
			RightEyeParam.NodeOut->DisableIfNotWritten(pEvaluateInfo);
#ifdef MORE_PARAMETRIC_VALUES
		if (LeftNostrilParam.NodeOut != nullptr)
			LeftNostrilParam.NodeOut->DisableIfNotWritten(pEvaluateInfo);
		if (RightNostrilParam.NodeOut != nullptr)
			RightNostrilParam.NodeOut->DisableIfNotWritten(pEvaluateInfo);
		if (LeftMouthCornerParam.NodeOut != nullptr)
			LeftMouthCornerParam.NodeOut->DisableIfNotWritten(pEvaluateInfo);
		if (RightMouthCornerParam.NodeOut != nullptr)
			RightMouthCornerParam.NodeOut->DisableIfNotWritten(pEvaluateInfo);
#endif
	}

	// compute mouth sticky lips positions
	
	ProcessStickyLips(pEvaluateInfo);

#ifdef PRINT_RAY_INTERSECT_INFO
	FBTrace ("==== \n" );
#endif

	return true;
}

bool ConstraintFacialRetargeting::AnimationNodeNotify(FBAnimationNode* pConnector, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo)
{
	if (Active == false)
		return false;

	FBTime currTime = pEvaluateInfo->GetLocalTime();

	if (mLastUpdateTime == FBTime::Infinity || currTime != mLastUpdateTime)
	{
		// TODO: evaluate positions
		ProcessAll(pEvaluateInfo);

		//mLastUpdateTime = currTime;
		mLastUpdateTime = FBTime::Infinity;
	}

	// let's record a position for our input connector
	/*
	bool isExist = false;

	const int startIndex = 0;
	const int endIndex = eMappingCount;

	MappingItem *pItem = &Items[startIndex];
	for (	int i=startIndex; 
			i<endIndex; 
			++i, ++pItem)
	{

		if (pConnector == mBoneOutTranslation[pItem->dstRefIndex]
		&& true == pItem->status)
		{
			pConnector->WriteData( pItem->bakedPosition, pEvaluateInfo );
			isExist = true;
			break;
		}
	}

	if (false == isExist)
	{
		pConnector->DisableIfNotWritten(pEvaluateInfo);
	}
	*/

	// disable all unwritten nodes - save performance
	
	for (int i=0; i<MAX_NUMBER_OF_NODES; ++i)
		if (mBoneOutTranslation[i] != nullptr)
			mBoneOutTranslation[i]->DisableIfNotWritten(pEvaluateInfo);
			
	return true;
}

bool ConstraintFacialRetargeting::ProcessEyesLevel(	FBCamera *pCameraForLeftEye,
													FBCamera *pCameraForRightEye,
													FBModel *backingModel,
													const FBMatrix &backingMatrix,
													FBEvaluateInfo *pEvaluateInfo )
{
	if (pCameraForLeftEye == nullptr || pCameraForRightEye == nullptr)
		return false;

	const double lDepth = 0.01 * UnProjectDepth;

	FBVector3d lSource;
	FBVector3d lCameraPosLeft, lCameraPosRight;
	FBMatrix lCamMVPLeft, lCamInvMVPLeft, lCamProjLeft, lCamModelViewLeft;
	FBMatrix lCamMVPRight, lCamInvMVPRight, lCamProjRight, lCamModelViewRight;
	FBTVector lResult, lResult1, lResult2, lOffset;

	lCamMVPLeft.Identity();
	lCamInvMVPLeft.Identity();
	lCamMVPRight.Identity();
	lCamInvMVPRight.Identity();
	lCameraPosLeft.Init();
	lCameraPosRight.Init();

	pCameraForLeftEye->GetVector(lCameraPosLeft, kModelTranslation, true, pEvaluateInfo);
	pCameraForRightEye->GetVector(lCameraPosRight, kModelTranslation, true, pEvaluateInfo);

	pCameraForLeftEye->GetCameraMatrix( lCamProjLeft, kFBProjection, pEvaluateInfo );	// this is const
	pCameraForLeftEye->GetCameraMatrix( lCamModelViewLeft, kFBModelView, pEvaluateInfo );
	pCameraForLeftEye->GetCameraMatrix( lCamMVPLeft, kFBModelViewProj, pEvaluateInfo );

	FBMatrixMult( lCamModelViewLeft, lCamModelViewLeft, mStabilizeMatrix );
	FBMatrixMult( lCamMVPLeft, lCamProjLeft, lCamModelViewLeft );

	FBMatrixInverse( lCamInvMVPLeft, lCamMVPLeft );

	pCameraForRightEye->GetCameraMatrix( lCamProjRight, kFBProjection, pEvaluateInfo );	// this is const
	pCameraForRightEye->GetCameraMatrix( lCamModelViewRight, kFBModelView, pEvaluateInfo );
	pCameraForRightEye->GetCameraMatrix( lCamMVPRight, kFBModelViewProj, pEvaluateInfo );

	FBMatrixMult( lCamModelViewRight, lCamModelViewRight, mStabilizeMatrix );
	FBMatrixMult( lCamMVPRight, lCamProjRight, lCamModelViewRight );

	FBMatrixInverse( lCamInvMVPRight, lCamMVPRight );

	MappingItem *pItem1 = &Items[eEyeCornerInnerLeft];
	MappingItem *pItem2 = &Items[eEyeCornerInnerRight];
	MappingItem *pItemResult = &Items[eEyesLevel];

	bool status;
	pItemResult->status = false;

	if (pItem1->srcRefIndex >= 0 && pItem2->srcRefIndex >= 0 && pItemResult->dstRefIndex >= 0)
	{
		status = mSrcTranslation[pItem1->srcRefIndex]->ReadData	( lSource, pEvaluateInfo );
		if (false == status)
		{
			FBTrace( "read data is failed!\n" );
			return false;
		}
		memcpy( pItem1->bakedSource, lSource, sizeof(double) * 3 );
		status = UnProjectPoint(MappingNames[pItem1->srcRefIndex], lSource, lDepth, lCamInvMVPLeft, lCameraPosLeft, backingModel, backingMatrix, lResult1);
		
		if (false == status)
		{
			return false;
		}

		status = mSrcTranslation[pItem2->srcRefIndex]->ReadData	( lSource, pEvaluateInfo );
		if (false == status)
		{
			FBTrace( "read data is failed!\n" );
			return false;
		}
		memcpy( pItem2->bakedSource, lSource, sizeof(double) * 3 );
		status = UnProjectPoint(MappingNames[pItem2->srcRefIndex], lSource, lDepth, lCamInvMVPRight, lCameraPosRight, backingModel, backingMatrix, lResult2);

		if (false == status)
		{
			return false;
		}

		// DONE: apply offset !

		FBSub( lResult, lResult2, lResult1 );
		FBMult( lResult, lResult, 0.5 );
		FBAdd( lResult, lResult1, lResult );

		if (pItemResult->needComputeOffset)
		{
			FBSub( lOffset, pItemResult->globalPosForOffset, lResult );
			pItemResult->Offset.SetData(lOffset);
			pItemResult->needComputeOffset = false;
		}
		else
		{
			pItemResult->Offset.GetData(lOffset, sizeof(double)*3, pEvaluateInfo);
		}

		FBAdd(lResult, lResult, lOffset);
	
		//
		if (true == status)
		{
			mBoneOutTranslation[pItemResult->dstRefIndex]->WriteData	( lResult, pEvaluateInfo );
			memcpy( pItemResult->bakedPosition, lResult, sizeof(double) * 3 );
			pItemResult->status = true;
		}
	}
	

	return true;
}

bool ConstraintFacialRetargeting::ProcessPositions(FBCamera *pCamera, 
													FBModel *backingModel,
													const FBMatrix &backingMatrix,
													FBModel *colModel,							
													const double colThickness, 
													const int startIndex, 
													const int endIndex, 
													FBEvaluateInfo* pEvaluateInfo)
{
	if (pCamera == nullptr)
		return false;

	const double lDepth = 0.01 * UnProjectDepth;

	FBVector3d lSource;
	FBVector3d lCameraPos;
	FBMatrix lCamMVP, lCamInvMVP, lCamProj, lCamModelView;
	FBTVector lResult, lOffset, lScaling, lPivot;

	lCamMVP.Identity();
	lCamInvMVP.Identity();
	lCameraPos.Init();

	pCamera->GetVector(lCameraPos, kModelTranslation, true, pEvaluateInfo);
	/*
	lCameraPos[0] += mStabilizeMatrix[12];
	lCameraPos[1] += mStabilizeMatrix[13];
	lCameraPos[2] += mStabilizeMatrix[14];
	*/
	pCamera->GetCameraMatrix( lCamProj, kFBProjection, pEvaluateInfo );	// this is const
	pCamera->GetCameraMatrix( lCamModelView, kFBModelView, pEvaluateInfo );
	pCamera->GetCameraMatrix( lCamMVP, kFBModelViewProj, pEvaluateInfo );

	FBMatrixMult( lCamModelView, lCamModelView, mStabilizeMatrix );
	FBMatrixMult( lCamMVP, lCamProj, lCamModelView );

	FBMatrixInverse( lCamInvMVP, lCamMVP );

	//pCamera->GetCameraMatrix( lCamMVP, kFBModelViewProj, pEvaluateInfo );
	//FBMatrixInverse( lCamInvMVP, lCamMVP );

	MappingItem *pItem = &Items[startIndex];
	for (	int i=startIndex; 
			i<=endIndex; 
			++i, ++pItem)
	{
		// feature - static eye corners
		if (StaticEyeCorner && (i==eEyeCornerInnerLeft||i==eEyeCornerInnerRight||i==eEyeCornerOuterLeft||i==eEyeCornerOuterRight))
			continue;

		pItem->status = false;
		if (pItem->srcRefIndex >= 0 && pItem->dstRefIndex >= 0)
		{
			bool status = mSrcTranslation[pItem->srcRefIndex]->ReadData	( lSource, pEvaluateInfo );
			if (false == status)
			{
				FBTrace( "failed to read data\n" );
				continue;
			}

			pItem->Scale.GetData(lScaling, sizeof(double)*3, pEvaluateInfo);
			if (lScaling[0] != 1.0 || lScaling[1] != 1.0 || lScaling[2] != 1.0)
			{
				pItem->PivotPoint.GetData(lPivot, sizeof(double)*3, pEvaluateInfo);

				lSource[0] = lPivot[0] + lScaling[0] * (lSource[0] - lPivot[0]);
				lSource[1] = lPivot[1] + lScaling[1] * (lSource[1] - lPivot[1]);
				lSource[2] = lPivot[2] + lScaling[2] * (lSource[2] - lPivot[2]);
			}

			memcpy( pItem->bakedSource, lSource, sizeof(double) * 3 );
			status = UnProjectPoint(MappingNames[pItem->srcRefIndex], lSource, lDepth, lCamInvMVP, lCameraPos, backingModel, backingMatrix, lResult);
			
			if (false == status)
			{
				continue;
			}

			// DONE: apply offset !

			if (pItem->needComputeOffset)
			{
				FBSub( lOffset, pItem->globalPosForOffset, lResult );
				pItem->Offset.SetData(lOffset);
				pItem->needComputeOffset = false;
			}
			else
			{
				pItem->Offset.GetData(lOffset, sizeof(double)*3, pEvaluateInfo);
			}

			FBAdd(lResult, lResult, lOffset);

			if ( (i==eEyeTopLeft||i==eEyeBottomLeft||i==eEyeTopRight||i==eEyeBottomRight) && colModel != nullptr )
			{
				// TODO: read collision matrix !
				FBMatrix m;
				m.Identity();
				status = PointCollideWith(pEvaluateInfo, colModel, m, colThickness, lResult);
			}
			
			if (i==eNoseLowerMiddle)
			{
				FBVector3d stabv = StabilizationVector;
				FBSub( lOffset, lResult, FBTVector(stabv[0], stabv[1], stabv[2], 1.0) );
				double len = FBLength(lOffset);
				StabilizationError = len * len;
			}

			if (true == status)
			{
				mBoneOutTranslation[pItem->dstRefIndex]->WriteData	( lResult, pEvaluateInfo );
				memcpy( pItem->bakedPosition, lResult, sizeof(double) * 3 );

				pItem->status = true;
			}
		}
	}

	return true;
}

bool ConstraintFacialRetargeting::ProcessStickyLips(FBEvaluateInfo* pEvaluateInfo)
{
	// input inner left corner, inner right corner
	// output stickylipUpLeft, DownLeft, UpRight, DownRight

	// RIGHT CORNER

	if (Items[eStickyLipUpRight].dstRefIndex >= 0 &&
		Items[eStickyLipDownRight].dstRefIndex >= 0 &&
		Items[eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_066].dstRefIndex >= 0 &&
		Items[eMouthLipInnerCornerRight].dstRefIndex >= 0 &&
		Items[eMouthLipInnerCornerRight_MouthLipInnerBottomMiddle_033].dstRefIndex >= 0
		)
	{
		FBVector3d v1, v2, c, m1, m2, m;

		v1 = Items[eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_066].bakedPosition;
		c = Items[eMouthLipInnerCornerRight].bakedPosition;
		v2 = Items[eMouthLipInnerCornerRight_MouthLipInnerBottomMiddle_033].bakedPosition;

		VectorMiddle(v1, c, m1);
		VectorMiddle(v2, c, m2);
		VectorMiddle(m1, m2, m);

		double stickyF, rightStickyF;
		if (StickyLips.IsAnimated() )
			StickyLips.GetAnimationNode()->ReadData( &stickyF, pEvaluateInfo );
		else stickyF = StickyLips;

		if (StickyRightLip.IsAnimated() )
			StickyRightLip.GetAnimationNode()->ReadData( &rightStickyF, pEvaluateInfo );
		else rightStickyF = StickyRightLip;

		stickyF = 0.01 * std::max(stickyF, rightStickyF);

		VectorLerp(m1, m, stickyF, m1);
		VectorLerp(m2, m, stickyF, m2);

		mBoneOutTranslation[Items[eStickyLipUpRight].dstRefIndex]->WriteData(m1, pEvaluateInfo);
		mBoneOutTranslation[Items[eStickyLipDownRight].dstRefIndex]->WriteData(m2, pEvaluateInfo);

		Items[eStickyLipUpRight].bakedPosition = m1;
		Items[eStickyLipDownRight].bakedPosition = m2;
	}

	// LEFT CORNER
	if (Items[eStickyLipUpLeft].dstRefIndex >= 0 &&
		Items[eStickyLipDownLeft].dstRefIndex >= 0 &&
		Items[eMouthLipInnerBottomMiddle_MouthLipInnerCornerLeft_066].dstRefIndex >= 0 &&
		Items[eMouthLipInnerCornerLeft].dstRefIndex >= 0 &&
		Items[eMouthLipInnerCornerLeft_MouthLipInnerTopMiddle_033].dstRefIndex >= 0)
	{
		FBVector3d v1, v2, c, m1, m2, m;

		v1 = Items[eMouthLipInnerCornerLeft_MouthLipInnerTopMiddle_033].bakedPosition;
		c = Items[eMouthLipInnerCornerLeft].bakedPosition;
		v2 = Items[eMouthLipInnerBottomMiddle_MouthLipInnerCornerLeft_066].bakedPosition;

		VectorMiddle(v1, c, m1);
		VectorMiddle(v2, c, m2);
		VectorMiddle(m1, m2, m);

		double stickyF, leftStickyF;
		if (StickyLips.IsAnimated() )
			StickyLips.GetAnimationNode()->ReadData( &stickyF, pEvaluateInfo );
		else stickyF = StickyLips;

		if (StickyLeftLip.IsAnimated() )
			StickyLeftLip.GetAnimationNode()->ReadData( &leftStickyF, pEvaluateInfo );
		else leftStickyF = StickyLeftLip;

		stickyF = 0.01 * std::max(stickyF, leftStickyF);

		VectorLerp(m1, m, stickyF, m1);
		VectorLerp(m2, m, stickyF, m2);

		mBoneOutTranslation[Items[eStickyLipUpLeft].dstRefIndex]->WriteData(m1, pEvaluateInfo);
		mBoneOutTranslation[Items[eStickyLipDownLeft].dstRefIndex]->WriteData(m2, pEvaluateInfo);

		Items[eStickyLipUpLeft].bakedPosition = m1;
		Items[eStickyLipDownLeft].bakedPosition = m2;
	}

	return true;
}

bool ConstraintFacialRetargeting::ProcessJawAim(FBCamera *pCamera, FBModel *backingModel, const FBMatrix &backingMatrix, FBEvaluateInfo* pEvaluateInfo)
{

	if (pCamera == nullptr)
		return false;

	const double lDepth = 0.01 * UnProjectDepth;

	FBVector3d lSource;
	FBVector3d lCameraPos;
	FBMatrix lCamMVP, lCamInvMVP;
	FBTVector lResult, lOffset;

	lCamMVP.Identity();
	lCamInvMVP.Identity();
	lCameraPos.Init();

	pCamera->GetVector(lCameraPos, kModelTranslation, true, pEvaluateInfo);

	pCamera->GetCameraMatrix( lCamMVP, kFBModelViewProj, pEvaluateInfo );
	FBMatrixInverse( lCamInvMVP, lCamMVP );

	if (Items[eJawRotation].DstObject.GetCount() > 0 && Items[eJawRotation].dstRefIndex >= 0)
	{
		// 1 - compute average source jaw target point

		int count = 0;
		FBVector3d target(0.0, 0.0, 0.0);

		for (int i=eJaw1; i<=eJaw3; ++i)
		{
			MappingItem *pItem = &Items[i];
			if (pItem->srcRefIndex >= 0)
			{
				mSrcTranslation[pItem->srcRefIndex]->ReadData	( lSource, pEvaluateInfo );
				VectorAdd(target, lSource, target);

				count += 1;
			}
		}

		if (count > 0)
		{
			VectorMult( target, 1.0 / count );

			UnProjectPoint("target", target, lDepth, lCamInvMVP, lCameraPos, backingModel, backingMatrix, lResult);

			// compute lookAt matrix
			FBVector3d eye, center;
			FBVector3d up = JawUpVector;
			if (JawRotationUp.GetCount() > 0)
			{
				FBModel *pRotationUp = (FBModel*) JawRotationUp.GetAt(0);
				FBMatrix rotMatrix;
				pRotationUp->GetMatrix(rotMatrix);
				rotMatrix[12] = 0.0;
				rotMatrix[13] = 0.0;
				rotMatrix[14] = 0.0;

				FBTVector up4(up[0], up[1], up[2], 1.0);
				FBVectorMatrixMult( up4, rotMatrix, up4 );
				memcpy( up, up4, sizeof(double)*3 );
				VectorNormalize(up);
			}

			FBModel *pJawBone = (FBModel*) Items[eJawRotation].DstObject.GetAt(0);
			pJawBone->GetVector(eye, kModelTranslation, true, pEvaluateInfo);
			FBSub( lResult, lResult, FBTVector(eye[0], eye[1], eye[2], 1.0) ); 

			FBMatrix matrix;
			center.Set(lResult);
			VectorNormalize(center);
			LookAt(FBVector3d(0.0, 0.0, 0.0), FBVector3d(-center[2], center[1], -center[0]), up, matrix);

			FBRVector rotation;
			FBMatrixToRotation(rotation, matrix);
			
			MappingItem &item = Items[eJawRotation];
			if (item.needComputeOffset)
			{
				FBQuaternion q, q1, q2;

				FBRotationToQuaternion( q1, rotation );
				FBRotationToQuaternion( q2, FBRVector(item.globalPosForOffset) );
				
				QuatInverse(q2);
				FBQMult(q, q1, q2);

				FBRVector r;
				FBQuaternionToRotation(r, q);
				item.Offset.SetData(r);
				memcpy(lOffset, r, sizeof(double)*3);

				item.needComputeOffset = false;
			}
			else
			{
				FBQuaternion q, q1, q2;

				item.Offset.GetData(lOffset, sizeof(double)*3, pEvaluateInfo);

				FBRotationToQuaternion( q1, rotation );
				FBRotationToQuaternion( q2, FBRVector(lOffset) );

				FBQMult( q, q1, q2 );
				FBQuaternionToRotation( rotation, q );
			}

			//mBoneOutTranslation[item.dstRefIndex]->WriteData(rotation, pEvaluateInfo);
			memcpy( item.bakedPosition, rotation, sizeof(double) * 3 );
		}
	}

	return true;
}

void ConstraintFacialRetargeting::DoMappingClear()
{
	int answer = FBMessageBox( "Facial Retargeting", "What do you want to clear ?", "All", "Only Source", "Only Destination" );

	for (int i=0; i<eMappingCount; ++i)
	{
		MappingItem &item = Items[i];

		if (item.srcInUse && (answer == 1 || answer == 2) )
			item.SrcObject.Clear();

		if (item.dstInUse && (answer == 1 || answer == 3) )
			item.DstObject.Clear();
	}
}

void ConstraintFacialRetargeting::DoMappingSave()
{
	FBFilePopup	lDialog;

	lDialog.Caption = "Choose file to save";
	lDialog.Filter = "*.xml";
	lDialog.Style = kFBFilePopupSave;

	if (lDialog.Execute() )
	{
		FBString filename( lDialog.FullFilename );

		TiXmlDocument		xmlDoc(filename);

		TiXmlElement		headElem("header");
		headElem.SetAttribute( "version", 1 );

		TiXmlElement		mappingElem("mapping");

		for (int i=0; i<eMappingCount; ++i)
		{
			TiXmlElement elem( MappingNames[i] );

			FBString srcName( mSrcMapping[i] );
			FBString dstName( mDstMapping[i] );

			MappingItem &item = Items[i];

			if (item.srcInUse && item.SrcObject.GetCount() > 0)
				srcName = item.SrcObject.GetAt(0)->Name;
			if (item.dstInUse && item.DstObject.GetCount() > 0)
				dstName = item.DstObject.GetAt(0)->Name;

			elem.SetAttribute( "label", MappingNames[i] );
			elem.SetAttribute( "source", srcName );
			elem.SetAttribute( "destination", dstName );

			mappingElem.InsertEndChild(elem);
		}

		xmlDoc.InsertEndChild(headElem);
		xmlDoc.InsertEndChild(mappingElem);
		xmlDoc.SaveFile();
	}
}

void ConstraintFacialRetargeting::DoMappingLoad()
{
	FBFilePopup	lDialog;

	lDialog.Caption = "Choose file to save";
	lDialog.Filter = "*.xml";
	lDialog.Style = kFBFilePopupOpen;

	if (lDialog.Execute() )
	{

		TiXmlDocument		xmlDoc;

		try
		{

			if (false == xmlDoc.LoadFile(lDialog.FullFilename) )
				throw std::exception(XML_ERROR_LOAD_FILE_FAILED);

			TiXmlNode *node = nullptr;
			TiXmlElement *headElement = nullptr;
			TiXmlElement *mappingElem = nullptr;
			TiXmlElement *elem = nullptr;

			TiXmlAttribute  *attrib = nullptr;

			node = xmlDoc.FirstChild("header");
			if (node == nullptr)
				throw std::exception(XML_ERROR_HEADER_NODE_NOT_EXIST);

			int version = 0;

			headElement = node->ToElement();
			if (headElement)
			{
			
				for (attrib = headElement->FirstAttribute(); attrib!=nullptr; attrib=attrib->Next() )
				{
					if ( strcmp(attrib->Name(), "version") == 0 )
						version = attrib->IntValue();
				}
			}

			if (version != 1)
				throw std::exception(XML_ERROR_WRONG_FILE_VERSION);

			node = xmlDoc.FirstChild("mapping");
			if (node == nullptr)
				throw std::exception(XML_ERROR_MAPPING_NODE_NOT_EXIST);

			mappingElem = node->ToElement();
			if (nullptr == mappingElem)
				throw std::exception(XML_ERROR_NULL_MAPPING_ELEMENT);

			mSrcMapping.Clear();
			mDstMapping.Clear();

			for (int i=0; i<eMappingCount; ++i)
			{
				elem = mappingElem->FirstChildElement( MappingNames[i] );

				bool assignedSrc = false;
				bool assignedDst = false;

				if (elem != nullptr)
				{
					for (attrib = elem->FirstAttribute(); 
						attrib!=nullptr; 
						attrib=attrib->Next() )
					{
						if ( strcmp(attrib->Name(), "source") == 0 )
						{
							mSrcMapping.Add( attrib->Value() );
							assignedSrc = true;
						}
						else if (strcmp(attrib->Name(), "destination") == 0)
						{
							mDstMapping.Add( attrib->Value() );
							assignedDst = true;
						}
					}
				}

				if (assignedSrc == false)
					mSrcMapping.Add(MappingNames[i]);
				if (assignedDst == false)
					mDstMapping.Add(MappingNames[i]);
			}

		}
		catch (std::exception	&e)
		{
			FBMessageBox( "Xml Error", e.what(), "Ok" ); 
		}

	}
}

void ConstraintFacialRetargeting::DoGrabPositions()
{
	if (Active == false)
		return;

	for (int i=0; i<eMappingCount; ++i)
	{
		MappingItem &item = Items[i];

		if (item.dstInUse==false || item.DstObject.GetCount() == 0)
			continue;

		// skip eye corners if doesn't use them
		if (StaticEyeCorner && (i==eEyeCornerInnerLeft||i==eEyeCornerInnerRight||i==eEyeCornerOuterLeft||i==eEyeCornerOuterRight))
			continue;


		FBModel *pModel = (FBModel*) item.DstObject.GetAt(0);

		if (i == eJawRotation)
		{
			pModel->SetVector(item.bakedPosition, kModelRotation);
		}
		else
		{
			pModel->SetVector(item.bakedPosition);
		}
	}
}

void ConstraintFacialRetargeting::DoApplyCameraSettings()
{
	auto fn_camera = [] (FBCamera *pCamera, const double fov, const int width, const int height) {

		pCamera->FieldOfView = fov;
		pCamera->ResolutionWidth = width;
		pCamera->ResolutionHeight = height;

		pCamera->ResolutionMode = kFBResolutionCustom;

	};

	// update values from connected source (if avaliable)

	FBModel *pModel = (Items[eNoseLowerMiddle].SrcObject.GetCount() > 0) ? (FBModel*)Items[eNoseLowerMiddle].SrcObject.GetAt(0) : nullptr;
	
	if (UseValuesFromOpticalRoot)
	{
		FBModel *pParent = FindParent(pModel);

		// use parent properties if exist
		FBProperty *prop = nullptr;

		prop = pParent->PropertyList.Find( "SourceWidth" );
		if (nullptr != prop)
			CameraWidth = prop->AsInt();
		prop = pParent->PropertyList.Find( "SourceHeight" );
		if (nullptr != prop)
			CameraHeight = prop->AsInt();
	}

	// apply to the assigned cameras

	if (CameraMouth.GetCount() > 0)
		fn_camera( (FBCamera*) CameraMouth.GetAt(0), CameraFOV, CameraWidth, CameraHeight );
	if (CameraNose.GetCount() > 0)
		fn_camera( (FBCamera*) CameraNose.GetAt(0), CameraFOV, CameraWidth, CameraHeight );
	if (CameraEyeLeft.GetCount() > 0)
		fn_camera( (FBCamera*) CameraEyeLeft.GetAt(0), CameraFOV, CameraWidth, CameraHeight );
	if (CameraEyeRight.GetCount() > 0)
		fn_camera( (FBCamera*) CameraEyeRight.GetAt(0), CameraFOV, CameraWidth, CameraHeight );
}

void ConstraintFacialRetargeting::DoZeroEyesOffsets()
{
	for (int i=eEyeTopLeft; i<=/*eEyeCornerOuterLeft*/ ePupilInnerLeft; ++i)
	{
		Items[i].Offset.SetAnimated(false);
		Items[i].Offset = FBVector3d(0.0, 0.0, 0.0);
	}

	for (int i=eEyeTopRight; i<=/*eEyeCornerInnerRight*/ ePupilInnerRight; ++i)
	{
		Items[i].Offset.SetAnimated(false);
		Items[i].Offset = FBVector3d(0.0, 0.0, 0.0);
	}
}

void ConstraintFacialRetargeting::DoZeroEyesLookAtOffsets()
{
	for (int i=ePupilOuterLeft; i<=ePupilInnerLeft; ++i)
	{
		Items[i].Offset.SetAnimated(false);
		Items[i].Offset = FBVector3d(0.0, 0.0, 0.0);
	}

	for (int i=ePupilOuterRight; i<=ePupilInnerRight; ++i)
	{
		Items[i].Offset.SetAnimated(false);
		Items[i].Offset = FBVector3d(0.0, 0.0, 0.0);
	}
}

void ConstraintFacialRetargeting::DoZeroMouthOffsets()
{
	for (int i=eMouthLipOuterVeeRight; i<=eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_033; ++i)
	{
		Items[i].Offset.SetAnimated(false);
		Items[i].Offset = FBVector3d(0.0, 0.0, 0.0);
	}
}

void ConstraintFacialRetargeting::PublishParametricProperty3d( const char *name, ParametricProperty3d &prop, const FBVector3d &dir )
{
	FBPropertyPublish( this, prop.Base, FBString(name, " Base"), nullptr, nullptr );
	FBPropertyPublish( this, prop.Dir, FBString(name, " Dir"), nullptr, nullptr );
	FBPropertyPublish( this, prop.Distance, FBString(name, " Distance"), nullptr, nullptr );
	FBPropertyPublish( this, prop.Low, FBString(name, " Low"), nullptr, nullptr );
	FBPropertyPublish( this, prop.High, FBString(name, " High"), nullptr, nullptr );
	FBPropertyPublish( this, prop.Position, FBString(name, " Position"), nullptr, nullptr );

	prop.Base = FBVector3d(0.0, 0.0, 0.0);
	prop.Dir = dir;
	prop.Distance = 0.0;
	prop.Distance.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	prop.Low = 0.0;
	prop.High = 1.0;
	prop.Position = 0.0;
	prop.Position.SetAnimated(true);
	prop.NodeOut = nullptr;
}

void ConstraintFacialRetargeting::SetupParametricAnimationNode( const int index, ParametricProperty3d &prop )
{
	if (prop.NodeOut == nullptr)
		prop.NodeOut = AnimationNodeInCreate( index, &prop.Position );
}

void ConstraintFacialRetargeting::ProcessParametricProperty3dOneWay( const Mapping itemId, ParametricProperty3d &prop, FBEvaluateInfo *pinfo )
{
	FBVector3d v1, v2, res, dir;

	v1 = Items[itemId].bakedSource;
	v2 = prop.Base;
	dir = prop.Dir;
	VectorNormalize(dir);

	VectorSub(v1, v2, res);
	VectorMult(res, dir, res);

	double len = VectorLength(res);
	VectorNormalize(res);
	len *= DotProduct(res, dir);

	prop.Distance = len;

	double low = prop.Low;
	double high = prop.High;

	len = clamp01( (len - low) / (high - low) );
	len = 100.0 * len;

	

	if (prop.NodeOut != nullptr)
		prop.NodeOut->WriteData( &len, pinfo );
}

void ConstraintFacialRetargeting::ProcessParametricProperty3dTwoWays( const Mapping itemId, const Mapping itemId2, ParametricProperty3d &prop, FBEvaluateInfo *pinfo, bool calibrate )
{
	FBVector3d v1, v2, res;

	v1 = Items[itemId].bakedSource;
	v2 = Items[itemId2].bakedSource;

	VectorSub(v1, v2, res);
	VectorMult(res, prop.Dir, res);

	double len = VectorLength(res);

	prop.Distance.SetData(&len);

	double low, high;
	
	prop.Low.GetData( &low, sizeof(double), pinfo );
	prop.High.GetData( &high, sizeof(double), pinfo );

	if (calibrate)
	{
		if (len < low)
		{
			prop.Low.SetData( &len );
			low = len;
		}
		if (len > high)
		{
			prop.High.SetData( &len );
			high = len;
		}
	}

	if (high - low != 0.0)
		len = clamp01( (len - low) / (high - low) );
	else
		len = 0.0;

	len = 100.0 * (1.0 - len);

	if (prop.NodeOut != nullptr)
		prop.NodeOut->WriteData( &len, pinfo );
}


bool ConstraintFacialRetargeting::StabilizePositions( FBCamera *pCamera, FBModel *pBakingMesh, const FBMatrix &backingMatrix,
						const int stabIndex,
						const double stabSourceU, const double stabSourceV,
						FBEvaluateInfo *pEvaluateInfo )
{
	if (pCamera == nullptr)
		return false;

	const double lDepth = 0.01 * UnProjectDepth;

	//!!
	// TODO: compute stab source on characterize !!
	FBTVector lSourceGoal(stabSourceU, stabSourceV, 0.0, 1.0);

	FBVector3d lCameraPos;
	FBMatrix lCamModelView, lCamProj, lCamMVP, lCamInvMVP;
	FBTVector lSource, lOffset, lGoal;
	FBTVector lCandidate, lBest;

	lCamModelView.Identity();
	lCamProj.Identity();
	lCamMVP.Identity();
	lCamInvMVP.Identity();
	lCameraPos.Init();

	pCamera->GetVector(lCameraPos, kModelTranslation, true, pEvaluateInfo);

	pCamera->GetCameraMatrix( lCamProj, kFBProjection, pEvaluateInfo );	// this is const
	pCamera->GetCameraMatrix( lCamModelView, kFBModelView, pEvaluateInfo );
	pCamera->GetCameraMatrix( lCamMVP, kFBModelViewProj, pEvaluateInfo );
	FBMatrixInverse( lCamInvMVP, lCamMVP );

	
	auto fn_unproject = [] (FBTVector &v, const double depth, const FBMatrix &camInvMVP) {

		v[0] = 0.01 * v[0] * 2.0 - 1.0;
		v[1] = 0.01 * v[1] * 2.0 - 1.0;
		v[2] = depth;
		v[3] = 1.0;

		FBVectorMatrixMult(v, camInvMVP, v);

		if (v[3] != 0.0)
		{
			v[0] /= v[3];
			v[1] /= v[3];
			v[2] /= v[3];
			v[3] = 1.0;
		}
	};

	//

	MappingItem &item = Items[stabIndex];
	if (item.srcRefIndex < 0 || item.dstRefIndex < 0)
		return false;

	lGoal = FBTVector( lSourceGoal[0], lSourceGoal[1], 0.0, 1.0 );
	//fn_unproject( lGoal, lDepth, lCamInvMVP );
	lCandidate = lGoal;

	mSrcTranslation[item.srcRefIndex]->ReadData	( lSource, pEvaluateInfo );

	fn_unproject( lSourceGoal, lDepth, lCamInvMVP );
	//fn_unproject( lSource, lDepth, lCamInvMVP );

	UnProjectPoint(MappingNames[item.srcRefIndex], FBVector3d(lSource), lDepth, lCamInvMVP, lCameraPos, pBakingMesh, backingMatrix, lSource );

	FBTVector vsub;
	FBSub( vsub, lSource, FBTVector( lCameraPos[0], lCameraPos[1], lCameraPos[2], 1.0) );
	double len = FBLength(vsub);

	FBSub( vsub, lSourceGoal, FBTVector( lCameraPos[0], lCameraPos[1], lCameraPos[2], 1.0) );
	double goalLen = FBLength( vsub );

	FBMult( lSourceGoal, vsub, len / goalLen );
	FBAdd( lSourceGoal, lSourceGoal, FBTVector( lCameraPos[0], lCameraPos[1], lCameraPos[2], 1.0) );

	StabilizationVector = FBVector3d( lSourceGoal );

	FBTranslationToMatrix( mStabilizeMatrix, FBTVector( lSource[0]-lSourceGoal[0], lSource[1]-lSourceGoal[1], lSource[2]-lSourceGoal[2], 1.0) );


	/*

	// compute our goal

	auto fn_unproject = [] (FBTVector &v, const double depth, const FBMatrix &camInvMVP) {

		v[0] = 0.01 * v[0] * 2.0 - 1.0;
		v[1] = 0.01 * v[1] * 2.0 - 1.0;
		v[2] = depth;
		v[3] = 1.0;

		FBVectorMatrixMult(v, camInvMVP, v);

		if (v[3] != 0.0)
		{
			v[0] /= v[3];
			v[1] /= v[3];
			v[2] /= v[3];
			v[3] = 1.0;
		}
	};

	int iter=0;
	const int maxNumberOfIterations = StabilizationIterations;
	const double stabEps = 0.01;

	mStabilizeMatrix.Identity();
	
	while (iter < maxNumberOfIterations)
	{
		// test it

		double deltaX = lGoal[0] - lCandidate[0];
		double deltaY = lGoal[1] - lCandidate[1];

		double lenx = deltaX * deltaX;
		double leny = deltaY * deltaY;

		if (lenx < stabEps && leny < stabEps)
		{
			// found a good candidate !
			break;
		}

		
		// make a new variation to try


		// prep camera matrices

		FBMatrixMult(lCamMVP, mStabilizeMatrix, lCamModelView);
		FBMatrixMult(lCamMVP, lCamProj, lCamMVP);
		FBMatrixInverse( lCamInvMVP, lCamMVP );


		// unproject

		lCandidate = lSource;
		fn_unproject( lCandidate, lDepth, lCamInvMVP );


		//!!
		// TODO: don't forget that first frame will be computer offset operation !!
		item.Offset.GetData(lOffset, sizeof(double)*3, pEvaluateInfo);

		FBAdd(lCandidate, lCandidate, lOffset);

		iter += 1;
	}
	*/

	return true;
}

void ConstraintFacialRetargeting::DoCalibrateParametric()
{
	double value;

	value = LeftEyeParam.Distance;
	LeftEyeParam.Low = value;
	LeftEyeParam.High = value;

	value = RightEyeParam.Distance;
	RightEyeParam.Low = value;
	RightEyeParam.High = value;
}

void ConstraintFacialRetargeting::ShowAbout()
{
	FBMessageBox( "Facial Constraint", " Version 0.6 Beta\n"
		" Author Sergey Solokhin (Neill3d) 2015-2017"
		"  e-mail to: s@neill3d.com\n"
		"	www.neill3d.com",
		"Ok");
}