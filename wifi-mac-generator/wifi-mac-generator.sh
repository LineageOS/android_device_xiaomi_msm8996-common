#!/vendor/bin/sh

# data format:
# hex binary file containing the mac
WLAN_MAC_HEX_PATH="/mnt/vendor/persist/wlan_bt/wlan.mac"

# data format:
# Intf0MacAddress=00AA00BB00CC
# Intf1MacAddress=00AA00BB00CD
# END
WLAN_MAC_PERSIST_PATH="/mnt/vendor/persist/wlan_mac.bin"

if [ ! -s "${WLAN_MAC_HEX_PATH}" ]; then
    exit
fi

# Read the mac from persist
raw_mac=$(xxd -p $WLAN_MAC_HEX_PATH)

# Convert to decimal
dec_mac=$(printf "%d" "0x$raw_mac")

# The MAC of the first interface is the decimal mac,
# converted to uppercase
first_mac=$(printf "%012X" "$dec_mac")

# Increment the decimal mac by one
dec_mac=$(expr $dec_mac + 1 )

# The MAC of the first interface is the decimal mac
# plus one, converted to uppercase
second_mac=$(printf "%012X" "$dec_mac")

# Write the MACs
echo "Intf0MacAddress=${first_mac}" > "${WLAN_MAC_PERSIST_PATH}"
echo "Intf1MacAddress=${second_mac}" >> "${WLAN_MAC_PERSIST_PATH}"
echo "END" >> "${WLAN_MAC_PERSIST_PATH}"
