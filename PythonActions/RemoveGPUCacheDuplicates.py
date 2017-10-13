
'''

    Script to removing duplicated gpu cache shader instances
    and collapse gpu cache model connection to one shader 

    MoPlugs Project (c) 2017

    Sergey Solokhin (Neill3d)
    e-mail to: s@neill3d.com
      www.neill3d.com

'''

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

# remove all gpu cache shaders duplicates !

gSystem = FBSystem()

gScene = gSystem.Scene

gpushaders = []
paths = []
removelist = []

for lShader in gScene.Shaders:

    if 'ORShaderGPUCache' == lShader.ClassName():
        gpushaders.append(lShader)
        
        prop = lShader.PropertyList.Find('FileName')
        if prop:
            paths.append(prop.Data)
        else:
            paths.append('Empty')

        removelist.append(False)

# sort gpu shaders by filepath

count = len(gpushaders)
print count
for i in range(count):

    if removelist[i] == True:
        continue

    path1 = 'Empty'
    prop = gpushaders[i].PropertyList.Find('FileName')
    if prop:
        path1 = prop.Data

    print path1

    for j in range(count):
        
        if i == j:
            continue
        
        if removelist[j] == True:
            continue
        
        path2 = 'Empty2'
        prop = gpushaders[j].PropertyList.Find('FileName')
        if prop:
            path2 = prop.Data
        
        if path1 == path2:
            removelist[j] = True
            
            lShader = gpushaders[j]
            dstCount = lShader.GetDstCount()
            
            lShaderModels = []
            
            for k in range(dstCount):
                plug = lShader.GetDst(k)

                if isinstance(plug, FBModel):
                    print plug.Name
                    lShaderModels.append(plug)
                
            for lModel in lShaderModels:
                FBDisconnect(lShader, lModel)
                FBConnect(gpushaders[i], lModel)
                
            print ("remove " + path2)
            
print removelist

for i in range(count):
    
    if removelist[i] == True:
        gpushaders[i].FBDelete()

del (gpushaders, removelist, paths)
    