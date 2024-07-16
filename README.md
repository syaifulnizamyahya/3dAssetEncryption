# 3d asset encryption by obfuscation

Simple project encrypting 3d assets. Using GLTF as 3d model format. The encrypted object can still be rendered.
Useful if 
1. Your application must fetch data from public server
2. You dont have control to the public server
3. You want to manage access control to your data hosted in public server

There are 3 encryption method in this example :
1. Obfuscating the vertices by applying XOR operations against random number.

![image](https://github.com/user-attachments/assets/3396cb7f-db67-4acd-a5dd-0b41d1f2c91e)

2. Obfuscating the vertices by shuffling the vertices index.

![image](https://github.com/user-attachments/assets/8654d48e-f8bc-443e-8ed1-33715e9e7e9e)

3. Obfuscating the vertex component by shuffling the vertex component index.

![image](https://github.com/user-attachments/assets/407698ba-4a6c-4f87-b61d-03afa260c0d7)

Although encrypting 3d assets secure the assets, user can still rip the assets using tools that can extract the data in GPU memory (which we have no control). 
Its like, if you show a video, people can still record it. 

All is not lost though. There are ways to protect 3d asset from ripping tools. You can refer to how industries protect their video, audio, software from piracy.

You can view GLTF models using https://gltf-viewer.donmccurdy.com/
