# Release Notes

## Common Information

### System Requirements
- OS: Windows 10 or later
- CPU: Intel Core i3 or equivalent
- RAM: 4 GB
- GPU: DirectX 11 compatible
- Storage: 500 MB available space
- Input: Keyboard and USB PC Joycard (Xbox Controller, DirectInput Controller supported)
- Runtime: Visual C++ Redistributable for Visual Studio 2015, 2017, 2019, and 2022

### Installation / Update Instructions
1. Download the ZIP file from the link above.
2. Extract the contents of the ZIP file to a folder of your choice.
3. Run the `mm2hack.exe` (or `mm2hack_x86.exe`) file to start the application.
  - Caution : Already existing `mm2hack.ini` file will not be overwritten. Delete or rename it if you want to reset the configuration.

### Troubleshooting
- If you don't start the application, make sure you have the required Visual C++ Redistributable installed.
- Uncontrollable input behavior may occur when using multiple input devices. Try disconnecting unnecessary devices.
- If you encounter crashes or freezes, try running the application as an administrator.
- For further assistance, please visit the [GitHub Issues page](https://github.com/unlimitedloop-admin/probable-enigma/issues).

---

## v0.0.2 - 2026/09/11

### Highlights
- ✨ Added basic gameplay logic, including a recreation of the player behavior from Mega Man 2.
- 🎮 Implemented player actions and effects, entity management, updated stage-map and scrolling systems, deterministic save states, DemoStage2 state restoration, BGM and continuous SE restoration, NES-style audio voice arbitration, save-file integrity and compatibility validation, and headless save-state tests.

### Downloads
- 🪟 Windows (ZIP): [probable-enigma_mm2hack_demo_v002.zip](https://www.loopunlimited-rootone.com/50000/publisher/softwares/probable-enigma/probable-enigma_mm2hack_demo_v002.zip)
- ❌ Sorry for not supported Mac OS ;(
- 🐈‍⬛ Get the source code: [GitHub Repository](https://github.com/unlimitedloop-admin/probable-enigma)

### What's Changed (Summary)
- **Features**:
  - Added a debug menu for development and gameplay testing.
  - Recreated Mega Man's ground and underwater movement with over 99% behavioral fidelity.
  - Implemented page-based scrolling and eight-way scrolling.
  - Implemented a dash action modeled after the dash mechanic from Mega Man X.
- **Improvements**:
  - Redesigned the save/load algorithms for deterministic state restoration.
  - Overhauled the sound driver and NES-style audio-channel arbitration.
  - Redesigned the stage-map binary format.
  - Revised the C++ coding standards.

### Known Issues
- Some gameplay constants remain hard-coded, including the player's projectile limit.
- Migration of stage definition (`.def`) data, including stage starting positions, is incomplete.
- Charged shots cannot be fired in a chosen direction while the player is in `LadderingState`.
- Some debug-menu entries, validation rules, and persistence behavior remain hard-coded or incomplete.
- Some stage-navigation checks still depend on the legacy scroll-type definitions.
- Stage completion triggers, item acquisition, and enemy-damage handling are not yet implemented.

### Checksums
- 897373FD764E38004F7BFDF6711C4D72780C2F1B1DCBD326AAB620716C4BD342  *probable-enigma_mm2hack_demo_v002.zip

---

## v0.0.1 - 2025/09/16

### Highlights
- ✨ Implemented basic functionality of the application.
- 🎮 Added some configuration options. (Input device, Graphics/BG/Sound, FPS, Time management, Screenshot, Save/Load)

### Downloads
- 🪟 Windows (ZIP): [probable-enigma_mm2hack_demo_v001.zip](https://www.loopunlimited-rootone.com/50000/publisher/softwares/probable-enigma/probable-enigma_mm2hack_demo_v001.zip)
- ❌ Sorry for not supported Mac OS ;(
- 🐈‍⬛ Get the source code: [GitHub Repository](https://github.com/unlimitedloop-admin/probable-enigma)

### What's Changed (Summary)
- **Features**:
  - XInput, DirectInput, Keyboard/Mouse input support. Button configuration available and save to static ini file.
  - Get a screenshot by pressing F12 key. Screenshot will be saved in the `screenshot` folder.
  - Added support for multiple monitors.
  - Added FPS limiter and V-Sync options. (Options > Graphics)
  - Added support for custom resolutions. (Not supported full screen mode)
- **Improvements**:
  - Lightweight and portable, no installation required.

### Known Issues
- Some users may experience input lag or performance issues on lower-end hardware.
- Fullscreen mode may not work correctly on some systems.
- Boot the window axis may not be centered on some multi-monitor setups.
- Some DirectInput controllers may not be recognized correctly.

### Checksums
- 75B68BF1A05EC4716DB18BADF42F30BFCF7F8B3716FB9775E929B7430D4853C3  *probable-enigma_mm2hack_demo_v001.zip

---

## Credits
- Developed by Loop Unlimited (https://www.loopunlimited-rootone.com)
- External Libraries:
  - [DxLib](https://dxlib.xsrv.jp/) - A game development library for C/C++.
  - [nlohmann/json](https://github.com/nlohmann/json) - A JSON library for C++.
- Special thanks to all beta testers and contributors! Your feedback and support are greatly appreciated :)
