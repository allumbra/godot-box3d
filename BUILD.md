# Building godot-box3d (Linux)

Verified 2026-07-11 on WSL2 Ubuntu (x86_64), GCC, CMake ≥3.22, Godot 4.7-stable mono.

## Prerequisites

- CMake + GCC/g++ (`build-essential`)
- .NET 8 SDK (only for the C# test scene)
- A Godot 4.7 binary. This setup uses the mono build at
  `/mnt/c/Users/allum/OneDrive/godot/burn-rubber-chicken-tower/tools/godot/godot`
  (any Godot ≥4.3 works for the GDScript tests; mono is required for C#).
- Vendored engine: `box3d/` submodule pinned at `8441b4a` (v0.1.0, 2026-06-30,
  "Missing functions (#21)"). `git submodule update --init` if it's empty.

## Build the extension

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

- First build compiles godot-cpp (fetched by CMake, ~870 files) — expect a long
  first build; incremental builds are fast.
- Output lands at `bin/libgodot-box3d.so` (repo root, not `build/`).

## Install into the test project

```bash
mkdir -p test_project/addons/godot-box3d/bin
cp bin/libgodot-box3d.so test_project/addons/godot-box3d/bin/
```

The manifest `test_project/addons/godot-box3d/godot-box3d.gdextension` expects
the library at that path. `test_project/project.godot` selects
`physics/3d/physics_engine="Box3D Physics (Extension)"`.

## Build the C# assembly (required before opening the editor)

```bash
dotnet build test_project/test_project.csproj
```

**Known issue:** running the editor/import on the mono binary *before*
`dotnet build` segfaults (missing project assembly). Build C# first, then:

```bash
godot --headless --path test_project --import   # exits 0 once C# is built
```

## Run the test suite (headless)

```bash
godot --headless --path test_project -s res://fall_test.gd     # PASS
godot --headless --path test_project -s res://settle_test.gd   # PASS (rest y≈0.5 + raycast)
godot --headless --path test_project -s res://joint_test.gd    # PASS (hinge)
godot --headless --path test_project -s res://area_test.gd     # PASS (enter/exit signals)
godot --headless --path test_project res://csharp_drop_test.tscn  # PASS (C#/.NET layer)
godot --headless --path test_project --quit-after 300 res://demo_scene.tscn  # runs clean, exit 0
```

Determinism harness (M1):

```bash
godot --headless --fixed-fps 60 --path test_project -s res://determinism_test.gd
# prints STATE_HASH: <sha256 of all body transforms+velocities after 1000 frames>
```

## Known benign output

Every run prints a few copies of:

```
ERROR: Parameter "area" is null.  at: _area_set_param (src/servers/box3d_physics_server_3d.cpp:313)
```

This is a wrapper defect (default-area params applied before the space's
default area exists), tracked as an M4 gap; it does not affect simulation.

Godot also warns that HingeJoint3D BIAS / LIMIT_BIAS / LIMIT_SOFTNESS /
LIMIT_RELAXATION have no Box3D equivalent — expected, box3d's solver has no
such knobs.
