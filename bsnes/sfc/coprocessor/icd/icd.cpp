#include <sfc/sfc.hpp>

namespace SuperFamicom {

ICD icd;
#include "gb-core.cpp"
#include "interface.cpp"
#include "io.cpp"
#include "boot-roms.cpp"
#include "serialization.cpp"

namespace GB {
  static auto hreset(GameBoyCore::Instance*) -> void {
    icd.ppuHreset();
  }

  static auto vreset(GameBoyCore::Instance*) -> void {
    icd.ppuVreset();
  }

  static auto icd_pixel(GameBoyCore::Instance*, uint8_t pixel) -> void {
    icd.ppuWrite(pixel);
  }

  static auto joyp_write(GameBoyCore::Instance*, uint8_t value) -> void {
    bool p14 = value & 0x10;
    bool p15 = value & 0x20;
    icd.joypWrite(p14, p15);
  }

  static auto read_memory(GameBoyCore::Instance*, uint16_t addr, uint8_t data) -> uint8_t {
    if(auto replace = icd.cheats.find(addr, data)) return replace();
    return data;
  }

  static auto rgb_encode(GameBoyCore::Instance*, uint8_t r, uint8_t g, uint8_t b) -> uint32_t {
    return r << 16 | g << 8 | b << 0;
  }

  static auto sample(GameBoyCore::Instance*, GameBoyCore::Sample* sample) -> void {
    float left  = sample->left  / 32768.0f;
    float right = sample->right / 32768.0f;
    icd.apuWrite(left, right);
  }

  static auto vblank(GameBoyCore::Instance*, int) -> void {
  }

  static auto log(GameBoyCore::Instance*, const char*, int) -> void {
  }
}

auto ICD::synchronizeCPU() -> void {
  if(clock >= 0) scheduler.resume(cpu.thread);
}

auto ICD::Enter() -> void {
  while(true) {
    scheduler.synchronize();
    icd.main();
  }
}

auto ICD::main() -> void {
  if(r6003 & 0x80) {
    auto clocks = gbcore.api.run(gameBoy);
    step(clocks >> 1);
  } else {  //DMG halted
    apuWrite(0.0, 0.0);
    step(128);
  }
  synchronizeCPU();
}

auto ICD::step(uint clocks) -> void {
  clock += clocks * (uint64_t)cpu.frequency;
}

//SGB1 uses the CPU oscillator (~2.4% faster than a real Game Boy)
//SGB2 uses a dedicated oscillator (same speed as a real Game Boy)
auto ICD::clockFrequency() const -> uint {
  return Frequency ? Frequency : system.cpuFrequency();
}

auto ICD::load() -> bool {
  information = {};

  if(!gbcore.open(configuration.superGameBoy.corePath)) return false;
  gbcore.api.randomSetEnabled(configuration.hacks.entropy != "None");
  gameBoy = gbcore.api.alloc();
  if(!gameBoy) return false;
  if(Frequency == 0) {
    gbcore.api.init(gameBoy, GameBoyCore::ModelSGB1);
    gbcore.api.loadBootROM(gameBoy, &SGB1BootROM[0], 256);
  } else {
    gbcore.api.init(gameBoy, GameBoyCore::ModelSGB2);
    gbcore.api.loadBootROM(gameBoy, &SGB2BootROM[0], 256);
  }
  gbcore.api.setSampleRateByClocks(gameBoy, 256);
  gbcore.api.setHighpassFilterMode(gameBoy, GameBoyCore::HighpassAccurate);
  gbcore.api.setHresetCallback(gameBoy, &GB::hreset);
  gbcore.api.setVresetCallback(gameBoy, &GB::vreset);
  gbcore.api.setPixelCallback(gameBoy, &GB::icd_pixel);
  gbcore.api.setJoypWriteCallback(gameBoy, &GB::joyp_write);
  gbcore.api.setReadMemoryCallback(gameBoy, &GB::read_memory);
  gbcore.api.setRgbEncodeCallback(gameBoy, &GB::rgb_encode);
  gbcore.api.setSampleCallback(gameBoy, &GB::sample);
  gbcore.api.setVblankCallback(gameBoy, &GB::vblank);
  gbcore.api.setLogCallback(gameBoy, &GB::log);
  gbcore.api.setPixelsOutput(gameBoy, &bitmap[0]);
  if(auto loaded = platform->load(ID::GameBoy, "Game Boy", "gb")) {
    information.pathID = loaded.pathID;
  } else return unload(), false;
  if(auto fp = platform->open(pathID(), "manifest.bml", File::Read, File::Required)) {
    auto manifest = fp->reads();
    cartridge.slotGameBoy.load(manifest);
  } else return unload(), false;
  if(auto fp = platform->open(pathID(), "program.rom", File::Read, File::Required)) {
    auto size = fp->size();
    auto data = (uint8_t*)malloc(size);
    cartridge.information.sha256 = Hash::SHA256({data, (uint64_t)size}).digest();
    fp->read(data, size);
    gbcore.api.loadROM(gameBoy, data, size);
    free(data);
  } else return unload(), false;
  if(auto fp = platform->open(pathID(), "save.ram", File::Read)) {
    auto size = fp->size();
    auto data = (uint8_t*)malloc(size);
    fp->read(data, size);
    gbcore.api.loadBattery(gameBoy, data, size);
    free(data);
  }
  return true;
}

auto ICD::save() -> void {
  if(!gameBoy) return;
  if(auto size = gbcore.api.saveBatterySize(gameBoy)) {
    auto data = (uint8_t*)malloc(size);
    gbcore.api.saveBattery(gameBoy, data, size);
    if(auto fp = platform->open(pathID(), "save.ram", File::Write)) {
      fp->write(data, size);
    }
    free(data);
  }
}

auto ICD::unload() -> void {
  save();
  if(gameBoy) {
    gbcore.api.free(gameBoy);
    gbcore.api.dealloc(gameBoy);
    gameBoy = nullptr;
  }
}

auto ICD::power(bool reset) -> void {
  auto frequency = clockFrequency() / 5;
  create(ICD::Enter, frequency);
  if(!reset) stream = Emulator::audio.createStream(2, frequency / 128);

  for(auto& packet : this->packet) packet = {};
  packetSize = 0;

  joypID = 0;
  joypLock = 1;
  pulseLock = 1;
  strobeLock = 0;
  packetLock = 0;
  joypPacket = {};
  packetOffset = 0;
  bitData = 0;
  bitOffset = 0;

  for(auto& n : output) n = 0xff;
  readBank = 0;
  readAddress = 0;
  writeBank = 0;

  r6003 = 0x00;
  r6004 = 0xff;
  r6005 = 0xff;
  r6006 = 0xff;
  r6007 = 0xff;
  for(auto& r : r7000) r = 0x00;
  mltReq = 0;

  hcounter = 0;
  vcounter = 0;

  gbcore.api.reset(gameBoy);
}

}
