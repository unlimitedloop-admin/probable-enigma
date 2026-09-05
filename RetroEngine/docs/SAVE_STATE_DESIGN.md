# State save/load design

## Goal

Restore a paused game from an external save-state file. This is a runtime
snapshot, not long-lived player progress or application settings.

The first end-to-end target is the debug sequence's `BackdoorMenu`, including
its `BgStarField`. Later milestones add gameplay scenes and entities.

## Current implementation

- `WindowMessageHandlers` accepts save/load only while the game is paused.
- `SaveSystem` writes a version followed by the in-memory bytes of `SaveData`.
- `SaveData` currently contains only sequence, scene, and phase integers.
- `StandardSequence::Save/Load` and `DebugSequence::Save/Load` only handle the
  sequence ID; neither delegates to `SceneManager`.
- `SceneManager` has no state I/O API.
- `BackdoorMenu::Save/Load` and `BgStarField::Save/Load` exist, but nothing in
  the application save path calls them.
- `DemoStage1` and `DemoStage2` contain empty state I/O methods.

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
3. sequence ID
4. scene ID
5. payload byte count
6. scene payload

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

### P0: Nondeterministic and call-order-dependent randomness blocks replay

`BgStarField` uses the process-global `rand()` and reseeds it with wall-clock
time. `SpriteAtlas::ApplyRandomHueToVariant` uses `random_device`. Both make the
same input stream produce different output between runs.

The charge particle effect uses a fixed-seed LCG, so it is deterministic from a
fresh scene. It is nevertheless mutable, call-order-dependent state: a mid-scene
load diverges unless the generator state is restored, and unrelated changes to
the number of calls can change the sequence.

Policy for simulation and replay:

- Do not read wall-clock time, `random_device`, or process-global random state.
- Prefer fixed animation/emission tables or a function of stable inputs such as
  scene tick, entity ID, and effect index for cosmetic variation.
- If procedural generation genuinely needs a PRNG, give it an explicit seed and
  state owned by the simulation, serialize both, and record the initial seed in
  replay metadata.
- Rendering must not consume simulation random state.

For the current star and charge effects, fixed scripted patterns are preferred
over serializing a PRNG.

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

### P0: Sequence replacement currently makes load failure destructive

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
transactional candidate construction may initially be impractical. Until those
resources are isolated, keep an in-memory copy of the validated snapshot and
provide a controlled recovery path rather than silently continuing from a
half-loaded scene.

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
| SS-006 | P1 | Blocked by SS-001/2 | Make slot replacement transactional | Failed writes preserve the previous slot |
| SS-007 | P0 | Done | Replace `BgStarField` randomness with a scripted pattern | Save/load and replay produce the same star sequence |
| SS-008 | P1 | Ready | Define DemoStage2 snapshot schema | Coverage list and reconstruction order are documented |
| SS-009 | P1 | Blocked by SS-008 | Restore player and entity state | Player/entities resume without stale references |
| SS-010 | P1 | Ready | Add save-format and corruption tests | Round-trip, truncation, oversized count, bad magic/version pass |
| SS-011 | P2 | Ready | Improve user-facing load errors | Missing/corrupt/unsupported/I/O cases are distinguishable |
| SS-012 | P0 | Done | Audit and remove nondeterministic random sources | No simulation/render path uses wall-clock seed, `rand()`, or `random_device` |
| SS-013 | P0 | Done | Replace charge-particle LCG with a stable pattern | Particle placement is reproducible without mutable random state |
| SS-014 | P1 | Ready | Add logical BGM snapshot/restore | A paused multi-stem BGM resumes at the saved transport position |
| SS-015 | P1 | Ready | Classify SE as transient or continuous | Transients stop and continuous emitters restore according to policy |
| SS-016 | P1 | Blocked by SS-012/13 | Define replay input/event format and checksum boundary | Same initial state and input log reproduce the same simulation checksums |
| SS-017 | P0 | Done | Add a frame-boundary snapshot barrier | Capture is rejected during update or unsupported transitions |
| SS-018 | P0 | Ready | Make sequence loading two-phase | Invalid scene/component payload does not discard the current paused game |
| SS-019 | P1 | Blocked by SS-008 | Add stable entity IDs and snapshot factory | Entity graphs rebuild without serialized pointers or resource handles |
| SS-020 | P1 | Ready | Add game/content compatibility identity | Incompatible runtime content is rejected with a specific result |

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
