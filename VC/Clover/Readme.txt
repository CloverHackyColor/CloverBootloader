2021-05
  Jief : I got "don't know how to make 'c'" when I launch edksetup.bat Rebuild. It was because of a Windows permission problem.

  After modifiying GenMake.py, launch :
    ../rsync\ to\ windows\ vmware\ ssh && ssh Clover-PC "rmdir /S /Q CloverBootloader_synced\\Conf & rmdir /S /Q CloverBootloader_synced\\Build\\Clover\\DEBUG_VS2017\\X64\\rEFIt_UEFI\\refit"