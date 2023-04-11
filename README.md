# EcoSim

Demo
--------
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/FLVmGrclKW4/0.jpg)](https://www.youtube.com/watch?v=FLVmGrclKW4)

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
