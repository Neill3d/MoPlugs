
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

gsystem = FBSystem()
gplayer = FBPlayerControl()

modellist = FBModelList()

FBGetSelectedModels(modellist)

if len(modellist) > 0:
    
    model = modellist[0]
    
    starttime = gplayer.ZoomWindowStart
    stoptime = gplayer.ZoomWindowStop
    
    curtime = starttime
    
    f = open("D:\\positions.txt", "w")
    
    numberOfFrames = stoptime.GetFrame() - starttime.GetFrame()
    f.write( str(numberOfFrames) + "\n" )

    v = FBVector3d()
    
    while (curtime <= stoptime):
        
        gplayer.Goto(curtime)
        gsystem.Scene.Evaluate()
        
        model.GetVector(v, FBModelTransformationType.kModelTranslation, False)
        
        f.write( str(v[0]) + " " + str(v[1]) + "\n" )
        
        curtime = curtime + FBTime(0,0,0,1)
    

    f.close()

    
    
    