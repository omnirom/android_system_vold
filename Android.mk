LOCAL_PATH:= $(call my-dir)

common_cflags := \
	-Werror=format \
	-Wno-unused-parameter

ifneq ($(BOARD_VOLD_MAX_PARTITIONS),)
	common_cflags += -DVOLD_MAX_PARTITIONS=$(BOARD_VOLD_MAX_PARTITIONS)
endif

ifeq ($(BOARD_VOLD_EMMC_SHARES_DEV_MAJOR), true)
	common_cflags += -DVOLD_EMMC_SHARES_DEV_MAJOR
endif

ifeq ($(BOARD_VOLD_DISC_HAS_MULTIPLE_MAJORS), true)
	common_cflags += -DVOLD_DISC_HAS_MULTIPLE_MAJORS
endif

common_src_files := \
	VolumeManager.cpp \
	CommandListener.cpp \
	VoldCommand.cpp \
	NetlinkManager.cpp \
	NetlinkHandler.cpp \
	Volume.cpp \
	DirectVolume.cpp \
	Process.cpp \
	Ext4.cpp \
	Fat.cpp \
	Ntfs.cpp \
	Exfat.cpp \
	Loop.cpp \
	Devmapper.cpp \
	ResponseCode.cpp \
	CheckBattery.cpp \
	VoldUtil.c \
	fstrim.c \
	cryptfs.c

common_c_includes := \
	system/extras/ext4_utils \
	system/extras/f2fs_utils \
	external/openssl/include \
	external/stlport/stlport \
	bionic \
	external/scrypt/lib/crypto \
	frameworks/native/include \
	system/security/keystore \
	hardware/libhardware/include/hardware \
	system/security/softkeymaster/include/keymaster \
	external/e2fsprogs/lib

common_shared_libraries := \
	libsysutils \
	libstlport \
	libbinder \
	libcutils \
	liblog \
	libdiskconfig \
	libhardware_legacy \
	liblogwrap \
	libext4_utils \
	libf2fs_sparseblock \
	libcrypto \
	libselinux \
	libutils \
	libhardware \
	libsoftkeymaster \
	libext2_blkid

common_static_libraries := \
	libfs_mgr \
	libscrypt_static \
	libmincrypt \
	libbatteryservice

include $(CLEAR_VARS)
LOCAL_MODULE := libvold
LOCAL_SRC_FILES := $(common_src_files)
LOCAL_C_INCLUDES := $(common_c_includes)
LOCAL_SHARED_LIBRARIES := $(common_shared_libraries)
LOCAL_STATIC_LIBRARIES := $(common_static_libraries)
LOCAL_CFLAGS := $(common_cflags)
LOCAL_MODULE_TAGS := eng tests
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= vold

LOCAL_SRC_FILES := \
	main.cpp \
	$(common_src_files)

LOCAL_C_INCLUDES := $(common_c_includes)
LOCAL_CFLAGS := $(common_cflags)
LOCAL_SHARED_LIBRARIES := $(common_shared_libraries)
LOCAL_STATIC_LIBRARIES := $(common_static_libraries)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= vdc.c
LOCAL_MODULE:= vdc
LOCAL_C_INCLUDES :=
LOCAL_CFLAGS := 
LOCAL_SHARED_LIBRARIES := libcutils
include $(BUILD_EXECUTABLE)
