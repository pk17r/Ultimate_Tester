                                                        Software Toolchain installations

1. Unpack WinAVR-20100110-install.zip archive
2. Run WinAVR-20100110-install.exe (in case of installation to use default arguments)
3. Unpack "avr8-gnu-toolchain" from "avr8-gnu-toolchain-installer-3.4.4.24-win32.any.x86.7z"
4. Add "avr8-gnu-toolchain" to Path variable
5. Add "C:\WinAVR-20100110\bin" and "C:\WinAVR-20100110\utils\bin" to Path variable as well
6. Copy "avrdude.conf" and "avrdude.exe" to "C:\WinAVR-20100110\bin"


                                                     Compilation of a firmware

1. To download from the author's site at the link https://www.mikrocontroller.net/svnbrowser/transistortester/Software/trunk/the current distribution kit of firmwares.
For this purpose to click in the bottom of the page on a line of ""Download GNU tarball"".
2. To unpack the downloaded distribution kit of ""transistortester-trunk.tar.gz"".
3. To create the folder in a root of a hard drive, for example Ñ:\Trunk (cyrillic in designation of directory name not to use).
4. To rewrite in Ñ:\Trunk all contents of the trunk folder from the unpacked transistortester-trunk.tar.gz archive.
5. To delete everything from the C:\Trunk\default folder, except the dep folder.
6. To copy the makefile file corresponding to the processor in the C:\Trunk\default folder.
5. To launch the WinAVR editor - C:\WinAVR-20100110\pn\pn.exe
6. To open and edit Makefile. For correction of parameters it is possible to open cyrillic Makefile.ru in a separate window.
7. To compile Makefile, for this purpose to execute menu commands: Tools-Make All.
8. If compilation came to the end successfully - we receive exit code of equal 0 (Process Exit Code:0).
The compiled TransistorTester.eep and TransistorTester.hex firmware files will be in the same C:\Trunk\default folder. For repeated compiling of new Makefile (the same version of a firmware), it is enough to copy of it in the default folder (anything it isn't necessary to delete).
9. For incorrect compilation in the environment of Win10 x64 it is necessary to replace one library to the address [The directory with the set WinAVR] \utils\bin\msys-1.0.dll.
