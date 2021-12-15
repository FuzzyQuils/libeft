# LibEFT: an EFT loading DLL for Emergency mods!

>"Ladies and gentlemen, I present...
>The disguised S3TC texture.
>There's no way it's anything else.
>I did the math."
>
> Jean-Luc Mackail, 2021

This is the source for the DLL that powers EFTXplorer.  :)
The program for loading and exporting EFT files from the Emergency games can be found here:
https://github.com/annabelsandford/EFTXplorer

(Jokes aside from the quote above, the process ended up being quite a bit more involved!)

Building:
- Currently, due to admittedly my stinginess with not using Visual Studio, the DLL currently only builds properly for 64-bit through MinGW (The 32-bit DLL works but interacts badly with C# code at the moment, works fine with C code otherwise)
- As a result, mingw on Windows is required to build this DLL. (Pull requests to get it built under Visual Studio are welcome)
- To build with mingw, assuming mingw is on your Windows PATH:

`C:\Users\YourUsername> mingw32-make all`

The DLLs will show up in the `/bin` folder.

Using:

todo: write this section. XD
might wanna make a few Python devs happy too at some point.

Credits:
- Annabel Jocelyn Sandford (Twitter > https://twitter.com/annie_sandford)
  Honestly, props to her for the frontend, I couldn't make a frontend to save my life. XD
  Well, maybe one day I will, but I love my low level stuff.

- Jean-Luc Mackail (Twitter > https://twitter.com/fuzzyquills)
  Yep, *the* FuzzyQuils! Tf anyone recognises the name from somewhere, it's me!
  The mastermind who reverse engineered the EFT format. Was quite the journey.
