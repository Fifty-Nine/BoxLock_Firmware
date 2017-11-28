CC:=arm-none-eabi-gcc
CXX:=arm-none-eabi-g++
LD:=arm-none-eabi-g++
GDB:=arm-none-eabi-gdb

all: firmware.elf

include config.mk

LINKER_SCRIPT:=samd21e18a_flash.ld
COMMON_FLAGS+= \
	-D__SAMD21E18A__ \
	-I$(CMSIS_INSTALL_PATH)/CMSIS/Core/Include \
	-I$(DFP_INSTALL_PATH)/include \
	-IRTOS \
	-I$(FREERTOS_INSTALL_PATH)/FreeRTOS/Source/include \
	-I$(FREERTOS_INSTALL_PATH)/FreeRTOS/Source/portable/GCC/ARM_CM0 \
	-Ipersistent_storage \
	-Iusb/ \
	-Iusb/device \
	-Iusb/class/cdc \
	-Iusb/class/cdc/device \
	-Iconfig \
	-isystem hal/include \
	-isystem hal/utils/include \
	-isystem hri \
	-isystem hpl/core \
	-isystem hpl/gclk \
	-isystem hpl/port \
	-isystem hpl/pm \
	-isystem hpl/tcc \
	-funsigned-char \
	-Wall \
	-Werror \
	-mthumb \
	-mcpu=cortex-m0plus \
	-ffunction-sections \
	-fdata-sections \
	-fshort-enums \
	-DBOARD_REV=$(BOARD_REV) \

CFLAGS+= \
	$(COMMON_FLAGS) \
	-std=gnu11 \

CXXFLAGS+= \
	$(COMMON_FLAGS) \
	-std=gnu++14 \
	-fno-exceptions \
	-fno-unwind-tables \
	-fno-rtti \

LDFLAGS+= \
	$(COMMON_FLAGS) \
	-T$(LINKER_SCRIPT) \
	-Wl,-gc-sections \

CMSIS_OBJS:= \
	$(DFP_INSTALL_PATH)/gcc/gcc/startup_samd21.o \
	$(DFP_INSTALL_PATH)/gcc/system_samd21.o \

HPL_OBJS:= \
	hpl/core/hpl_init.o \
	hpl/gclk/hpl_gclk.o \
	hpl/nvmctrl/hpl_nvmctrl.o \
	hpl/pm/hpl_pm.o \
	hpl/sysctrl/hpl_sysctrl.o \
	hpl/tcc/hpl_tcc.o \
	hpl/usb/hpl_usb.o \

HAL_OBJS:= \
	hal/src/hal_atomic.o \
	hal/src/hal_flash.o \
	hal/src/hal_pwm.o \
	hal/src/hal_usb_device.o \
	hal/utils/src/utils_list.o \

PERSISTENT_STORAGE_OBJS:= \
	persistent_storage/nv_internal_flash_ultra.o \
	persistent_storage/nv_storage.o \

USB_OBJS:= \
	usb/class/cdc/device/cdcdf_acm.o \
	usb/usb_protocol.o \
	usb/device/usbdc.o \

RTOS_OBJS:= \
	$(FREERTOS_INSTALL_PATH)/FreeRTOS/Source/tasks.o \
	$(FREERTOS_INSTALL_PATH)/FreeRTOS/Source/list.o \
	$(FREERTOS_INSTALL_PATH)/FreeRTOS/Source/portable/GCC/ARM_CM0/port.o \
	$(FREERTOS_INSTALL_PATH)/FreeRTOS/Source/timers.o \
	$(FREERTOS_INSTALL_PATH)/FreeRTOS/Source/queue.o \
	$(FREERTOS_INSTALL_PATH)/FreeRTOS/Source/event_groups.o \

OBJDIR?=objs
OBJS:= \
	$(CMSIS_OBJS) \
	$(HPL_OBJS) \
	$(HAL_OBJS) \
	$(PERSISTENT_STORAGE_OBJS) \
	$(USB_OBJS) \
	$(RTOS_OBJS) \
	managed_task.o \
	nvmem.o \
	usb.o \
	linenoise.o \
	utility.o \
	lock_control.o \
	console.o \
	mtb.o \
	keypad.o \
	faults.o \
	syscalls.o \
	rtos_support.o \
	mcu.o \
	sleep.o \
	main.o

OBJDIR_OBJS:=$(OBJS:%.o=$(OBJDIR)/%.o)

all: firmware.elf

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(shell dirname $@)
	$(CXX) -c $(CXXFLAGS) -MMD -o $@ $<

$(OBJDIR)/%.o: %.c
	mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) -MMD -o $@ $<

$(OBJDIR_OBJS): Makefile config.mk

-include $(OBJDIR_OBJS:%.o=%.d)

clean:
	rm -rf $(OBJDIR) firmware.elf firmware.map

firmware.elf: $(OBJDIR_OBJS) $(LINKER_SCRIPT)
	$(LD) -Wl,-Map="firmware.map" -o $@ $(LDFLAGS) $(OBJDIR_OBJS)

$(OBJDIR)/uploaded: firmware.elf
	$(GDB) \
		-batch \
		-ex 'target extended-remote localhost:3333' \
		-ex 'monitor reset halt' \
		-ex 'monitor at91samd chip-erase' \
		-ex 'load' \
		-ex 'monitor reset halt' \
		firmware.elf
	touch $@

upload: $(OBJDIR)/uploaded

debug: upload
	$(GDB) -ex 'target extended-remote localhost:3333' firmware.elf
