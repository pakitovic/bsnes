name := bsnes_libretro
local := false
openmp := true
flags += -Wno-narrowing -Wno-multichar -g -fPIC

#a libretro core must ship as a single file, so the Game Boy core is linked in
#statically (built from the SameBoy repository with: make lib BSNES=1)
sameboy.lib ?= ../../SameBoy/build/lib/libsameboy.a
flags += -DGB_CORE_BUILTIN
options += $(sameboy.lib)

ifeq ($(platform), ios-arm64)
	flags += -fPIC -miphoneos-version-min=11.0 -Wno-error=implicit-function-declaration -DHAVE_POSIX_MEMALIGN
	options += -dynamiclib
else ifeq ($(platform), tvos-arm64)
	flags += -fPIC -mtvos-version-min=11.0 -Wno-error=implicit-function-declaration -DHAVE_POSIX_MEMALIGN
	options += -dynamiclib
endif

objects := libretro $(objects)
objects := $(patsubst %,obj/%.o,$(objects))

obj/libretro.o: target-libretro/libretro.cpp

all: $(objects)
ifeq ($(platform),linux)
	$(strip $(compiler) -o out/$(name).so -shared $(objects) -Wl,--no-undefined -Wl,--version-script=target-libretro/link.T -lgomp -Wl,-Bdynamic $(options))
else ifeq ($(platform),haiku)
	$(strip $(compiler) -o out/$(name).so -shared $(objects) -Wl,--no-undefined -Wl,--version-script=target-libretro/link.T -lgomp -Wl,-Bdynamic $(options))
else ifeq ($(platform),windows)
	$(strip $(compiler) -o out/$(name).dll -shared $(objects) -Wl,--no-undefined -Wl,--version-script=target-libretro/link.T -lgomp -Wl,-Bdynamic $(options))
else ifeq ($(platform),macos)
	$(strip $(compiler) -o out/$(name).dylib -shared $(objects) $(options))
else ifeq ($(platform), ios-arm64)
    ifeq ($(IOSSDK),)
       IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
    endif
	$(strip c++ -arch arm64 -marm -miphoneos-version-min=11.0 -isysroot $(IOSSDK) -o out/$(name)_ios.dylib -shared $(objects) -lpthread -ldl)
else ifeq ($(platform), tvos-arm64)
    ifeq ($(IOSSDK),)
       IOSSDK := $(shell xcodebuild -version -sdk appletvos Path)
    endif
	$(strip c++ -arch arm64 -marm -mtvos-version-min=11.0 -isysroot $(IOSSDK) -o out/$(name)_tvos.dylib -shared $(objects) -lpthread -ldl)
endif
