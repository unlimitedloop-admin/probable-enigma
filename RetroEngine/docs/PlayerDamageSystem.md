# Player damage system — staged implementation plan

## Context

The player currently cannot take damage at all: `PlayerEntity::OnEntityCollision()` is a no-op TODO (`PlayerEntity.cpp:339-343`), and the only way the player ever dies today is `OnTileCollision()`'s `TileAttribute::InstantDeath` check calling `Kill()` directly (`PlayerEntity.cpp:333`) — with zero reaction (no HUD, no VFX/SFX, `EntityManager` just silently sweeps the dead entity next tick). The user wants to build this out — vitality, hit reaction (knockback + SFX), a hit/damage table, invincibility frames, and miss handling — but explicitly wants it done as a sequence of small, independently testable steps rather than one big change, since a bug buried in a monolithic damage system would be hard to isolate.

Research this session found that **most of the underlying infrastructure already exists and is fully generic** — built for `EnemyEntity`/`BreakableBlockEntity` but with zero enemy-specific coupling:
- `combat::IDamageable` (`apps/systems/combat/IDamageable.h`) — interface any HP-bearing entity implements.
- `combat::HealthComponent` (`apps/systems/combat/HealthComponent.h/.cpp`) — reusable HP + per-weapon `DamageTable`, composed (not inherited) by the owner.
- `combat::DamageTable` (`apps/systems/combat/DamageTable.h`) — per-`WeaponId` percent-damage table; `Neutral()`/`Invincible()`/`SetPercent()`/`SetImmune()`.
- `WeaponId::EnemyShot` (`apps/systems/physics/WeaponId.h:23-26`) already carries a comment stating it was reserved specifically for this: *"The player isn't combat::IDamageable yet... reserved so that piece can land later without reworking anything upstream of it."* `ProjectileEntity` already tags every enemy shot with it and a real `AttackPower()`.
- `AbstractActionPhase`'s post-collision pipeline (`captureDamageableHp_`/`spawnDestructionEffectsForTheDead_`/`spawnHitEffectsForTheSurvivors_`, `AbstractActionPhase.cpp:760-842`) already detects HP deltas and death purely via `dynamic_cast<IDamageable*>` on every collider — no `EnemyEntity`-specific code. A `PlayerEntity : IDamageable` gets HP-delta/death *detection* for free, though its SE/VFX (`"hit_attack"`, `"enemy_small_explosion"`) are enemy-flavored and not meant for the player (see Phase 1 below).
- `AvatarStatus` (`AvatarStatus.h:16-32`) already reserves `Disabled=-1, Uncontrollable=0, Setback, Damaged` ahead of the real locomotion states — unregistered, unused, but clearly placeholders for exactly this feature.
- `PlayerEntity::_collidable`/`SetCollidable()`/`IsCollidable()` (`PlayerEntity.h:123,190`) already exists, is already save-stated, and `ICollider::IsCollidable()`'s own doc comment literally says `// Enabled/Disabled (e.g., switch traps, invincibility frames)` — built for this, never called.
- `renderSpriteId_()` (`PlayerEntity.cpp:521-542`) already does modulo-based sprite-id/frame swapping for the charge effect — the exact mechanical pattern an invincibility blink needs.
- `CollisionMatrix::Set(Player, ProjectileEnemy, ...)` is deliberately commented out (`CollisionLayer.cpp:16-22`) with a comment naming this exact moment as when to re-enable it.

Nothing player-specific exists yet: no HP field on `PlayerEntity`/`PlayerEntityState`, no knockback/invincibility tuning, no HUD, no death/miss/lives flow (`core/GameState.h` has no `GameOver`/`Miss` state; `GameStateManager` has no lives concept at all).

Per discussion, this lands in four phases. **This plan covers Phase 1 in implementation-ready detail; Phases 2-4 are scoped but will each get their own short design pass when we get there**, per the "one at a time" approach.

## Phase breakdown (build order)

1. **Vitality + projectile damage detection** — HP goes down when hit by an enemy shot. No knockback, no i-frames, no real HUD yet. *(this session's implementation target)*
2. **Invincibility frames** — reuse `_collidable`, add a blink render, prevent one overlapping touch from registering as N hits.
3. **Knockback + damage SFX** — wire `AvatarStatus::Setback` as a real `IPlayerState` (input-frozen, scripted recoil), fire a `PlayerEventType::Damaged` event for SFX/VFX.
4. **Miss/death handling** — lives, game-over/continue flow via `PhaseResult`, re-entering `ActionPhaseState::Intro`. Deferred: needs its own planning pass, not detailed here.

Explicitly deferred out of Phase 1 (per discussion): **enemy body contact damage** (touching Met directly). `EnemyEntity` doesn't implement `IAttackInfo` yet (`EnemyEntity.cpp:378-386`'s own comment marks this future work, independent of player-projectile damage) — that's its own later step, not bundled here.

---

## Phase 1 detail: vitality + projectile damage

### 1. `PlayerEntity` becomes `combat::IDamageable`

`apps/world/entity/avatar/PlayerEntity.h`:
- Add `, public systems::combat::IDamageable` to the class's base-class list (alongside whatever `ICollider` base it already has).
- Add a `systems::combat::HealthComponent _health{};` member, mirroring `EnemyEntity.h:241-244`'s `_health`/`_toughness` pattern — except the player has no "toughness" concept; construct it directly with a fixed max-HP tuning constant (see below) and `DamageTable::Neutral()` (only `WeaponId::EnemyShot` will ever reach the player in Phase 1; no special resistances yet).
- Implement the four `IDamageable` methods by delegating to `_health`, exactly like `EnemyEntity::ApplyAttack()`/`CurrentHP()`/`MaxHP()`/`IsDead()` (`EnemyEntity.h:216-219`, `EnemyEntity.cpp:391-394`).
- New tuning constant for starting/max HP — add it as a small dedicated struct (mirroring `AttackTuning`'s precedent of "small dedicated struct passed alongside `PlayerTuning`" rather than bloating `PlayerTuning` itself), e.g. `PlayerVitalityTuning` in `PlayerParams.h` with just `int maxHp` for now (room to grow in Phase 2/3 without touching `PlayerTuning`).

### 2. Persistence

`apps/world/entity/avatar/PlayerEntity.h`/`.cpp` (`PlayerEntityState`, see `PlayerEntity.cpp:64-88` for the existing shape):
- Add `hp` (int) to `PlayerEntityState`, save/load/validate it the same way `EnemyEntityState::hp` already does (`EnemyEntity.cpp:46,71,109`), and wire it through `CaptureState()`/`RestoreState()`.

### 3. Taking a hit

`apps/world/entity/avatar/PlayerEntity.cpp`, `OnEntityCollision()` (currently the no-op at line 339-343):
- Mirror `EnemyEntity::OnEntityCollision()`'s shape (`EnemyEntity.cpp:371-381`): early-out if `!IsAlive()`; `dynamic_cast<systems::physics::IAttackInfo*>(&other)`; if non-null, call `ApplyAttack(*attack)`.
- Do **not** call `Kill()` on a lethal result yet — that's Phase 4's job (miss handling). For Phase 1, just let `HealthComponent` clamp at 0 and stay alive; `IsDead()` will report true but nothing acts on it yet. (Worth a one-line comment noting this is deliberate/temporary, so it isn't mistaken for an oversight later.)
- Do not add knockback or invincibility here — Phase 1 is HP-tracking only.

### 4. Re-enable the collision pairing

`apps/systems/physics/CollisionLayer.cpp:12-22` — replace the "deliberately NOT set yet" comment block with `Set(CollisionLayer::Player, CollisionLayer::ProjectileEnemy, true);`, matching the existing `Set(Player, Enemy, true)`/`Set(Player, Item, true)` lines right above it.

### 5. Verification hook (no real HUD yet)

Since there is no HUD at all yet (confirmed greenfield — only `core::overlay::DebugHud` exists, an FPS/position debug overlay gated by `config::ConfigUIManager`), add the player's current/max HP as one more line to that existing debug overlay (`core/overlay/DebugHud.cpp`, same pattern as its existing player-position readout) so Phase 1 is actually verifiable in-game without waiting for the real vitality gauge (which is its own future UI task, not bundled here).

### 6. What Phase 1 deliberately does NOT touch

- `AvatarStatus::Setback`/`Damaged`, `PlayerStateMachine` — untouched (Phase 3).
- `_collidable`/`SetCollidable()` — untouched (Phase 2).
- `PlayerFrameOutput`/`PlayerEventType` — untouched (no new event yet; Phase 3 adds `Damaged` for SFX/VFX once there's a reaction worth announcing).
- `spawnHitEffectsForTheSurvivors_`/`spawnDestructionEffectsForTheDead_` in `AbstractActionPhase.cpp` — left as-is. They'll start firing their (enemy-flavored) SE for the player automatically once `IDamageable` lands; that's an acceptable, temporary placeholder reaction for Phase 1 (better than total silence for verifying the pipeline works), to be superseded by Phase 3's dedicated player SFX.

## Verification (Phase 1)

1. Build (`MSBuild RetroEngine\RetroEngine.sln /t:mm2hack /p:Configuration=Debug /p:Platform=x64`) — clean, no warnings, as every change this session.
2. Launch, enter a stage with Met, enable the debug HUD's HP readout, let Met's shot hit the player: confirm the HP number decrements by the shot's `AttackPower()` and the existing `"hit_attack"` SE plays (shared pipeline, see §6).
3. Confirm the player is otherwise completely unaffected by a hit — no knockback, no flicker, no forced state, no death at 0 HP (should just sit at 0 and stay controllable — this looks "wrong" visually but is the correct, deliberate Phase 1 boundary).
4. Save-state round-trip: take a hit (HP now < max), save, reload, confirm HP persisted correctly.
5. Confirm the Rock Buster and enemy shots still behave correctly against enemies (regression check on the existing `Enemy`/`ProjectilePlayer` pairing — unrelated to this change, but re-enabling a matrix pair is exactly the kind of edit worth a quick sanity pass elsewhere).