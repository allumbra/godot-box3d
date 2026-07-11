# Determinism verdict (M1)

**Date:** 2026-07-11 · **Environment:** WSL2 Ubuntu x86_64, Godot 4.7-stable mono (Linux), extension built Release/GCC, box3d vendored @ `8441b4a` (v0.1.0).

## Harness

`test_project/determinism_test.gd` — run with:

```bash
godot --headless --fixed-fps 60 --path test_project -s res://determinism_test.gd
```

Fixed scene: 64 boxes (4×4×4 grid, integer-arithmetic jitter + fixed rotations) +
6 dropped spheres + one zero-gravity mid-air collision pair = 72 rigid bodies,
hardcoded transforms, no RNG. Steps exactly 1000 physics frames at fixed delta
(`--fixed-fps 60` locks one tick per frame), then SHA-256-hashes every body's
`global_transform` basis+origin, linear velocity, and angular velocity as exact
float32 bytes, in creation order. `PASSES = 2` repeats the whole scene in the
same process (teardown + identical respawn) to catch state leakage.

## Results — box3d (this extension)

| Leg | Result |
|---|---|
| (a) 3 separate processes | **IDENTICAL** hash all 3 runs |
| (b) incremental rebuild of the `.so`, rerun | **IDENTICAL** hash |
| (c) two passes in one process (teardown + identical respawn) | pass 2 ≠ pass 1 (reused world), but **pass 2 is itself IDENTICAL across all 3 process restarts** |
| (d) cross-machine / cross-OS | **UNTESTED** (single machine available) |

Reference hashes (1000 frames, 72 bodies):

```
pass 1 (fresh world): 827b357642d67483b532a6ea8c39b260dc407f410b531d7fec645b9dcd5af26f
pass 2 (reused world): 44e8a1d86d99fb3bd96cba08cef6e10b569906d4b705ca4d8f48fef7aa820d37
```

Pass 2 differing from pass 1 is expected (box3d reuses internal ids/broadphase
slots after destruction, so a reused world is a different-but-valid initial
state); what matters for lockstep/replay is that BOTH passes hash identically
on every run — the whole trajectory is a pure function of the world's history.

**Verdict: box3d through the GDExtension layer is run-to-run and
rebuild-to-rebuild deterministic on the same machine.** This is the property
Jolt was proven NOT to have in the game repo's flake investigation, and it is
the foundation lockstep netcode and replays need. Cross-machine determinism
(box3d's stronger claim) still needs a second machine to confirm.

## Control — stock Godot Jolt, same harness, same machine

| Leg | Result |
|---|---|
| 3 separate processes | identical hashes each run (pass1 `feb1d050…`, pass2 `1beaabc0…`) |
| two passes in one process | ran fine, but **pass 2 hash ≠ pass 1 hash** — reused-world state changes results |

Two honest caveats cut both ways:

1. This simple single-threaded scene did NOT reproduce Jolt's run-to-run
   divergence — Jolt was restart-stable here. The game's observed Jolt
   non-determinism involves a full game scene (threads, more contacts). So this
   harness shows box3d ≥ Jolt, not Jolt broken.
2. Jolt's in-process pass-2 divergence shows world reuse is not neutral there,
   while box3d's equivalent leg is blocked by the crash below — neither engine
   has a clean "reused world is deterministic" result yet.

## Crash found (and lost) by the harness — treat as open

On the pre-wheel-joint build of the extension, pass 2 segfaulted 100% (4/4
runs) on the first step after free-all-bodies + respawn. On the current build
(which added joint lifecycle fixes — none in the no-joint path this scene
uses) the crash does not reproduce, and targeted probes of the same pattern
(free-only / respawn-only / keep-ground / free+respawn) all survive. A crash
that is binary-layout-sensitive and vanishes on rebuild is characteristically
latent UB (likely a stale pointer read that usually lands in still-mapped
memory). **Status: not root-caused; needs an ASAN or gdb pass (neither
available on this machine yet — no sudo). Tracked as an M4 item.**

## Notes for the game project (BRCT)

- Updates the open question in `docs/research/2026-07-10-box3d-vs-jolt.md`:
  box3d's determinism claim **survives the GDExtension layer** for
  restart/rebuild on one machine.
- Adoption-grade evidence still missing: cross-machine hash equality, and a
  bigger scene with sustained contact islands (closer to a real match).
