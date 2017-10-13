
##############################################################
#
# script uses local X axis to compute element Length and snapshot it along a curve
# 21.02.2016
# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com
#       www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
############################################################

from pyfbsdk import *
import math
import os.path
import sys

##################################### LIB CHECK CODE ###
gScriptFileName = "libDuplicateAlongACurve.py"

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
import libDuplicateAlongACurve

reload(libDuplicateAlongACurve)

gDialogTitle = "Duplicate Along A Curve"

def Main():

    duplicateAxis = 3      # +Z by default
    duplicateType = 1      # instance or clone ?
    duplicateAlign = 1     # up vector or follow a path
    
    lLib = libDuplicateAlongACurve.CDublicateAlongACurve()

    curve, brick = lLib.GetSelectedModels()
    if curve == None or brick == None:
        FBMessageBox( gDialogTitle, "You should select a path model and a geometry model", "Ok" )
        return 0
        
    #
    duplicateType = FBMessageBox(gDialogTitle, "Please choose a type of duplication?", "Instance", "Copy", "Cancel" )
    
    if duplicateType < 3:
        duplicateAlign = FBMessageBox(gDialogTitle, "Please choose an aling type", "Up Vector", "Follow A Path" )
    else:
        return 0

    setupConstraint = FBMessageBox( gDialogTitle, "Do you want to setup a constraint?", "Wall Bricks", "Path Wrap", "No" )

    #
    # run a script function
    lLib(curve, brick, False, 90.0, duplicateAxis, duplicateType, duplicateAlign, setupConstraint )

    return 1
    
#

Main()    
    



        
    