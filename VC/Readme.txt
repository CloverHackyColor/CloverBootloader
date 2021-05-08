2021-05
How to use Visual C project ?

You need a first compilation with edk2 build system to succeed in order to compile all the libraries and generate autogen.c/.h

To do so :
-> cd to Clover folder
-> edksetup.bat
-> edksetup.bat Rebuild
-> cbuild --debug (you can also build the release version if you'd like to compile the release configuration)

-> double click on ./VC/Clover/Clover.sln
-> build as usual in VS

The result is : ./VC/Clover/x64/Debug/CloverX64.efi

NOTE :
if some options need to be adjusted, do not set them in the property page of the project. If you do that, you'll have to modify for both debug and release configuration and, let's be honest, we don't do it :-).
Instead, open the Property Manager and modify the property page "Clover" of "CloverDebug" for debug only options.
