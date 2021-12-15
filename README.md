# LibEFT: an EFT loading DLL for Emergency mods!

This is the source for the DLL that powers EFTXplorer, made by myself and Annabel Jocelyn Sandford. :)
The program for loading and exporting EFT files from the Emergency games can be found here:
https://github.com/annabelsandford/EFTXplorer

Honestly, props to her for the frontend, I couldn't make a frontend to save my life. XD

Building:
- Currently, due to admittedly my stinginess with not using Visual Studio, the DLL currently only builds properly for 64-bit through MinGW (The 32-bit DLL works but interacts badly with C# code at the moment, works fine with C code otherwise)
- As a result, mingw on Windows is required to build this DLL. (Pull requests to get it built under Visual Studio are welcome)
- To build with mingw, assuming mingw is on your Windows PATH:

`C:\Users\YourUsername> mingw32-make eft_loader_x86.dll eft_loader_x64.dll`

The DLLs will show up in the `/bin` folder.

Using:
todo: write this section. XD

Credits:
- 
