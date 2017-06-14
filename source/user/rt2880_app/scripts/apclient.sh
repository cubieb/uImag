#!/bin/sh

if [ "$1" != "" ]; then
        ssid="$1"
else
        ssid="CMCC~Smart"
fi


if [ "$2" != "" ]; then
        password="$2"
else
        password="znyjjszx"
fi

ifconfig apcli0 up
brctl addif br0 ra0
brctl addif br0 apcli0
iwpriv apcli0 set ApCliEnable=0
iwpriv apcli0 set ApCliAuthMode=WPA2PSK
iwpriv apcli0 set ApCliEncrypType=AES
iwpriv apcli0 set ApCliSsid=$ssid
iwpriv apcli0 set ApCliWPAPSK=$password
iwpriv apcli0 set ApCliSsid=$ssid
#iwpriv apcli0 set ApCliSsid=CMCC~Smart
#iwpriv apcli0 set ApCliWPAPSK=znyjjszx
#iwpriv apcli0 set ApCliSsid=CMCC~Smart
iwpriv apcli0 set ApCliEnable=1
