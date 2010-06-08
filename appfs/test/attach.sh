#!/bin/bash

cd $(dirname $0)

dd if=../appfs of=./apptest.out count=1 bs=1M conv=sync
FILESIZE=$(stat -c%s ./apptest.out)
EXTENDSIZE=$[1048576-$FILESIZE]
if [ $EXTENDSIZE -gt 0 ]; then
	dd if=/dev/zero of=./apptest.out oflag=append conv=notrunc count=1 bs=$EXTENDSIZE
fi
dd if=./fs.dat of=./apptest.out oflag=append conv=notrunc
chmod u+x ./apptest.out
