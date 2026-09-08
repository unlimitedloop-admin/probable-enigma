# NES-style audio driver design

## Goal

Preserve the NES APU limitation that only one source may be audible on each of
the five logical voices at a time, while keeping audio state reconstructible
for save/load. Native sound handles and backend channel numbers are runtime
resources and are never serialized.

## Mandatory invariants

The logical voices are `Pulse1`, `Pulse2`, `Triangle`, `Noise`, and `Dpcm`.

- Each logical voice has at most one audible owner.
- BGM owns every voice used by the current track unless an SE temporarily
  preempts it.
- An SE stem explicitly declares its logical voice. Array position and native
  playback-channel index have no semantic meaning.
- A multi-stem SE acquires all required voices atomically or does not play.
- Competing SE stems are resolved per voice by priority. A later request
  replaces an existing request of equal priority.
- While an SE owns a voice, the corresponding BGM transport continues silently.
- Releasing an SE voice restores the BGM voice at its current effective volume,
  including master volume and fade progress.
- Volume, fade, pause, and resume operations must preserve voice ownership.

These rules emulate APU voice contention rather than mixing independently
recorded WAV files without restriction.

## Ownership and playback layers

`ChannelManager` remains a low-level owner of backend playback channels. It
loads files, controls transport and volume, and exposes no BGM/SE policy.

The audio-driver layer owns five stable logical voice slots. Each slot records
whether its audible owner is BGM, an SE instance, or none. BGM and SE managers
request changes through this layer instead of independently changing the other
manager's channel volume.

Configuration uses explicit voice names instead of numeric
`target_bgm_channels` values. Loading rejects unknown voice names and duplicate
voices within one BGM or SE definition before any configuration is committed.

## Save/load boundary

The logical snapshot contains stable names, voice identities, transport state,
volumes, fade state, and restorable continuous-SE instances. It excludes native
handles and backend channel indices.

Restore order:

1. Parse and validate the entire audio snapshot without changing live audio.
2. Reload the scene's audio configuration and registered sound resources.
3. Reconstruct BGM transport while paused.
4. Discard transient one-shot SE and reconstruct continuous SE instances.
5. Rebuild logical voice ownership and effective output volumes.
6. Commit the reconstructed audio state, still respecting the game's pause.

## Task board

| ID | Priority | Status | Task | Acceptance criterion |
|---|---:|---|---|---|
| AUD-001 | P0 | Done | Add stable APU voice IDs and explicit configuration mapping | BGM and SE stems no longer infer their voice from array position or a numeric BGM index |
| AUD-002 | P0 | Done | Add five-slot voice ownership and per-voice SE arbitration | No two audible sources can own the same APU voice; multi-stem acquisition is atomic |
| AUD-003 | P0 | Done | Route effective BGM volume through ownership | Fade and volume changes cannot make a preempted BGM voice audible |
| AUD-004 | P1 | Done | Remove duplicate SE channel ownership from `AudioManager` | Each backend channel has one clear owner and release path |
| AUD-005 | P1 | Ready | Add pure arbitration and configuration tests | Voice conflicts, priorities, atomic acquisition, and invalid mappings are covered without audio hardware |
| AUD-006 | P1 | Ready | Add logical BGM transport snapshots | Paused multi-stem BGM restores by stable track and voice IDs |
| AUD-007 | P1 | Ready | Classify and restore continuous SE | One-shots stop on load; configured continuous emitters restore with ownership |

`AUD-001` through `AUD-005` are prerequisites for save-state tasks `SS-014`
and `SS-015`.
