LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PACKAGE_NAME := XiaomiParts
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_REQUIRED_MODULES := parts_whitelist.xml
LOCAL_PRODUCT_MODULE := true

LOCAL_USE_AAPT2 := true

LOCAL_STATIC_ANDROID_LIBRARIES := \
    androidx.core_core \
    androidx.preference_preference

LOCAL_STATIC_JAVA_LIBRARIES := \
    org.lineageos.platform.internal

LOCAL_RESOURCE_DIR := \
    $(LOCAL_PATH)/res \
    $(TOP)/packages/resources/devicesettings/res

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_MODULE_TAGS := optional

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE := parts_whitelist.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT_ETC)/sysconfig
LOCAL_PRODUCT_MODULE := true
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
