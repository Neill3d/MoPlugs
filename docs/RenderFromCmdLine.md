[Back To Main Page](README.md)

[Back To Plugins Page](Plugins.md)

# Rendering from command line

First of all you need to enter that block
```
--sdk-begin <put custom cmds here> --sdk-end
```
Inside that block you can enter a list of arguments from the following:
```
--render <arg>,
        where arg can be
            layers - render according to the render layers plugin setup
            offline - render in offline mode
            window - render in online mode
--close        close mobu after render operation is finished
```
next arguments are only for window and offline mode
```
-- camera <arg>,
        where arg is a camera name in the scene
-- antialiasing <arg>,
        where arg can be
            true - enable antialiasing (sampling)
            false - disable

-- startTime <arg>
-- stopTime <arg>
-- stepTime <arg>
-- pictureFormat <arg>
-- showTimeCode <arg>
-- showSafeArea <arg>
-- showCameraLabel <arg>

-- codec <arg>,
        where arg
            uncompressed - save without any compression
            asked - ask user to choose codec
            stored - use scene saved user codec
-- outputFormat <arg>            
-- outputFile <arg>,
        where arg is a render output path and file
```

## Examples

Several examples of command lines for running a rendering in MotionBuilder

1) Running a render layers rendering query
```
start /d "C:\Program Files\Autodesk\MotionBuilder 2014\bin\x64\" motionbuilder.exe -console -suspendMessages --sdk-begin --render layers --close --sdk-end X:\_MB_UNIT_TEST_SCENES\Reflections.fbx
```
2) Running a simple window rendering
```
start /d "C:\Program Files\Autodesk\MotionBuilder 2014\bin\x64\" motionbuilder.exe -console -suspendMessages --sdk-begin --render window --close --sdk-end X:\_MB_UNIT_TEST_SCENES\Reflections.fbx
```
