# EcoSim

![Houdini Render](https://github.com/gserena01/EcoSim/blob/main/Render1.png)

Demo
--------
[![Beta Demo](https://img.youtube.com/vi/FLVmGrclKW4/0.jpg)](https://www.youtube.com/watch?v=FLVmGrclKW4)

Uploading your own files:
--------
This plug-in allows you to input your own geometry and texture files. For textures, please upload an 32x32 pixel image file (yes, it's tiny!). For geometry files, any .fbx, .obj, or other standard geometry file will work, though we suggest using lower poly geometry to keep the plugin running quickly, since the input geometry will be replicated many times.

Exporting Geometry:
--------
![image](https://user-images.githubusercontent.com/60444726/235263101-1394fb21-1b47-4eff-9c1c-fa666683b155.png)

To export the current geometry, click on the "export" file node, opt to display/render it, and click reload geometry. By default, the geometry will be exported to "Exported_Geometry.obj".

Features Guide 
---------------------------
Our Houdini plug-in tool is meant to accurately simulate how water and vegetation interact together to produce an ecosystem. There are many potential uses for this tool, including an accurate demonstration of the impact of climate change in order to bring about an increased understanding of global environmental issues. This plug-in  also allows users to visualize local ecoclimate phenomena. We expect artists and game designers to use this tool to model a natural environment. The changing environment creates an immersive experience, so the artist or game designer can show how their environment will change as time progresses in a game or animation. Furthermore, this tool could also be used by educators or scientists to visualize and increase awareness of the effects of climate change to wide audiences. 

When you first load in the plug-in, a starting environment is automatically generated. The user can change the settings of the environment using the GUI that will appear, or by manurally altering the node structure if they are more familiar with Houdini. The main feature of this tool is the "Years" slider, which shows how the climate will change based on accurate simulation of the water cycle. As the slider progresses, the user will see trees growing, maturing, spawning new seeds, and those seeds growing as well. 

The parameters of the enviornment are changable by the user to produce different environmental effect. First, all the "Geometery Files" are up to the user. The user can add their own trees, clouds, or objs to the scene which will grow and change between the set files. At start-up, the terrain is set with a noise-based generated image. However, this image of the correct (32x32) proportions can be inputted by the user to create terrain of any shape. 

Under the Geometery section, we see the controls. Here, the user will see the "Years" slider, which propogates the simulated environment over time, as described earlier. Then they see a "Climate Dryness" input. Mathematically, this constant represents the "evaporation constant" in the back-end of the simulation, controlling how much water units will be evaporated each year based on the climate. This parameter alone can be used to vary the way the environment will behave over time. The GUI begins with the default climate wetness of 1.5, which represents the climate of a deciduous forest. However, if the user changes this value, they will see drastic effects. Bringing it down to 0.0 shows the effects of a rainforest climate, meaning also none of the water is evaporated out of the soil. Therefore, this will create an extremely wet and prolific climate, where the vegetation multiplies. On the other hand, increasing the drying to it's maximum value creates a desert-like climate where almost all the water is evaporated out of the soil. Here, all the trees will dry out and die over time. 

Finally, you will see the "Starter Tree Placement" setters. Upon start-up, the starting tree positions are set automatically based on a noise function. However, the user can also precisely choose the location of their trees. The terrain is set up like a grid, counting cells from 0 to 1024. The user can add numbers to the input to describe where they want the trees of the corresponding age to be. The input strings that the system can handle include single numbers ("200"), listed numbers ("1, 4, 10, 50"), numerical ranges ("100-200"), or a combination of all the above ("1, 2, 200-300, 4, 521-634"). 

Future Work 
--------------
 - If we had more time, we would have liked to give the user even more control over the environment. Namely, although the user can control the climate throughout time, they cannot control the initial values for how much water is already in the air (vapor water) and in the soil (soil water). With more time, we could have also created an input (much like the terrain input), where the user could input a greyscale image that the system could read to create the starting vapor and soil water values. 
 - Clouds only appear by Year 2 because of problems associated with the noise-based vapor values. It looks odd and unrealistic for the first two years because of the inital settings. However, by year 2, all the precipitation and cloud values are fully based on present vegetation. If we were able to input user-specified accurate vapor values, clouds should be able to appear right away in theory. 
 - Although the user can input any object they would like into the system, we can other handle one theoretical tree species. Regardless of model, the mathematics in the back-end of the system is meant to represent a decidous and conifer species of trees. In the future, we would like to be able to handle many different species, all modeled accurated. 
 - In future work, we would like the resulting model to be more aethetically pleasing. For example, coloring the terrain based on soil water (to represent grass) rather than height. Or, to be able to use more high-poly geometry. However, due to our limited computational power, and our priority for speed, we were limited to simpler models. 
 - Overall, due to the limitations and scope of this project, we sometimes had to choose between aethetics and realism. Unfortunately, our model is not as perfectly biologically realistic and the source paper, which touted that it could be usable for real ecologic studies. Instead, we tried to prioritize control for the artists, being able to change the vegetation and climate values freely. In future work, we would like to have been able to improve our model to be even more realistic. 

