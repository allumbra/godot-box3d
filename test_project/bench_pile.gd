extends SceneTree

# BENCH bodies=<n> engine=<name> total_ms=<x> avg_ms=<x> max_ms=<x>
# Also prints SLEEP bodies=<n> engine=<name> asleep=<n>/<n>

const WARMUP_FRAMES := 60
const BENCH_FRAMES := 600

var frame_count: int = 0
var bodies: Array = []
var frame_start_usec: int = 0
var bench_start_usec: int = 0
var max_frame_usec: int = 0
var body_count: int = 500
var engine_name: String = "unknown"
var allow_sleep: bool = true


func _initialize() -> void:
	engine_name = str(ProjectSettings.get_setting("physics/3d/physics_engine"))
	body_count = 500
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("--bodies="):
			body_count = int(arg.substr(9))
		elif arg == "--no-sleep":
			# Fairness mode: sleep heuristics differ between engines, so disabling
			# sleep compares raw solver cost on identical active-body counts.
			allow_sleep = false

	_spawn_ground()
	_spawn_pile(body_count)

	print("BENCH_START bodies=", body_count, " engine=", engine_name)


func _spawn_ground() -> void:
	var ground := StaticBody3D.new()
	var ground_shape := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(40, 1, 40)
	ground_shape.shape = box
	ground.add_child(ground_shape)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)


func _spawn_pile(n: int) -> void:
	# Deterministic grid layout, alternating boxes and spheres, stacked in layers
	# so bodies fall onto each other and interact (piling), not just onto the ground.
	var per_layer := 100
	var grid_side := 10  # 10x10 grid per layer
	var spacing := 1.2
	var layer_height := 1.5
	var origin_offset := (grid_side - 1) * spacing * 0.5

	for i in range(n):
		var layer := i / per_layer
		var idx_in_layer := i % per_layer
		var gx := idx_in_layer % grid_side
		var gz := (idx_in_layer / grid_side) % grid_side

		var body := RigidBody3D.new()
		var shape_node := CollisionShape3D.new()

		if i % 2 == 0:
			var box_shape := BoxShape3D.new()
			box_shape.size = Vector3(0.5, 0.5, 0.5)
			shape_node.shape = box_shape
		else:
			var sphere_shape := SphereShape3D.new()
			sphere_shape.radius = 0.3
			shape_node.shape = sphere_shape

		body.add_child(shape_node)
		body.can_sleep = allow_sleep
		body.position = Vector3(
			gx * spacing - origin_offset,
			2.0 + layer * layer_height,
			gz * spacing - origin_offset
		)
		root.add_child(body)
		bodies.append(body)


func _process(delta: float) -> bool:
	frame_count += 1

	if frame_count == WARMUP_FRAMES:
		bench_start_usec = Time.get_ticks_usec()
		max_frame_usec = 0
	elif frame_count > WARMUP_FRAMES:
		var elapsed := Time.get_ticks_usec() - frame_start_usec
		if elapsed > max_frame_usec:
			max_frame_usec = elapsed

	frame_start_usec = Time.get_ticks_usec()

	if frame_count == WARMUP_FRAMES + BENCH_FRAMES:
		var total_usec := Time.get_ticks_usec() - bench_start_usec
		var total_ms := total_usec / 1000.0
		var avg_ms := total_ms / float(BENCH_FRAMES)
		var max_ms := max_frame_usec / 1000.0

		var asleep := 0
		for b in bodies:
			if b.sleeping:
				asleep += 1

		print("BENCH bodies=", body_count, " engine=", engine_name,
			" total_ms=", total_ms, " avg_ms=", avg_ms, " max_ms=", max_ms)
		print("SLEEP bodies=", body_count, " engine=", engine_name,
			" asleep=", asleep, "/", bodies.size())

		quit()
		return true

	return false
