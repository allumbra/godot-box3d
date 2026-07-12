# Pile benchmark — box3d extension vs stock Godot Jolt (M4)

**Date:** 2026-07-11 · WSL2 Ubuntu x86_64, Godot 4.7-stable mono, extension Release/GCC.

## Method

`test_project/bench_pile.gd`: deterministic pile (alternating boxes/spheres,
10×10 per layer) dropped onto a ground box. 60-frame untimed warmup + 600 timed
physics frames, `--headless --fixed-fps 60` (engine steps as fast as possible,
so wall time ≈ physics cost). Bodies ∈ {200, 500, 1000}, 3 runs per engine per
count; Jolt control = same script in a minimal project with
`3d/physics_engine="Jolt Physics"`.

## Median results

| Bodies | Engine | total ms (600 frames) | avg ms/frame | max ms (worst frame) | asleep at end |
|---|---|---|---|---|---|
| 200 | box3d | 86.6 | 0.144 | 9.7 | 200/200 |
| 200 | Jolt | 128.8 | 0.215 | 17.1 | 200/200 |
| 500 | box3d | 6339.8 | 10.57 | 46.0 | 49/500 |
| 500 | Jolt | 5411.1 | 9.02 | 94.9 | 114/500 |
| 1000 | box3d* | 11405.5 (n=2) | 19.01 (n=2) | 57.4 (n=2) | 51/1000 |
| 1000 | Jolt | 18758.2 | 31.26 | 112.7 | 59/1000 |

## Reading the numbers

- box3d is faster at 200 bodies (~1.5×) and at 1000 bodies (~1.6× lower
  per-frame cost, and its worst single frame is half of Jolt's at every size —
  relevant for frame pacing).
- Jolt is ~17% faster at 500 bodies, but with a confound: Jolt had put 114/500
  bodies to sleep vs box3d's 49/500, so the engines were not simulating the
  same amount of active work. Sleep-heuristic differences, not raw solver cost,
  may dominate that cell.
- \* One box3d 1000-body run segfaulted (2/3 runs valid). **Caveat: the
  benchmark ran concurrently with extension rebuilds that hot-swapped the .so
  in test_project, which can itself crash a running process — this cell needs a
  clean rerun before the crash is treated as a real finding.** Tracked as an
  open item.

## Reproduce

Run each engine/count: `<godot> --headless --fixed-fps 60 --path test_project -s res://bench_pile.gd -- --bodies=<n>`
