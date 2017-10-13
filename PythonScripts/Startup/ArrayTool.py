
#########################################################
##
## Array Tool script for Autodesk MotionBuilder
##
## Author Sergey Solohin (Neill3d) 2014
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
#########################################################

from pyfbsdk import *
from pyfbsdk_additions import *

gDevelopment = True

ARRAY_TOOL_VERSION = 0.59

lApp = FBApplication()
lSystem = FBSystem()

Count1D = FBEditNumber()
Count2D = FBEditNumber()
Count3D = FBEditNumber()

arrayTransformation = []
arrayTransformation2 = []
arrayTransformation3 = []

lastNamespace = ""

# models to clone
srcModels = []
# models to drive (move, rotate, scale)
srcDrv = []

buttonSelection = FBButton()
buttonFileMerge = FBButton()

editFileName = FBEdit()

buttonModeAll = FBButton()
buttonModeRoot = FBButton()
buttonModeObject = FBButton()

editModelName = FBEdit()

###########################################################

def DoCloneModel( mdl, translation, rotation, scale ):
    
    newModel = mdl.Clone()

    newModel.SetVector(translation)
    newModel.SetVector(rotation, FBModelTransformationType.kModelRotation)
    newModel.SetVector(scale, FBModelTransformationType.kModelScaling) 
    #
       
def DoMergeModel(fileName, options):

    lApp.FileMerge(fileName, False, options)
    #       
       
def findRoots(modelList, namespace):
    
    # 3 modes - all models, root only, or use specified model name
    
    result = []
    
    if buttonModeAll.State == 1:
        result = modelList
        
    elif buttonModeRoot.State == 1:
        
        for mdl in modelList:
            if not isinstance(mdl, FBModel): continue
            skip = False
            
            parent = mdl.Parent
            if parent:
                for mdl2 in modelList:
                    if mdl == mdl2 or not isinstance(mdl, FBModel): continue
                    if parent == mdl2:
                        skip = True
                        break
                        
            if skip == False:
                result.append(mdl)
                
    elif editModelName.Text != "":
        strNamespace = namespace + ':' + editModelName.Text
        lModel = FBFindModelByLabelName( strNamespace )
        if lModel:
            result.append(lModel)
          
    return result              
       
def DoArrayElement(i, j, k, modelFileName, lOptions):
    global lastNamespace
    
    strNamespaceUnique = 'Array_' + str(i) + "_" + str(j) + "_" + str(k)
    strNamespace = "imported"

    lastNamespace = ""
    if lastNamespace != "":
        # rename all materials, shaders, textures and video clips
    
        comps = FBComponentList()
        
        lSystem.Scene.NamespaceGetContentList(comps, lastNamespace, FBPlugModificationFlag.kFBPlugAllContent, True, FBMaterial.TypeInfo, True)
        for elem in comps: elem.ProcessObjectNamespace(FBNamespaceAction.kFBReplaceNamespace, lastNamespace, strNamespace) 
        
        lSystem.Scene.NamespaceGetContentList(comps, lastNamespace, FBPlugModificationFlag.kFBPlugAllContent, True, FBVideo.TypeInfo, False)
        for elem in comps: elem.ProcessObjectNamespace(FBNamespaceAction.kFBReplaceNamespace, lastNamespace, strNamespace)                 
        
        lSystem.Scene.NamespaceGetContentList(comps, lastNamespace, FBPlugModificationFlag.kFBPlugAllContent, True, FBVideoClip.TypeInfo, False)
        for elem in comps: elem.ProcessObjectNamespace(FBNamespaceAction.kFBReplaceNamespace, lastNamespace, strNamespace) 
        
        lSystem.Scene.NamespaceGetContentList(comps, lastNamespace, FBPlugModificationFlag.kFBPlugAllContent, True, FBVideoClipImage.TypeInfo, False)
        for elem in comps: elem.ProcessObjectNamespace(FBNamespaceAction.kFBReplaceNamespace, lastNamespace, strNamespace)                 
        
        lSystem.Scene.NamespaceGetContentList(comps, lastNamespace, FBPlugModificationFlag.kFBPlugAllContent, True, FBTexture.TypeInfo, True)
        for elem in comps: elem.ProcessObjectNamespace(FBNamespaceAction.kFBReplaceNamespace, lastNamespace, strNamespace) 
        
        lSystem.Scene.NamespaceGetContentList(comps, lastNamespace, FBPlugModificationFlag.kFBPlugAllContent, True, FBLayeredTexture.TypeInfo, True)
        for elem in comps: elem.ProcessObjectNamespace(FBNamespaceAction.kFBReplaceNamespace, lastNamespace, strNamespace) 
        
        del (comps)
        
    lOptions.NamespaceList = strNamespace 
    lOptions.SetAll(FBElementAction.kFBElementActionMerge, True)
    lApp.FileMerge(modelFileName, False, lOptions)        

    #lastNamespace = strNamespace

    importedModels = FBComponentList()

    lnamespace = lSystem.Scene.NamespaceGet( strNamespace )
    lnamespace.GetContentList(importedModels)
    
    for mdl in importedModels:
        if isinstance(mdl, FBModel):
            namespace = mdl.OwnerNamespace
            
            mdl.ProcessObjectNamespace(FBNamespaceAction.kFBConcatNamespace, strNamespaceUnique)
            mdl.ProcessObjectNamespace(FBNamespaceAction.kFBReplaceNamespace, strNamespace, "imported_models")
    
    # FBPlugModificationFlag.kFBPlugAllContent, True, FBModel.TypeInfo, True)
    roots = []
    roots = findRoots(importedModels, strNamespace)
    
    v = FBVector3d()
    r = FBVector3d()
    s = FBVector3d()
    
    for mdl in roots:
        mdl.GetVector(v)
        mdl.GetVector(r, FBModelTransformationType.kModelRotation)
        mdl.GetVector(s, FBModelTransformationType.kModelScaling)
        
        v[0] += arrayTransformation[0].Value * i + arrayTransformation2[0].Value * j + arrayTransformation3[0].Value * k
        v[1] += arrayTransformation[1].Value * i + arrayTransformation2[1].Value * j + arrayTransformation3[1].Value * k
        v[2] += arrayTransformation[2].Value * i + arrayTransformation2[2].Value * j + arrayTransformation3[2].Value * k
    
        r[0] += arrayTransformation[3].Value
        r[1] += arrayTransformation[4].Value
        r[2] += arrayTransformation[5].Value
                
        s[0] += arrayTransformation[6].Value
        s[1] += arrayTransformation[7].Value
        s[2] += arrayTransformation[8].Value
        
        mdl.SetVector(v)
        mdl.SetVector(r, FBModelTransformationType.kModelRotation)
        mdl.SetVector(s, FBModelTransformationType.kModelScaling)
    #
    
    del( importedModels, roots, v, r, s )
        
def DoArray(control, event):

    global lastNamespace

    pList = FBModelList()
    FBGetSelectedModels(pList)
    count = int(Count1D.Value)
    count2d = int(Count2D.Value)
    count3d = int(Count3D.Value)
    
    v = FBVector3d()
    r = FBVector3d()
    s = FBVector3d()

    v2 = FBVector3d()
    r2 = FBVector3d()
    s2 = FBVector3d()
    
    v3 = FBVector3d()
    r3 = FBVector3d()
    s3 = FBVector3d()
    
    modelFileName = editFileName.Text
    
    lastNamespace = ""
    
    if modelFileName != "" and buttonFileMerge.State == 1:
        lOptions = FBFbxOptions( True, modelFileName )
        #lOptions.NamespaceList = strNamespace
        
        lOptions.Materials = FBElementAction.kFBElementActionMerge
        lOptions.Textures = FBElementAction.kFBElementActionMerge
        lOptions.Video = FBElementAction.kFBElementActionMerge 
        
        #lOptions.Materials = FBElementAction.kFBElementActionDiscard
        #lOptions.Textures = FBElementAction.kFBElementActionDiscard
        #lOptions.Video = FBElementAction.kFBElementActionDiscard 
    
        i = 0
        j = 0
        k = 0
        
        if count2d == 0 and count3d == 0:
            for i in range(count):
                DoArrayElement( i, 0, 0, modelFileName, lOptions )
        elif count3d == 0:
            for i in range(count):                
                for j in range(count2d):    
                    DoArrayElement( i, j, k, modelFileName, lOptions )
        else:
            for i in range(count):                
                for j in range(count2d):    
                    for k in range(count3d):
                        DoArrayElement( i, j, k, modelFileName, lOptions )                
            #    for k in range(count3d):        
        #
    else:
        for mdl in pList:
    
            mdl.GetVector(v)
            mdl.GetVector(r, FBModelTransformationType.kModelRotation)
            mdl.GetVector(s, FBModelTransformationType.kModelScaling)
        
            for i in range(count):
            
                v2 = FBVector3d(v)
                r2 = FBVector3d(r)
                s2 = FBVector3d(s)
            
                if i != 0:
                    DoCloneModel( mdl, v, r, s )
                
                for j in range(count2d):
                    
                    v3 = FBVector3d(v2)
                    r3 = FBVector3d(r2)
                    s3 = FBVector3d(s2)
                    
                    if j != 0:
                        DoCloneModel( mdl, v2, r2, s2 )
                
                
                    for k in range(count3d):
                        
                        if k != 0:    
                            DoCloneModel( mdl, v3, r3, s3 )              
                            
                        v3[0] += arrayTransformation3[0].Value
                        v3[1] += arrayTransformation3[1].Value
                        v3[2] += arrayTransformation3[2].Value

                    v2[0] += arrayTransformation2[0].Value
                    v2[1] += arrayTransformation2[1].Value
                    v2[2] += arrayTransformation2[2].Value
                        
                v[0] += arrayTransformation[0].Value
                v[1] += arrayTransformation[1].Value
                v[2] += arrayTransformation[2].Value
            
                r[0] += arrayTransformation[3].Value
                r[1] += arrayTransformation[4].Value
                r[2] += arrayTransformation[5].Value
                        
                s[0] += arrayTransformation[6].Value
                s[1] += arrayTransformation[7].Value
                s[2] += arrayTransformation[8].Value
    #

def DoRadioSelect(control, event):
    if control == buttonSelection:
        buttonFileMerge.State = 0
    else:
        buttonSelection.State = 0

def DoRadioMode(control, event):
    
    buttonModeAll.State = (control == buttonModeAll)
    buttonModeRoot.State = (control == buttonModeRoot)
    buttonModeObject.State = (control == buttonModeObject)
    

def DoChooseFileName(control, event):
    #
    popup = FBFilePopup()
    popup.Caption = "Choose fbx file"
    popup.Filter = "*.fbx"
    
    if (popup.Execute() ):
        editFileName.Text = popup.FullFilename

def DoReset(control, event):
    Count1D.Value = 10
    Count2D.Value = 0
    Count3D.Value = 0

def PopulateTool(t):
    global arrayTransformation, arrayTransformation2, arrayTransformation3
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelTM","labelTM", x, y, w, h)

    label = FBLabel()
    label.Caption = "Array Transformation (Global, Incremental)"
    label.Style = FBTextStyle.kFBTextStyleBold
    t.SetControl( "labelTM", label );

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"labelTM")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachHeight,"", 0.35)
    t.AddRegion("gridTM","gridTM", x, y, w, h)

    grid = FBGridLayout()

    grid.SetRowRatio(0, 3.0)

    # Add buttons in the first row
    labelNames = ["", "X", "Y", "Z"]
    
    for i in range(4):
        b = FBLabel()
        b.Caption = labelNames[i]
        b.Justify = FBTextJustify.kFBTextJustifyCenter
        grid.Add(b,0,i)
        
    # add buttons in the first column
    labelNames = ["", "Move", "Rotate", "Scale"]
    for i in range(4):
        b = FBLabel()
        b.Caption = labelNames[i]
        grid.Add(b,i,0)
    
    arrayTransformation = []
    for i in range(3):
        for j in range(3):
            b = FBEditNumber()
            b.Value = 0.0
            grid.Add(b, i+1, j+1)
            
            arrayTransformation.append(b)
    
    # set the height of the row 0
    grid.SetRowHeight(0, 25)

    t.SetControl("gridTM",grid)

    ####
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"gridTM")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelDim","labelDim", x, y, w, h)

    label = FBLabel()
    label.Caption = "Array Dimentions"
    label.Style = FBTextStyle.kFBTextStyleBold
    t.SetControl( "labelDim", label );

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"labelDim")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-160,FBAttachType.kFBAttachBottom,"")
    t.AddRegion("gridDim","gridDim", x, y, w, h)

    grid = FBGridLayout()

    # Add buttons in the first row
    labelNames = ["", "Count", "", "Incremental Row", ""]
    
    for i in range(5):
        b = FBLabel()
        b.Caption = labelNames[i]
        grid.Add(b,0,i)
        
    # add buttons in the first column
    labelNames = ["1D", "2D", "3D"]
    for i in range(3):
        b = FBLabel()
        b.Caption = labelNames[i]
        grid.Add(b,i+1,0)
    
    labelNames = ["X", "Y", "Z"]
    for i in range(3):
        b = FBLabel()
        b.Caption = labelNames[i]
        b.Justify = FBTextJustify.kFBTextJustifyCenter
        grid.Add(b,1,i+2)
    
    controls = [Count1D, Count2D, Count3D]
    for i in range(3):
        b = controls[i]
        b.Value = 0
        b.Precision = 1
        b.Min = 0
        grid.Add(b, i+1, 1)
    
    arrayTransformation2 = []
    arrayTransformation3 = []
    
    for j in range(3):
        b = FBEditNumber()
        b.Value = 0.0
        grid.Add(b, 2, j+2)
        
        arrayTransformation2.append(b)

    for j in range(3):
        b = FBEditNumber()
        b.Value = 0.0
        grid.Add(b, 3, j+2)
        
        arrayTransformation3.append(b)
    
    # set the height of the row 0
    grid.SetRowHeight(0, 25)
    
    t.SetControl("gridDim",grid)


    ####
    
    x = FBAddRegionParam(15,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"gridDim")
    w = FBAddRegionParam(90,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("radioSel","radioSel", x, y, w, h)

    
    buttonSelection.Caption = "Selection"
    buttonSelection.Style = FBButtonStyle.kFBRadioButton
    buttonSelection.OnClick.Add( DoRadioSelect )
    buttonSelection.State = 1
    t.SetControl( "radioSel", buttonSelection );

    x = FBAddRegionParam(15,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"radioSel")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("radioFile","radioFile", x, y, w, h)

    buttonFileMerge.Caption = "File Merge"
    buttonFileMerge.Style = FBButtonStyle.kFBRadioButton
    buttonFileMerge.OnClick.Add( DoRadioSelect )    
    t.SetControl( "radioFile", buttonFileMerge );

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"radioFile")
    y = FBAddRegionParam(0,FBAttachType.kFBAttachTop,"radioFile")
    w = FBAddRegionParam(-40,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editFile","editFile", x, y, w, h)

    editFileName.Text = "C:\\Work\\TheWALL\\003_nohat_02.fbx"
    t.SetControl( "editFile", editFileName );
    
    x = FBAddRegionParam(2,FBAttachType.kFBAttachRight,"editFile")
    y = FBAddRegionParam(0,FBAttachType.kFBAttachTop,"radioFile")
    w = FBAddRegionParam(-2,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonFile","buttonFile", x, y, w, h)

    b = FBButton()
    b.Caption = "..."
    b.OnClick.Add( DoChooseFileName )
    t.SetControl( "buttonFile", b );

    #################################################
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachBottom,"radioFile")
    w = FBAddRegionParam(160,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelAffect","labelAffect", x, y, w, h)

    b = FBLabel()
    b.Caption = "Transformation applies to"
    b.Style = FBTextStyle.kFBTextStyleBold
    t.SetControl( "labelAffect", b );

    lastItem = ""
    labelNames = ["All", "Root", "Object"]
    controls = [buttonModeAll, buttonModeRoot, buttonModeObject]
    
    for i in range(3):

        itemName = "radio" + str(i)

        if lastItem == "":
            x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft, lastItem)
        else:
            x = FBAddRegionParam(5,FBAttachType.kFBAttachRight, lastItem)
        y = FBAddRegionParam(10,FBAttachType.kFBAttachBottom,"labelAffect")
        w = FBAddRegionParam(55,FBAttachType.kFBAttachNone,"")
        h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
        t.AddRegion(itemName,itemName, x, y, w, h)
    
        controls[i].Caption = labelNames[i]
        controls[i].Style = FBButtonStyle.kFBRadioButton
        controls[i].OnClick.Add( DoRadioMode )
        t.SetControl( itemName, controls[i] );    
        
        lastItem = itemName

    buttonModeRoot.State = 1

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,lastItem)
    y = FBAddRegionParam(10,FBAttachType.kFBAttachBottom, "labelAffect")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelObjectName","labelObjectName", x, y, w, h)

    editModelName.Text = ""
    t.SetControl( "labelObjectName", editModelName );

    ######################

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachBottom,"labelObjectName")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonOk","buttonOk", x, y, w, h)

    b = FBButton()
    b.Caption = "Ok"
    b.OnClick.Add( DoArray )
    t.SetControl( "buttonOk", b );

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonOk")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachBottom,"labelObjectName")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonCancel","buttonCancel", x, y, w, h)

    b = FBButton()
    b.Caption = "Cancel"
    t.SetControl( "buttonCancel", b );

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonCancel")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachBottom,"labelObjectName")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonReset","buttonReset", x, y, w, h)

    b = FBButton()
    b.Caption = "Reset"
    b.OnClick.Add( DoReset )
    t.SetControl( "buttonReset", b );

def CreateTool():
    t = None

    try:
        t = FBCreateUniqueTool("Array Tool " + str(ARRAY_TOOL_VERSION) )
    except NameError:
        t = CreateUniqueTool("Array Tool " + str(ARRAY_TOOL_VERSION) )
        print "supporting MoBu 2010"
        
    if t:
        t.StartSizeX = 292
        t.StartSizeY = 514
        PopulateTool(t)
        
        if gDevelopment:
            ShowTool(t)
        
CreateTool()