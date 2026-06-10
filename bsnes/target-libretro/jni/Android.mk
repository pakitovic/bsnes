LOCAL_PATH := $(call my-dir)

SRCDIR := $(LOCAL_PATH)/../../..

# The Game Boy core is linked in statically (see GB_CORE_BUILTIN in
# sfc/coprocessor/icd/gb-core.cpp). Pass the path to any static library
# implementing the GB_* core API, cross-built for the target ABI:
#   ndk-build GBCORE_LIB=/path/to/$(TARGET_ARCH_ABI)/libgbcore.a
# (with multiple APP_ABIs, embed $(TARGET_ARCH_ABI) in the path so each
# ABI picks its own build of the library)
ifeq ($(GBCORE_LIB),)
  $(error GBCORE_LIB must point to a static Game Boy core library built for the target ABI, e.g. ndk-build GBCORE_LIB=/path/to/libgbcore.a)
endif

include $(CLEAR_VARS)
LOCAL_MODULE    := gbcore
LOCAL_SRC_FILES := $(GBCORE_LIB)
include $(PREBUILT_STATIC_LIBRARY)

INCFLAGS  := -I$(SRCDIR) -I$(SRCDIR)/bsnes
COREFLAGS := -fomit-frame-pointer -ffast-math -D__LIBRETRO__ $(INCFLAGS)
COREFLAGS += -DGB_CORE_BUILTIN
# nall specializes std::is_signed/is_unsigned for its Natural/Integer types,
# which newer libc++ (NDK r29+) rejects by default
COREFLAGS += -Wno-invalid-specialization

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

SRCFILES := $(SRCDIR)/bsnes/target-libretro/libretro.cpp \
				$(SRCDIR)/bsnes/emulator/emulator.cpp \
				$(SRCDIR)/libco/libco.c \
				$(SRCDIR)/bsnes/filter/filter.cpp \
				$(SRCDIR)/bsnes/lzma/lzma.cpp \
				$(SRCDIR)/bsnes/sfc/interface/interface.cpp \
				$(SRCDIR)/bsnes/sfc/system/system.cpp \
				$(SRCDIR)/bsnes/sfc/controller/controller.cpp \
				$(SRCDIR)/bsnes/sfc/cartridge/cartridge.cpp \
				$(SRCDIR)/bsnes/sfc/memory/memory.cpp \
				$(SRCDIR)/bsnes/sfc/cpu/cpu.cpp \
				$(SRCDIR)/bsnes/sfc/smp/smp.cpp \
				$(SRCDIR)/bsnes/sfc/dsp/dsp.cpp \
				$(SRCDIR)/bsnes/sfc/ppu/ppu.cpp \
				$(SRCDIR)/bsnes/sfc/ppu-fast/ppu.cpp \
				$(SRCDIR)/bsnes/sfc/expansion/expansion.cpp \
				$(SRCDIR)/bsnes/sfc/coprocessor/coprocessor.cpp \
				$(SRCDIR)/bsnes/sfc/slot/slot.cpp \
				$(SRCDIR)/bsnes/processor/arm7tdmi/arm7tdmi.cpp \
				$(SRCDIR)/bsnes/processor/spc700/spc700.cpp \
				$(SRCDIR)/bsnes/processor/wdc65816/wdc65816.cpp

include $(CLEAR_VARS)
LOCAL_MODULE          := retro
LOCAL_SRC_FILES       := $(SRCFILES)
LOCAL_CPPFLAGS        := -std=c++17 $(COREFLAGS)
LOCAL_CFLAGS          := $(COREFLAGS)
LOCAL_LDFLAGS         := -Wl,-version-script=$(SRCDIR)/bsnes/target-libretro/link.T
LOCAL_CPP_FEATURES    := exceptions rtti
LOCAL_STATIC_LIBRARIES := gbcore
include $(BUILD_SHARED_LIBRARY)
