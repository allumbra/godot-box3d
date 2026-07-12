extends SceneTree

# Regression test for mass/inertia semantics: shapes carry density 1.0, and setting an
# explicit mass must SCALE the automatic inertia (Godot semantics). Before the fix a
# 100 kg unit cube kept its 1 kg-scale inertia (~0.167) — off-center forces then spun
# bodies ~100x too hard (the "cars shake/fly" bug).

var frames := 0
var body: RigidBody3D
var com_body: RigidBody3D


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	body = RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(1, 1, 1)
	cs.shape = box
	body.add_child(cs)
	body.position = Vector3(0, 5, 0)
	body.mass = 100.0
	root.add_child(body)

	# Custom (lowered) center of mass: automatic inertia must gain the parallel-axis
	# term about the new center (vehicles lower COM for rollover resistance).
	com_body = RigidBody3D.new()
	var cs2 := CollisionShape3D.new()
	var box2 := BoxShape3D.new()
	box2.size = Vector3(1, 1, 1)
	cs2.shape = box2
	com_body.add_child(cs2)
	com_body.position = Vector3(5, 5, 0)
	com_body.mass = 100.0
	com_body.center_of_mass_mode = RigidBody3D.CENTER_OF_MASS_MODE_CUSTOM
	com_body.center_of_mass = Vector3(0, -0.5, 0)
	root.add_child(com_body)


func _process(_delta: float) -> bool:
	frames += 1
	if frames == 5:
		var inertia: Vector3 = PhysicsServer3D.body_get_param(
			body.get_rid(), PhysicsServer3D.BODY_PARAM_INERTIA)
		# Unit cube, mass 100: I = m * (1^2 + 1^2) / 12 = 16.667 on every axis.
		print("Inertia readback: ", inertia)
		var expected := 100.0 * 2.0 / 12.0
		if absf(inertia.x - expected) < 1.0 and absf(inertia.y - expected) < 1.0 and absf(inertia.z - expected) < 1.0:
			print("RESULT: PASS - auto inertia scales with explicit mass (", inertia.x, " ~= ", expected, ")")
		else:
			print("RESULT: FAIL - inertia ", inertia, " expected ~", expected)

		# Lowered COM: I about the new center = m/6 + m*d^2 = 16.667 + 100*0.25 = 41.667
		# on the two perpendicular axes; the vertical (Y) axis is unchanged.
		var com_inertia: Vector3 = PhysicsServer3D.body_get_param(
			com_body.get_rid(), PhysicsServer3D.BODY_PARAM_INERTIA)
		print("Custom-COM inertia readback: ", com_inertia)
		var expected_perp := expected + 100.0 * 0.25
		if absf(com_inertia.x - expected_perp) < 1.0 and absf(com_inertia.z - expected_perp) < 1.0 \
				and absf(com_inertia.y - expected) < 1.0:
			print("RESULT: PASS - parallel-axis correction applied for custom COM")
		else:
			print("RESULT: FAIL - custom-COM inertia ", com_inertia, " expected (~", expected_perp, ", ~", expected, ", ~", expected_perp, ")")
		quit()
	return false
