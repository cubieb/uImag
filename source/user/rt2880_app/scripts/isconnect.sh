#/bin/sh
ifconfig apcli0 down
ifconfig apcli0 up         
brctl addif br0 ra0        
brctl addif br0 apcli0     
iwpriv apcli0 set ApCliEnable=0
iwpriv ra0 set Channel="$1"
iwpriv apcli0 set ApCliAuthMode="$2"
iwpriv apcli0 set ApCliEncrypType="$3"
iwpriv apcli0 set ApCliSsid="$4"      
iwpriv apcli0 set ApCliWPAPSK="$5"
iwpriv apcli0 set ApCliSsid="$4"      
#iwpriv apcli0 set ApCliSsid=CMCC~Smart
#iwpriv apcli0 set ApCliWPAPSK=znyjjszx
#iwpriv apcli0 set ApCliSsid=CMCC~Smart
iwpriv apcli0 set ApCliEnable=1
