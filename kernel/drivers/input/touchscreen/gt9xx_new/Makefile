#gt915 touchpanel driver

ifeq ($(strip $(PROJECT_NAME)),blu_us)
ccflags-y +=	-DCONFIG_L5221BP_PROJECT
endif

obj-$(CONFIG_GT9XX_TOUCHPANEL_DRIVER)	+= gt9xx.o
#gt915 update file
obj-$(CONFIG_GT9XX_TOUCHPANEL_UPDATE)	+= gt9xx_update.o
#debug tool
obj-$(CONFIG_GT9XX_TOUCHPANEL_DEBUG)	+= goodix_tool.o
