#!/bin/bash
sudo mount fs.img /mnt
sudo cp /home/zbtrs/testsuits-for-oskernel/sdcard/iozone /mnt/
sudo umount fs.img
make clean