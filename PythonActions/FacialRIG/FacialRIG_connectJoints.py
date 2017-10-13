
# create point constraints between objects

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

modelList = FBModelList()
FBGetSelectedModels(modelList)

for dstmodel in modelList:
    
    dstname = dstmodel.Name
    corrname = dstname.replace("_MOUTH", "-MOUTH")
    corrname = corrname.replace("_0_", "-0.")
    corrname = corrname.replace("33334", "66667")
    #print corrname
    
    srcmodel = FBFindModelByLabelName( corrname + "_dst" )
    if srcmodel:
    
        conn = FBConstraintManager().TypeCreateConstraint("Position")
        if conn:
            conn.ReferenceAdd(0, dstmodel)
            conn.ReferenceAdd(1, srcmodel)
            conn.Snap()
            
    else:
        print ("missing - " + dstmodel.Name)