#!/bin/bash

echo "Creating initrd cpio..."

cd initrd
find bin | cpio -ov -H newc > ../build/initrd.img
cd ..

echo Done
