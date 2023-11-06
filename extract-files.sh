#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

function blob_fixup() {
    case "${1}" in
    system/lib64/com.qualcomm.qti.ant@1.0.so)
        "${PATCHELF}" --replace-needed "libhidlbase.so" "libhidlbase-v32.so" "${2}"
        ;;
    system_ext/etc/init/dpmd.rc)
        sed -i "s/\/system\/product\/bin\//\/system\/system_ext\/bin\//g" "${2}"
        ;;
    system_ext/etc/permissions/com.qti.dpmframework.xml)
        ;&
    system_ext/etc/permissions/dpmapi.xml)
        sed -i "s/\/system\/product\/framework\//\/system\/system_ext\/framework\//g" "${2}"
        ;;
    system_ext/etc/permissions/qcrilhook.xml)
        ;&
    system_ext/etc/permissions/telephonyservice.xml)
        sed -i "s/\/system\/framework\//\/system\/system_ext\/framework\//g" "${2}"
        ;;
    system_ext/etc/permissions/qti_libpermissions.xml)
        sed -i "s/name=\"android.hidl.manager-V1.0-java/name=\"android.hidl.manager@1.0-java/g" "${2}"
        ;;
    system_ext/lib64/com.qualcomm.qti.imscmservice@1.0.so)
        "${PATCHELF}" --replace-needed "libhidlbase.so" "libhidlbase-v32.so" "${2}"
        ;;
    system_ext/lib64/lib-imscamera.so)
        grep -q "libgui_shim.so" "${2}" || "${PATCHELF}" --add-needed "libgui_shim.so" "${2}"
        ;;
    system_ext/lib64/lib-imsvideocodec.so)
        grep -q "libui_shim.so" "${2}" || "${PATCHELF}" --add-needed "libui_shim.so" "${2}"
        ;;
    system_ext/lib64/lib-imsvt.so)
        "${PATCHELF}" --replace-needed "libhidlbase.so" "libhidlbase-v32.so" "${2}"
        grep -q "libgui_shim.so" "${2}" || "${PATCHELF}" --add-needed "libgui_shim.so" "${2}"
        ;;
    system_ext/lib64/libdpmframework.so)
        sed -i "s/libhidltransport.so/libcutils-v29.so\x00\x00\x00/" "${2}"
        ;;
    vendor/bin/hw/android.hardware.bluetooth@1.0-service-qti|vendor/bin/hw/vendor.display.color@1.0-service|vendor/bin/ATFWD-daemon|vendor/bin/ims_rtp_daemon)
        "${PATCHELF}" --replace-needed "libhidlbase.so" "libhidlbase-v32.so" "${2}"
        ;;
    vendor/bin/imsrcsd)
        "${PATCHELF}" --replace-needed "libhidlbase.so" "libhidlbase-v32.so" "${2}"
        sed -i "s/libhidltransport.so/libbase_shim.so\x00\x00\x00\x00/" "${2}"
        ;;
    vendor/bin/pm-service)
        grep -q libutils-v33.so "${2}" || "${PATCHELF}" --add-needed "libutils-v33.so" "${2}"
        ;;
    vendor/lib64/hw/android.hardware.bluetooth@1.0-impl-qti.so)
        sed -i "s/libhidltransport.so/libbase_shim.so\x00\x00\x00\x00/" "${2}"
        ;;
    vendor/lib64/hw/vulkan.msm8996.so)
        sed -i "s/vulkan.msm8953.so/vulkan.msm8996.so/g" "${2}"
        ;;
    vendor/lib64/lib-dplmedia.so)
        "${PATCHELF}" --remove-needed "libmedia.so" "${2}"
        ;;
    vendor/lib64/lib-uceservice.so)
        sed -i "s/libhidltransport.so/libbase_shim.so\x00\x00\x00\x00/" "${2}"
        ;;
    vendor/lib64/libril-qc-qmi-1.so)
        "${PATCHELF}" --replace-needed "libhidlbase.so" "libhidlbase-v32.so" "${2}"
        ;;
    vendor/lib/libchromaflash.so|vendor/lib/libmmcamera_hdr_gb_lib.so|vendor/lib/libmorpho_easy_hdr.so|vendor/lib/libmorpho_hdr_checker.so|vendor/lib/libmorpho_image_stab4.so|vendor/lib/libmpbase.so|vendor/lib/liboptizoom.so|vendor/lib/libseemore.so|vendor/lib/libubifocus.so)
        "${PATCHELF}" --replace-needed "libstdc++.so" "libstdc++_vendor.so" "${2}"
        ;;
    vendor/lib/libmmcamera2_isp_modules.so)
        "${SIGSCAN}" -p "06 9B 03 F5 30 2C 0C F2 5C 40 FE F7 5C EC 06 9A 02 F5 30 21 01 F5 8B 60 FE F7 5A EC 0C B9" \
                     -P "7C B9 06 9B 03 F5 30 2C 0C F2 5C 40 FE F7 5A EC 06 9A 02 F5 30 21 01 F5 8B 60 FE F7 5A EC" \
                     -f "${2}"
        ;;
    vendor/lib/libmmcamera2_sensor_modules.so)
        sed -i "s/\/system\/etc\/camera\//\/vendor\/etc\/camera\//g" "${2}"
        ;;
    vendor/lib/libmmcamera2_stats_modules.so)
        "${PATCHELF}" --remove-needed "libandroid.so" "${2}"
        "${PATCHELF}" --replace-needed "libgui.so" "libgui_vendor.so" "${2}"
        ;;
    esac
}

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

ONLY_COMMON=
ONLY_TARGET=
KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        --only-common )
                ONLY_COMMON=true
                ;;
        --only-target )
                ONLY_TARGET=true
                ;;
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"; shift
                CLEAN_VENDOR=false
                ;;
        * )
                SRC="${1}"
                ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

if [ -z "${ONLY_TARGET}" ]; then
    # Initialize the helper for common device
    setup_vendor "${DEVICE_COMMON}" "${VENDOR}" "${ANDROID_ROOT}" true "${CLEAN_VENDOR}"

    extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
fi

if [ -z "${ONLY_COMMON}" ] && [ -s "${MY_DIR}/../${DEVICE}/proprietary-files.txt" ]; then
    # Reinitialize the helper for device
    source "${MY_DIR}/../${DEVICE}/extract-files.sh"
    setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

    extract "${MY_DIR}/../${DEVICE}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
fi

"${MY_DIR}/setup-makefiles.sh"
