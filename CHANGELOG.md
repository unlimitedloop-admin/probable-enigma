# Changelog
All notable changes to this project will be documented in this file.

---
## [0.0.0] - 2025/06/16
- The RetroEngine project was launched.

---
## [0.0.1] - 2025/09/16
- Initial release of mm2hack v0.0.1.
### Added
- Implements the core application components.
- Basic user interface for interaction.
  - Input device management
  - Graphics/BG/Sound management
  - FPS control
  - Time management
  - Screenshot function
  - Save/load function
### Known Issues
- Does not support full screen mode.
- Multi-monitor setups may cause window centering issues.
- Some DirectInput controllers may not be recognized correctly.

---
## [0.0.2] - 2026/09/12
- Added basic gameplay logic, including a recreation of the player behavior from Mega Man 2.
### Added
- Player actions and effects
- Entity management
- Updated stage-map and scrolling systems
- Deterministic save states
- DemoStage2 state restoration
- BGM and continuous SE restoration
- NES-style audio voice arbitration
- Save-file integrity and compatibility validation
- Headless save-state tests
### Known Issues
- Some gameplay constants remain hard-coded, including the player's projectile limit.
- Migration of stage definition (`.def`) data, including stage starting positions, is incomplete.
- Charged shots cannot be fired in a chosen direction while the player is in `LadderingState`.
- Some debug-menu entries, validation rules, and persistence behavior remain hard-coded or incomplete.
- Some stage-navigation checks still depend on the legacy scroll-type definitions.
- Stage completion triggers, item acquisition, and enemy-damage handling are not yet implemented.
