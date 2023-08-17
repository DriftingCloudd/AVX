#!/bin/bash
sudo mount fs.img /mnt
sudo cp /home/zbtrs/testsuits-for-oskernel/sdcard/netperf /mnt/
sudo cp /home/zbtrs/testsuits-for-oskernel/sdcard/netserver /mnt/
sudo umount fs.img
