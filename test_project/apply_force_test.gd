extends SceneTree

# Regression test: apply_impulse/apply_force `position` is a GLOBAL-space offset from
# the body origin. Before the fix the wrapper rotated it by the body basis, so on a
# yawed body the application point swung away from its true location (vehicles rolled
# over in turns as wheel forces landed at rotated points).

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
	body.rotation = Vector3(0, PI / 2.0, 0) # yawed 90 deg: the discriminator
	body.gravity_scale = 0.0
	root.add_child(body)


func _process(_delta: float) -> bool:
	frames += 1
	if frames == 5:
		# +Y impulse at global offset +X from the origin: torque r x F = X x Y = +Z.
		# The old (rotated) interpretation turned the offset into -Z, giving +X torque.
		body.apply_impulse(Vector3(0, 2, 0), Vector3(1, 0, 0))
	if frames == 8:
		var av := body.angular_velocity
		print("Angular velocity: ", av)
		if absf(av.z) > absf(av.x) * 4.0 and av.z > 0.1:
			print("RESULT: PASS - global-offset impulse produced +Z torque on a yawed body")
		else:
			print("RESULT: FAIL - torque axis wrong (", av, ") - offset was rotated by the basis")
		quit()
	return false
