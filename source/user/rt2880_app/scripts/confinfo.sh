###########################################
## 本文件用于解析config文件
##
## config文件内容格式：
##
##   vendor = CMIOT
##   .....
###########################################

#!/bin/sh

cd /etc_ro

CONFIG_DIR=conf

#
if [ -d "$CONFIG_DIR" ]; then
	rm -rf $CONFIG_DIR
fi

mkdir $CONFIG_DIR

#
#configParse()
#{
while read line
do
	echo $line
	#echo ${line%%=*}
	#echo ${line##*=}

	echo -n ${line##*=} > $CONFIG_DIR/${line%%=*}
	
done < $1
#}

#configParse confinfo

