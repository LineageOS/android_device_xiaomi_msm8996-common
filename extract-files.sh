#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2019 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

LINEAGE_ROOT="${MY_DIR}"/../../..

HELPER="${LINEAGE_ROOT}/vendor/lineage/build/tools/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

function blob_fixup() {
    case "${1}" in
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
    system_ext/lib64/libdpmframework.so)
        sed -i "s/libhidltransport.so/libcutils-v29.so\x00\x00\x00/" "${2}"
        ;;
    vendor/bin/imsrcsd)
        patchelf --add-needed "libbase_shim.so" "${2}"
        ;;
    vendor/bin/slim_daemon)
        patchelf --replace-needed "android.frameworks.sensorservice@1.0.so" "android.frameworks.sensorservice@1.0-v27.so" "${2}"
        ;;
    vendor/lib64/hw/android.hardware.bluetooth@1.0-impl-qti.so)
        patchelf --add-needed "libbase_shim.so" "${2}"
        ;;
    vendor/lib64/hw/vulkan.msm8996.so)
        sed -i "s/vulkan.msm8953.so/vulkan.msm8996.so/g" "${2}"
        ;;
    vendor/lib64/lib-uceservice.so)
        patchelf --add-needed "libbase_shim.so" "${2}"
        ;;
    vendor/lib64/libsettings.so)
        patchelf --replace-needed "libprotobuf-cpp-full.so" "libprotobuf-cpp-full-v28.so" "${2}"
        ;;
    vendor/lib64/vendor.qti.gnss@1.0_vendor.so)
        patchelf --replace-needed "android.hardware.gnss@1.0.so" "android.hardware.gnss@1.0-v27.so" "${2}"
        ;;
    vendor/lib/hw/vulkan.msm8996.so)
        sed -i "s/vulkan.msm8953.so/vulkan.msm8996.so/g" "${2}"
        ;;
    vendor/lib/libwvhidl.so)
        patchelf --replace-needed "libprotobuf-cpp-lite.so" "libprotobuf-cpp-lite-v28.so" "${2}"
        ;;
    esac
}

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

ONLY_COMMON=
SECTION=
KANG=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        -o | --only-common )
                ONLY_COMMON=false
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

# Initialize the helper for common device
setup_vendor "${DEVICE_COMMON}" "${VENDOR}" "${LINEAGE_ROOT}" true "${CLEAN_VENDOR}"

extract "${MY_DIR}/proprietary-files.txt" "${SRC}" \
        "${KANG}" --section "${SECTION}"

if [ -z "${ONLY_COMMON}" ] && [ -s "${MY_DIR}/../${DEVICE}/proprietary-files.txt" ]; then
    # Reinitialize the helper for device
    source "${MY_DIR}/../${DEVICE}/extract-files.sh"
    setup_vendor "${DEVICE}" "${VENDOR}" "${LINEAGE_ROOT}" false "${CLEAN_VENDOR}"

    extract "${MY_DIR}/../${DEVICE}/proprietary-files.txt" "${SRC}" \
            "${KANG}" --section "${SECTION}"
fi

"${MY_DIR}/setup-makefiles.sh"
