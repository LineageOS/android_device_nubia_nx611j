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
SRC=~/
# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

LINEAGE_ROOT="$MY_DIR"/../../..

HELPER="$LINEAGE_ROOT"/vendor/lineage/build/tools/extract_utils.sh
if [ ! -f "$HELPER" ]; then
    echo "Unable to find helper script at $HELPER"
    exit 1
fi
. "$HELPER"

while [ "$1" != "" ]; do
    case $1 in
        -n | --no-cleanup )     CLEAN_VENDOR=false
                                ;;
        -s | --section )        shift
                                SECTION=$1
                                CLEAN_VENDOR=false
                                ;;
        * )                     SRC=$1
                                ;;
    esac
    shift
done

if [ -z "$SRC" ]; then
    SRC=adb
fi

function blob_fixup() {
    case "${1}" in
    product/lib64/libdpmframework.so)
        "${PATCHELF}" --add-needed "libshim_dpmframework.so" "${2}"
        ;;
    esac
}
# Initialize the helper
setup_vendor "$DEVICE" "$VENDOR" "$LINEAGE_ROOT" false "$CLEAN_VENDOR"

extract "$MY_DIR"/proprietary-files.txt "$SRC" "$SECTION"
extract "$MY_DIR"/proprietary-files-nubia.txt "$SRC" "$SECTION"

BLOB_ROOT="$LINEAGE_ROOT"/vendor/"$VENDOR"/"$DEVICE"/proprietary

patchelf --add-needed libprocessgroup.so "$BLOB_ROOT"/vendor/lib/hw/sound_trigger.primary.sdm660.so
patchelf --add-needed libprocessgroup.so "$BLOB_ROOT"/vendor/lib64/hw/sound_trigger.primary.sdm660.so

# Load libshim_camera.so
patchelf --add-needed libshim_camera.so "$BLOB_ROOT"/vendor/lib/hw/camera.sdm660.so

"$MY_DIR"/setup-makefiles.sh
