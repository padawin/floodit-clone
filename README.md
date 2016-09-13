# Floodit

## Compilation

	make

To compile for the GCW-zero, you need the toolchain (downloadable from the
console's website), then:

	make CROSS_COMPILE=/opt/gcw0-toolchain/usr/bin/mipsel-linux-

To package for the gcw zero, you will need the package squashfs-tools, then run

	make opk
