extends Node3D

# Box3D showcase: cycles through 4 demos, 8 s each, auto-advancing and looping.
# Number keys 1-4 jump to a demo. Each demo is built programmatically when it
# becomes active and freed on switch (all bodies/joints are plain Godot nodes;
# Box3D is the active physics engine per project.godot).

const DEMO_SECONDS := 8.0
const DEMO_NAMES := ["Stacking", "Ragdoll", "Character", "CCD"]

var demo_root: Node3D
var camera: Camera3D
var label: Label
var server: Object = PhysicsServer3D

var current := -1
var demo_time := 0.0
var orbit_angle := 0.0

# Per-demo camera framing: [look-at target, orbit radius, height].
var cam_cfg := [
	[Vector3(0, 2, 0), 13.0, 7.0],
	[Vector3(0, 2, 0), 8.0, 4.0],
	[Vector3(0, 1, 0), 13.0, 9.0],
	[Vector3(0, 2, 0), 11.0, 4.0],
]

# --- stacking state ---
var stack_sphere_spawned := false
# --- character state ---
const CH_RADIUS := 0.4
const CH_HALF := 0.5
var ch_pos := Vector3.ZERO
var ch_vel := Vector3.ZERO
var ch_angle := 0.0
var ch_mesh: MeshInstance3D

# --- materials ---
var mat_ground: StandardMaterial3D
var mat_box: StandardMaterial3D
var mat_heavy: StandardMaterial3D
var mat_limb: StandardMaterial3D
var mat_head: StandardMaterial3D
var mat_char: StandardMaterial3D
var mat_obstacle: StandardMaterial3D
var mat_ccd_on: StandardMaterial3D
var mat_ccd_off: StandardMaterial3D
var mat_wall: StandardMaterial3D


func _ready() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))
	_make_materials()

	var light := DirectionalLight3D.new()
	light.rotation_degrees = Vector3(-55, -40, 0)
	light.light_energy = 1.1
	add_child(light)

	var env := WorldEnvironment.new()
	var e := Environment.new()
	e.background_mode = Environment.BG_COLOR
	e.background_color = Color(0.12, 0.13, 0.17)
	e.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	e.ambient_light_color = Color(0.4, 0.42, 0.48)
	e.ambient_light_energy = 0.6
	env.environment = e
	add_child(env)

	camera = Camera3D.new()
	add_child(camera)

	var layer := CanvasLayer.new()
	add_child(layer)
	label = Label.new()
	label.add_theme_font_size_override("font_size", 26)
	label.position = Vector2(16, 12)
	layer.add_child(label)

	_switch_to(0)


func _make_materials() -> void:
	mat_ground = _mat(Color(0.3, 0.32, 0.36))
	mat_box = _mat(Color(0.85, 0.6, 0.25))
	mat_heavy = _mat(Color(0.9, 0.2, 0.2))
	mat_limb = _mat(Color(0.35, 0.6, 0.9))
	mat_head = _mat(Color(0.95, 0.8, 0.55))
	mat_char = _mat(Color(0.3, 0.85, 0.5))
	mat_obstacle = _mat(Color(0.55, 0.4, 0.7))
	mat_ccd_on = _mat(Color(0.3, 0.9, 0.4))
	mat_ccd_off = _mat(Color(0.9, 0.35, 0.35))
	mat_wall = _mat(Color(0.7, 0.72, 0.78))


func _mat(c: Color) -> StandardMaterial3D:
	var m := StandardMaterial3D.new()
	m.albedo_color = c
	return m


func _input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and not event.echo:
		match event.keycode:
			KEY_1: _switch_to(0)
			KEY_2: _switch_to(1)
			KEY_3: _switch_to(2)
			KEY_4: _switch_to(3)


func _switch_to(idx: int) -> void:
	current = idx
	demo_time = 0.0
	label.text = "%d / 4  —  %s" % [idx + 1, DEMO_NAMES[idx]]

	if demo_root:
		demo_root.queue_free()
	demo_root = Node3D.new()
	add_child(demo_root)

	match idx:
		0: _build_stacking()
		1: _build_ragdoll()
		2: _build_character()
		3: _build_ccd()


func _process(delta: float) -> void:
	demo_time += delta
	if demo_time >= DEMO_SECONDS:
		_switch_to((current + 1) % 4)

	# Stacking: lob a heavy sphere into the pyramid at t = 4 s.
	if current == 0 and not stack_sphere_spawned and demo_time >= 4.0:
		stack_sphere_spawned = true
		var s := _rigid_sphere(Vector3(9, 4.5, 0), 0.9, mat_heavy)
		s.mass = 12.0
		s.linear_velocity = Vector3(-14, 1.5, 0)

	# Slow camera orbit around the active demo.
	orbit_angle += delta * 0.35
	var cfg = cam_cfg[current]
	var target: Vector3 = cfg[0]
	var r: float = cfg[1]
	var h: float = cfg[2]
	camera.position = target + Vector3(cos(orbit_angle) * r, h, sin(orbit_angle) * r)
	camera.look_at(target, Vector3.UP)


func _physics_process(delta: float) -> void:
	if current == 2:
		_step_character(delta)


# ---------------------------------------------------------------------------
# Demo 1: Stacking
# ---------------------------------------------------------------------------

func _build_stacking() -> void:
	stack_sphere_spawned = false
	_ground(Vector3(60, 1, 60))

	var box := 0.62
	var gap := 0.02
	var pitch := box + gap
	# Pyramid: base 8 wide, shrinking to 1 at the top.
	for row in range(8):
		var count := 8 - row
		var y := 0.31 + row * (box + gap)
		var x0 := -(count - 1) * 0.5 * pitch
		for i in range(count):
			var b := _rigid_box(Vector3(x0 + i * pitch, y, 0), Vector3(box, box, box), mat_box)
			b.mass = 1.0


# ---------------------------------------------------------------------------
# Demo 2: Ragdoll
# ---------------------------------------------------------------------------

func _build_ragdoll() -> void:
	_ground(Vector3(60, 1, 60))

	# A ledge for the ragdoll to tumble over.
	var ledge := StaticBody3D.new()
	var lcs := CollisionShape3D.new()
	var lbox := BoxShape3D.new()
	lbox.size = Vector3(3.0, 1.0, 3.0)
	lcs.shape = lbox
	ledge.add_child(lcs)
	ledge.position = Vector3(0, 0.5, 0)
	_mesh_for(ledge, lbox.size, mat_obstacle)
	demo_root.add_child(ledge)

	# Humanoid built in local (upright) space, then dropped from ~3 m already tilted
	# so it topples over the ledge edge and tumbles instead of landing on its feet.
	var spawn := Transform3D(Basis(Vector3(0, 0, 1), deg_to_rad(62)), Vector3(-0.6, 3.4, 0))
	var drift := Vector3(2.5, 0, 0) # horizontal momentum carries it over the edge

	var torso := _capsule_body(spawn, Vector3(0, 0.0, 0), 0.22, 0.7, mat_limb, drift)
	var head := _capsule_body(spawn, Vector3(0, 0.72, 0), 0.18, 0.14, mat_head, drift)

	var l_uarm := _capsule_body(spawn, Vector3(-0.42, 0.42, 0), 0.1, 0.36, mat_limb, drift)
	var l_larm := _capsule_body(spawn, Vector3(-0.42, -0.06, 0), 0.09, 0.34, mat_limb, drift)
	var r_uarm := _capsule_body(spawn, Vector3(0.42, 0.42, 0), 0.1, 0.36, mat_limb, drift)
	var r_larm := _capsule_body(spawn, Vector3(0.42, -0.06, 0), 0.09, 0.34, mat_limb, drift)

	var l_uleg := _capsule_body(spawn, Vector3(-0.16, -0.72, 0), 0.12, 0.42, mat_limb, drift)
	var l_lleg := _capsule_body(spawn, Vector3(-0.16, -1.34, 0), 0.11, 0.42, mat_limb, drift)
	var r_uleg := _capsule_body(spawn, Vector3(0.16, -0.72, 0), 0.12, 0.42, mat_limb, drift)
	var r_lleg := _capsule_body(spawn, Vector3(0.16, -1.34, 0), 0.11, 0.42, mat_limb, drift)

	# Ball joints for neck, shoulders and hips (anchors in the same spawn frame).
	_pin(head, torso, spawn * Vector3(0, 0.6, 0))
	_pin(l_uarm, torso, spawn * Vector3(-0.28, 0.58, 0))
	_pin(r_uarm, torso, spawn * Vector3(0.28, 0.58, 0))
	_pin(l_uleg, torso, spawn * Vector3(-0.16, -0.42, 0))
	_pin(r_uleg, torso, spawn * Vector3(0.16, -0.42, 0))

	# Hinge joints (with limits) for elbows and knees.
	_hinge(l_larm, l_uarm, spawn * Vector3(-0.42, 0.16, 0), -2.2, 0.0)
	_hinge(r_larm, r_uarm, spawn * Vector3(0.42, 0.16, 0), -2.2, 0.0)
	_hinge(l_lleg, l_uleg, spawn * Vector3(-0.16, -1.06, 0), 0.0, 2.2)
	_hinge(r_lleg, r_uleg, spawn * Vector3(0.16, -1.06, 0), 0.0, 2.2)


func _pin(a: PhysicsBody3D, b: PhysicsBody3D, at: Vector3) -> void:
	var j := PinJoint3D.new()
	j.position = at
	demo_root.add_child(j)
	j.node_a = j.get_path_to(a)
	j.node_b = j.get_path_to(b)


func _hinge(a: PhysicsBody3D, b: PhysicsBody3D, at: Vector3, lo: float, hi: float) -> void:
	var j := HingeJoint3D.new()
	j.position = at
	# Hinge axis along Z (local x rotated) so elbows/knees fold in the view plane.
	j.rotation_degrees = Vector3(0, 90, 0)
	demo_root.add_child(j)
	j.node_a = j.get_path_to(a)
	j.node_b = j.get_path_to(b)
	j.set_flag(HingeJoint3D.FLAG_USE_LIMIT, true)
	j.set_param(HingeJoint3D.PARAM_LIMIT_LOWER, lo)
	j.set_param(HingeJoint3D.PARAM_LIMIT_UPPER, hi)


# ---------------------------------------------------------------------------
# Demo 3: Character (kinematic mover via Box3D server API)
# ---------------------------------------------------------------------------

func _build_character() -> void:
	_ground(Vector3(40, 1, 40))

	ch_angle = 0.0
	ch_pos = Vector3(4.0, 0.95, 0.0)
	ch_vel = Vector3.ZERO

	# Obstacle course on the walking circle (radius 4): a step and a ramp.
	var step := StaticBody3D.new()
	var scs := CollisionShape3D.new()
	var sbox := BoxShape3D.new()
	sbox.size = Vector3(2.0, 0.3, 2.0)
	scs.shape = sbox
	step.add_child(scs)
	step.position = Vector3(0, 0.15, 4.0)
	_mesh_for(step, sbox.size, mat_obstacle)
	demo_root.add_child(step)

	var ramp := StaticBody3D.new()
	var rcs := CollisionShape3D.new()
	var rbox := BoxShape3D.new()
	rbox.size = Vector3(2.5, 0.2, 2.0)
	rcs.shape = rbox
	ramp.add_child(rcs)
	ramp.position = Vector3(0, 0.25, -4.0)
	ramp.rotation_degrees = Vector3(15, 0, 0)
	_mesh_for(ramp, rbox.size, mat_obstacle)
	demo_root.add_child(ramp)

	# Visible capsule the mover drives (no physics body).
	ch_mesh = MeshInstance3D.new()
	var cmesh := CapsuleMesh.new()
	cmesh.radius = CH_RADIUS
	cmesh.height = CH_HALF * 2.0 + CH_RADIUS * 2.0
	ch_mesh.mesh = cmesh
	ch_mesh.material_override = mat_char
	ch_mesh.position = ch_pos
	demo_root.add_child(ch_mesh)


func _step_character(delta: float) -> void:
	if ch_mesh == null or not server.has_method("space_collide_mover"):
		return
	var space: RID = get_world_3d().space

	# Steer horizontally along a circle of radius 4 centred at origin.
	ch_angle += delta * 0.8
	var tangent := Vector3(-sin(ch_angle), 0, cos(ch_angle))
	ch_vel.x = tangent.x * 3.0
	ch_vel.z = tangent.z * 3.0
	ch_vel.y -= 9.8 * delta

	var c1 := ch_pos + Vector3(0, -CH_HALF, 0)
	var c2 := ch_pos + Vector3(0, CH_HALF, 0)
	var planes: Array = server.space_collide_mover(space, c1, c2, CH_RADIUS, 0xFFFFFFFF)
	var solved: Dictionary = server.solve_mover_planes(ch_vel * delta, planes)
	ch_pos += solved["delta"]
	ch_vel = server.clip_mover_velocity(ch_vel, solved["planes"])
	ch_mesh.position = ch_pos


# ---------------------------------------------------------------------------
# Demo 4: CCD
# ---------------------------------------------------------------------------

func _build_ccd() -> void:
	_ground(Vector3(60, 1, 60))

	# Wall of thin boxes at x = 0, spheres fired from -x.
	for row in range(6):
		for col in range(5):
			var y := 0.6 + row * 0.9
			var z := (col - 2) * 0.9
			var w := _rigid_box(Vector3(0, y, z), Vector3(0.05, 0.85, 0.85), mat_wall)
			w.mass = 0.5

	# Fast spheres: green use continuous CD (stop at wall), red do not (may tunnel).
	for i in range(6):
		var z := (i - 2.5) * 0.8
		var use_ccd := i % 2 == 0
		var mat: StandardMaterial3D = mat_ccd_on if use_ccd else mat_ccd_off
		var s := _rigid_sphere(Vector3(-10, 2.4, z), 0.18, mat)
		s.mass = 2.0
		s.gravity_scale = 0.0
		s.continuous_cd = use_ccd
		s.linear_velocity = Vector3(60, 0, 0)


# ---------------------------------------------------------------------------
# Builders
# ---------------------------------------------------------------------------

func _ground(size: Vector3) -> void:
	var g := StaticBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = size
	cs.shape = box
	g.add_child(cs)
	g.position = Vector3(0, -0.5, 0)
	_mesh_for(g, size, mat_ground)
	demo_root.add_child(g)


func _rigid_box(pos: Vector3, size: Vector3, mat: StandardMaterial3D) -> RigidBody3D:
	var b := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = size
	cs.shape = box
	b.add_child(cs)
	b.position = pos
	_mesh_for(b, size, mat)
	demo_root.add_child(b)
	return b


func _rigid_sphere(pos: Vector3, radius: float, mat: StandardMaterial3D) -> RigidBody3D:
	var b := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var sph := SphereShape3D.new()
	sph.radius = radius
	cs.shape = sph
	b.add_child(cs)
	b.position = pos
	var mi := MeshInstance3D.new()
	var sm := SphereMesh.new()
	sm.radius = radius
	sm.height = radius * 2.0
	mi.mesh = sm
	mi.material_override = mat
	b.add_child(mi)
	demo_root.add_child(b)
	return b


func _capsule_body(spawn: Transform3D, local: Vector3, radius: float, seg: float, mat: StandardMaterial3D, vel: Vector3) -> RigidBody3D:
	var b := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var cap := CapsuleShape3D.new()
	cap.radius = radius
	cap.height = seg + radius * 2.0
	cs.shape = cap
	b.add_child(cs)
	# Place in the spawn frame so the whole ragdoll shares one tilt.
	b.transform = Transform3D(spawn.basis, spawn * local)
	b.linear_velocity = vel
	var mi := MeshInstance3D.new()
	var cm := CapsuleMesh.new()
	cm.radius = radius
	cm.height = seg + radius * 2.0
	mi.mesh = cm
	mi.material_override = mat
	b.add_child(mi)
	demo_root.add_child(b)
	return b


func _mesh_for(body: Node3D, size: Vector3, mat: StandardMaterial3D) -> void:
	var mi := MeshInstance3D.new()
	var bm := BoxMesh.new()
	bm.size = size
	mi.mesh = bm
	mi.material_override = mat
	body.add_child(mi)
