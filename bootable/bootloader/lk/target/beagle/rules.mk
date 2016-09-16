LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += \
	-I$(LOCAL_DIR)/include

PLATFORM := omap3

MODULES += \
	dev/pmic/twl4030

MEMSIZE := 0x08000000	# 128MB

DEFINES += \
	SDRAM_SIZE=$(MEMSIZE)
