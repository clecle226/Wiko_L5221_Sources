LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LK_TOP_DIR)/platform/msm_shared/include

DEFINES += ASSERT_ON_TAMPER=1

OBJS += \
	$(LOCAL_DIR)/aboot.o \
	$(LOCAL_DIR)/fastboot.o \
	$(LOCAL_DIR)/recovery.o 
	
#++ lk menu  TN:peter
ifeq ($(TINNO_LK_MENU),1)
OBJS += \
	$(LOCAL_DIR)/hide_power_key_func.o
endif
#-- lk menu
