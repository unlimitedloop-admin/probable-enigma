# State save/load design

## Goal

Restore a paused game from an external save-state file. This is a runtime
snapshot, not long-lived player progress or application settings.

The first end-to-end target is the debug sequence's `BackdoorMenu`, including
its `BgStarField`. Later milestones add gameplay scenes and entities.

## Current implementation

- `WindowMessageHandlers` accepts save/load only while the game is paused.
- `SaveSystem` writes an explicit versioned envelope with fixed-width fields and
  a CRC-32 covering its metadata and scene payload.
- `SaveData` carries the sequence ID, scene ID, and a variable-sized scene
  payload without exposing its in-memory object layout.
- `StandardSequence` and `DebugSequence` delegate scene state to `SceneManager`.
- Target scene payloads are validated before sequence replacement, and loaders
  commit parsed state only after complete validation.
- `BackdoorMenu` and `DemoStage2` serialize their owned runtime state;
  unsupported scenes such as `DemoStage1` do not advertise save support.

## Problems and design decisions

### P0: The save path stops at the sequence

`Sequence -> SceneManager -> Scene -> Phase/components` must form one explicit
ownership-aligned serialization path. A parent writes the identity and payload
of the child it owns; runtime services and resource handles are not serialized.

### P0: Raw `SaveData` memory is not a file format

Writing `sizeof(SaveData)` makes the file depend on compiler layout and prevents
`SaveData` from safely owning variable-sized scene data. The file must have an
explicit header and fixed-width fields:

1. magic value
2. format version
3. game/content compatibility ID
4. sequence ID
5. scene ID
6. payload byte count
7. CRC-32
8. scene payload

All counts must be bounded before allocating or looping.

### P0: Loading is destructive before validation

`BgStarField::Load` clears live state before confirming that the input is
complete and valid. Every loader should deserialize into temporary state,
validate it, and commit only after the entire payload succeeds.

### P0: A phase ID alone does not restore a phase

`BackdoorMenu::Load` changes `_phaseId` but leaves the phase object created by
`onEnter_` in place. Loading must reconstruct the matching phase object and then
apply phase-specific state such as menu cursor, navigation stack, and room edit
state.

### P1: The current binary representation is platform-dependent

`size_t`, `int`, raw `float`, and native byte order are written directly. Use
fixed-width integer fields and one documented byte order. Floating-point fields
may use their IEEE-754 bit representation with a compile-time check.

### P1: File writes are not transactional

A crash or disk error can destroy the previous slot. Write to a sibling
temporary file, flush/close it, and atomically replace the slot only after the
new file is complete.

### P1: Version checking has no migration policy

The initial implementation may reject unsupported versions, but the error must
distinguish missing, corrupt, unsupported, and I/O-failed files. Component
payload versions should be independent of the outer file version where useful.

An older well-formed save is "unsupported", not corrupt. Truncated data,
impossible sizes/IDs/values, unexpected trailing bytes, and a CRC-32 mismatch
are corrupt. The checksum covers the canonical little-endian compatibility ID,
sequence ID, scene ID, payload byte count, and scene payload. It detects
accidental corruption but is not authentication: a deliberate editor can
recalculate it.

`SaveSystem::Load` reports `Success`, `FileNotFound`, `Corrupt`,
`UnsupportedVersion`, `IncompatibleContent`, or `IoError` without changing the
destination on failure. The command UI maps these results to distinct messages.
A structurally valid file that cannot be reconstructed by the current runtime
is reported separately from an outer-file read or integrity failure.

The 64-bit game/content compatibility ID is stable across builds and across
asset-only changes that do not alter saved-state interpretation. Increment it
when map structure, collision/tuning data, entity semantics, or other runtime
content changes make existing logical snapshots unsafe to resume. It is an
explicit compatibility contract rather than a hash of every packaged byte, so
cosmetic or audio balancing changes do not invalidate saves unnecessarily.

### P0: Nondeterministic and call-order-dependent randomness blocks replay

`BgStarField` uses the process-global `rand()` and reseeds it with wall-clock
time. `SpriteAtlas::ApplyRandomHueToVariant` uses `random_device`. Both make the
same input stream produce different output between runs.

The charge particle effect uses a fixed-seed LCG, so it is deterministic from a
fresh scene. It is nevertheless mutable, call-order-dependent state: a mid-scene
load diverges unless the generator state is restored, and unrelated changes to
the number of calls can change the sequence.

Policy for simulation and replay:

- Do not read wall-clock time, `random_device`, or process-global random state
  while simulation or rendering is advancing.
- Initial entropy may be consumed when a new game or cosmetic pattern is
  created, provided the resulting pattern ID is immediately owned by game
  state and recorded in saves and replay metadata.
- Prefer fixed animation/emission tables or a function of stable inputs such as
  pattern ID, scene tick, entity ID, and effect index for cosmetic variation.
- If procedural generation genuinely needs a PRNG, give it an explicit seed and
  state owned by the simulation, serialize both, and record the initial seed in
  replay metadata.
- Rendering must not consume simulation random state.

The current star field creates one pattern ID at initialization, then derives
each spawn decision and star property directly from that ID, the elapsed tick,
and a stream index. Its NES palette scheme is derived from the same ID. The ID,
elapsed tick, and active stars are serialized, so continuing after load does not
depend on mutable PRNG state or prior call count. Replay can supply the recorded
ID through the explicit initialization overload. The charge effect remains a
fixed scripted pattern.

### P1: Audio is a presentation system with two kinds of restoration

Native audio handles must not be serialized. Save logical transport state and
reconstruct it after the scene has reloaded its audio configuration.

BGM restoration is feasible with the current backend because it can query and
set playback positions. A BGM snapshot should contain:

- registered track ID/name, playing/paused state, and transport position
- per-stem positions when a track uses multiple synchronized files
- master/current volumes and muted channels
- active fade target, step/progress, and loop parameters

The current pause implementation already remembers a position inside each
`SoundChannel`, but that position is private and is destroyed when scene loading
releases/reinitializes audio. `SoundChannel` therefore needs an explicit logical
snapshot API that works while playing and while paused.

SE restoration needs a policy rather than blindly resuming every sample:

- Transient one-shot SE (jump, hit, menu click): stop on load. Resuming halfway
  is usually surprising and replay will emit it again from the game event.
- Continuous/looped SE (wind, machinery, charge hum): restore as named logical
  emitters with position and loop state.
- Gameplay-critical timing must live in simulation state, never in whether an
  audio sample happens to be playing.

Audio playback timing is not suitable for a replay determinism checksum. Replay
reproduces simulation events; the audio system consumes those events as
presentation side effects.

### P1: IDs need validation

Sequence, scene, phase, sprite-tile type, counts, positions, and velocities must
be checked before object construction. Unknown IDs must fail cleanly rather
than leaving a partially initialized runtime.

### P2: Snapshot coverage must be explicit

Gameplay restoration will eventually need player/entity state, the player state
machine, room/scroll state, phase and fade state, animation counters, pending
commands, RNGs, and relevant time counters. Audio playback and GPU/resource IDs
should normally be reconstructed from logical state rather than serialized.

## DemoStage2 snapshot schema

`DemoStage1` is intentionally unsupported. It targets the obsolete pre-header
stage-map binary format and cannot be launched by the current `BGPageHeader`
reader. `LaunchingGame` is also intentionally unsupported because it is a
resource-validation transition rather than a resumable scene.

`DemoStage2` is the only action-stage snapshot target. Its current version 3
schema is:

1. Scene payload version and action-phase type ID.
2. Stable scene state: current room/page identity and BG animation tick.
3. `AbstractActionPhase` state: action/intro mode, intro step and timers,
   operation flags, previous player position, and `StageIntroUI` progress.
4. `ScrollController` aggregate: mode, page, camera/view positions, target and
   tracked-object positions, active page-scroll animation, pending request,
   carry distance, and freeze state.
5. `EntityManager` aggregate: next instance ID and ordered live entity records.
6. Each entity record: stable type ID, stable instance ID, component version,
   bounded payload size, and entity-owned logical payload.
7. Optional versioned stage-script state. The current DemoStage2 script is null,
   so no script payload is recorded.
8. Versioned logical BGM transport: registered track name, playback status,
   stable APU voices, per-voice positions and logical volumes, master volume,
   loop range, and fade progress.
9. Versioned continuous-SE transport: configured emitter names, playback
   status, stable APU voices, per-voice positions and logical volumes, and SE
   master volume. Transient SE is intentionally absent.

Runtime sprite/BG handles, service pointers, map caches, collision-service
objects, and other reconstructible resources are excluded. Debug-only display
counters are also excluded. Transient visual entities are included initially
because their payloads are small and exact visual continuation is easier to
reason about than a mixed discard policy. Logical BGM restoration is included
as of version 2, and continuous-SE restoration is included as of version 3.
One-shot SE is not resumed. Versions 1 and 2 are intentionally rejected by the
current decoder rather than partially restoring audio.

The save barrier requires an active phase, no pending phase/scene transition,
an interactive fader, and an `EntityManager` that is neither updating nor
holding pending additions. A player with unconsumed output also rejects capture.

Reconstruction order is fixed:

1. Parse and validate the entire save into lightweight DTOs without touching
   the live scene.
2. Initialize DemoStage2 resources, map data, and runtime services.
3. Restore BG timing and scrolling before entities.
4. Create all entities in saved order and build the instance-ID lookup.
5. Inject runtime services and resolve entity references in a second pass.
6. Restore phase/UI/script state, then expose the scene as current.

### DemoStage2 implementation plan

| ID | Status | Task | Acceptance criterion |
|---|---|---|---|
| DS2-001 | Done | Complete fixed-width primitive state I/O and freeze the schema | Boolean, 8/16-bit integers, and IEEE-754 double values have strict portable codecs |
| DS2-002 | Done | Add pure target DTO validation and replace snapshot rollback | Invalid target payload leaves the current scene untouched without calling its `Save`; incomplete DemoStage2 DTOs remain gated |
| DS2-003 | Done | Add stable phase, BG animation, and scroll snapshots | Restored camera/page/animation continues from the captured tick |
| DS2-004 | Done | Add entity type/instance IDs and manager record envelope | Entity order and bounded payloads round-trip without pointers or resource handles |
| DS2-005 | Done | Restore projectile and transient effect entities | Active entity position, velocity, lifetime, and animation tick resume exactly |
| DS2-006 | Done | Restore Player and owned state machines | Movement, animation, attack, charge, buffers, and pending requests resume exactly |
| DS2-007 | Done | Integrate DemoStage2 scene save/load | A paused action phase round-trips through an external slot |
| DS2-008 | Done | Add corruption and deterministic-continuation tests | Invalid records are non-destructive and subsequent simulation checksums match |

The headless regression suite is available from the Debug executable with
`mm2hack.exe --state-tests`. It validates the canonical DemoStage2 payload,
rejects every truncated form plus unknown IDs, inconsistent pages, trailing
bytes, unsupported entity versions, and duplicate players. It also verifies
that a rejected entity restore preserves the current aggregate and compares
serialized Player checksums for 240 simulation frames after restoration.

### P0: Sequence replacement must not make load failure destructive

The window command calls `SequenceManager::LoadSequence` before applying the
payload. That destroys the current sequence and initializes a replacement. If
scene or component loading then fails, the previous paused game has already
been lost.

Use a two-phase load:

1. Read and validate the complete file into snapshot DTOs without touching the
   live runtime.
2. Rebuild and hydrate a candidate runtime, then commit the replacement only
   after successful restoration.

Some resources are global and mutate during scene initialization, so fully
transactional candidate construction is currently impractical. The implemented
first stage parses and validates the target payload without calling `Save` on
the current scene or initializing target resources. BackdoorMenu and DemoStage2
both have complete pure validators, and only a validated target may replace the
current sequence/scene. A failure while loading external resources after
validation is an environment/runtime failure and leaves the application with no
active sequence, rather than attempting an in-memory snapshot rollback.

### P0: Save must occur at a defined frame boundary

The current menu restriction (paused only) is useful but not a complete
snapshot barrier. A save must not observe `EntityManager` during an update,
unconsumed player output, or half-applied pending commands/transitions. Define
`CanCaptureState()` at the sequence/scene boundary and initially permit capture
only after a completed tick. Transition and fade states can either be fully
serialized or explicitly rejected until supported.

### P1: Polymorphic entities have no persistent identity or factory

`EntityManager` owns `unique_ptr<IEntity>` values but entities have no stable
instance ID, persisted type ID, or snapshot interface. Native pointers and
runtime sprite handles cannot be saved. Gameplay state restoration needs:

- stable entity type IDs and instance IDs
- a type registry/factory for reconstruction
- per-type logical state DTOs
- references encoded as entity IDs and resolved in a second pass
- deterministic entity ordering for updates and replay checksums

The player state machine has the same issue at a smaller scale: restore the
active state ID plus the internal state of handlers whose values survive state
transitions, such as attack/charge timing.

### P1: Mid-transition state is distributed across several objects

Scrolling has animator progress, pending requests, carry distance, camera/page,
and freeze state. Phase changes have active/pending phase objects, fade plans,
fade counters, and pending parameters. Saving only a page or phase ID is not
enough. Each subsystem needs one aggregate logical snapshot so restoration
order is explicit and private partial state is not missed.

### P1: Replay needs a stable tick and compatibility identity

The fixed timestep is a good foundation. Save/load must also restore the
simulation tick used by input snapshots and deterministic patterns. Save files
and replays should identify the compatible game/content schema; loading the
same scene ID against changed map or tuning data can otherwise produce a valid
but incorrect state.

## Proposed API direction

- A small `StateWriter` / `StateReader` owns primitive encoding, bounds checks,
  and error state.
- `SaveSystem` only handles the outer file envelope and slot I/O.
- `SaveData` is a logical envelope containing fixed-width IDs and an opaque
  scene payload; it is never dumped with `reinterpret_cast` as a whole.
- `ISequence` delegates its owned scene to `SceneManager`.
- `SceneManager` creates the saved scene first, initializes its resources, and
  then asks the scene to restore its logical state.
- Each scene delegates to the active phase and owned stateful components.
- Load returns a structured result eventually; `bool` is acceptable during the
  first vertical slice if no diagnostic detail is discarded internally.

## Task board

| ID | Priority | Status | Task | Acceptance criterion |
|---|---:|---|---|---|
| SS-001 | P0 | Done | Add bounded binary reader/writer | Truncated reads fail without mutating destination state |
| SS-002 | P0 | Done | Replace raw `SaveData` dump with explicit file envelope | Header fields are fixed-width; payload size is bounded |
| SS-003 | P0 | Done | Connect Sequence -> SceneManager -> Scene | Saved scene ID and payload reach the active scene on load |
| SS-004 | P0 | Done | Make `BgStarField` load transactional | Invalid counts/values/truncation preserve the old star field |
| SS-005 | P0 | Done | Restore `BackdoorMenu` phase objects | Credit, top menu, and inside menu resume with matching phase state |
| SS-006 | P1 | Done | Make slot replacement transactional | Failed writes preserve the previous slot |
| SS-007 | P0 | Done | Give `BgStarField` a persisted deterministic pattern ID | Save/load and replay produce the same star sequence and NES palette scheme |
| SS-008 | P1 | Done | Define DemoStage2 snapshot schema | Coverage list and reconstruction order are documented |
| SS-009 | P1 | Done | Restore player and entity state | Player/entities resume without stale references |
| SS-010 | P1 | Done | Add outer `.sav` envelope and corruption tests | Slot-file round-trip, truncation, oversized payload, and bad magic/version pass |
| SS-011 | P2 | Done | Improve user-facing load errors | Missing/corrupt/unsupported/I/O cases are distinguishable |
| SS-012 | P0 | Done | Isolate nondeterministic entropy from simulation | Only new-pattern creation may use entropy; simulation/render paths use persisted stable inputs |
| SS-013 | P0 | Done | Replace charge-particle LCG with a stable pattern | Particle placement is reproducible without mutable random state |
| SS-014 | P1 | Done | Connect BGM transport to the DemoStage2 save payload | The AUD-006 DTO round-trips through an external slot and restores after audio resources are registered |
| SS-015 | P1 | Done | Connect continuous SE transport to the DemoStage2 save payload | The AUD-007 DTO round-trips through an external slot; transient SE stops and charge audio restores with ownership |
| SS-016 | P1 | Ready | Define replay input/event format and checksum boundary | Same initial state and input log reproduce the same simulation checksums |
| SS-017 | P0 | Done | Add a frame-boundary snapshot barrier | Capture is rejected during update or unsupported transitions |
| SS-018 | P0 | Done | Make sequence loading target-validated and two-phase | Invalid target payload preserves the current scene without requiring it to support save |
| SS-019 | P1 | Done | Add stable entity IDs and snapshot factory | Entity graphs rebuild without serialized pointers or resource handles |
| SS-020 | P1 | Done | Add game/content compatibility identity | Incompatible runtime content is rejected with a specific result |
| SS-021 | P0 | Done | Persist star-field initial entropy | Pattern ID is saved, restored, and injectable by replay/new-game setup |
| SS-022 | P1 | Done | Add save payload integrity checking | Accidental byte corruption is rejected before runtime reconstruction |
| SS-023 | P1 | Ready (deferred) | Reconcile restored charge state with live attack input | A loaded charge snapshot follows an explicit cancel-or-resume policy and its continuous SE cannot diverge from simulation state |

`AUD-006` and `AUD-007` completed the backend-independent BGM/SE DTOs,
validation, and manager-level restoration. `SS-014` and `SS-015` now refer only
to serialization and DemoStage2 / external-slot integration; they do not repeat
the audio-driver work.

### Current implementation order

1. `SS-014` (Done): add the versioned BGM section to the DemoStage2 payload and
   connect BGM capture, validation, and restoration.
2. `SS-015` (Done): serialize continuous-SE state and complete
   BGM/SE ownership restoration. Transient SE remains intentionally absent.
3. `SS-023` (Deferred): choose how a restored charge interacts with the live
   attack-button state, then keep simulation and charge audio synchronized.
4. `SS-010` (Done): cover the outer slot-file envelope, including bad magic,
   unsupported version, oversized payload, trailing bytes, and every truncation.
5. `SS-022` (Done): protect the envelope metadata and scene payload with CRC-32
   and reject mismatches before runtime reconstruction.
6. `SS-011` (Done): classify missing, corrupt, unsupported-version, and I/O
   failures and show a specific load message for each result.
7. `SS-020` (Done): persist a stable game/content compatibility ID and reject a
   valid save made for incompatible runtime content with a specific result.

The remaining independent hardening task is deferred `SS-023`. Replay format
work is tracked separately as `SS-016`.

## Milestones

1. Foundation: SS-001, SS-002, SS-010.
2. First vertical slice: SS-003, SS-004, SS-005. A paused `BackdoorMenu`
   round-trips through a slot file.
3. Determinism: SS-007, SS-012, SS-013, SS-016.
4. Reliability and audio: SS-006, SS-011, SS-014, SS-015.
5. Gameplay snapshot: SS-008, SS-009, followed by scroll, fade, animation,
   command-queue, and time coverage as required by testing.

## Definition of done for a scene

- Saving does not mutate live state.
- Loading a corrupt or incompatible payload does not mutate live state.
- Runtime resources are initialized before logical state is applied.
- Every variable that changes subsequent gameplay is either restored or listed
  as an intentional exclusion.
- A save/load round-trip test compares the scene's logical snapshot.
- At least truncated and oversized payloads are rejected.
