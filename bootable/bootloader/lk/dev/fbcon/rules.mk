LOCAL_DIR := $(GET_LOCAL_DIR)

OBJS += \
	$(LOCAL_DIR)/fbcon.o

#++ lk menu  TN:peter	
ifeq ($(TINNO_LK_MENU),1)	
OBJS += \
	$(LOCAL_DIR)/cfb_console.o 
endif
#-- lk menu
