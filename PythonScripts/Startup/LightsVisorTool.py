
'''
    Tool for overview all the existing lighting lists
    Lights could be assigned from the scene, from the composition, from the shader

    DONE: Check if we are connecting a light that is already in the connection list
    DONE: add some UI refresh possibility on scene changes (even manual)
'''

#
# Author Sergey Solohin (Neill3d) 2017
#  e-mail to: s@neill3d.com
#   www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *
from pyfbsdk_additions import *

gDevelopment = True

gSystem = FBSystem()
gApp = FBApplication()

gSceneLights = FBList()
gSceneLists = FBList()
gListLights = FBList()

REFRESH_ALL = 0
REFRESH_LIGHTS = 1
REFRESH_LISTS = 2
REFRESH_LISTLIGHTS = 3 

def FindListLightsProperty(comp):
    
    if comp != None:
        
        prop = comp.PropertyList.Find('AffectingLights')
        if prop != None:
            return prop
        else:
            prop = comp.PropertyList.Find('Lights')
            return prop
            
    return None


def IsAlreadyConnected(prop, light):
    
    count = prop.GetSrcCount()
    for i in range(count):
        if prop.GetSrc(i) == light:
            return True
    return False

def RefreshListLights(comp=None):
    
    if comp == None:
    
        itemIndex = gSceneLists.ItemIndex
        
        if itemIndex >= 0:
            key = gSceneLists.Items[itemIndex]
            
            for comp in gSystem.Scene.UserObjects:
                if comp.LongName == key:
                    RefreshListLights(comp)
                    break
                    
            for shader in gSystem.Scene.Shaders:
                if shader.LongName == key:
                    RefreshListLights(shader)
                    break
                    
    else:
        gListLights.Items.removeAll()
        
        prop = FindListLightsProperty(comp)
        if prop != None:
            
            count = prop.GetSrcCount()
            for i in range(count):
                plug = prop.GetSrc(i)

                if isinstance(plug, FBLight):
                    gListLights.Items.append(plug.Name)
                    
def RefreshUI(refreshWhat):
   
    # refresh scene lights
    if refreshWhat in [REFRESH_ALL, REFRESH_LIGHTS]:
        gSceneLights.Items.removeAll()
        
        for light in gSystem.Scene.Lights:
            
            isEnabled = light.CastLightOnObject
            
            if isEnabled:
                gSceneLights.Items.append(light.LongName)
            else:
                gSceneLights.Items.append(light.LongName + ' [disabled]')
    
    # refresh light lists
    if refreshWhat in [REFRESH_ALL, REFRESH_LISTS]:
        gSceneLists.Items.removeAll()
        gListLights.Items.removeAll()
        
        gSceneLists.Items.append( '== Compositions ==' )
        
        for comp in gSystem.Scene.UserObjects:
            
            prop = comp.PropertyList.Find( 'Lights' )
            if prop!=None:
                gSceneLists.Items.append(comp.LongName)
        
        gSceneLists.Items.append( '== Shaders ==' )
        
        for shader in gSystem.Scene.Shaders:
            
            prop = shader.PropertyList.Find('AffectingLights')
            if prop != None:
                gSceneLists.Items.append(shader.LongName)
              

    # refresh list lights
    if refreshWhat in [REFRESH_ALL, REFRESH_LISTLIGHTS]:
        gListLights.Items.removeAll()
        RefreshListLights()
        

def FindCurrentSceneLight():

    lightIndex = gSceneLights.ItemIndex
    
    if lightIndex >= 0:
        lightName = gSceneLights.Items[lightIndex]
        
        if lightName.find(' [disabled]') > 0:
            lightName = lightName[:-11];
        
        for light in gSystem.Scene.Lights:
            if light.LongName == lightName:
                return light
    return None

def FindCurrentList():
    
    listIndex = gSceneLists.ItemIndex
    
    if listIndex >= 0:
        listName = gSceneLists.Items[listIndex]
        
        for comp in gSystem.Scene.UserObjects:
            if comp.LongName == listName:
                return comp
        
        for shader in gSystem.Scene.Shaders:
            if shader.LongName == listName:
                return shader
    return None

def FindCurrentListLight():

    lightIndex = gListLights.ItemIndex
    
    if lightIndex >= 0:
        lightName = gListLights.Items[lightIndex]
        
        for light in gSystem.Scene.Lights:
            
            if light.LongName == lightName:
                return light
    return None

############################################## SCENE LIGTHS EVENTS


def EventButtonLightSelectClick(control, event):

    light = FindCurrentSceneLight()
    if light != None:
        light.Selected = True            
    
def EventButtonLightEnableClick(control, event):

    light = FindCurrentSceneLight()
    if light != None:
        light.CastLightOnObject = True
        
        RefreshUI(REFRESH_LIGHTS)
    
def EventButtonLightDisableClick(control, event):

    light = FindCurrentSceneLight()
    if light != None:
        light.CastLightOnObject = False
        
        RefreshUI(REFRESH_LIGHTS)
    
def EventButtonLightConnectClick(control, event):
    
    light = FindCurrentSceneLight()
    lightlist = FindCurrentList()

    if light != None and lightlist != None:
        
        prop = FindListLightsProperty(lightlist)
        if prop != None:
            
            if False == IsAlreadyConnected(prop, light):
                FBConnect(light, prop)              
                RefreshUI(REFRESH_LISTLIGHTS)
    
def EventButtonLightDisconnectClick(control, event):

    light = FindCurrentSceneLight()
    lightlist = FindCurrentList()
    
    if light != None and lightlist != None:
        
        prop = FindListLightsProperty(lightlist)
        if prop != None:
            FBDisconnect(light, prop)
                
            RefreshUI(REFRESH_LISTLIGHTS)

############################################## LIGHT LISTS EVENTS

def EventListItemChange(control, event):
    RefreshListLights()

def EventButtonSelectListClick(control, event):
    
    lightlist = FindCurrentList()

    if lightlist != None:
        lightlist.Selected = True

def EventButtonFocusListClick(control, event):

    lightlist = FindCurrentList()

    if lightlist != None:
        lightlist.HardSelect()

def EventButtonDisconnectListClick(control, event):
    
    lightlist = FindCurrentList()

    if lightlist != None:
        prop = FindListLightsProperty(lightlist)
        if prop != None:
            
            prop.DisconnectAllSrc()
    
        #
        RefreshUI(REFRESH_LISTS)

############################################## EVENT LIST LIGHTS

def EventButtonListLightDisconnectClick(control, event):
    print 'disconnect'
    
    light = FindCurrentListLight()
    lightlist = FindCurrentList()
    
    if light != None and lightlist != None:
        
        prop = FindListLightsProperty(lightlist)
        if prop != None:
            FBDisconnect(light, prop)
                
            RefreshUI(REFRESH_LISTLIGHTS)

################################################ MAIN UI

def EventShowTool(control, event):
    
    RefreshUI(REFRESH_ALL)

def PopulateLayout(mainLyt):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBHBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    # TODO: add more controls
    
    lyt = FBVBoxLayout()
    
    b = FBLabel()
    b.Caption = 'All Lights'
    
    lyt.Add(b, 25)
    
    toolbar = FBHBoxLayout()
    
    b = FBButton()
    b.Caption = 'S'
    b.Hint = 'Select lights from the list below'
    b.OnClick.Add( EventButtonLightSelectClick )
    toolbar.Add(b, 25)
    
    b = FBButton()
    b.Caption = 'E'
    b.Hint = 'Enable light casting for the selected lights from the list below'
    b.OnClick.Add( EventButtonLightEnableClick )
    toolbar.Add(b, 25)
    
    b = FBButton()
    b.Caption = 'D'
    b.Hint = 'Disable light casting for the selected lights'
    b.OnClick.Add( EventButtonLightDisableClick )
    toolbar.Add(b, 25)
    
    b = FBButton()
    b.Caption = '->'
    b.Hint = 'Connect selected lights to the selected light group'
    b.OnClick.Add( EventButtonLightConnectClick )
    toolbar.Add(b, 30)
    
    b = FBButton()
    b.Caption = 'X'
    b.Hint = 'Disconnect selected lights from the selected light group'
    b.OnClick.Add( EventButtonLightDisconnectClick )
    toolbar.Add(b, 25)
    
    lyt.Add(toolbar, 30)
    
    gSceneLights.Style = FBListStyle.kFBVerticalList
    gSceneLights.Items.SetString( 'Light 1~Light 2 [Dont cast lighting]' )
    lyt.AddRelative(gSceneLights, 0.9)
     
    main.AddRelative(lyt, 0.33)
    
    # Light lists
    
    lyt = FBVBoxLayout()
    
    b = FBLabel()
    b.Caption = 'Light Lists'
    
    lyt.Add(b, 25)
    
    toolbar = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = ''
    toolbar.Add(b, 40)
    
    b = FBButton()
    b.Caption = 'S'
    b.Hint = 'Select the highlighted light list in the scene'
    b.OnClick.Add( EventButtonSelectListClick )
    toolbar.Add(b, 25)
    
    b = FBButton()
    b.Caption = 'F'
    b.Hint = 'Select and Focus the selected light list in the Navigator Window'
    b.OnClick.Add( EventButtonFocusListClick )
    toolbar.Add(b, 25)
    
    b = FBButton()
    b.Caption = 'D'
    b.Hint = 'Disconnect all lights from the selected light list'
    b.OnClick.Add( EventButtonDisconnectListClick )
    toolbar.Add(b, 25)
    
    lyt.Add(toolbar, 30)
    
    gSceneLists.Style = FBListStyle.kFBVerticalList
    gSceneLists.Items.SetString( 'Composition 1~ProjTex Shader 1' )
    gSceneLists.OnChange.Add( EventListItemChange )
    lyt.AddRelative(gSceneLists, 0.9)
    
    main.AddRelative(lyt, 0.33)
    
    # Lights inside the list
    
    lyt = FBVBoxLayout()
    
    b = FBLabel()
    b.Caption = 'Lights inside the list'
    lyt.Add(b, 25)
    
    toolbar = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = ''
    toolbar.Add(b, 40)
    
    b = FBButton()
    b.Caption = '<-'
    b.Hint = 'Disconnect selected light from the selected list'
    b.OnClick.Add( EventButtonListLightDisconnectClick )
    toolbar.Add(b, 25)
    
    lyt.Add(toolbar, 30)
    
    gListLights.Style = FBListStyle.kFBVerticalList
    gListLights.Items.SetString( 'Light 1' )
    lyt.AddRelative(gListLights, 0.9)    
    
    main.AddRelative(lyt, 0.33)
    
    RefreshUI(REFRESH_ALL)
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Lights Visor Tool [Experimental]")
    PopulateLayout(t)
    t.StartSizeX = 500
    t.StartSizeY = 350
    t.OnShow.Add( EventShowTool )

    if gDevelopment:
        ShowTool(t)

CreateTool()
