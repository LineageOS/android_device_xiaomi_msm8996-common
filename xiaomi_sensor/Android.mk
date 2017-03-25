ifeq ($(TARGET_BUILD_JAVA_SUPPORT_LEVEL),platform)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src/java)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := com.xiaomi.sensor

include $(BUILD_JAVA_LIBRARY)

endif
