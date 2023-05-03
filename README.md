# EcoSim

![Houdini Render](https://github.com/gserena01/EcoSim/blob/main/Render1.png)

Demo
--------
[![Beta Demo](https://img.youtube.com/vi/9EOKsgRmGsI/0.jpg)](https://www.youtube.com/watch?v=9EOKsgRmGsI)

Uploading your own files:
--------
This plug-in allows you to input your own geometry and texture files. For textures, please upload an 32x32 pixel image file (yes, it's tiny!). For geometry files, any .fbx, .obj, or other standard geometry file will work, though we suggest using lower poly geometry to keep the plugin running quickly, since the input geometry will be replicated many times.

Exporting Geometry:
--------
![image](https://user-images.githubusercontent.com/60444726/235263101-1394fb21-1b47-4eff-9c1c-fa666683b155.png)

To export the current geometry, click on the "export" file node, opt to display/render it, and click reload geometry. By default, the geometry will be exported to "Exported_Geometry.obj".

Features Guide 
---------------------------
Our Houdini plug-in tool is meant to accurately simulate how water and vegetation interact together to produce an ecosystem. There are many potential uses for this tool, including an accurate demonstration of the impact of climate change in order to bring about an increased understanding of global environmental issues. This plug-in also allows users to visualize local ecoclimate phenomena. We expect artists and game designers to use this tool to model a natural environment. The changing environment created by this tool creates an immersive experience, so the artist can show how their environment will change as time progresses. Furthermore, this tool could also be used by educators or scientists to visualize and increase awareness of the effects of climate change to wide audiences. 

When you first load in the plug-in, a starting environment is automatically generated. The user can change the settings of the environment using the GUI that will appear, or by manually altering the node structure if they are more familiar with Houdini. The main feature of this tool is the "Years'' slider, which shows how the climate will change based on accurate simulation of the water cycle. As the slider progresses, the user will see trees growing, maturing, spawning new seeds, and those seeds growing as well. 

The parameters of the environment are changeable by the user to produce different environmental effects. First, all the "Geometry Files" are up to the user. The user can add their own trees, clouds, or objs to the scene which will grow and change between the set files. At start-up, the terrain is set with a noise-based generated image. However, this image of the correct (32x32) proportions can be inputted by the user to create terrain of any shape. The scale, slope, and height of this input terrain can be altered using the node in Houdini. Furthermore, all the geometry files for each growth stage of the trees and the clouds are changeable by the user. They can upload whatever geometry they desire, however, we recommend low-poly meshes to increase the speed of the simulation. These features allow for a fully customizable visual output fir the artist. 

Under the Geometry section, we see the controls. Here, the user will see the "Years'' slider, which propagates the simulated environment over time, as described earlier. Then they see a "Climate Dryness" input. Mathematically, this constant represents the "evaporation constant" in the back-end of the simulation, controlling how much water units will be evaporated each year based on the climate. This parameter alone can be used to vary the way the environment will behave over time. The GUI begins with the default climate wetness of 1.5, which represents the climate of a deciduous forest. However, if the user changes this value, they will see drastic effects. Bringing it down to 0.0 shows the effects of a rainforest climate, meaning none of the water is evaporated out of the soil. Therefore, this will create an extremely wet and prolific climate, where the vegetation multiplies. On the other hand, increasing the drying to its maximum value creates a desert-like climate where almost all the water is evaporated out of the soil. Here, all the trees will dry out and die over time. 

Finally, you will see the "Starter Tree Placement" setters. Upon start-up, the starting tree positions are set automatically based on a noise function. However, the user can also precisely choose the location of their trees. The terrain is set up like a grid, counting cells from 0 to 1024. The user can add numbers to the input to describe where they want the trees of the corresponding age to be. The input strings that the system can handle include single numbers ("200"), listed numbers ("1, 4, 10, 50"), numerical ranges ("100-200"), or a combination of all the above ("1, 2, 200-300, 4, 521-634"). To return to the noise-based trees, the user must simply delete their input from all four text boxes. 
