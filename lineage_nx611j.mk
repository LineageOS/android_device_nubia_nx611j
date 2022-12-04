# 
# Copyright (C) 2018 The lineage Project
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

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_o_mr1.mk)

# Enable updating of APEXes
$(call inherit-product, $(SRC_TARGET_DIR)/product/updatable_apex.mk)

# Inherit from nx611j device
$(call inherit-product, device/nubia/nx611j/device.mk)

# Inherit some common lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

PRODUCT_NAME := lineage_nx611j
PRODUCT_BRAND := nubia
PRODUCT_DEVICE := nx611j
PRODUCT_MANUFACTURER := nubia
PRODUCT_MODEL := Nubia Z18 Mini

PRODUCT_GMS_CLIENTID_BASE := android-nubia

PRODUCT_BUILD_PROP_OVERRIDES += \
    TARGET_DEVICE="nx611j" \
    PRODUCT_NAME="nx611j" \
    BUILD_FINGERPRINT="nubia/NX611J/NX611J:8.1.0/OPM1.171019.011/nubia.20200618.184338:user/release-keys" \
    PRIVATE_BUILD_DESC="NX611J-user 8.1.0 OPM1.171019.011 eng.nubia.20200618.184338 release-keys"

TARGET_VENDOR := nubia
