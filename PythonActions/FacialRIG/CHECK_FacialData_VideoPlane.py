
######################################################################
#
# batch check
# got into all sub-folder and find which ends with "FACIAL_DATA"
# open each fbx in that folder and check if VideoPlane is there !
#
# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com
#       www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
######################################################################

import os
import os.path

from pyfbsdk import *

dialog = FBFolderPopup()
paths = []
files = []

gApp = FBApplication()

def AppendFolders(folder):
    
    for o in os.listdir(folder):
        if os.path.isdir(os.path.join(folder,o)):
            newdir = os.path.join(folder,o)
            paths.append(newdir)
            
            AppendFolders(newdir)

if dialog.Execute():

    d=dialog.Path
    paths = []
    files = []
    
    AppendFolders(d)

    for path in paths:
        if path.find("_FACIAL_DATA") >= 0 and path.find(".bck") == -1:
            print path
            
            for o in os.listdir(path):
                lowero = o.lower()
                if os.path.isfile(os.path.join(path,o)) and lowero.find(".fbx") >= 0:
                    files.append(os.path.join(path,o))
        
    # open each file and check
    
    for f in files:
        print f
        
        gApp.FileNew()
        gApp.FileOpen(f)
        
        mdl = FBFindModelByLabelName( "VideoPlane" )
        if mdl != None:
            print "> FILE IS OK !"
        else:
            print "> ERROR: VideoPlane doesn't exist!!!"

            mdl = FBFindModelByLabelName( "Video Plane" )
            if mdl == None:
                mdl = FBFindModelByLabelName( "Video plane" )
                
                if mdl == None:
                    mdl = FBFindModelByLabelName( "Videoplane" )

            if mdl != None:
                print "> FIX video plane name !!!"
                mdl.Name = "VideoPlane"
                
                gApp.FileSave(f)
        
    gApp.FileNew()      