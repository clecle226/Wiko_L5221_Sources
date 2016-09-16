#!/system/bin/sh

#########################################################################
# File Name: logd-catch.sh
# Author: Shaobin
# mail: shaobin.zhang@tinno.com
# Created Time: Wed Apr 22 16:26:44 2015
#########################################################################

#/system/bin/sleep 30
WAIT_BOOTCOMPLETED=`/system/bin/getprop sys.boot_completed`
if test "$WAIT_BOOTCOMPLETED" = ""
then
/system/bin/sleep 10
fi

LOG_PATH=/storage/sdcard0/Logs
DATE_INFO=`date  "+%Y_%m%d_%H%M"`
mkdir -p $LOG_PATH/AP_Logs
#mkdir -p $LOG_PATH/logs-back
mkdir -p $LOG_PATH/BugreportLogs
mkdir -p /data/BugreportLogs
chown system.system $LOG_PATH/AP_Logs
#chown system.system $LOG_PATH/logs-back
chown system.system $LOG_PATH/BugreportLogs
chown system.system /data/BugreportLogs
#back-up last logs
#/system/bin/mv $LOG_PATH/AP_Logs/* $LOG_PATH/logs-back

#catch logcat && radio_log && kernel_log into $LOG_PATH
mkdir -p $LOG_PATH/AP_Logs/APLogs_$DATE_INFO

OPT=$1
if test "$OPT" = "all"
then
/system/bin/logcat -v threadtime -b all -r 10000 -f $LOG_PATH/AP_Logs/APLogs_$DATE_INFO/logcat_$DATE_INFO.log 
elif test "$OPT" = "radio"
then
/system/bin/logcat -v threadtime -b radio -r 10000 -f $LOG_PATH/AP_Logs/APLogs_$DATE_INFO/logcat_radio_$DATE_INFO.log 
elif test "$OPT" = "dmesg"
then
/system/bin/dmesg -r 100m -f $LOG_PATH/AP_Logs/APLogs_$DATE_INFO/dmesg_$DATE_INFO.log
elif test "$OPT" = "tcp"
then
tcpdump -l -w $LOG_PATH/AP_Logs/APLogs_$DATE_INFO/tcpdump_$DATE_INFO.pcap
fi
