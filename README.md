# LibEFT: an EFT loading DLL for Emergency 3/Emergency 4 mods!

>"Ladies and gentlemen, I present...
>The disguised S3TC texture.
>There's no way it's anything else.
>I did the math."
>
> Jean-Luc Mackail, 2021, on the first reverse-engineering attempt.

> "So that's what happens when you smoke during pregnancy huh
> You get a developer who came up with this"
>
> Annabel Jocelyn Sandford, 2021, talking about one particularly tricky tiling issue we were getting.

This is the source for the DLL that powers EFTXplorer.  :)
The program for loading and exporting EFT files from the Emergency games can be found here:
https://github.com/annabelsandford/EFTXplorer

Please note this library only works on EM3 and EM4 EFT files, EFT files from the 2012 and higher games are a completely different format and currently I don't have the slightest idea of how they work.

Building:
- Currently, due to admittedly my stinginess with not using Visual Studio, the DLL currently only builds properly for 64-bit through MinGW (The 32-bit DLL works but interacts badly with C# code at the moment, works fine with C code otherwise)
- As a result, mingw on Windows is required to build this DLL. (Pull requests to get it built under Visual Studio are welcome)
- To build with mingw, assuming mingw is on your Windows PATH:

`C:\Users\YourUsername> mingw32-make all`

The DLLs will show up in the `/bin` folder.

Using:

todo: write this section. XD
Might wanna make a few Python devs happy too at some point.

Credits:
- Annabel Jocelyn Sandford (Twitter > https://twitter.com/annie_sandford)
  Honestly, props to her for the frontend, I couldn't make a frontend to save my life. XD
  Well, maybe one day I will, but I love my low level stuff.

- Jean-Luc Mackail (Twitter > https://twitter.com/fuzzyquills)
  Yep, *the* FuzzyQuils! If anyone recognises the name from somewhere, it's me!
  The mastermind who reverse engineered the EFT format. Was quite the journey.
