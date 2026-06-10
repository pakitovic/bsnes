bsnes
=====

![bsnes logo](bsnes/target-bsnes/resource/logo.png)

bsnes is a multi-platform Super Nintendo (Super Famicom) emulator, originally
developed by Near, which focuses on performance,
features, and ease of use.

Unique Features
---------------

  - True Super Game Boy emulation (via an interchangeable Game Boy core library)
  - HD mode 7 graphics with optional supersampling (by DerKoun)
  - Low-level emulation of all SNES coprocessors (DSP-n, ST-01n, Cx4)
  - Multi-threaded PPU graphics renderer
  - Speed mode settings which retain smooth audio output (50%, 75%, 100%, 150%, 200%)
  - Built-in games database with thousands of game entries
  - Built-in cheat code database for hundreds of popular games (by mightymo)
  - Built-in save state manager with screenshot previews and naming capabilities
  - Customizable per-byte game mappings to support any cartridges, including prototype games
  - 7-zip decompression support
  - Extensive Satellaview emulation, including BS Memory flash write and wear-leveling emulation
  - Optional higan game folder support (standard game ROM files are also fully supported!)
  - Advanced mapping system allowing multiple bindings to every emulated input

Standard Features
-----------------

  - MSU1 support
  - BPS and IPS soft-patching support
  - Save states with undo and redo support (for reverting accidental saves and loads)
  - OpenGL multi-pass pixel shaders
  - Several built-in software filters, including HQ2x (by MaxSt) and snes_ntsc (by blargg)
  - Adaptive sync and dynamic rate control for perfect audio/video synchronization
  - Just-in-time input polling for minimal input latency
  - Run-ahead support for removing internal game engine input latency
  - Support for Direct3D exclusive mode video
  - Support for WASAPI exclusive mode audio
  - Periodic auto-saving of game saves
  - Auto-saving of states when unloading games, and auto-resuming of states when reloading games
  - Sprite limit disable support
  - Cubic audio interpolation support
  - Optional high-level emulation of most SNES coprocessors
  - Optional emulation of flaws in older emulators for compatibility with older unofficial software
  - CPU, SA1, and SuperFX overclocking support
  - Frame advance support
  - Screenshot support
  - Cheat code search support
  - Movie recording and playback support
  - Rewind support
  - HiDPI support
  - Multi-monitor support
  - Turbo support for controller inputs

Game Boy Core
-------------

bsnes does not bundle a Game Boy emulator for the Super Game Boy: it uses an
external core implementing the `GB_*` core C API.

  - **Standalone**: the core is loaded at runtime from a dynamic library
    (`*.dylib` / `*.so` / `*.dll`). Place one or more in a `Firmware` folder
    (inside the app bundle, or in the user data directory) and select one
    under Settings → Emulator → Super Game Boy.
  - **libretro**: a libretro core must ship as a single file, so the Game Boy
    core is linked in statically. Pass a static library when building:

        make -C bsnes target=libretro gbcore.lib=/path/to/libgbcore.a

    For the Android build: `ndk-build GBCORE_LIB=/path/to/libgbcore.a`
    (cross-built for each target ABI).

Links
-----

  - [Official git repository](https://github.com/bsnes-emu/bsnes)
  - [Official Discord](https://discord.gg/B27hf27ZVf)

Nightly Builds
--------------

  - [Windows](https://github.com/bsnes-emu/bsnes/releases/download/nightly/bsnes-windows.zip)
  - [macOS](https://github.com/bsnes-emu/bsnes/releases/download/nightly/bsnes-macos.zip)
  - [Linux](https://github.com/bsnes-emu/bsnes/releases/download/nightly/bsnes-ubuntu.zip)
  - [FreeBSD](https://api.cirrus-ci.com/v1/artifact/github/bsnes-emu/bsnes/freebsd-x86_64-binaries/bsnes-nightly/bsnes-nightly.zip)

Preview
-------

![bsnes user interface](.assets/user-interface.png)
![bsnes running Bahamut Lagoon](.assets/bahamut-lagoon.png)
![bsnes running Tengai Makyou Zero](.assets/tengai-makyou-zero.png)
