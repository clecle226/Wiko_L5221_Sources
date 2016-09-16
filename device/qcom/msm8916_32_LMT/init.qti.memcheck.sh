 #!/system/bin/sh
 # Copyright (c) 2015, The Linux Foundation. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met:
 #     * Redistributions of source code must retain the above copyright
 #       notice, this list of conditions and the following disclaimer.
 #     * Redistributions in binary form must reproduce the above copyright
 #       notice, this list of conditions and the following disclaimer in the
 #       documentation and/or other materials provided with the distribution.
 #     * Neither the name of The Linux Foundation nor
 #       the names of its contributors may be used to endorse or promote
 #       products derived from this software without specific prior written
 #       permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 # AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 # NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 # OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 # WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 # OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 # ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 #
 # The script will check total_ram and enable features like ksm on devices
 # with total_ram less or equals to 1GB
 
 MemTotalStr=`cat /proc/meminfo | grep MemTotal`
 MemTotal=${MemTotalStr:16:8}
 MEMORY_THRESHOLD=1048576
 IsLowMemory=0
 ((IsLowMemory=MemTotal<MEMORY_THRESHOLD?1:0))
 if [ $IsLowMemory -eq 1 ]; then
 #features/properties applicable up to 1GB
     if [ -f /sys/kernel/mm/ksm/run ]; then
         echo 100 > /sys/kernel/mm/ksm/pages_to_scan
         echo 500 > /sys/kernel/mm/ksm/sleep_millisecs
         echo 1 > /sys/kernel/mm/ksm/deferred_timer
         echo 1 > /sys/kernel/mm/ksm/run
     fi
 else
 #features/properties applicable above 1GB
     if [ -f /sys/kernel/mm/ksm/run ]; then
         echo 0 > /sys/kernel/mm/ksm/run
     fi
 fi
