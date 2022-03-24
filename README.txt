BinThumbnail by spooonsss

Windows Explorer uses BinThumbnail to generate thumbnails for SNES 4 bpp .bin files.

To use:
Run install.cmd (with BinThumbnail.dll in the same directory)
Open Windows Explorer (explorer.exe) and navigate to a directory containing SNES 4 bpp .bin files (e.g. ExGraphics/)
View->Large Icons
Windows Explorer shows icons for the .bin files rendered with SMW palette $A


Limitations:
.bin files contain no metadata, so the thumbnails are always generated assuming 4 bpp.
.bin files contain no palette information, so the thumbnails are always generated with SMW palette $A
