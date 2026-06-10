GameBoyCore gbcore;

#if defined(GB_CORE_BUILTIN)
//a libretro core must ship as a single file (and some libretro platforms have
//no dlopen at all), so there the Game Boy core is linked in statically and the
//":builtin:" location binds the table to it directly, with no dynamic loading.
extern "C" {
  auto GB_alloc() -> GameBoyCore::Instance*;
  auto GB_allocation_size() -> size_t;
  auto GB_dealloc(GameBoyCore::Instance*) -> void;
  auto GB_init(GameBoyCore::Instance*, int) -> GameBoyCore::Instance*;
  auto GB_free(GameBoyCore::Instance*) -> void;
  auto GB_reset(GameBoyCore::Instance*) -> void;
  auto GB_run(GameBoyCore::Instance*) -> uint;
  auto GB_load_boot_rom_from_buffer(GameBoyCore::Instance*, const uint8_t*, size_t) -> void;
  auto GB_load_rom_from_buffer(GameBoyCore::Instance*, const uint8_t*, size_t) -> void;
  auto GB_load_battery_from_buffer(GameBoyCore::Instance*, const uint8_t*, size_t) -> void;
  auto GB_save_battery_size(GameBoyCore::Instance*) -> int;
  auto GB_save_battery_to_buffer(GameBoyCore::Instance*, uint8_t*, size_t) -> int;
  auto GB_set_sample_rate_by_clocks(GameBoyCore::Instance*, double) -> void;
  auto GB_set_highpass_filter_mode(GameBoyCore::Instance*, int) -> void;
  auto GB_set_icd_hreset_callback(GameBoyCore::Instance*, GameBoyCore::HresetCallback) -> void;
  auto GB_set_icd_vreset_callback(GameBoyCore::Instance*, GameBoyCore::VresetCallback) -> void;
  auto GB_set_icd_pixel_callback(GameBoyCore::Instance*, GameBoyCore::PixelCallback) -> void;
  auto GB_set_joyp_write_callback(GameBoyCore::Instance*, GameBoyCore::JoypWriteCallback) -> void;
  auto GB_set_read_memory_callback(GameBoyCore::Instance*, GameBoyCore::ReadMemoryCallback) -> void;
  auto GB_set_rgb_encode_callback(GameBoyCore::Instance*, GameBoyCore::RgbEncodeCallback) -> void;
  auto GB_apu_set_sample_callback(GameBoyCore::Instance*, GameBoyCore::SampleCallback) -> void;
  auto GB_set_vblank_callback(GameBoyCore::Instance*, GameBoyCore::VblankCallback) -> void;
  auto GB_set_log_callback(GameBoyCore::Instance*, GameBoyCore::LogCallback) -> void;
  auto GB_set_pixels_output(GameBoyCore::Instance*, uint32_t*) -> void;
  auto GB_icd_set_joyp(GameBoyCore::Instance*, uint8_t) -> void;
  auto GB_get_save_state_size(GameBoyCore::Instance*) -> size_t;
  auto GB_save_state_to_buffer(GameBoyCore::Instance*, uint8_t*) -> void;
  auto GB_load_state_from_buffer(GameBoyCore::Instance*, const uint8_t*, size_t) -> int;
  auto GB_random_set_enabled(bool) -> void;
}
#endif

//resolves the entire function table from the library at `location`.
//every symbol is required: a partial core would crash mid-emulation otherwise.
auto GameBoyCore::open(string location) -> bool {
  if(loaded() && location == this->location) return true;
  close();

#if defined(GB_CORE_BUILTIN)
  if(location == ":builtin:") {
    api.alloc                 = GB_alloc;
    api.allocationSize        = GB_allocation_size;
    api.dealloc               = GB_dealloc;
    api.init                  = GB_init;
    api.free                  = GB_free;
    api.reset                 = GB_reset;
    api.run                   = GB_run;
    api.loadBootROM           = GB_load_boot_rom_from_buffer;
    api.loadROM               = GB_load_rom_from_buffer;
    api.loadBattery           = GB_load_battery_from_buffer;
    api.saveBatterySize       = GB_save_battery_size;
    api.saveBattery           = GB_save_battery_to_buffer;
    api.setSampleRateByClocks = GB_set_sample_rate_by_clocks;
    api.setHighpassFilterMode = GB_set_highpass_filter_mode;
    api.setHresetCallback     = GB_set_icd_hreset_callback;
    api.setVresetCallback     = GB_set_icd_vreset_callback;
    api.setPixelCallback      = GB_set_icd_pixel_callback;
    api.setJoypWriteCallback  = GB_set_joyp_write_callback;
    api.setReadMemoryCallback = GB_set_read_memory_callback;
    api.setRgbEncodeCallback  = GB_set_rgb_encode_callback;
    api.setSampleCallback     = GB_apu_set_sample_callback;
    api.setVblankCallback     = GB_set_vblank_callback;
    api.setLogCallback        = GB_set_log_callback;
    api.setPixelsOutput       = GB_set_pixels_output;
    api.setJoyp               = GB_icd_set_joyp;
    api.saveStateSize         = GB_get_save_state_size;
    api.saveState             = GB_save_state_to_buffer;
    api.loadState             = GB_load_state_from_buffer;
    api.randomSetEnabled      = GB_random_set_enabled;
    builtin = true;
    this->location = location;
    return api.allocationSize() != 0;
  }
#endif

  if(!location || !handle.openAbsolute(location)) {
    print("GameBoyCore: cannot open library: ", location, "\n");
    return false;
  }

  bool complete = true;
  auto resolve = [&](auto& slot, const char* name) {
    using T = std::decay_t<decltype(slot)>;
    slot = (T)handle.sym(name);
    if(!slot) {
      print("GameBoyCore: missing symbol '", name, "' in: ", location, "\n");
      complete = false;
    }
  };

  resolve(api.alloc,                 "GB_alloc");
  resolve(api.allocationSize,        "GB_allocation_size");
  resolve(api.dealloc,               "GB_dealloc");
  resolve(api.init,                  "GB_init");
  resolve(api.free,                  "GB_free");
  resolve(api.reset,                 "GB_reset");
  resolve(api.run,                   "GB_run");

  resolve(api.loadBootROM,           "GB_load_boot_rom_from_buffer");
  resolve(api.loadROM,               "GB_load_rom_from_buffer");
  resolve(api.loadBattery,           "GB_load_battery_from_buffer");
  resolve(api.saveBatterySize,       "GB_save_battery_size");
  resolve(api.saveBattery,           "GB_save_battery_to_buffer");

  resolve(api.setSampleRateByClocks, "GB_set_sample_rate_by_clocks");
  resolve(api.setHighpassFilterMode, "GB_set_highpass_filter_mode");

  resolve(api.setHresetCallback,     "GB_set_icd_hreset_callback");
  resolve(api.setVresetCallback,     "GB_set_icd_vreset_callback");
  resolve(api.setPixelCallback,      "GB_set_icd_pixel_callback");
  resolve(api.setJoypWriteCallback,  "GB_set_joyp_write_callback");
  resolve(api.setReadMemoryCallback, "GB_set_read_memory_callback");
  resolve(api.setRgbEncodeCallback,  "GB_set_rgb_encode_callback");
  resolve(api.setSampleCallback,     "GB_apu_set_sample_callback");
  resolve(api.setVblankCallback,     "GB_set_vblank_callback");
  resolve(api.setLogCallback,        "GB_set_log_callback");
  resolve(api.setPixelsOutput,       "GB_set_pixels_output");

  resolve(api.setJoyp,               "GB_icd_set_joyp");

  resolve(api.saveStateSize,         "GB_get_save_state_size");
  resolve(api.saveState,             "GB_save_state_to_buffer");
  resolve(api.loadState,             "GB_load_state_from_buffer");

  resolve(api.randomSetEnabled,      "GB_random_set_enabled");

  if(!complete || api.allocationSize() == 0) return close(), false;
  this->location = location;
  return true;
}

auto GameBoyCore::close() -> void {
  handle.close();
  api = {};
  builtin = false;
  location = "";
}
