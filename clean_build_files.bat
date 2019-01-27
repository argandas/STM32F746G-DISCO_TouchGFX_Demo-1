ECHO  Deleting Object files...
find . -type f -name '*.d' -delete
find . -type f -name '*.o' -delete
find . -type f -name '*.ko' -delete
find . -type f -name '*.obj' -delete
find . -type f -name '*.elf' -delete

ECHO Deleting Linker output...
find . -type f -name '*.ilk' -delete
find . -type f -name '*.map' -delete
find . -type f -name '*.exp' -delete

ECHO Deleting Debug files...
find . -type f -name '*.pdb' -delete
find . -type f -name '*.idb' -delete

ECHO Done.
