extends SceneTree

# Tests the Box3D-specific body_set_rolling_resistance server API: of two identical
# spheres launched with the same velocity, the one with rolling resistance must
# stop sooner (box3d applies rolling resistance to spheres/capsules).

var frames: int = 0
var free_roller: RigidBody3D
var resisted_roller: RigidBody3D


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	var ground := StaticBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(400, 1, 400)
	cs.shape = box
	ground.add_child(cs)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)

	free_roller = _make_roller(Vector3(0, 0.5, 0))
	resisted_roller = _make_roller(Vector3(0, 0.5, 3))

	# Untyped on purpose: the wheel/rolling methods only exist on the Box3D server, so
	# they must be dispatched dynamically (a typed PhysicsServer3D call fails to parse).
	var server: Object = PhysicsServer3D
	if not server.has_method("body_set_rolling_resistance"):
		print("RESULT: FAIL - body_set_rolling_resistance not exposed")
		quit()
		return
	server.body_set_rolling_resistance(resisted_roller.get_rid(), 0.8)
	print("Resisted readback: ", server.body_get_rolling_resistance(resisted_roller.get_rid()))

	free_roller.linear_velocity = Vector3(8, 0, 0)
	resisted_roller.linear_velocity = Vector3(8, 0, 0)


func _make_roller(pos: Vector3) -> RigidBody3D:
	var body := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var sphere := SphereShape3D.new()
	sphere.radius = 0.5
	cs.shape = sphere
	body.add_child(cs)
	body.position = pos
	root.add_child(body)
	return body


func _process(_delta: float) -> bool:
	frames += 1
	if frames == 300:
		var free_x: float = free_roller.global_position.x
		var resisted_x: float = resisted_roller.global_position.x
		print("Free roller x: ", free_x, "  resisted x: ", resisted_x)
		if resisted_x < free_x - 1.0:
			print("RESULT: PASS - rolling resistance slowed the sphere")
		else:
			print("RESULT: FAIL - rolling resistance had no effect")
		quit()
	return false
