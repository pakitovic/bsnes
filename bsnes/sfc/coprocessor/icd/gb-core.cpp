GameBoyCore gbcore;

//resolves the entire function table from the library at `location`.
//every symbol is required: a partial core would crash mid-emulation otherwise.
auto GameBoyCore::open(string location) -> bool {
  if(loaded() && location == this->location) return true;
  close();

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
  location = "";
}
