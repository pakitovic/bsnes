auto ICD::serialize(serializer& s) -> void {
  Thread::serialize(s);

  auto size = gbcore.api.saveStateSize(gameBoy);
  auto data = new uint8_t[size];
  if(s.mode() == serializer::Save) {
    gbcore.api.saveState(gameBoy, data);
  }
  s.array(data, size);
  if(s.mode() == serializer::Load) {
    if(gbcore.api.loadState(gameBoy, data, size) != 0) {
      //a state from a different core build may pass bsnes' size check yet still be rejected here
      gbcore.api.reset(gameBoy);
      print("ICD: Game Boy state incompatible with the loaded core; the Game Boy was reset\n");
    }
  }
  delete[] data;

  for(auto n : range(64)) s.array(packet[n].data);
  s.integer(packetSize);

  s.integer(joypID);
  s.integer(joypLock);
  s.integer(pulseLock);
  s.integer(strobeLock);
  s.integer(packetLock);
  s.array(joypPacket.data);
  s.integer(packetOffset);
  s.integer(bitData);
  s.integer(bitOffset);

  s.array(output);
  s.integer(readBank);
  s.integer(readAddress);
  s.integer(writeBank);

  s.integer(r6003);
  s.integer(r6004);
  s.integer(r6005);
  s.integer(r6006);
  s.integer(r6007);
  s.array(r7000);
  s.integer(mltReq);

  s.integer(hcounter);
  s.integer(vcounter);
}
