
# script to change facial heirarchy order (for stabilization)

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

import os.path
import sys

##################################### LIB CHECK CODE ###
gScriptFileName = "libReStructuring.py"

def SetupLibPath(libFilename):

    paths = FBSystem().GetPythonStartupPath()
    for path in paths:        
        if os.path.isfile(path + "\\" + libFilename):
            
            # check if that path is inside sys.path
            if not path in sys.path:
                sys.path.append(path)
    #
#
########################################################

# lib should be one of python startup folders
SetupLibPath(gScriptFileName)
import libReStructuring

reload(libReStructuring)


gRestructuring = libReStructuring.CRestructuring()

##########################################################

gOriginalDict = {
    "HEAD_RIG_ROOT" : "Lameece_Head",
    "jointJaw" : "HEAD_RIG_ROOT",
    "JAW_MIDDLE" : "jointJaw",
    "jointMouthInner" : "HEAD_RIG_ROOT",
    "jointFaceGroup" : "HEAD_RIG_ROOT",
    "BROW_INNER_LEFT" : "jointFaceGroup",
    "BROW_INNER_LEFT_BROW_MIDDLE_LEFT_0_5" : "jointFaceGroup",
    "BROW_MIDDLE_LEFT" : "jointFaceGroup",
    "EYE_CORNER_INNER_LEFT" : "jointFaceGroup",
    "EYE_LACRIME_LEFT" : "EYE_CORNER_INNER_LEFT",
    "EYE_TOP_LEFT" : "jointFaceGroup",
    "EYE_CORNER_OUTER_LEFT" : "jointFaceGroup",
    "EYE_BOTTOM_LEFT" : "jointFaceGroup",
    "EYE_CORNER_INNER_RIGHT" : "jointFaceGroup",
    "EYE_LACRIME_RIGHT" : "EYE_CORNER_INNER_RIGHT",
    "EYE_TOP_RIGHT" : "jointFaceGroup",
    "EYE_CORNER_OUTER_RIGHT" : "jointFaceGroup",
    "EYE_BOTTOM_RIGHT" : "jointFaceGroup",
    "L_upperCheeks_1_jnt" : "jointFaceGroup",
    "L_upperCheeks_2_jnt" : "jointFaceGroup",
    "L_upperCheeks_3_jnt" : "jointFaceGroup",
    "R_upperCheeks_1_jnt" : "jointFaceGroup",
    "R_upperCheeks_2_jnt" : "jointFaceGroup",
    "R_upperCheeks_3_jnt" : "jointFaceGroup",
    "L_cheeks_1_jnt" : "jointFaceGroup",
    "L_cheeks_2_jnt" : "jointFaceGroup",
    "L_cheeks_3_jnt" : "jointFaceGroup",
    "L_cheeks_4_jnt" : "jointFaceGroup",
    "R_cheeks_4_jnt" : "jointFaceGroup",
    "R_cheeks_3_jnt" : "jointFaceGroup",
    "R_cheeks_2_jnt" : "jointFaceGroup",
    "R_cheeks_1_jnt" : "jointFaceGroup",
    "NOSTRIL_OUTER_LEFT" : "jointFaceGroup",
    "NOSE_LOWER_MIDDLE" : "jointFaceGroup",
    "NOSTRIL_OUTER_RIGHT" : "jointFaceGroup",
    "L_crease_3_jnt" : "jointFaceGroup",
    "L_crease_2_jnt" : "jointFaceGroup",
    "L_crease_1_jnt" : "jointFaceGroup",
    "R_crease_1_jnt" : "jointFaceGroup",
    "R_crease_2_jnt" : "jointFaceGroup",
    "R_crease_3_jnt" : "jointFaceGroup",
    "BROW_INNER_RIGHT" : "jointFaceGroup",
    "BROW_INNER_RIGHT_BROW_MIDDLE_RIGHT_0_5" : "jointFaceGroup",
    "BROW_MIDDLE_RIGHT" : "jointFaceGroup",
    "jointMouthGroup" : "HEAD_RIG_ROOT",
    "MOUTH_LIP_INNER_TOP_MIDDLE" : "jointMouthGroup",
    "MOUTH_LIP_INNER_CORNER_LEFT_MOUTH_LIP_INNER_TOP_MIDDLE_0_66667" : "jointMouthGroup",
    "MOUTH_LIP_INNER_CORNER_LEFT_MOUTH_LIP_INNER_TOP_MIDDLE_0_33333" : "jointMouthGroup",
    "MOUTH_LIP_INNER_CORNER_LEFT" : "jointMouthGroup",
    "MOUTH_LIP_INNER_BOTTOM_MIDDLE_MOUTH_LIP_INNER_CORNER_LEFT_0_66667" : "jointMouthGroup",
    "MOUTH_LIP_INNER_BOTTOM_MIDDLE_MOUTH_LIP_INNER_CORNER_LEFT_0_33333" : "jointMouthGroup",
    "MOUTH_LIP_INNER_BOTTOM_MIDDLE" : "jointMouthGroup",
    "MOUTH_LIP_INNER_CORNER_RIGHT_MOUTH_LIP_INNER_BOTTOM_MIDDLE_0_66667" : "jointMouthGroup",
    "MOUTH_LIP_INNER_CORNER_RIGHT_MOUTH_LIP_INNER_BOTTOM_MIDDLE_0_33333" : "jointMouthGroup",
    "MOUTH_LIP_INNER_CORNER_RIGHT" : "jointMouthGroup",
    "MOUTH_LIP_INNER_TOP_MIDDLE_MOUTH_LIP_INNER_CORNER_RIGHT_0_66667" : "jointMouthGroup",
    "MOUTH_LIP_INNER_TOP_MIDDLE_MOUTH_LIP_INNER_CORNER_RIGHT_0_33333" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_VEE_RIGHT" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_TOP_MIDDLE" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_VEE_LEFT" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_CORNER_LEFT_MOUTH_LIP_OUTER_VEE_LEFT_0_66667" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_CORNER_LEFT_MOUTH_LIP_OUTER_VEE_LEFT_0_33333" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_CORNER_LEFT" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_BOTTOM_MIDDLE_MOUTH_LIP_OUTER_CORNER_LEFT_0_66667" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_BOTTOM_MIDDLE_MOUTH_LIP_OUTER_CORNER_LEFT_0_33333" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_BOTTOM_MIDDLE" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_CORNER_RIGHT_MOUTH_LIP_OUTER_BOTTOM_MIDDLE_0_66667" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_CORNER_RIGHT_MOUTH_LIP_OUTER_BOTTOM_MIDDLE_0_33333" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_CORNER_RIGHT" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_VEE_RIGHT_MOUTH_LIP_OUTER_CORNER_RIGHT_0_66667" : "jointMouthGroup",
    "MOUTH_LIP_OUTER_VEE_RIGHT_MOUTH_LIP_OUTER_CORNER_RIGHT_0_33333" : "jointMouthGroup",
    "MOUTH_LIP_INNER_STICKY_LEFT_UP" : "jointMouthGroup",
    "MOUTH_LIP_INNER_STICKY_LEFT_DOWN" : "jointMouthGroup",
    "MOUTH_LIP_INNER_STICKY_RIGHT_DOWN" : "jointMouthGroup",
    "MOUTH_LIP_INNER_STICKY_RIGHT_UP" : "jointMouthGroup",
    "groupGlasses" : "HEAD_RIG_ROOT",
    "jointGlasses" : "groupGlasses",
    "groupEyes_DONT_MOVE" : "HEAD_RIG_ROOT",
    "groupEyes" : "groupEyes_DONT_MOVE",
    "jointRightEye" : "groupEyes",
    "jointLeftEye" : "groupEyes",
    "jointMouthPlane" : "HEAD_RIG_ROOT"
    }


def CollectModels(modelList, model):
    
    modelList.append(model)
    
    for child in model.Children:
        CollectModels(modelList, child)

def BackToOriginal():
    print "back to original"
    
    # HEAD_RIG_ROOT
    modelList = FBModelList()    

    rigRoot = FBFindModelByLabelName( "HEAD_RIG_ROOT" )
    if rigRoot:
        CollectModels(modelList, rigRoot)
    
    if len(modelList) > 0:
        gRestructuring.Hold(modelList)
    
    # look at the group "facial joints"
    
    for (modelName, parentName) in gOriginalDict.items():
        model = FBFindModelByLabelName(modelName)
        parent = FBFindModelByLabelName(parentName)
        
        if model and parent:
            model.Parent = parent
    
    if len(modelList) > 0:
        gRestructuring.Fetch()
    
def printModelHierarchy(model):
    
    parentName = "None"
    if model.Parent:
        parentName = model.Parent.Name
        
    print ( '\"' + model.Name + '\" : ' + '\"' + model.Parent.Name + '\"' )
    
    for child in model.Children:
        printModelHierarchy(child)

def printHierarchy():
    
    lscene = FBSystem().Scene
    
    for lcomp in lscene.Components:
        if lcomp.Name == "HEAD_RIG_ROOT":
            printModelHierarchy(lcomp)

#
#
BackToOriginal()
