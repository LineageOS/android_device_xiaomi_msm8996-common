ifneq ($(strip $(USE_DEVICE_SPECIFIC_CAMERA)),true)
include $(call all-subdir-makefiles)
endif
