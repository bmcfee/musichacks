nesasm -raw make_fds.txt
copy /b fdshdr.bin + make_fds.nes + fdsboot.bin nesmusic.fds
del make_fds.nes
