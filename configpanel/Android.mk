LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PACKAGE_NAME := ConfigPanel
LOCAL_REQUIRED_MODULES := XiaomiPocketMode

LOCAL_STATIC_JAVA_LIBRARIES := \
    org.cyanogenmod.platform.internal

LOCAL_AAPT_FLAGS := --auto-add-overlay

LOCAL_RESOURCE_DIR := \
    $(LOCAL_PATH)/res \
    $(LOCAL_PATH)/../../../../packages/resources/devicesettings/res

LOCAL_PRIVILEGED_MODULE := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_PACKAGE)
