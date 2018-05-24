#!/bin/sh -x

DEV=ttyACM
DEV_NAME="WeSense"
DEV_COUNT=0
PROD_ID="1afe/9/100"

check_connection() {
	#check if a WeSense is connected
	echo "checking connection" > /dev/kmsg
	if [[ "$ACTION" == "add" ]]; then
		VENDOR=$(lsusb -v | grep idVendor | grep 0x1afe | awk '{print $2}')
		if [ -z $VENDOR ]; then
			exit 0
		else
			search_tty
		fi
	elif [[ "$ACTION" == "remove" ]]; then
		echo "in remove $DEVNAME" > /dev/kmsg
		echo "DEVPATH: /sys${DEVPATH}/ ;ACTION: ${ACTION}; PRODUCT: ${PRODUCT}" > /dev/kmsg
		if [[ "${PRODUCT}" == "$PROD_ID" ]]; then
			remove_tty
		else
			exit 0
		fi
	else
		echo "unknown Action: ${ACTION}" > /dev/kmsg
	fi
}

remove_tty() {
	WESENSE_LIST=$( ls /dev/ | grep ${DEV_NAME} )
	USB_PATH="$(dirname /sys${DEVPATH})"
	REMAINING_DEVICES=$(grep -rs "ACM" ${USB_PATH} | awk -F "=" '{ print $2 }')
 	echo "USB_PATH: ${USB_PATH}; REMAINING_DEVICES: ${REMAINING_DEVICES}" > /dev/kmsg
	for WESENSE in $WESENSE_LIST; do
		for R_DEV in $REMAINING_DEVICES; do
			if [[ "/dev/${R_DEV}" == "$( readlink /dev/$WESENSE )" ]]; then
				WESENSE_LIST=$(printf '%s\n' "${WESENSE_LIST//$WESENSE/}")
				echo "WESENSE_LIST: $WESENSE_LIST" > /dev/kmsg
			fi
		done
	done

	for WESENSE in ${WESENSE_LIST}; do
		rm /dev/${WESENSE}
	done
}

create_link() {
	echo "Creating link for ${DEV_NAME}$2 -> /dev/$1" > /dev/kmsg
	ln -s /dev/$1 /dev/${DEV_NAME}$2
}

add_wesense() {
	WESENSE_LIST=$( ls /dev/ | grep ${DEV_NAME} )
	WESENSE_COUNT=$(echo "${WESENSE_LIST}" | wc -w)

	if [[ $WESENSE_COUNT == 0 ]]; then
		create_link $1 0 
	else
		create_link $1 $((WESENSE_COUNT))
	fi
}

search_tty() {
	CONNECTED_DEVICE=$(ls /sys/${DEVPATH}/tty/ | grep $DEV)
	if [ -z $CONNECTED_DEVICE ]; then
		echo "No tty device connected on path /sys/${DEVPATH}/tty/" > /dev/kmsg
		exit 1
	fi
	
	add_wesense $CONNECTED_DEVICE
}

check_connection
