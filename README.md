# 3d asset encryption by obfuscation

Simple project encrypting 3d assets. Using GLTF as 3d model format. The encrypted object can still be rendered.

There are 2 encryption method in this example :
1. Obfuscating the vertices by applying XOR operations against random number.
2. Obfuscating the vertices by shuffling the vertex component index.

Although encrypting 3d assets secure the assets, user can still rip the assets using tools that can extract the data in GPU memory. 
Its like, if you show a video, people can still record it. 

You can view GLTF models using https://gltf-viewer.donmccurdy.com/
