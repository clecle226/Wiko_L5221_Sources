#!/system/bin/sh

#########################################################################
# File Name: logd-catch.sh
# Author: Shaobin
# mail: shaobin.zhang@tinno.com
# Created Time: Wed Apr 22 16:26:44 2015
#########################################################################

#/system/bin/sleep 30

mount -t ext4 /dev/block/bootdevice/by-name/apedata /apedata
if [ $? -eq 0 ];then
       echo "apedata partition have already mounted!"
else
       echo "need format apedata partition to ext4."
       #format apedata partition as ext4 partition
       make_ext4fs /dev/block/bootdevice/by-name/apedata
       mount -t ext4 /dev/block/bootdevice/by-name/apedata /apedata
fi

echo "restorecon -R /apedata"
restorecon -R /apedata

if [ -f /apedata/Areadata ];then
    echo "chmod 777 /apedata/Areadata"
    chmod 777 /apedata/Areadata
fi

#backup RIDL config files
if [ ! -f /data/SelfHost/RIDL.db ]; then
	mkdir /data/SelfHost/ || true
	cp /system/vendor/RIDL/RIDL.db /data/SelfHost/
	cp /system/vendor/RIDL/ReadMe.txt /data/SelfHost/
	cp /system/vendor/RIDL/GoldenLogmask.dmc /data/SelfHost/
	cp /system/vendor/RIDL/OriginalGoldenLog.dmc /data/SelfHost/
	cp /system/vendor/RIDL/qdss.cfg /data/SelfHost/
fi
