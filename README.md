# EcoSim

Demo
--------
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/lGmsBU356ZI/0.jpg)](https://www.youtube.com/watch?v=lGmsBU356ZI)

Alpha Version Features
---------------------------
For my progress in the alpha version, you can now create an EcoSim node in Houdini (which is a fully functional class). 
The class takes in a vapor value, which populates all the 3D environment map with the same value, and it takes in a soil water value, which populates all the soil in the terrain map with the same amount of water. 
Then given a number of iterations (or generations), the feedback loop ages the forest, correctly growing, decaying, and dispering new trees throughout the environement based on the amount of water in the soil.
While the forest may look like it behaves oddly at the moment, these are all expected values and behaviors for this version of the project. All the climatic functions and processes are working correctly and as expected.

Next Steps 
------------
* We will be implement and connect actual tress meshes (as opposed to stick heights). 
* Tweak function values to represent a realistic forest based on our model research. 
* Combine with Serena's Houdini node structure. 
