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

---
## [0.0.3] - 2026/11/15
- Added the first combat loop: enemies, hit detection, player damage, the miss sequence and lives.
### Added
- Entity-vs-entity hit detection with per-weapon damage resistance.
- Data-driven enemies, starting with Met (Metall): animation state graph, per-kind abilities and palette presets in JSON, and a deterministic alternating attack pattern.
- Enemy placement from the stage definition (`.def`), with per-placement respawn rules.
- Helmet deflection: shots bounce off a hidden Met, switch to a dedicated bounced graphic and no longer collide.
- Player damage: knockback, invincibility frames and contact damage.
- Player vitality meter HUD.
- Miss sequence (0 HP or falling into a pit) and stage restart from READY.
- Lives (remain), kept across restarts. The HUD shows the number of spare lives.
- Sound effects and effects for hits, destruction, enemy fire, damage taken and deflection.
- Debug tools: hit box overlay (HUD menu) and the SPRITE TEST screen (backdoor menu).
### Changed
- The player's start position is read from the stage definition (`.def`).
- A fixed page scroll now clears enemies as well as shots and effects.
- Sound effects can be paused and resumed on their own while the BGM keeps playing.
- Save data from earlier versions can no longer be loaded. Save compatibility with older versions is not guaranteed while the application is under development, and this applies to future versions as well.
### Fixed
- Sound effect pause could leave the BGM voices it used muted.
- Palette-animated background tiles did not fade with the rest of the background.
- A crash when entities were spawned while iterating over them.
- Enemy shots used up the player's own on-screen shot limit.
- Save-state restoration of projectile appearances, paused BGM and enemy animation states.
### Known Issues
- GAME OVER is not implemented. The stage restarts even when no lives remain.
- There is no way to gain extra lives yet (1UP items, score thresholds).
- Enemies have no invulnerability window after taking a hit.
- Weapon switching is not implemented. The HUD always shows the Rock Buster.
- Some gameplay constants remain hard-coded, including the player's projectile limit.
- Migration of stage definition (`.def`) data is incomplete. Player start and enemy placements are migrated; other entity types are not.
- Charged shots cannot be fired in a chosen direction while the player is in `LadderingState`.
- Some debug-menu entries, validation rules, and persistence behavior remain hard-coded or incomplete.
- Some stage-navigation checks still depend on the legacy scroll-type definitions.
- Stage completion triggers and item acquisition are not yet implemented.
