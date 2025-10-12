#!/bin/bash

cd $HOME/projects/comp521/assignment1/kernal_modules/
make clean
ls
make
sudo insmod pidinfo.ko
vim &
ps -gx
cat /proc/pidinfo


lsmod | head
sleep 8
cat /proc/jiffies
cat /proc/seconds
sudo rmmod jiffies
sudo rmmod seconds
make clean
