#!/bin/sh

ApCliEnable_Nvram=`nvram_get 2860 ApCliEnable`
ApCliAuthMode_Nvram=`nvram_get 2860 ApCliAuthMode`
ApCliEncrypType_Nvram=`nvram_get 2860 ApCliEncrypType`
ApCliSsid_Nvram=`nvram_get 2860 ApCliSsid`
ApCliWPAPSK_Nvram=`nvram_get 2860 ApCliWPAPSK`

# debug
echo "ApCliEnable=$ApCliEnable_Nvram"
echo "ApCliAuthMode=$ApCliAuthMode_Nvram"
echo "ApCliEncrypType=$ApCliEncrypType_Nvram"
echo "ApCliSsid=$ApCliSsid_Nvram"
echo "ApCliWPAPSK=$ApCliWPAPSK_Nvram"
echo "ApCliSsid=$ApCliSsid_Nvram"

if [ "$ApCliEnable_Nvram" = "1" ]; then
	ifconfig apcli0 up
	brctl addif br0 ra0
	brctl addif br0 apcli0
	iwpriv apcli0 set ApCliEnable=0
	iwpriv apcli0 set ApCliAuthMode="$ApCliAuthMode_Nvram"
	iwpriv apcli0 set ApCliEncrypType="$ApCliEncrypType_Nvram"
	iwpriv apcli0 set ApCliSsid="$ApCliSsid_Nvram"
	iwpriv apcli0 set ApCliWPAPSK="$ApCliWPAPSK_Nvram"
	iwpriv apcli0 set ApCliSsid="$ApCliSsid_Nvram"
	iwpriv apcli0 set ApCliEnable=1
fi


