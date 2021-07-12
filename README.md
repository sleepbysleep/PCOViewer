# PCOViewer
Simple C++ example (with MFC of Visual Studio C++ 2019) to view image from pco.Panda 4.2 

# Camera
pco.panda 4.2 - sCMOS, monochrome,  2048 x 2048 pixel, USB 3.1 Gen1 interface.
(https://www.pco-imaging.com/scientific-cameras/pcopanda-42/)

# Dependencies
pco.sdk -software development kits for microsoft windows
(https://www.pco-imaging.com/support/software/scientific-cameras-1/pcopanda-42/)

PictureCtrl.h and PictureCtrl.cpp
(Author: Tobias Eiseler, E-Mail: tobias.eiseler@sisternicky.com, Function: A MFC Picture Control to display an image on a Dialog, etc.)

# Setting for VS C++
## C/C++
$(PCO_SDK_Dir)include
if PCO_SDK_Dir is not defined, the full path may be "C:\Program Files (x86)\PCO Digital Camera Toolbox\pco.sdk\include"
## Linker
$(PCO_SDK_Dir)lib64\sc2_cam.lib
if PCO_SDK_Dir is not defined, the full path may be 
"C:\Program Files (x86)\PCO Digital Camera Toolbox\pco.sdk\lib64\sc2_cam.lib" in the case of WIN32_64
otherwise "C:\Program Files (x86)\PCO Digital Camera Toolbox\pco.sdk\lib\sc2_cam.lib"

# Screenshot
![alt text](./screenshot.png?raw=true)
