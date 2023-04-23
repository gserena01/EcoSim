# EcoSim

![Houdini Render](https://github.com/gserena01/EcoSim/blob/main/Render1.png)

Demo
--------
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/FLVmGrclKW4/0.jpg)](https://www.youtube.com/watch?v=FLVmGrclKW4)

Uploading your own files:
--------
This plug-in allows you to input your own geometry and texture files. For textures, please upload an 32x32 pixel image file (yes, it's tiny!). For geometry files, any .fbx, .obj, or other standard geometry file will work, though we suggest using lower poly geometry to keep the plugin running quickly, since the input geometry will be replicated many times.

Exporting Geometry:
--------
To export the current geometry, click on the "export" file node, opt to display/render it, and click reload geometry. By default, the geometry will be exported to "Exported_Geometry.obj".

Beta Version Features
---------------------------
In the beta version, you can create an EcoSim node in Houdini, which automaticially starts with noise-based distribution of soil water, vapor water, and starter trees. Additionally, the nodes already connect to the included tree mesh files. This node structure in Houdini is fully developed and allows the user to either control the environment via the GUI or by directly manipulating the node parameters if they are more familiar with the software.  
Using the GUI, you can control how many years of growth cycle occurs in the forest. The cycle is controlled by mathematical feedback loops representing the water cycle with accurate and realistic numbers. In each year, the vegetations grows, decays, or spreads new seeds throughout the environment based on the amount of water distributed in the soil. 
This tool could be used by an artist to demonstrate how an environment develops overtime.  

Next Steps 
------------
* Give the artist more control over the initial environment. 
* We want the user to be able to select specific cells where they might want to place a tree in the starter state.
* The user should also be able to input a picture-based vapor or soil map at the initial state of the algorithm.
* Add more visual output to the plug-in, such that the user might be able to visually see how much water in the soil or atmosphere. 
* Tweak function values to represent an even more realistic forest based on our specific model parameters. 
