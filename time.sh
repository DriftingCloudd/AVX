#!/bin/bash
sudo mount fs.img /mnt
sudo mv ~/testsuits-for-oskernel/time-test/time-test /mnt
sudo umount fs.img