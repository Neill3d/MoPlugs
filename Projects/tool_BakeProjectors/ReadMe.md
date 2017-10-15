

# Bake Projectors Plugin for Autodesk MotionBuilder #

Dependencies - nvFX, shader - ProjectiveBaking.glslfx

libraries - mobu 2014 x64
 library_MoCode
 sg_base
 sg_shared

Output filename rule - <user filename> + <model file name in Save per model model> + <frame number in add frame number or bake range> + .ext <jpg or tif>

---

Sergey Solokhin (c) 2015

TODO:

+ 1) ui for changing resolution, for bake image sequence
+ 2) render using framebuffer to needed resolution, pan, zoom in the view


LOG

11.01.15
 + save to jpeg
 + isolate models function

27.11.15
 UI improvments, add ability to enter individual resolution of each model in the list

14.10.15
 added feature for render only projected images without diffuse

07.10.15
 time range baking

05.10.15
 add ui with options and buttons for frame and time range grabbing

02.10.15
 first working version with rendering model into uv layout space
 and working version with baked projectors that I can save into the image
