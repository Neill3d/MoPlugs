###############################
##
## Script to store relation boxes connections
##
## Author Sergey Solohin (Neill3d) 2014
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
################################

from pyfbsdk import *
from pyfbsdk_additions import *
import string

gDevelopment = True
gSystem = FBSystem()

def PrintAnimationNode(node):
    print node.Name
    
    for child in node.Nodes:
        PrintAnimationNode(child)

def StoreConstraints(filename):

    f = open(filename, 'w')
    try:
        
        for constraint in gSystem.Scene.Constraints:
            if constraint.ClassName() == 'FBConstraintRelation':    
                f.write(constraint.Name + '\n')
                #
                # store boxes with classname FBModelPlaceHolder
                #     and Box_PoseReader
                #
            
                for box in constraint.Boxes:
                    if box.ClassName() == 'Box_PoseReader':
                        # store only input connections
                        f.write(box.Name + '\n')
                    
                        #IN  Animation Nodes
                        lAnimationNodesIN = box.AnimationNodeInGet().Nodes
                        for lAnimationNode in lAnimationNodesIN:
                            for i in range(lAnimationNode.GetSrcCount()):
                                if lAnimationNode.GetSrc(0) != None:
                                    f.write(lAnimationNode.UserName + '\n')

                                    owner = lAnimationNode.GetSrc(0).GetOwner()
                                    if owner.ClassName() == 'FBModelPlaceHolder':
                                        f.write(owner.Model.Name + '\n')
                                    else:
                                        f.write(lAnimationNode.GetSrc(0).GetOwner().Name + '\n')                                        
                                    f.write(lAnimationNode.GetSrc(0).UserName + '\n')
                        f.write("NEXT\n")        
                f.write("STOP\n")        
    finally:
        f.close()

# Define a utility function to look up a named animation node
# under a given parent. Will not recurse in the hierarchy.
def FindAnimationNode( pParent, pName ):
    lResult = None
    for lNode in pParent.Nodes:
        if lNode.Name == pName:
            lResult = lNode
            break
    return lResult


class NodeConnector:
    """ A class to restore connection from node to node in given constraint """
    def __init__(self):
        self.constraint = 0
        # box in is a pose reader box
        self.BoxIn = 0
        self.BoxPropertyIn = 0
        # box out is a scene model (modelholder box)
        self.BoxOut = 0
        self.BoxPropertyOut = 0

    def SetUp(self, constraint, boxinName, boxpropertyinName, boxoutName, boxpropertyoutName):
        self.constraint = constraint
        
        self.BoxIn = 0
        self.BoxPropertyIn = 0
        self.BoxOut = 0
        self.BoxPropertyOut = 0
        
        #print "%s %s -> %s %s" % (boxoutName, boxpropertyoutName, boxinName, boxpropertyinName)
        
        
        
        # try to find boxin
        for box in constraint.Boxes:
            
            if box.ClassName() == 'FBModelPlaceHolder':
                
                tmGlobal = boxpropertyoutName.find('Lcl')<0
                
                if box.UseGlobalTransforms != tmGlobal: continue
                
                if boxinName == box.Model.Name:
                    self.BoxIn = box
                    self.BoxPropertyIn = FindAnimationNode(box.Box.AnimationNodeInGet(), boxpropertyinName)
                elif box.Model.Name == boxoutName:
                    self.BoxOut = box
                    self.BoxPropertyOut = FindAnimationNode(box.Model.AnimationNode, boxpropertyoutName)

                    for node in box.AnimationNodeOutGet().Nodes:
                        if node.Name.find(boxpropertyoutName) >= 0:
                            self.BoxPropertyOut = node
                            break

            else:       
                if boxinName == box.Name:
                    self.BoxIn = box
                    self.BoxPropertyIn = FindAnimationNode(box.AnimationNodeInGet(), boxpropertyinName)
                elif boxoutName == box.Name:
                    #print 'assigned %s' % boxoutName
                    self.BoxOut = box
                    self.BoxPropertyOut = FindAnimationNode(box.AnimationNodeOutGet(), boxpropertyoutName)
                  
        if self.BoxOut == 0:
            # create new relation box with such a name
            model = FBFindModelByName(boxoutName)
            if model: 
                
                if boxpropertyoutName.find('Lcl') >= 0:
                    BoxOutLocal = constraint.SetAsSource(model)
                    BoxOutLocal.UseGlobalTransforms = False
                    self.BoxOut = BoxOutLocal

                else:
                    BoxOutGlobal = constraint.SetAsSource(model)
                    BoxOutGlobal.UseGlobalTransforms = True
                    self.BoxOut = BoxOutGlobal

                for node in self.BoxOut.AnimationNodeOutGet().Nodes:
                    if node.Name.find(boxpropertyoutName) >= 0:
                        self.BoxPropertyOut = node
                        break
                        
    def Connect(self):
        # if find all the boxes, then let's connect them
        #print 'connect elements in connector'
        
        if self.BoxIn and self.BoxPropertyIn and self.BoxOut and self.BoxPropertyOut:
            FBConnect(self.BoxPropertyOut, self.BoxPropertyIn)
        else: print 'failed to connect %s %s %s %s' % (self.BoxIn, self.BoxPropertyIn, self.BoxOut, self.BoxPropertyOut)

    def Print(self):
        print 'connector info'
        if self.BoxIn: print self.BoxIn.Name
        if self.BoxOut: print self.BoxOut.Name

def RestoreConstraints(filename):

    f = open(filename, 'r')
    try:
        lines = f.readlines()

        for idx in range(len(lines)):
            lines[idx] = string.rstrip(lines[idx], '\n')
            
        idx = 0
        while (idx < len(lines)):
            line = lines[idx]
          
            for constraint in gSystem.Scene.Constraints:
                if constraint.ClassName() == 'FBConstraintRelation' and line == constraint.Name:    
                    print constraint.Name
                    #
                    # restore the constraint
                    #

                    
                    idx = idx+1
                    line = lines[idx]
                    
                    while line != 'STOP':
                        boxin = line
                        idx=idx+1                    
                        line = lines[idx]
                       
                        while line != 'NEXT' and line != 'STOP':
                            
                            connector = NodeConnector()
                            connector.SetUp(constraint, boxin, lines[idx], lines[idx+1], lines[idx+2])
                            connector.Connect()
                            
                            idx=idx+3
                            line = lines[idx]
                            


                        if line == 'STOP': break
                        idx = idx+1
                        line = lines[idx]
                        
            idx = idx + 1
    
    finally:
        f.close()
    

def InspectConstraints():
    for constraint in gSystem.Scene.Constraints:
        if constraint.ClassName() == 'FBConstraintRelation':    
    
            #
            # store boxes with classname FBModelPlaceHolder
            #     and Box_PoseReader
            #
            
            for box in constraint.Boxes:
                
                if box.ClassName() == 'FBModelPlaceHolder':
                    #print box.Model.Name
                    
                    count = box.Box.GetDstCount()
                    #print count
                    
    
                    # store only output connections
                    animnode = box.Box.AnimationNodeOutGet()
                    #PrintAnimationNode(animnode)
                    
                elif box.ClassName() == 'Box_PoseReader':
                    # store only input connections
                    print box.Name
                    
                    
                    animnode = box.AnimationNodeInGet()
                    #PrintAnimationNode(animnode)            
                
                    #IN  Animation Nodes
                    lAnimationNodesIN = box.AnimationNodeInGet().Nodes
                    for lAnimationNode in lAnimationNodesIN:
                        for i in range(lAnimationNode.GetSrcCount()):
                            if lAnimationNode.GetSrc(0) != None:
                                print "IN: %s (%s) > %s (%s) " % (box.Name, lAnimationNode.UserName, lAnimationNode.GetSrc(0).GetOwner().Name, lAnimationNode.GetSrc(0).UserName)                    
                    print
       
def TraversingRelationConstraint():
         
    lCons = FBSystem().Scene.Constraints
    #Going through the constraints in the scene
    print "********************************************"
    print "***These are the constraints in the scene***"
    print "********************************************"
    for lCon in lCons:   
        print "Constraint Name ->", lCon.Name
                
    print    
    for lCon in lCons:   
        if lCon.Is( FBConstraintRelation_TypeInfo() ):
            print "*************************************************"
            print "***These are the %d boxes in the %s Constraint***" % (len(lCon.Boxes), lCon.Name)
            print "*************************************************"
            for lBox in lCon.Boxes:
                lOut = 0
                lIn = 0
                lAnimationNodesOUT = lBox.AnimationNodeOutGet().Nodes
                for lAnimationNode in lAnimationNodesOUT:
                    for i in range(lAnimationNode.GetDstCount()):
                        if lAnimationNode.GetDst(0) != None: 
                            #Workaround for why it's printing in nodes that aren't connected
                            if not lAnimationNode.GetDst(i).GetOwner().Name.startswith("Relation"):
                                lOut = 1
                #IN  Animation Nodes
                lAnimationNodesIN = lBox.AnimationNodeInGet().Nodes
                for lAnimationNode in lAnimationNodesIN:
                    for i in range(lAnimationNode.GetSrcCount()):
                        if lAnimationNode.GetSrc(0) != None:
                            lIn = 1
                if lOut == 0 and lIn == 1:
                    print "%s is a Receiver Box" % lBox.Name
                elif lOut == 1 and lIn == 0:
                    print "%s is a Sender Box" % lBox.Name
                else:
                    print "%s is a Function (operator) Box" % lBox.Name  
            print
            print "**********************************************************************"
            print "***These are the connections between the boxes in the %s Constraint***" % lCon.Name
            print "**********************************************************************"
            for lBox in lCon.Boxes:
                print "-------%s Box connected Animation Nodes-------" % lBox.Name
                #OUT Animation Nodes
                #looks like we have an issue with Out Animation Nodes, I think it prints in nodes too :(
                lAnimationNodesOUT = lBox.AnimationNodeOutGet().Nodes
        
                for lAnimationNode in lAnimationNodesOUT:
                    for i in range(lAnimationNode.GetDstCount()):
                        if lAnimationNode.GetDst(0) != None: 
                            #Workaround for why it's printing in nodes that aren't connected
                            if not lAnimationNode.GetDst(i).GetOwner().Name.startswith("Relation"):
                                print "OUT: %s (%s) > %s (%s) " % (lBox.Name, lAnimationNode.UserName, lAnimationNode.GetDst(i).GetOwner().Name, lAnimationNode.GetDst(i).UserName)
                
                #IN  Animation Nodes
                lAnimationNodesIN = lBox.AnimationNodeInGet().Nodes
                for lAnimationNode in lAnimationNodesIN:
                    for i in range(lAnimationNode.GetSrcCount()):
                        if lAnimationNode.GetSrc(0) != None:
                            print "IN: %s (%s) > %s (%s) " % (lBox.Name, lAnimationNode.UserName, lAnimationNode.GetSrc(0).GetOwner().Name, lAnimationNode.GetSrc(0).UserName)                    
                print 
    
    
#InspectConstraints()
#StoreConstraints('c:\\test.txt')
#RestoreConstraints('c:\\test.txt')

def EventButtonStore(control, event):
    StoreConstraints('c:\\test.txt')

def EventButtonRestore(control, event):
    RestoreConstraints('c:\\test.txt')   

def EventButtonInspect(control, event):
    InspectConstraints()

def PopulateLayout(mainLyt):
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    b = FBButton()
    b.Caption = "Store"
    b.OnClick.Add(EventButtonStore)
    main.AddRegion("zero", "zero", x, y, w, h)
    main.SetControl("zero", b)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"zero")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    b = FBButton()
    b.Caption = "Restore"
    b.OnClick.Add(EventButtonRestore)
    main.AddRegion("red", "red", x, y, w, h)
    main.SetControl("red", b)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"red")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    b = FBButton()
    b.Caption = "Inspect"
    b.OnClick.Add(EventButtonInspect)
    main.AddRegion("green", "green", x, y, w, h)
    main.SetControl("green", b)
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Relation boxes store\\restore")
    PopulateLayout(t)
    t.StartSizeX = 200
    t.StartSizeY = 300
    if gDevelopment:
        ShowTool(t)


CreateTool()
