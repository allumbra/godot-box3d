extends Node3D

# Box3D showcase: cycles through 12 demos, 8 s each, auto-advancing and looping.
# Number keys 1-9, 0, -, = jump to a demo. Each demo is built programmatically when
# it becomes active and freed on switch (all bodies/joints are plain Godot nodes;
# Box3D is the active physics engine per project.godot).

const DEMO_SECONDS := 8.0
const DEMO_COUNT := 12
const DEMO_NAMES := [
	"Stacking", "Ragdoll", "Character", "CCD",
	"Driving", "Ragdoll Pile", "Dominoes & Jenga", "Bounce House",
	"Character Course", "Card House", "Explosion", "Junkyard",
]

# --- driving tuning (Box3DWheelJoint3D) ---
const DRIVE_SPEED := 18.0
const DRIVE_TORQUE := 90.0
const STEER_ANGLE := 0.45
const WHEEL_RADIUS := 0.35
# --- bounce house ---
const BOUNCE_SPEED := 40.0

var demo_root: Node3D


func _notification(what: int) -> void:
	# Free the active demo's bodies/joints before quitting so the physics server
	# tears down with no live RIDs (silences the ~RID_PtrOwner leak warning).
	if what == NOTIFICATION_WM_CLOSE_REQUEST:
		if demo_root:
			demo_root.free()
			demo_root = null
		get_tree().quit()
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
	[Vector3(0, 1.5, 0), 15.0, 7.0],
	[Vector3(0, 1.5, 0), 11.0, 6.0],
	[Vector3(2.5, 1.0, 1.5), 15.0, 8.0],
	[Vector3(0, 2.0, 0), 9.5, 4.0],
	[Vector3(0.5, 0.9, 0), 17.0, 9.0],
	[Vector3(0, 1.3, 0), 8.0, 4.5],
	[Vector3(0, 1.4, 0), 13.0, 5.5],
	[Vector3(0, 1.2, 0), 13.0, 6.5],
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
# --- driving state ---
var car_chassis: RigidBody3D
var car_rear: Array = []
var car_front: Array = []
# --- ragdoll pile state ---
var pile_count := 0
# --- bounce house state ---
var bounce_bodies: Array = []
# --- character course state ---
var cc_pos := Vector3.ZERO
var cc_vel := Vector3.ZERO
var cc_mesh: MeshInstance3D
var cc_platform: AnimatableBody3D
var cc_platform_x := 0.0
# --- card house state ---
var card_bodies: Array = []
var card_hit := false
# --- explosion state ---
var expl_done := false
var flash: MeshInstance3D
var flash_t := -1.0
# --- junkyard state ---
var junk_count := 0
var junk_imploded := false
var junk_exploded := false

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
var mat_car: StandardMaterial3D
var mat_wheel: StandardMaterial3D
var mat_domino: StandardMaterial3D
var mat_jenga: StandardMaterial3D
var mat_ball: StandardMaterial3D
var mat_card: StandardMaterial3D
var mat_platform: StandardMaterial3D
var mat_flash: StandardMaterial3D


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

	# Optional: SHOWCASE_START=<0-11> jumps straight to a demo (used for fast
	# single-demo verification renders). Defaults to the first demo.
	var start := 0
	var start_env := OS.get_environment("SHOWCASE_START")
	if start_env != "":
		start = clampi(int(start_env), 0, DEMO_COUNT - 1)
	_switch_to(start)


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
	mat_car = _mat(Color(0.85, 0.25, 0.3))
	mat_wheel = _mat(Color(0.15, 0.15, 0.18))
	mat_domino = _mat(Color(0.4, 0.75, 0.85))
	mat_jenga = _mat(Color(0.8, 0.65, 0.4))
	mat_ball = _mat(Color(0.95, 0.85, 0.3))
	mat_card = _mat(Color(0.9, 0.9, 0.95))
	mat_platform = _mat(Color(0.9, 0.55, 0.2))
	mat_flash = _mat(Color(1.0, 0.5, 0.15))
	mat_flash.emission_enabled = true
	mat_flash.emission = Color(1.0, 0.4, 0.1)
	mat_flash.emission_energy_multiplier = 4.0


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
			KEY_5: _switch_to(4)
			KEY_6: _switch_to(5)
			KEY_7: _switch_to(6)
			KEY_8: _switch_to(7)
			KEY_9: _switch_to(8)
			KEY_0: _switch_to(9)
			KEY_MINUS: _switch_to(10)
			KEY_EQUAL: _switch_to(11)


func _switch_to(idx: int) -> void:
	current = idx
	demo_time = 0.0
	label.text = "%d / %d  —  %s" % [idx + 1, DEMO_COUNT, DEMO_NAMES[idx]]

	# Reset per-demo state and drop references into the demo being torn down.
	car_chassis = null
	car_rear.clear()
	car_front.clear()
	pile_count = 0
	bounce_bodies.clear()
	cc_platform = null
	card_bodies.clear()
	card_hit = false
	expl_done = false
	flash = null
	flash_t = -1.0
	junk_count = 0
	junk_imploded = false
	junk_exploded = false

	if demo_root:
		demo_root.queue_free()
	demo_root = Node3D.new()
	add_child(demo_root)

	match idx:
		0: _build_stacking()
		1: _build_ragdoll()
		2: _build_character()
		3: _build_ccd()
		4: _build_driving()
		5: _build_pile()
		6: _build_dominoes_jenga()
		7: _build_bounce_house()
		8: _build_character_course()
		9: _build_card_house()
		10: _build_explosion()
		11: _build_junkyard()


func _process(delta: float) -> void:
	demo_time += delta
	if demo_time >= DEMO_SECONDS:
		_switch_to((current + 1) % DEMO_COUNT)

	# Stacking: lob a heavy sphere into the pyramid at t = 4 s.
	if current == 0 and not stack_sphere_spawned and demo_time >= 4.0:
		stack_sphere_spawned = true
		var s := _rigid_sphere(Vector3(9, 4.5, 0), 0.9, mat_heavy)
		s.mass = 12.0
		s.linear_velocity = Vector3(-14, 1.5, 0)

	# Ragdoll pile: drop one humanoid roughly every 0.34 s (staggered over ~2 s).
	if current == 5 and pile_count < 6 and demo_time >= pile_count * 0.34:
		_spawn_pile_ragdoll(pile_count)
		pile_count += 1

	# Card house: unfreeze the cards and roll a heavy sphere in at t = 5 s.
	if current == 9 and not card_hit and demo_time >= 5.0:
		card_hit = true
		for c in card_bodies:
			if is_instance_valid(c):
				c.freeze = false
		var s := _rigid_sphere(Vector3(-4.5, 0.5, 0), 0.5, mat_heavy)
		s.mass = 8.0
		s.linear_velocity = Vector3(9.0, 0, 0)

	# Explosion: fire the radial blast + red flash at t = 3 s, then fade the flash.
	if current == 10:
		if not expl_done and demo_time >= 3.0:
			expl_done = true
			_explode(Vector3(0, 0.5, 0), 4.0, 2.0, 8.0)
			flash = _spawn_flash(Vector3(0, 0.5, 0))
			flash_t = demo_time
		if flash and is_instance_valid(flash):
			var age := demo_time - flash_t
			if age > 0.4:
				flash.queue_free()
				flash = null
			else:
				var s := 1.0 + age * 12.0
				flash.scale = Vector3(s, s, s)

	# Junkyard: fill the pile over ~1 s, implode at t = 4 s, explode at t = 6 s.
	if current == 11:
		if junk_count < 50 and demo_time < 1.2:
			var target := int(min(50, (demo_time / 1.0) * 50.0)) + 1
			while junk_count < target and junk_count < 50:
				_spawn_junk(junk_count)
				junk_count += 1
		if not junk_imploded and demo_time >= 4.0:
			junk_imploded = true
			_explode(Vector3(0, 1.0, 0), 8.0, 1.5, -9.0)
		if not junk_exploded and demo_time >= 6.0:
			junk_exploded = true
			_explode(Vector3(0, 1.0, 0), 8.0, 1.5, 16.0)

	# Slow camera orbit around the active demo (tracks the car while driving).
	orbit_angle += delta * 0.35
	var cfg = cam_cfg[current]
	var target: Vector3 = cfg[0]
	var r: float = cfg[1]
	var h: float = cfg[2]
	if current == 4 and is_instance_valid(car_chassis):
		target = car_chassis.global_position
	camera.position = target + Vector3(cos(orbit_angle) * r, h, sin(orbit_angle) * r)
	camera.look_at(target, Vector3.UP)


func _physics_process(delta: float) -> void:
	match current:
		2: _step_character(delta)
		4: _step_driving()
		7: _step_bounce()
		8: _step_character_course(delta)


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
# Demo 2: Ragdoll (single, tumbling over a ledge)
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

	# Dropped from ~3 m already tilted so it topples over the ledge and tumbles.
	var spawn := Transform3D(Basis(Vector3(0, 0, 1), deg_to_rad(62)), Vector3(-0.6, 3.4, 0))
	_spawn_ragdoll(spawn, Vector3(2.5, 0, 0))


# Builds one jointed humanoid in the given spawn frame with an initial velocity.
func _spawn_ragdoll(spawn: Transform3D, drift: Vector3) -> void:
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

	for row in range(6):
		for col in range(5):
			var y := 0.6 + row * 0.9
			var z := (col - 2) * 0.9
			var w := _rigid_box(Vector3(0, y, z), Vector3(0.05, 0.85, 0.85), mat_wall)
			w.mass = 0.5

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
# Demo 5: Driving (4-wheel vehicle on Box3DWheelJoint3D)
# ---------------------------------------------------------------------------

func _build_driving() -> void:
	_ground(Vector3(200, 1, 200))

	# The wheel axle is world X, so the car travels along Z. Course runs along +Z:
	# flat approach, a ramp up, then a drop off the high (+Z) end.
	var ramp := StaticBody3D.new()
	var rcs := CollisionShape3D.new()
	var rbox := BoxShape3D.new()
	rbox.size = Vector3(20.0, 0.4, 10.0)
	rcs.shape = rbox
	ramp.add_child(rcs)
	# Sunk so its low edge is below ground: wheels roll onto the slope smoothly.
	ramp.transform = Transform3D(Basis(Vector3(1, 0, 0), deg_to_rad(-11)), Vector3(0, 0.75, 3.2))
	_mesh_for(ramp, rbox.size, mat_obstacle)
	demo_root.add_child(ramp)

	car_chassis = RigidBody3D.new()
	var ccs := CollisionShape3D.new()
	var cbox := BoxShape3D.new()
	cbox.size = Vector3(1.2, 0.35, 2.4)
	ccs.shape = cbox
	car_chassis.add_child(ccs)
	car_chassis.mass = 20.0
	car_chassis.position = Vector3(0, 0.6, -5.0)
	_mesh_for(car_chassis, cbox.size, mat_car)
	demo_root.add_child(car_chassis)

	# x = axle side, z = front(-)/rear(+); wheels sit below the chassis.
	_add_wheel(Vector3(-0.75, -0.25, -0.9), true)
	_add_wheel(Vector3(0.75, -0.25, -0.9), true)
	_add_wheel(Vector3(-0.75, -0.25, 0.9), false)
	_add_wheel(Vector3(0.75, -0.25, 0.9), false)


func _add_wheel(local_pos: Vector3, is_front: bool) -> void:
	var wheel := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var sphere := SphereShape3D.new()
	sphere.radius = WHEEL_RADIUS
	cs.shape = sphere
	wheel.add_child(cs)
	wheel.mass = 2.0
	wheel.position = car_chassis.position + local_pos
	var mesh := MeshInstance3D.new()
	var sphere_mesh := SphereMesh.new()
	sphere_mesh.radius = WHEEL_RADIUS
	sphere_mesh.height = WHEEL_RADIUS * 2.0
	mesh.mesh = sphere_mesh
	mesh.material_override = mat_wheel
	wheel.add_child(mesh)
	demo_root.add_child(wheel)

	var joint := Box3DWheelJoint3D.new()
	# Joint frame: local X = up (suspension/steer axis), local Z = axle.
	joint.transform = Transform3D(
		Basis(Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(1, 0, 0)),
		car_chassis.position + local_pos)
	joint.suspension_enabled = true
	joint.suspension_hertz = 4.0
	joint.suspension_damping_ratio = 0.7
	joint.suspension_limit_enabled = true
	joint.suspension_limit_lower = -0.15
	joint.suspension_limit_upper = 0.15
	# All-wheel drive for reliable ramp climbing; front wheels also steer.
	joint.motor_enabled = true
	joint.motor_max_torque = DRIVE_TORQUE
	joint.motor_speed = 0.0
	if is_front:
		joint.steering_enabled = true
		joint.steering_hertz = 8.0
		joint.steering_damping_ratio = 1.0
		joint.steering_max_torque = 200.0
		joint.steering_limit_enabled = true
		joint.steering_limit_lower = -STEER_ANGLE
		joint.steering_limit_upper = STEER_ANGLE
	demo_root.add_child(joint)
	joint.node_a = joint.get_path_to(car_chassis)
	joint.node_b = joint.get_path_to(wheel)

	# Every wheel is driven; front wheels also steer.
	car_rear.append(joint)
	if is_front:
		car_front.append(joint)


func _step_driving() -> void:
	# Ease the throttle in so the car doesn't wheelie off the line.
	var drive := clampf((demo_time - 0.4) / 1.5, 0.0, 1.0)
	var steer := sin(demo_time * 0.8) * 0.08
	for j in car_rear:
		if is_instance_valid(j):
			j.motor_speed = drive * DRIVE_SPEED
	for j in car_front:
		if is_instance_valid(j):
			j.steering_target_angle = steer * STEER_ANGLE
	if OS.get_environment("SHOWCASE_START") != "" and Engine.get_physics_frames() % 30 == 0:
		print("DRIVE t=%.1f chassis=%s spd=%.2f" % [
			demo_time, car_chassis.global_position, car_chassis.linear_velocity.length()])


# ---------------------------------------------------------------------------
# Demo 6: Ragdoll Pile
# ---------------------------------------------------------------------------

func _build_pile() -> void:
	# Small pit walls keep the pile compact under the camera.
	_ground(Vector3(40, 1, 40))
	pile_count = 0


func _spawn_pile_ragdoll(i: int) -> void:
	var ox := sin(i * 1.3) * 0.7
	var oz := cos(i * 1.7) * 0.7
	var height := 3.3 + i * 0.15
	var axis := Vector3(0, 0, 1) if i % 2 == 0 else Vector3(1, 0, 0)
	var spawn := Transform3D(Basis(axis, deg_to_rad(25 + i * 22)), Vector3(ox, height, oz))
	_spawn_ragdoll(spawn, Vector3(ox * 0.5, 0, oz * 0.5))


# ---------------------------------------------------------------------------
# Demo 7: Dominoes & Jenga
# ---------------------------------------------------------------------------

func _build_dominoes_jenga() -> void:
	_ground(Vector3(80, 1, 80))

	# Curved run of 25 thin dominoes (thin along local Z = fall direction).
	var n := 25
	for i in range(n):
		var p := _domino_pos(i)
		var dir := (_domino_pos(i + 1) - _domino_pos(i - 1)).normalized()
		var d := _rigid_box(p, Vector3(0.34, 0.7, 0.08), mat_domino)
		d.mass = 0.2
		d.rotation = Vector3(0, atan2(dir.x, dir.z), 0)

	# Nudge sphere: shoved into the first domino to start the cascade.
	var dir0 := (_domino_pos(1) - _domino_pos(0)).normalized()
	var nudge := _rigid_sphere(_domino_pos(0) - dir0 * 0.7 + Vector3(0, 0.15, 0), 0.22, mat_heavy)
	nudge.mass = 1.5
	nudge.linear_velocity = dir0 * 5.5

	# 8-level Jenga tower off to the side (3 blocks/level, alternating orientation).
	var cx := 6.0
	var cz := 3.0
	for level in range(8):
		var y := 0.16 + level * 0.29
		for k in range(3):
			var off := (k - 1) * 0.3
			var size: Vector3
			var pos: Vector3
			if level % 2 == 0:
				size = Vector3(0.9, 0.28, 0.28)
				pos = Vector3(cx, y, cz + off)
			else:
				size = Vector3(0.28, 0.28, 0.9)
				pos = Vector3(cx + off, y, cz)
			var blk := _rigid_box(pos, size, mat_jenga)
			blk.mass = 0.3

	# Runner sphere rolls along the ground into the Jenga base and topples it.
	var runner := _rigid_sphere(Vector3(1.5, 0.4, cz), 0.4, mat_ball)
	runner.mass = 4.0
	runner.linear_velocity = Vector3(7.0, 0, 0)


func _domino_pos(i: int) -> Vector3:
	var x := -5.0 + i * 0.45
	var z := 1.6 * sin(i * 0.16)
	return Vector3(x, 0.35, z)


# ---------------------------------------------------------------------------
# Demo 8: Bounce House (CCD stress: fast spheres in a thin-walled room)
# ---------------------------------------------------------------------------

func _build_bounce_house() -> void:
	var bouncy := PhysicsMaterial.new()
	bouncy.bounce = 1.0
	bouncy.friction = 0.0

	# Room: 6 x 4 x 6 interior centred at (0,2,0). Five rendered walls + an
	# invisible collider on the open front face so nothing escapes.
	_wall(Vector3(0, -0.05, 0), Vector3(6.2, 0.1, 6.2), bouncy, true)   # floor
	_wall(Vector3(0, 4.05, 0), Vector3(6.2, 0.1, 6.2), bouncy, true)    # ceiling
	_wall(Vector3(0, 2, -3.05), Vector3(6.2, 4.0, 0.1), bouncy, true)   # back
	_wall(Vector3(-3.05, 2, 0), Vector3(0.1, 4.0, 6.2), bouncy, true)   # left
	_wall(Vector3(3.05, 2, 0), Vector3(0.1, 4.0, 6.2), bouncy, true)    # right
	_wall(Vector3(0, 2, 3.05), Vector3(6.2, 4.0, 0.1), bouncy, false)   # front (open, invisible collider)

	var dirs := [
		Vector3(1, 0.6, 0.4), Vector3(-0.7, 1, 0.5), Vector3(0.5, -0.8, 1),
		Vector3(-1, -0.4, -0.6), Vector3(0.8, 0.9, -0.7), Vector3(-0.6, 0.5, 1),
		Vector3(1, -0.7, -0.5), Vector3(-0.9, -0.6, 0.8),
	]
	var spots := [
		Vector3(-1.5, 1.5, -1), Vector3(1.5, 2.5, 1), Vector3(0, 1, 1.5),
		Vector3(-1, 3, 0.5), Vector3(1, 1.5, -1.5), Vector3(-1.5, 2, 1.2),
		Vector3(1.4, 3.2, -0.8), Vector3(0.5, 2.2, -1.3),
	]
	for i in range(8):
		var b := _rigid_sphere(spots[i], 0.22, mat_ball)
		b.mass = 1.0
		b.gravity_scale = 0.0
		b.continuous_cd = true
		b.physics_material_override = bouncy
		b.linear_velocity = dirs[i].normalized() * BOUNCE_SPEED
		bounce_bodies.append(b)


func _wall(pos: Vector3, size: Vector3, mat: PhysicsMaterial, visible: bool) -> void:
	var w := StaticBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = size
	cs.shape = box
	w.add_child(cs)
	w.position = pos
	w.physics_material_override = mat
	if visible:
		_mesh_for(w, size, mat_wall)
	demo_root.add_child(w)


func _step_bounce() -> void:
	# Keep every ball at constant speed so it ricochets indefinitely (also guards
	# against any restitution loss).
	for b in bounce_bodies:
		if is_instance_valid(b):
			var v: Vector3 = b.linear_velocity
			if v.length() > 0.01:
				b.linear_velocity = v.normalized() * BOUNCE_SPEED


# ---------------------------------------------------------------------------
# Demo 9: Character Course (kinematic mover over a richer level)
# ---------------------------------------------------------------------------

func _build_character_course() -> void:
	_ground(Vector3(60, 1, 60))
	cc_pos = Vector3(-7.0, 0.9, 0.0)
	cc_vel = Vector3.ZERO

	# 4-step staircase (0.25 m risers) rising to y = 1.0.
	for i in range(4):
		var h := 0.25 * (i + 1)
		var step := _static_box(Vector3(-5.6 + i * 0.8, h * 0.5, 0), Vector3(0.8, h, 3.0), mat_obstacle)
		step.name = "step%d" % i

	# Top landing, a narrow bridge, a gap (filled by the platform), a far landing.
	_static_box(Vector3(-1.8, 0.5, 0), Vector3(2.0, 1.0, 3.0), mat_obstacle)      # top landing
	_static_box(Vector3(0.5, 0.9, 0), Vector3(2.6, 0.2, 1.0), mat_wall)           # narrow bridge
	_static_box(Vector3(4.4, 0.5, 0), Vector3(1.6, 1.0, 3.0), mat_obstacle)       # far landing

	# Descending ramp back to the ground (+X end lower).
	var ramp := _static_box(Vector3(6.6, 0.55, 0), Vector3(3.0, 0.3, 3.0), mat_obstacle)
	ramp.transform = Transform3D(Basis(Vector3(0, 0, 1), deg_to_rad(-20)), Vector3(6.6, 0.6, 0))

	# Moving kinematic platform that ferries the mover across the gap.
	cc_platform = AnimatableBody3D.new()
	var pcs := CollisionShape3D.new()
	var pbox := BoxShape3D.new()
	pbox.size = Vector3(1.8, 0.2, 1.8)
	pcs.shape = pbox
	cc_platform.add_child(pcs)
	cc_platform.position = Vector3(2.5, 0.9, 0)
	cc_platform_x = 2.5
	_mesh_for(cc_platform, pbox.size, mat_platform)
	demo_root.add_child(cc_platform)

	# Visible capsule (no physics body — driven by the mover API).
	cc_mesh = MeshInstance3D.new()
	var cmesh := CapsuleMesh.new()
	cmesh.radius = CH_RADIUS
	cmesh.height = CH_HALF * 2.0 + CH_RADIUS * 2.0
	cc_mesh.mesh = cmesh
	cc_mesh.material_override = mat_char
	cc_mesh.position = cc_pos
	demo_root.add_child(cc_mesh)


func _step_character_course(delta: float) -> void:
	if cc_mesh == null or not server.has_method("space_collide_mover"):
		return
	var space: RID = get_world_3d().space

	# Ferry the platform along X and carry the mover if it is standing on it.
	var px := 2.5 + 0.5 * (1.0 - cos(demo_time * 1.7))
	if is_instance_valid(cc_platform):
		var dx := px - cc_platform_x
		cc_platform.position = Vector3(px, 0.9, 0)
		if absf(cc_pos.x - px) < 0.9 and absf(cc_pos.z) < 0.9 and absf(cc_pos.y - 1.9) < 0.4:
			cc_pos.x += dx
		cc_platform_x = px

	# Head +X along the course; in the gap, only step forward when floor is ahead.
	var want_x := 2.5
	if cc_pos.x > 1.6 and cc_pos.x < 3.7:
		var ahead := cc_pos.x + 0.5
		if ahead <= px - 0.9 or ahead >= px + 0.9:
			want_x = 0.0
	cc_vel.x = want_x
	cc_vel.z = -cc_pos.z * 2.0
	cc_vel.y -= 9.8 * delta

	var c1 := cc_pos + Vector3(0, -CH_HALF, 0)
	var c2 := cc_pos + Vector3(0, CH_HALF, 0)
	var planes: Array = server.space_collide_mover(space, c1, c2, CH_RADIUS, 0xFFFFFFFF)
	var solved: Dictionary = server.solve_mover_planes(cc_vel * delta, planes)
	cc_pos += solved["delta"]
	cc_vel = server.clip_mover_velocity(cc_vel, solved["planes"])

	if cc_pos.x > 8.5:  # completed the course — loop back to the start
		cc_pos = Vector3(-7.0, 0.9, 0.0)
		cc_vel = Vector3.ZERO
	cc_mesh.position = cc_pos


# ---------------------------------------------------------------------------
# Demo 10: Card House
# ---------------------------------------------------------------------------

func _build_card_house() -> void:
	# Low-friction floor.
	var floor_mat := PhysicsMaterial.new()
	floor_mat.friction = 0.4
	var g := StaticBody3D.new()
	var gcs := CollisionShape3D.new()
	var gbox := BoxShape3D.new()
	gbox.size = Vector3(40, 1, 40)
	gcs.shape = gbox
	g.add_child(gcs)
	g.position = Vector3(0, -0.5, 0)
	g.physics_material_override = floor_mat
	_mesh_for(g, gbox.size, mat_ground)
	demo_root.add_child(g)

	# 3-level house of cards: leaning tents bridged by flat cards. Cards are frozen
	# so the house stands until the sphere hits (then they release and collapse).
	_card_tent(-0.9, 0.0)
	_card_tent(0.0, 0.0)
	_card_tent(0.9, 0.0)
	_card_flat(-0.45, 0.88)
	_card_flat(0.45, 0.88)

	_card_tent(-0.45, 0.89)
	_card_tent(0.45, 0.89)
	_card_flat(0.0, 1.77)

	_card_tent(0.0, 1.78)


func _card_tent(cx: float, base_y: float) -> void:
	# Two thin cards leaning together into an A-frame.
	_card(Vector3(cx - 0.22, base_y + 0.45, 0), deg_to_rad(-15))
	_card(Vector3(cx + 0.22, base_y + 0.45, 0), deg_to_rad(15))


func _card(pos: Vector3, rotz: float) -> void:
	var c := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(0.02, 0.9, 0.6)
	cs.shape = box
	c.add_child(cs)
	c.mass = 0.05
	c.transform = Transform3D(Basis(Vector3(0, 0, 1), rotz), pos)
	c.freeze_mode = RigidBody3D.FREEZE_MODE_STATIC
	c.freeze = true
	_mesh_for(c, box.size, mat_card)
	demo_root.add_child(c)
	card_bodies.append(c)


func _card_flat(cx: float, y: float) -> void:
	var c := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(0.9, 0.02, 0.6)
	cs.shape = box
	c.add_child(cs)
	c.mass = 0.05
	c.position = Vector3(cx, y, 0)
	c.freeze_mode = RigidBody3D.FREEZE_MODE_STATIC
	c.freeze = true
	_mesh_for(c, box.size, mat_card)
	demo_root.add_child(c)
	card_bodies.append(c)


# ---------------------------------------------------------------------------
# Demo 11: Explosion (server.space_explode radial impulse)
# ---------------------------------------------------------------------------

func _build_explosion() -> void:
	_ground(Vector3(60, 1, 60))

	# 6 x 6 x 3 wall of small boxes (boxes are hulls — the blast acts on them).
	for i in range(6):
		for j in range(6):
			for k in range(3):
				var pos := Vector3((i - 2.5) * 0.42, 0.22 + j * 0.42, (k - 1) * 0.42)
				var b := _rigid_box(pos, Vector3(0.4, 0.4, 0.4), mat_box)
				b.mass = 0.3

	# A few spheres and capsules scattered in front of the wall.
	_rigid_sphere(Vector3(-1.2, 0.3, 2.2), 0.3, mat_ccd_on).mass = 0.4
	_rigid_sphere(Vector3(1.0, 0.3, 2.6), 0.3, mat_ccd_off).mass = 0.4
	_rigid_sphere(Vector3(0.2, 0.3, 3.0), 0.25, mat_ball).mass = 0.3
	_rigid_capsule(Vector3(-0.6, 0.5, 2.0), 0.2, 0.4, mat_char).mass = 0.4
	_rigid_capsule(Vector3(1.6, 0.5, 2.0), 0.2, 0.4, mat_limb).mass = 0.4


func _spawn_flash(pos: Vector3) -> MeshInstance3D:
	var mi := MeshInstance3D.new()
	var sm := SphereMesh.new()
	sm.radius = 0.4
	sm.height = 0.8
	mi.mesh = sm
	mi.material_override = mat_flash
	mi.position = pos
	demo_root.add_child(mi)
	return mi


# ---------------------------------------------------------------------------
# Demo 12: Junkyard (implosion then explosion)
# ---------------------------------------------------------------------------

func _build_junkyard() -> void:
	_ground(Vector3(60, 1, 60))
	junk_count = 0


func _spawn_junk(i: int) -> void:
	var m := _mat(Color.from_hsv(fposmod(i * 0.13, 1.0), 0.55, 0.9))
	var x := sin(i * 2.3) * 1.1
	var z := cos(i * 1.9) * 1.1
	var y := 1.4 + (i % 10) * 0.24
	var b: RigidBody3D
	match i % 3:
		0:
			var s := 0.3 + (i % 3) * 0.08
			b = _rigid_box(Vector3(x, y, z), Vector3(s, s * 1.4, s), m)
		1:
			b = _rigid_sphere(Vector3(x, y, z), 0.22 + (i % 4) * 0.04, m)
		_:
			b = _rigid_capsule(Vector3(x, y, z), 0.16, 0.35, m)
	b.mass = 0.5


func _explode(center: Vector3, radius: float, falloff: float, impulse: float) -> void:
	if not server.has_method("space_explode"):
		return
	server.space_explode(get_world_3d().space, center, radius, falloff, impulse, 0xFFFFFFFF)


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


func _static_box(pos: Vector3, size: Vector3, mat: StandardMaterial3D) -> StaticBody3D:
	var b := StaticBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = size
	cs.shape = box
	b.add_child(cs)
	b.position = pos
	_mesh_for(b, size, mat)
	demo_root.add_child(b)
	return b


func _rigid_capsule(pos: Vector3, radius: float, seg: float, mat: StandardMaterial3D) -> RigidBody3D:
	var b := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var cap := CapsuleShape3D.new()
	cap.radius = radius
	cap.height = seg + radius * 2.0
	cs.shape = cap
	b.add_child(cs)
	b.position = pos
	var mi := MeshInstance3D.new()
	var cm := CapsuleMesh.new()
	cm.radius = radius
	cm.height = seg + radius * 2.0
	mi.mesh = cm
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
