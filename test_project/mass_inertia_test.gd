extends SceneTree

# Regression test for mass/inertia semantics: shapes carry density 1.0, and setting an
# explicit mass must SCALE the automatic inertia (Godot semantics). Before the fix a
# 100 kg unit cube kept its 1 kg-scale inertia (~0.167) — off-center forces then spun
# bodies ~100x too hard (the "cars shake/fly" bug).

var frames := 0
var body: RigidBody3D


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
		quit()
	return false
