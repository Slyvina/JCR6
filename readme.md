![JCR6](https://github.com/Slyvina/JCR6/blob/main/Logo/JCR.png?raw=true)

# Jeroen's Collected Resource

JCR6 is the 5th full revision of the original JCR format, and therefore the 6th full version (hence the 6).

JCR6 collects all files together and can (with the proper drivers tied to it) even perform compression in any algorith (as long as there's a driver for JCR6, it can handle it).

JCR6 is therefore a kind of archive, like ZIP, RAR, TAR, 7z, ARJ and many more, but the focus for JCR6 lies most of all in having assets for games packaged together in order not to have tons of files with my game releases, but just a few files.
This focus dir make that a few things are different from your regular archiver:

- First of all JCR6 is modular. And that means
  - It can be used to read several packed file forms (providing the drivers are made for JCR6)
  - It's easy to add any compression algoritm with it as long as you know how to use that compression algorithm so you can code a driver for JCR6
- It's possible to even have JCR6's file table compressed
- JCR6 supports patching. Did you ever patch DOOM with .WAD files? Well, it works just like that.
- JCR6 can also do some "auto-patching", that means it can call to other JCR6 supported files and automatically patch them in (in my RPG games I used this feature to easily make versions with and without music). Both optional external files as required files can be handled this way.
- Now even the data in the file table is very extendable. If there's more than the entry data itself you need, you can just add it. I don't know what you may need to put in all extra, but hey, it's possible.
- JCR6 can even have multiple files put in one block and has that entire block packed as one block of data, in stead of packed per file. (Silimar to "solid archiving" in 7z and RAR). This can even be done in combination in one packed file.
- JCR6 accesses all entries inside a packed file case insenstively. Yes even on Linux and Unix, when it comes to entry names, JCR6 has its own rules regardless of platform.

Now just like regular archivers comments can be stored inside a JCR6 file. As a matter of fact, you can put multiple comments inside a JCR6 file. 

What one should note is that JCR6 files are limited to 2 Gigabytes in filesize max. It is however possible to bypass that by making JCR6 files able to require other JCR6 files.


# Slyvina

Now this version of JCR6 is part of the Slyvina project and it will need the Slyvina Units to be present. 