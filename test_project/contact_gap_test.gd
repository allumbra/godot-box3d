extends SceneTree

# Measures the resting surface gap between a dynamic box pushed against a static
# box wall. Surfaces should touch within ~1cm (solver slop). A large gap means
# collision geometry is inflated beyond the authored shape.

var frames := 0
var pusher: RigidBody3D


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	var ground := StaticBody3D.new()
	var gcs := CollisionShape3D.new()
	var gbox := BoxShape3D.new()
	gbox.size = Vector3(40, 1, 40)
	gcs.shape = gbox
	ground.add_child(gcs)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)

	# Wall: 1m thick box, near face at z = 0 (center z = 0.5).
	var wall := StaticBody3D.new()
	var wcs := CollisionShape3D.new()
	var wbox := BoxShape3D.new()
	wbox.size = Vector3(6, 3, 1)
	wcs.shape = wbox
	wall.add_child(wcs)
	wall.position = Vector3(0, 1.5, 0.5)
	root.add_child(wall)

	# Pusher: 1m cube sliding +Z into the wall; its front face should rest at z ~= 0.
	pusher = RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(1, 1, 1)
	cs.shape = box
	pusher.add_child(cs)
	pusher.position = Vector3(0, 0.55, -4)
	pusher.lock_rotation = true # keep the front face planar so the gap measure is valid
	root.add_child(pusher)


func _process(_delta: float) -> bool:
	frames += 1
	# Keep pushing gently so it settles against the wall.
	pusher.apply_central_force(Vector3(0, 0, 40))
	if frames == 300:
		var front_z: float = pusher.global_position.z + 0.5
		var gap := 0.0 - front_z
		print("pusher front z: ", front_z, "  gap to wall face: ", -front_z,
			"  rotation: ", pusher.rotation, "  vel: ", pusher.linear_velocity.z)
		if absf(gap) < 0.02:
			print("RESULT: PASS - surfaces touch (gap ", gap, " m)")
		else:
			print("RESULT: FAIL - resting gap of ", gap, " m between surfaces")
		quit()
	return false
