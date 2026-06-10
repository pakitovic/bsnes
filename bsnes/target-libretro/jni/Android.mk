LOCAL_PATH := $(call my-dir)

SRCDIR := $(LOCAL_PATH)/../../..

# The Game Boy core is linked in statically (see GB_CORE_BUILTIN in
# sfc/coprocessor/icd/gb-core.cpp) and built from a SameBoy checkout;
# point SAMEBOY_DIR at it (defaults to a sibling of this repository).
SAMEBOY_DIR ?= $(SRCDIR)/../SameBoy
ifeq ($(wildcard $(SAMEBOY_DIR)/Core/gb.c),)
  $(error SameBoy sources not found at $(SAMEBOY_DIR); set SAMEBOY_DIR=/path/to/SameBoy)
endif
SAMEBOY_VERSION := $(shell sed -n 's/^VERSION := //p' $(SAMEBOY_DIR)/version.mk 2>/dev/null || echo unknown)

INCFLAGS  := -I$(SRCDIR) -I$(SRCDIR)/bsnes
COREFLAGS := -fomit-frame-pointer -ffast-math -D__LIBRETRO__ $(INCFLAGS)
COREFLAGS += -DPLATFORM_ANDROID -DGB_CORE_BUILTIN
# flags the SameBoy sources are built with (mirrors its 'make lib BSNES=1' preset)
COREFLAGS += -DGB_INTERNAL -DGB_DISABLE_DEBUGGER -DGB_DISABLE_CHEATS -DGB_DISABLE_CHEAT_SEARCH
COREFLAGS += -D_GNU_SOURCE -DGB_VERSION=\"$(SAMEBOY_VERSION)\"

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
				$(SAMEBOY_DIR)/Core/apu.c \
				$(SAMEBOY_DIR)/Core/camera.c \
				$(SAMEBOY_DIR)/Core/display.c \
				$(SAMEBOY_DIR)/Core/gb.c \
				$(SAMEBOY_DIR)/Core/joypad.c \
				$(SAMEBOY_DIR)/Core/mbc.c \
				$(SAMEBOY_DIR)/Core/memory.c \
				$(SAMEBOY_DIR)/Core/printer.c \
				$(SAMEBOY_DIR)/Core/random.c \
				$(SAMEBOY_DIR)/Core/rewind.c \
				$(SAMEBOY_DIR)/Core/rumble.c \
				$(SAMEBOY_DIR)/Core/save_state.c \
				$(SAMEBOY_DIR)/Core/sgb.c \
				$(SAMEBOY_DIR)/Core/sm83_cpu.c \
				$(SAMEBOY_DIR)/Core/timing.c \
				$(SRCDIR)/bsnes/processor/arm7tdmi/arm7tdmi.cpp \
				$(SRCDIR)/bsnes/processor/spc700/spc700.cpp \
				$(SRCDIR)/bsnes/processor/wdc65816/wdc65816.cpp

include $(CLEAR_VARS)
LOCAL_MODULE       := retro
LOCAL_SRC_FILES    := $(SRCFILES)
LOCAL_CPPFLAGS     := -std=c++17 $(COREFLAGS)
LOCAL_CFLAGS       := $(COREFLAGS)
LOCAL_LDFLAGS      := -Wl,-version-script=$(SRCDIR)/bsnes/target-libretro/link.T
LOCAL_CPP_FEATURES := exceptions rtti
include $(BUILD_SHARED_LIBRARY)
