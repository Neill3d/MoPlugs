# Copyright 2016-2017 Sergey Solokhin (Neill3d)
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
# Script description:
# Creating property views for the Composition Toolkit components
#
# Topic: Composition Toolkit
#
from pyfbsdk import *

gClassStr = ''
gCompositionModelsPath = 'Browsing/Templates/Shading Elements/Composition/Models'  
lMgr = FBPropertyViewManager()

def AddGlobalPropertyView(propName, hierarchy, isFolder=False):
    
        lMgr.AddPropertyView(gClassStr, propName, hierarchy)

def AddPropertyToViewList(pOwner, pPropertyName, pViewList, pHierarchy, pSetOpen=False):
    lProperty = pOwner.PropertyList.Find(pPropertyName)
    
    if lProperty != None:
        lView = pViewList.AddPropertyView(lProperty, pHierarchy)
    
        if pSetOpen:
            lView.SetOpen(pSetOpen,True)
    
        return lView

    return None

#################################

def PropertyViewForShadowZone():
    
    viewName = 'Shadow Zone View'
    
    lModel = FBCreateObject( gCompositionModelsPath, 'Model Shadow Zone', 'test zone')
    
    # Create local(by object) property view called 'PythonCreatedView'
    lViewList = lMgr.FindPropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)
    if lViewList != None:
        lMgr.RemovePropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)

    lViewList = lMgr.CreatePropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)
    
    AddPropertyToViewList(lModel, 'Enabled', lViewList, '')
    
    # Add 'Show' property under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Show', lViewList, 'Visibility Options')
    # Add 'Visibility' property under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Visibility', lViewList, 'Visibility Options')
    # Here we add 'Visibility Inheritance' under 'Visibility' which is under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Visibility Inheritance', lViewList, 'Visibility Options.Visibility')
        
    # Same this as above, adding properties under 'Transformation Options'
    AddPropertyToViewList(lModel, 'Translation (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Rotation (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Scaling (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Quaternion Rotation', lViewList, 'Transformation Options')

    AddPropertyToViewList(lModel, 'Resolution', lViewList, 'Quality')    
    AddPropertyToViewList(lModel, 'Bias', lViewList, 'Quality')    
    AddPropertyToViewList(lModel, 'Enable Offset', lViewList, 'Quality')    
    AddPropertyToViewList(lModel, 'Offset Factor', lViewList, 'Quality')    
    AddPropertyToViewList(lModel, 'Offset Units', lViewList, 'Quality')                
    
    AddPropertyToViewList(lModel, 'Zone Blend Mode', lViewList, '')
    AddPropertyToViewList(lModel, 'Master Light', lViewList, '')
    AddPropertyToViewList(lModel, 'Shadow Color', lViewList, '')
    AddPropertyToViewList(lModel, 'Density', lViewList, '')
    AddPropertyToViewList(lModel, 'Feather', lViewList, '')
    
    AddPropertyToViewList(lModel, 'Near Plane', lViewList, '')
    AddPropertyToViewList(lModel, 'Far Plane', lViewList, '')        
    
    AddPropertyToViewList(lModel, 'Include Object List', lViewList, 'Models Filter')
    AddPropertyToViewList(lModel, 'Exclude Object List', lViewList, 'Models Filter')
    AddPropertyToViewList(lModel, 'Use GPU Cache', lViewList, 'Models Filter')

    AddPropertyToViewList(lModel, 'Render Transparency', lViewList, 'Models Filter')
    AddPropertyToViewList(lModel, 'Auto Volume Culling', lViewList, 'Models Filter')
    
    AddPropertyToViewList(lModel, 'Draw Node Count', lViewList, 'Stats')
    AddPropertyToViewList(lModel, 'Draw GeomCache Count', lViewList, 'Stats')
    
    AddPropertyToViewList(lModel, 'RealTime Update', lViewList, '')
    AddPropertyToViewList(lModel, 'Update', lViewList, '')
    AddPropertyToViewList(lModel, 'Update On TimeSlider Change', lViewList, 'Update Options')
    AddPropertyToViewList(lModel, 'Update When Selected', lViewList, 'Update Options')
    AddPropertyToViewList(lModel, 'Update Skip Frames', lViewList, 'Update Options')
                    
    # Select model to see our new property view in Properties tool
    #lModel.Selected = True
        
    # In this case we don't have to refresh, but if you update already existing View, you should do it.
    lMgr.RefreshPropertyViews()    
    
    if lModel != None:
        lModel.FBDelete()

def PropertyViewForFogVolume():
    
    viewName = 'Volume Fog View'
    
    lModel = FBCreateObject( gCompositionModelsPath, 'Model Fog Volume', 'test volume')
    
    # Create local(by object) property view called 'PythonCreatedView'
    lViewList = lMgr.FindPropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)
    if lViewList != None:
        lMgr.RemovePropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)

    lViewList = lMgr.CreatePropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)
    
    AddPropertyToViewList(lModel, 'Enabled', lViewList, '')
    
    # Add 'Show' property under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Show', lViewList, 'Visibility Options')
    # Add 'Visibility' property under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Visibility', lViewList, 'Visibility Options')
    # Here we add 'Visibility Inheritance' under 'Visibility' which is under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Visibility Inheritance', lViewList, 'Visibility Options.Visibility')
        
    # Same this as above, adding properties under 'Transformation Options'
    AddPropertyToViewList(lModel, 'Translation (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Rotation (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Scaling (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Quaternion Rotation', lViewList, 'Transformation Options')

    AddPropertyToViewList(lModel, 'Volume Blend Mode', lViewList, '')
    AddPropertyToViewList(lModel, 'Color RGB', lViewList, '')
    AddPropertyToViewList(lModel, 'Volume Density', lViewList, '')
    AddPropertyToViewList(lModel, 'Volume Feather', lViewList, '')
    
    # Select model to see our new property view in Properties tool
    #lModel.Selected = True
        
    # In this case we don't have to refresh, but if you update already existing View, you should do it.
    lMgr.RefreshPropertyViews()    
    
    if lModel != None:
        lModel.FBDelete()

def PropertyViewForFogTarget():
    
    viewName = 'Volume Target View'
    
    lModel = FBCreateObject( gCompositionModelsPath, 'Model Fog Target', 'test target')
    
    # Create local(by object) property view called 'PythonCreatedView'
    lViewList = lMgr.FindPropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)
    if lViewList != None:
        lMgr.RemovePropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)

    lViewList = lMgr.CreatePropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)
    
    AddPropertyToViewList(lModel, 'Enabled', lViewList, '')
    
    # Add 'Show' property under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Show', lViewList, 'Visibility Options')
    # Add 'Visibility' property under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Visibility', lViewList, 'Visibility Options')
    # Here we add 'Visibility Inheritance' under 'Visibility' which is under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Visibility Inheritance', lViewList, 'Visibility Options.Visibility')
        
    # Same this as above, adding properties under 'Transformation Options'
    AddPropertyToViewList(lModel, 'Translation (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Rotation (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Scaling (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Quaternion Rotation', lViewList, 'Transformation Options')

    AddPropertyToViewList(lModel, 'Target Blend Mode', lViewList, '')
    AddPropertyToViewList(lModel, 'Color RGB', lViewList, '')
    AddPropertyToViewList(lModel, 'Target Density', lViewList, '')
    AddPropertyToViewList(lModel, 'Target Feather', lViewList, '')
    AddPropertyToViewList(lModel, 'Target Near Plane', lViewList, '')
    AddPropertyToViewList(lModel, 'Target Far Plane', lViewList, '')
    
    # Select model to see our new property view in Properties tool
    #lModel.Selected = True
        
    # In this case we don't have to refresh, but if you update already existing View, you should do it.
    lMgr.RefreshPropertyViews()    
    
    if lModel != None:
        lModel.FBDelete()

def PropertyViewForDecal():
    
    viewName = 'Model Decal View'
    
    lModel = FBCreateObject( gCompositionModelsPath, 'Model Decal', 'test decal')
    
    # Create local(by object) property view called 'PythonCreatedView'
    lViewList = lMgr.FindPropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)
    if lViewList != None:
        lMgr.RemovePropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)

    lViewList = lMgr.CreatePropertyList(lModel, FBPropertyViewType.kFBViewByObjectType, viewName)
    
    AddPropertyToViewList(lModel, 'Enabled', lViewList, '')
    AddPropertyToViewList(lModel, 'Opacity', lViewList, '')
    
    # Add 'Show' property under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Show', lViewList, 'Visibility Options')
    # Add 'Visibility' property under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Visibility', lViewList, 'Visibility Options')
    # Here we add 'Visibility Inheritance' under 'Visibility' which is under 'Visibility Options' node
    AddPropertyToViewList(lModel, 'Visibility Inheritance', lViewList, 'Visibility Options.Visibility')
        
    # Same this as above, adding properties under 'Transformation Options'
    AddPropertyToViewList(lModel, 'Translation (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Rotation (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Scaling (Lcl)', lViewList, 'Transformation Options')
    AddPropertyToViewList(lModel, 'Quaternion Rotation', lViewList, 'Transformation Options')

    AddPropertyToViewList(lModel, 'Resolution Width', lViewList, 'Texture Info')
    AddPropertyToViewList(lModel, 'Resolution Height', lViewList, 'Texture Info')
    AddPropertyToViewList(lModel, 'Aspect Ratio', lViewList, 'Texture Info')
    AddPropertyToViewList(lModel, 'Uses Story Track', lViewList, 'Texture Info')
    AddPropertyToViewList(lModel, 'Video Clip Path', lViewList, 'Texture Info')

    AddPropertyToViewList(lModel, 'Near Plane Distance', lViewList, 'Projection Options')
    AddPropertyToViewList(lModel, 'Far Plane Distance', lViewList, 'Projection Options')
    AddPropertyToViewList(lModel, 'Projection Type', lViewList, 'Projection Options')
    AddPropertyToViewList(lModel, 'Field Of View', lViewList, 'Projection Options')
    AddPropertyToViewList(lModel, 'Ortho Scale', lViewList, 'Projection Options')
    
    AddPropertyToViewList(lModel, 'Contrast', lViewList, 'Color Correction')
    AddPropertyToViewList(lModel, 'Saturation', lViewList, 'Color Correction')
    AddPropertyToViewList(lModel, 'Brightness', lViewList, 'Color Correction')
    AddPropertyToViewList(lModel, 'Gamma', lViewList, 'Color Correction')
    AddPropertyToViewList(lModel, 'Inverse', lViewList, 'Color Correction')
    AddPropertyToViewList(lModel, 'Hue', lViewList, 'Color Correction')
    AddPropertyToViewList(lModel, 'Hue Saturation', lViewList, 'Color Correction')
    AddPropertyToViewList(lModel, 'Lightness', lViewList, 'Color Correction')    

    AddPropertyToViewList(lModel, 'Grab Values', lViewList, 'Camera Controls')
    AddPropertyToViewList(lModel, 'Apply To Current', lViewList, 'Camera Controls')
    AddPropertyToViewList(lModel, 'Apply To Perspective', lViewList, 'Camera Controls')
    
    AddPropertyToViewList(lModel, 'Blend Mode', lViewList, '')
    AddPropertyToViewList(lModel, 'Texture', lViewList, '')
    AddPropertyToViewList(lModel, 'Refresh', lViewList, '')
    
    
    # Select model to see our new property view in Properties tool
    #lModel.Selected = True
        
    # In this case we don't have to refresh, but if you update already existing View, you should do it.
    lMgr.RefreshPropertyViews()    
    
    if lModel != None:
        lModel.FBDelete()


     
def GlobalPropertyViewForComposition():
    
    global gClassStr
    gClassStr = 'ObjectComposition'
    
    AddGlobalPropertyView("", "Statistics", True);
    AddGlobalPropertyView( 'Stats Width', "Statistics" )
    AddGlobalPropertyView( 'Stats Height', "Statistics" )
    AddGlobalPropertyView( 'Stats Compute Shaders', "Statistics" )
    AddGlobalPropertyView( 'Stats Dispatch Groups', "Statistics" )
    AddGlobalPropertyView( 'Stats Textures Count', "Statistics" )
    AddGlobalPropertyView( 'Stats Textures Memory', "Statistics" )
                        
    AddGlobalPropertyView("", "Background", True);
    AddGlobalPropertyView("Background Width", "Background");
    AddGlobalPropertyView("Background Height", "Background");
    AddGlobalPropertyView("Background Color", "Background");
    AddGlobalPropertyView("Background Alpha", "Background");
    AddGlobalPropertyView("Use Background Gradient", "Background");
    AddGlobalPropertyView("Background Upper Color", "Background");
    AddGlobalPropertyView("Background Upper Alpha", "Background");
    AddGlobalPropertyView("Background Lower Color", "Background");
    AddGlobalPropertyView("Background Lower Alpha", "Background");
    #AddGlobalPropertyView("Use Background Texture", "Background");
    AddGlobalPropertyView("Background Texture", "Background");
    AddGlobalPropertyView("Back Texture Width", "Background");
    AddGlobalPropertyView("Back Texture Height", "Background");
    
    AddGlobalPropertyView("", "Size Control", True);
    AddGlobalPropertyView("Override Size", "Size Control");
    AddGlobalPropertyView("User Width", "Size Control");
    AddGlobalPropertyView("User Height", "Size Control");
    AddGlobalPropertyView("ReSize Factor", "Size Control");
    
    AddGlobalPropertyView("", "Batch processing", True);
    AddGlobalPropertyView("Use For Batch Processing", "Batch processing");
    AddGlobalPropertyView("Batch input", "Batch processing");
    AddGlobalPropertyView("Batch output", "Batch processing");

    AddGlobalPropertyView("Input", "")
    AddGlobalPropertyView("Out Width", "")
    AddGlobalPropertyView("Out Height", "")

def GlobalPropertyViewForFilter(classstr):
    
    global gClassStr
    gClassStr = classstr
    
    AddGlobalPropertyView("Active", "")
    AddGlobalPropertyView("Reload Shader", "")
    AddGlobalPropertyView("Opacity", "")
    
    AddGlobalPropertyView("", "Masking", True)
    AddGlobalPropertyView("Use Composite Mask", "Masking")
    AddGlobalPropertyView("Select Composite Mask", "Masking")
    AddGlobalPropertyView("Invert Composite Mask", "Masking")
    AddGlobalPropertyView("Custom Mask", "Masking")

    AddGlobalPropertyView("", "Info", True)
    AddGlobalPropertyView("Out Width", "Info")
    AddGlobalPropertyView("Out Height", "Info")

def GlobalForFogFilter(classstr):

    global gClassStr
    gClassStr = classstr

    AddGlobalPropertyView("Use Model Properties", "")    
    AddGlobalPropertyView("Color RGB", "")
    AddGlobalPropertyView("Mode", "")
    AddGlobalPropertyView("Density", "")
    AddGlobalPropertyView("Alpha Texture", "")
    AddGlobalPropertyView("Fog Near", "")
    AddGlobalPropertyView("Fog Far", "")
    AddGlobalPropertyView("Fog Feather", "")        
    AddGlobalPropertyView("Target Type", "")
    AddGlobalPropertyView("Target Object", "")
    AddGlobalPropertyView("Create Target", "Target Object")
    AddGlobalPropertyView("Target Near Object", "")
    AddGlobalPropertyView("Target Far Object", "")
    AddGlobalPropertyView("Create Near/Far Targets", "Target Near Object")
    AddGlobalPropertyView("Volume Object", "")
    AddGlobalPropertyView("Create Volume", "Volume Object")
    
def GlobalPropertyViewForFilters():
    GlobalPropertyViewForFilter( 'ObjectFilter3dDecal' )
    GlobalPropertyViewForFilter( 'ObjectFilter3dDOF' )
    GlobalPropertyViewForFilter( 'ObjectFilter3dFog' )
    GlobalForFogFilter('ObjectFilter3dFog' )
    GlobalPropertyViewForFilter( 'ObjectFilterLUT' )
    GlobalPropertyViewForFilter( 'ObjectFilterBlur' )
    GlobalPropertyViewForFilter( 'ObjectFilterChangeColor' )
    GlobalPropertyViewForFilter( 'ObjectFilterColorCorrection' )
    GlobalPropertyViewForFilter( 'ObjectFilterCrossStitching' )
    GlobalPropertyViewForFilter( 'ObjectFilterCrosshatch' )
    GlobalPropertyViewForFilter( 'ObjectFilterFilmGrain' )
    GlobalPropertyViewForFilter( 'ObjectFilterHalfTone' )
    GlobalPropertyViewForFilter( 'ObjectFilterToonLines' )
    GlobalPropertyViewForFilter( 'ObjectFilterPosterization' )

def GlobalPropertyViewForLayerCommon(classstr):
    
    global gClassStr
    gClassStr = classstr
    
    AddGlobalPropertyView( "Active", "" )
    AddGlobalPropertyView( "Input", "")
    AddGlobalPropertyView( "Reload Shader", "" )
    AddGlobalPropertyView( "Opacity", "" )
    
    AddGlobalPropertyView( "", "Transformation", True )
    AddGlobalPropertyView( "Translation", "Transformation" )
    AddGlobalPropertyView( "Rotation", "Transformation" )
    AddGlobalPropertyView( "Uniform Scaling", "Transformation" )
    AddGlobalPropertyView( "Scaling", "Transformation" )
    AddGlobalPropertyView( "Pivot Offset", "Transformation" )
    AddGlobalPropertyView( "Transform Init", "Transformation" )
    AddGlobalPropertyView( "Fit Image Size", "Transformation" )
    AddGlobalPropertyView( "", "Masking", True )
    AddGlobalPropertyView( "Use Composite Mask", "Masking" )
    AddGlobalPropertyView( "Select Composite Mask", "Masking" )
    AddGlobalPropertyView( "Invert Composite Mask", "Masking" )
    AddGlobalPropertyView( "Custom Mask", "Masking" )

    AddGlobalPropertyView("", "Info", True)
    AddGlobalPropertyView("Out Width", "Info")
    AddGlobalPropertyView("Out Height", "Info")

def GlobalPropertyViewForLayers():
    
    GlobalPropertyViewForLayerCommon( 'ObjectCompositionRender' )
    GlobalPropertyViewForLayerCommon( 'ObjectCompositionColor' )    
    GlobalPropertyViewForLayerCommon( 'ObjectCompositionShadow' )
        
####################################################################
#### MAIN ####

PropertyViewForShadowZone()
PropertyViewForFogVolume()
PropertyViewForFogTarget()
PropertyViewForDecal()
##
GlobalPropertyViewForComposition()
## filters
GlobalPropertyViewForFilters()
## layers
GlobalPropertyViewForLayers()