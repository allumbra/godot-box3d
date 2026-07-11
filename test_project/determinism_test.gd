extends SceneTree

# M1 determinism harness.
# Run with:  godot --headless --fixed-fps 60 -s determinism_test.gd
# Spawns a fixed pile (64 boxes + 6 spheres, hardcoded transforms) plus one
# mid-air collision pair, steps exactly N physics frames at fixed delta, then
# hashes every body's transform + velocities as exact float bytes (SHA-256).
# Identical hash across runs/restarts/rebuilds == deterministic.

const STEP_COUNT: int = 1000
const PASSES: int = 2  # in-process repeat: catches state leakage between runs

var bodies: Array[RigidBody3D] = []
var frames: int = 0
var current_pass: int = 1
var ground: StaticBody3D

func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))
	print("Physics ticks/s: ", Engine.physics_ticks_per_second)
	_spawn_scene()


func _spawn_scene() -> void:
	ground = StaticBody3D.new()
	var ground_shape := CollisionShape3D.new()
	var ground_box := BoxShape3D.new()
	ground_box.size = Vector3(40, 1, 40)
	ground_shape.shape = ground_box
	ground.add_child(ground_shape)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)

	# 64 boxes in a 4x4x4 grid with deterministic (integer-arithmetic) jitter.
	var box_shape := BoxShape3D.new()
	box_shape.size = Vector3(0.5, 0.5, 0.5)
	for i in range(4):
		for j in range(4):
			for k in range(4):
				var idx: int = i * 16 + j * 4 + k
				var jitter_x: float = float((idx * 7919) % 100) / 1000.0
				var jitter_z: float = float((idx * 104729) % 100) / 1000.0
				var body := _make_body(box_shape, Vector3(
					float(i) * 0.7 - 1.05 + jitter_x,
					2.0 + float(j) * 0.7,
					float(k) * 0.7 - 1.05 + jitter_z))
				body.rotation = Vector3(0.0, float(idx) * 0.1, 0.0)

	# 6 spheres dropped above the pile.
	var sphere_shape := SphereShape3D.new()
	sphere_shape.radius = 0.3
	for s in range(6):
		_make_body(sphere_shape, Vector3(float(s) * 0.5 - 1.25, 6.0 + float(s) * 0.4, 0.3))

	# Mid-air collision pair: two spheres on a collision course.
	var a := _make_body(sphere_shape, Vector3(-4.0, 8.0, 0.0))
	a.gravity_scale = 0.0
	a.linear_velocity = Vector3(3.0, 0.0, 0.0)
	var b := _make_body(sphere_shape, Vector3(4.0, 8.0, 0.0))
	b.gravity_scale = 0.0
	b.linear_velocity = Vector3(-3.0, 0.0, 0.0)

	print("Bodies spawned: ", bodies.size())


func _make_body(shape: Shape3D, pos: Vector3) -> RigidBody3D:
	var body := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	cs.shape = shape
	body.add_child(cs)
	body.position = pos
	body.can_sleep = true
	root.add_child(body)
	bodies.append(body)
	return body


func _process(_delta: float) -> bool:
	frames += 1
	if frames == STEP_COUNT:
		_report()
		if current_pass >= PASSES:
			quit()
		else:
			current_pass += 1
			frames = 0
			_teardown_scene()
			_spawn_scene()
	return false


func _teardown_scene() -> void:
	for body in bodies:
		body.free()
	bodies.clear()
	ground.free()


func _report() -> void:
	var floats := PackedFloat32Array()
	for body in bodies:
		var t := body.global_transform
		for v in [t.basis.x, t.basis.y, t.basis.z, t.origin,
				body.linear_velocity, body.angular_velocity]:
			floats.push_back(v.x)
			floats.push_back(v.y)
			floats.push_back(v.z)
	var ctx := HashingContext.new()
	ctx.start(HashingContext.HASH_SHA256)
	ctx.update(floats.to_byte_array())
	var digest: PackedByteArray = ctx.finish()
	print("Physics frames observed: ", Engine.get_physics_frames())
	print("Frames stepped (pass ", current_pass, "): ", frames)
	print("STATE_HASH: ", digest.hex_encode())
