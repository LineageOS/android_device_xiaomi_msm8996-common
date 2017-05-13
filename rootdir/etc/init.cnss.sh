#!/system/bin/sh

hw_version=$(getprop ro.boot.hwversion | cut -c 1)
need_fem_fix=$(grep -c "$(printf '\x01')" /persist/wlan_bt/ff_flag)

case $hw_version in
    "4")
        echo -n "bd30_a4.bin" > /sys/module/cnss_common/parameters/bdwlan_file
        ;;
    "7")
        if [ $need_fem_fix -ne 1 ]; then
            echo -n "bd30_a7.b02" > /sys/module/cnss_common/parameters/bdwlan_file
        else
            echo -n "bd30_a7.bin" > /sys/module/cnss_common/parameters/bdwlan_file
        fi
        ;;
    "8")
        echo -n "bd30_a8.bin" > /sys/module/cnss_common/parameters/bdwlan_file
        ;;
    "9")
        if [ $need_fem_fix -ne 1 ]; then
            echo -n "bd30_b7.b02" > /sys/module/cnss_common/parameters/bdwlan_file
        else
            echo -n "bd30_b7.bin" > /sys/module/cnss_common/parameters/bdwlan_file
        fi
        ;;
esac
