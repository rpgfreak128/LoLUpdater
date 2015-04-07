LoLUpdater C++ (Visual Studio 2013 project)
==========
LoLUpdater is an attempt to increase FPS in the game League of Legends while keeping the same graphics, also makes the PVPnet client more responsive and minimally reduced its RAM usage.

Files changed by LoLUpdater:
----------------------------

msvcp120.dll is the CLR version from the VS 12 CTP without any versioninfo 

msvcr120.dll is the CLR version from the VS 12 CTP without any versioninfo

msvcp110.dll is the CLR version from the VS 12 CTP without any versioninfo 

msvcr110.dll is the CLR version from the VS 12 CTP without any versioninfo

tbb.dll is built with the project settings here in the repo, then with the version info and Manifest file removed. (the tbb library is open source [(Currently using 4.3 Update 4)](https://www.threadingbuildingblocks.org/download) )

The adobe air/flash files are installed by downloading the latest Adobe Air beta installer and the latest Adobe Flash beta installer (url has to be changed manually from code every major update), currently using [Adobe Air Beta 17](http://labs.adobe.com/downloads/air.html) and [Adobe Flash Beta 17](http://labs.adobe.com/downloads/flashplayer.html)

The Cg DLLs are installed using the latest (Although CG is now deprecated) installer (This installer is embedded in the project)
[Cg Toolkit 3.1 - April 2012 (3.1.0013)](http://developer.download.nvidia.com/cg/Cg_3.1/Cg-3.1_April2012_Setup.exe)  
<br />
<br />
[LoLUpdater Website](http://LoLUpdater.com)
