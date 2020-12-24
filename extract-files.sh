#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

DEVICE=nx611j
VENDOR=nubia

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

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
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

function blob_fixup() {
    case "${1}" in
        system_ext/etc/init/dpmd.rc)
            sed -i "s|/system/product/bin/|/system/system_ext/bin/|g" "${2}"
            ;;
        system_ext/etc/permissions/com.qti.dpmframework.xml | system_ext/etc/permissions/dpmapi.xml | system_ext/etc/permissions/telephonyservice.xml)
            sed -i "s|/system/product/framework/|/system/system_ext/framework/|g" "${2}"
            ;;
        system_ext/etc/permissions/qcrilhook.xml)
            sed -i 's|/product/framework/qcrilhook.jar|/system_ext/framework/qcrilhook.jar|g' "${2}"
            ;;
        system_ext/lib64/libdpmframework.so)
            for LIBDPM_SHIM in $(grep -L "libcutils_shim.so" "${2}"); do
                "${PATCHELF}" --add-needed "libcutils_shim.so" "$LIBDPM_SHIM"
            done
            ;;
        vendor/lib/hw/camera.sdm660.so)
            "${PATCHELF}" --add-needed "libcamera_sdm660_shim.so" "${2}"
         ;;
        vendor/lib/libNubiaImageAlgorithm.so)
            "${PATCHELF}" --add-needed "libNubiaImageAlgorithmShim.so" "${2}"
            "${PATCHELF}" --remove-needed "libjnigraphics.so" "${2}"
            "${PATCHELF}" --remove-needed "libnativehelper.so" "${2}"
            "${PATCHELF}" --add-needed "libui_shim.so" "${2}"
        ;;
        vendor/lib/libmmcamera_ppeiscore.so|vendor/lib/libmmcamera_bokeh.so|vendor/lib/libnubia_effect.so|vendor/lib64/libnubia_effect.so|vendor/lib64/libnubia_media_player.so)
            "${PATCHELF}" --remove-needed "libandroid.so" "${2}"
         ;;
        vendor/lib64/libnubia_media_player.so)
            "${PATCHELF}" --remove-needed "libandroid_runtime.so" "${2}"
         ;;
        vendor/lib/libmmcamera_ppeiscore.so|vendor/lib/libmmcamera_bokeh.so|vendor/lib/libnubia_effect.so|vendor/lib64/libnubia_effect.so|vendor/lib64/libnubia_media_player.so)
            "${PATCHELF}" --remove-needed "libgui.so" "${2}"
         ;;
        vendor/lib64/hw/fingerprint.sdm660.so)
            "${PATCHELF}" --replace-needed "libunwind.so" "libunwind-vendor.so" "${2}"
            "${PATCHELF}" --replace-needed "libbacktrace.so" "libbacktrace-vendor.so" "${2}"
         ;;
    esac
}

# Initialize the helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"

extract "${MY_DIR}/proprietary-files-nubia.txt" "${SRC}" "${KANG}" --section "${SECTION}"

"${MY_DIR}/setup-makefiles.sh"
