# SDL2 Nintendo Switch

SOURCE_ROOT := .
BUILD_DIR := $(SOURCE_ROOT)/build/

ifeq ($(strip $(LIBTRANSISTOR_HOME)),)
$(error "Please set LIBTRANSISTOR_HOME in your environment. export LIBTRANSISTOR_HOME=<path to libtransistor>")
endif

include $(LIBTRANSISTOR_HOME)/libtransistor.mk

SDL2: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) install

# "LDFLAGS=-L$(LIBTRANSISTOR_HOME)/build/newlib/aarch64-none-switch/newlib/ -lc -lm" \

$(BUILD_DIR)/Makefile:
	mkdir -p $(@D)
	cd $(@D); $(realpath $(SOURCE_ROOT))/configure \
		"CFLAGS=$(CFLAGS)" \
		"CC=$(CC)" "AR=$(AR)" \
		"RANLIB=$(RANLIB)" \
		--host=aarch64-none-switch \
		--disable-audio \
		--enable-joystick \
		--disable-power \
		--disable-filesystem \
		--disable-threads \
		--disable-cpuinfo \
		--enable-timers \
		--enable-video \
		--disable-shared \
		--enable-static \
		--prefix=$(LIBTRANSISTOR_HOME)

clean:
	rm -rf $(BUILD_DIR)
