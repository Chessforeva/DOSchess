
rem Create palette data from 256 color windows 24bit standard bitmap

bmp2bin.exe -t colp256.bmp colp256.rgb



rem Convert bitmaps to 1byte arrays matched to palette color

bmp2bin.exe -1 colp256.rgb BO.bmp BO.bin

bmp2bin.exe -1 colp256.rgb BB.bmp BB.bin
bmp2bin.exe -1 colp256.rgb BK.bmp BK.bin
bmp2bin.exe -1 colp256.rgb BN.bmp BN.bin

bmp2bin.exe -1 colp256.rgb BP.bmp BP.bin
bmp2bin.exe -1 colp256.rgb BQ.bmp BQ.bin
bmp2bin.exe -1 colp256.rgb BR.bmp BR.bin
bmp2bin.exe -1 colp256.rgb WB.bmp WB.bin
bmp2bin.exe -1 colp256.rgb WK.bmp WK.bin
bmp2bin.exe -1 colp256.rgb WN.bmp WN.bin
bmp2bin.exe -1 colp256.rgb WP.bmp WP.bin
bmp2bin.exe -1 colp256.rgb WQ.bmp WQ.bin
bmp2bin.exe -1 colp256.rgb WR.bmp WR.bin

bmp2bin.exe
pause