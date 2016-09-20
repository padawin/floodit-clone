# Floodit

## Compilation

	make

To compile for the GCW-zero, you need the toolchain (downloadable from the
console's website), then:

	make CROSS_COMPILE=/opt/gcw0-toolchain/usr/bin/mipsel-linux-

To package for the gcw zero, you will need the package squashfs-tools, then run

	make opk

Finally, to use your opk, copy it in your GCW-Zero. I use scp for that, but via
ftp or on a memory card should work as well:

	scp floodit.opk root@[gcwIp]:/media/data/apps/

## Credits

Font ClearSans from
https://01.org/clear-sans/blogs/2013/clear-sans-new-font-optimized-small-screen-readability
