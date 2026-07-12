extends SceneTree

# M4 tests: per-pair collision exceptions (filter joints) + pin anchor mutation.

var frames: int = 0
var faller: RigidBody3D
var normal_faller: RigidBody3D
var platform: StaticBody3D
var platform2: StaticBody3D
var anchor: StaticBody3D
var bob: RigidBody3D
var pin_rid: RID
var anchor_moved := false


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	var ground := StaticBody3D.new()
	var gcs := CollisionShape3D.new()
	var gbox := BoxShape3D.new()
	gbox.size = Vector3(100, 1, 100)
	gcs.shape = gbox
	ground.add_child(gcs)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)

	# Exception pair: faller must pass THROUGH platform; control must land ON platform2.
	platform = _platform(Vector3(0, 1.5, 0))
	platform2 = _platform(Vector3(5, 1.5, 0))
	faller = _ball(Vector3(0, 4, 0))
	normal_faller = _ball(Vector3(5, 4, 0))
	faller.add_collision_exception_with(platform)
	print("Exceptions on faller: ", faller.get_collision_exceptions().size())

	# Pin joint: bob hangs below a static anchor; anchor point mutated mid-run.
	anchor = StaticBody3D.new()
	var acs := CollisionShape3D.new()
	var asph := SphereShape3D.new()
	asph.radius = 0.1
	acs.shape = asph
	anchor.add_child(acs)
	anchor.position = Vector3(-5, 4, 0)
	root.add_child(anchor)
	bob = _ball(Vector3(-5, 2.5, 0))
	pin_rid = PhysicsServer3D.joint_create()
	PhysicsServer3D.joint_make_pin(pin_rid, anchor.get_rid(), Vector3(0, 0, 0), bob.get_rid(), Vector3(0, 1.5, 0))


func _platform(pos: Vector3) -> StaticBody3D:
	var p := StaticBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(2, 0.4, 2)
	cs.shape = box
	p.add_child(cs)
	p.position = pos
	root.add_child(p)
	return p


func _ball(pos: Vector3) -> RigidBody3D:
	var body := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var sph := SphereShape3D.new()
	sph.radius = 0.3
	cs.shape = sph
	body.add_child(cs)
	body.position = pos
	root.add_child(body)
	return body


func _process(_delta: float) -> bool:
	frames += 1
	if frames == 200 and not anchor_moved:
		anchor_moved = true
		# Mutate the pin's anchor on the static side: shift it 1m along +X.
		PhysicsServer3D.pin_joint_set_local_a(pin_rid, Vector3(1, 0, 0))
		print("pin local_a readback: ", PhysicsServer3D.pin_joint_get_local_a(pin_rid))
	if frames == 400:
		print("Faller y: ", faller.global_position.y, "  control y: ", normal_faller.global_position.y)
		if faller.global_position.y < 1.0 and normal_faller.global_position.y > 1.5:
			print("RESULT: PASS - collision exception let the pair tunnel; control landed on platform")
		else:
			print("RESULT: FAIL - exception did not behave (faller ", faller.global_position.y, ")")

		print("Bob position: ", bob.global_position)
		if bob.global_position.x > -4.7:
			print("RESULT: PASS - pin anchor mutation shifted the pendulum")
		else:
			print("RESULT: FAIL - bob did not move after anchor mutation (x=", bob.global_position.x, ")")
		quit()
	return false
