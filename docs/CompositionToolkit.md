[Back To Main Page](README.md)

[Back To Plugins Page](Plugins.md)

[Back To Scripts Page](Scripts.md)

# Composite Toolkit for Autodesk MotionBuilder

Tool for real-time compositing

Each composition has
* ability to use as a dynamic texture to assign to an object material or to another composition
* output to your viewer window or composite your render image

Composition tree is built with nodes.

Nodes divided into 4 categories:
* Final node - root of the composition, resulting image
* Source of the image data - Render node (image comes from the viewer), Solid Color node (image comes from solid color or selected scene texture)
* Filtering or processing nodes - Blur filter node, Constrast Saturation Brightness node, Halftone node, etc.
* Child nodes blending - Blend node

Items are adding to the composition tree by drag’n’drop operation from assets. Processing of nodes goes in order from last child (which is data source) up to the final node (composition final node)
* Source nodes are always children for other types of nodes and source nodes cound’t have attached children nodes.
* Filter nodes can have only one chilren - one node to process
* Blending nodes as usual have 2 input nodes to blend them together.
* Final node receives one child node and use it as a composition result.

## Composite Master Tool UI

** This is an old and deprecated user guide.** 

User interface top toolbar
“+” - add a new composition into the scene
“-” remove current composition from the scene
“R” - rename current composition
“D” - dublicate current composition
Setup selected - setup batch processing for materials of the objects selection
Remove setup - remove batch processing for materials of the objects selection

### Composition Nodes

#### Dynamic Composite

This is a final node of a composition and it uses it’s child as a composition result.

Type - final node

Posible number of children - 1

Properties:
* Active - enable or disable a composition processing
* Render Output - use this composition as a result in the viewer window. Only one composition in scene can use that flag at the same time.
* Render Auto Size - if composition is used as render output, it will automaticaly assign composition width and height from the camera parameters
* Width - width of the composition. If you want to composition your viewer image, you should specify a width of a scene camera
* Height - height of a composition. It has the same tip as for the width
* Video - result of a composition stored as a scene video element and dynamic texture. You can assign it to another composition or to the object materials channel
* Batch processing - state of a processing mode
* Batch auto size - flag for auto changing composition dimentions based on the input source processing image
* Batch input - input processing textures (these textures will replace one by one source texture property in Solid Color node with replacable flag on)
* Batch output - output textures after batch processing

#### Render node

This node take image data from the viewer window or renderer.

Type - source node

Posible number of children - 0


Properties:
* Depth output - flag switch between image source. Should it be RGBA color or 3d scene Depth

#### Solid color node

This node takes image from solid color or scene texture.

Type - source node

Posible number of children - 0

Properties:
* Color RGB - source image color
* Use gradient - fill image with vertical gradient
* Color upper - gradient upper color
* Color lower - gradient lower color
* Use texture - use texture as an image source
* Texture - reference to the scene texture
* Repleceable - flag for batch processing

#### Blend Node

Blend two children nodes together.

Type - blend node

Posible number of children - 2

Properties:
* Mode - blending mode (simular to photoshop layer blending option)
* Opacity - opacity of the second child in blending
* Mask - specify dynamic mask texture
* Use mask - flag to inform if blend node should use a mask for processing
* Use last mask - auto assign last created dynamic mask (from MoPlugs plugin Dynamic Masking Tool)

#### HalfTone node

this is a filter of a HalfTone effect.

Type - filter node

Posible number of children - 1

Properties:
* Frequency - filter pattern frequency

#### Contrast Staturation Brightness node

This is a color correction filter.

Type - filter node

Posible number of children - 1

Properties:
* Contrast - control a contrast
* Saturation - control a saturation
* Brighness - control a brightness
* Gamma - control a gamma level

#### Blur node

This is a blur filter.

Type - filter node

Posible number of children - 1

Properties:
* Blur Horz - level of a horizontal blur
* Blur Vert -  level of a vertical blur
