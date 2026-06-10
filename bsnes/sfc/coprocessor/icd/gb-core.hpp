//GameBoyCore: bsnes' own interface for the Game Boy emulation core used by the
//Super Game Boy (ICD). The core is not compiled into bsnes: it is loaded at
//runtime from a dynamic library selected by the user (Settings -> Emulator ->
//Super Game Boy). Any library exporting this C ABI (the GB_* symbol family,
//API version 1.0.3) can serve as a core; the symbol names are resolved in
//gb-core.cpp and appear nowhere else.

struct GameBoyCore {
  struct Instance;  //opaque: allocated and owned by the loaded core

  //GB_model_t (ABI values)
  enum : int {
    ModelSGB1 = 0x084,  //Super Game Boy, SNES side handled by bsnes
    ModelSGB2 = 0x181,  //Super Game Boy 2, SNES side handled by bsnes
  };

  //GB_highpass_mode_t (ABI values)
  enum : int {
    HighpassOff      = 0,
    HighpassAccurate = 1,
  };

  //GB_sample_t (ABI layout)
  struct Sample {
    int16_t left;
    int16_t right;
  };

  using HresetCallback     = auto (*)(Instance*) -> void;
  using VresetCallback     = auto (*)(Instance*) -> void;
  using PixelCallback      = auto (*)(Instance*, uint8_t row) -> void;
  using JoypWriteCallback  = auto (*)(Instance*, uint8_t value) -> void;
  using ReadMemoryCallback = auto (*)(Instance*, uint16_t address, uint8_t data) -> uint8_t;
  using RgbEncodeCallback  = auto (*)(Instance*, uint8_t r, uint8_t g, uint8_t b) -> uint32_t;
  using SampleCallback     = auto (*)(Instance*, Sample*) -> void;
  using VblankCallback     = auto (*)(Instance*, int type) -> void;
  using LogCallback        = auto (*)(Instance*, const char* message, int attributes) -> void;

  //function table, resolved from the loaded library
  struct API {
    auto (*alloc)() -> Instance*;
    auto (*allocationSize)() -> size_t;
    auto (*dealloc)(Instance*) -> void;
    auto (*init)(Instance*, int model) -> Instance*;
    auto (*free)(Instance*) -> void;
    auto (*reset)(Instance*) -> void;
    auto (*run)(Instance*) -> uint;

    auto (*loadBootROM)(Instance*, const uint8_t* data, size_t size) -> void;
    auto (*loadROM)(Instance*, const uint8_t* data, size_t size) -> void;
    auto (*loadBattery)(Instance*, const uint8_t* data, size_t size) -> void;
    auto (*saveBatterySize)(Instance*) -> int;
    auto (*saveBattery)(Instance*, uint8_t* data, size_t size) -> int;

    auto (*setSampleRateByClocks)(Instance*, double cyclesPerSample) -> void;
    auto (*setHighpassFilterMode)(Instance*, int mode) -> void;

    auto (*setHresetCallback)(Instance*, HresetCallback) -> void;
    auto (*setVresetCallback)(Instance*, VresetCallback) -> void;
    auto (*setPixelCallback)(Instance*, PixelCallback) -> void;
    auto (*setJoypWriteCallback)(Instance*, JoypWriteCallback) -> void;
    auto (*setReadMemoryCallback)(Instance*, ReadMemoryCallback) -> void;
    auto (*setRgbEncodeCallback)(Instance*, RgbEncodeCallback) -> void;
    auto (*setSampleCallback)(Instance*, SampleCallback) -> void;
    auto (*setVblankCallback)(Instance*, VblankCallback) -> void;
    auto (*setLogCallback)(Instance*, LogCallback) -> void;
    auto (*setPixelsOutput)(Instance*, uint32_t* output) -> void;

    auto (*setJoyp)(Instance*, uint8_t value) -> void;

    auto (*saveStateSize)(Instance*) -> size_t;
    auto (*saveState)(Instance*, uint8_t* data) -> void;
    auto (*loadState)(Instance*, const uint8_t* data, size_t size) -> int;

    auto (*randomSetEnabled)(bool) -> void;
  } api = {};

  //gb-core.cpp
  auto open(string location) -> bool;
  auto close() -> void;
  auto loaded() const -> bool { return builtin || handle.open(); }

  string location;

private:
  bool builtin = false;  //true when bound to a statically linked core (":builtin:")
  library handle;
};

extern GameBoyCore gbcore;
