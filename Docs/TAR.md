# Tape ARchive (TAR)

TAR was a file format that is easy to parse, but yet it has given me a bit of trouble. Its support within JCR6 is therefore very limited.
TAR archives, only containing files, without anything more fancy than that, should be read fine. However there are a few important catches.


- Compressed TAR files are NOT supported. So only raw TAR files will do.
- TAR was never meant for random access, and random access is precisely where JCR6 excels. Not to mention as a game resource system it's its core power. Now JCR6 can get the data out in order to make it random access, but be prepared that big TAR archives, can give you a bit of trouble (well, a lot of time consumption while analysing what files are in there). Once JCR6 managed to read the TAR file's directory, everything should work fine.
- JCR6 does NOT allow files to start with a / and TAR does allow that. This can be a bit bothersome and lead to some funny behavior by JCR6.
- JCR6 does not support directories, and TAR does. JCR6 will just ignore them.
- TAR does allow symlinking and JCR6 does not, however, JCR6 will try to link itself to the file SymLinks refer to if they exist, and if configured to do so. If the variable TAR_SymLinking is set to true, JCR6 will try this. This feature has NOT be thoroughly tested, so use at your own risk. Also note that differences in directory structure between OSes can also get you here.
- There are many features TAR allows (particularly when using USTAR) that JCR6 does not, and also a lot of features that I could not really figure out what they were for (nor could I find documentation about those). The TAR driver will simply stop when this happens to prevent further trouble. Normally it will trigger JCR6 to panick, however if the TAR_ReturnWhatIHave variable has been set to true, JCR6 will just stop reading there and at least allow you access to everything read so far. This may lead to TAR files only being half-read. 
- Please note, JCR6 will ALWAYS put the entries stored in alphabetic order. In the case of TAR this may not be the most desirable behavior, but hey after everything has been read, JCR6 will be just as quick as always with its random access, so it shouldn't really matter.
- Unlike TAR JCR6 is case INSENSITIVE. If there's a file name "hello" and a file "HELLO" inside the TAR archive JCR6 will just stick with the file it finds last. Remember the JCR6 format is the prime interst here. Not the TAR format.
- TAR has no official method to enable JCR6 to recognize it. By default it will just check if the file ends with ".tar" (case insensitively). When you set TAR_ThoroughCheck to true, it will try if the first entry is read succesfully. This method is not guaranteed to produce good results, though.


Now if you think you can improve TAR support in JCR6, you are welcome to give it a shot, but remember, the focus of the JCR6 project lies on JCR6, so any improvements to support of other files may never undermine JCR6 support.
That being said, it would always be very unwise to use JCR6 as a serious option for TAR unpacking, but hey, I had fun trying to make it happen. :)
