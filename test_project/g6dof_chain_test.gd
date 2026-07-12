extends SceneTree

# Repro of BRCT's chicken spine: vertical chain of capsules linked by
# Generic6DOFJoint3D (linear locked 0/0, angular limits +/-0.7 with softness/damping),
# top pinned. A sideways impulse must produce inter-segment articulation (>20 deg),
# like the game's smoke test expects.

var frames := 0
var segs: Array[RigidBody3D] = []
var max_rel := 0.0


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

	var anchor := StaticBody3D.new()
	var acs := CollisionShape3D.new()
	var asph := SphereShape3D.new()
	asph.radius = 0.05
	acs.shape = asph
	anchor.add_child(acs)
	anchor.position = Vector3(0, 8, 0)
	root.add_child(anchor)

	var span := 1.0
	for i in range(3):
		var seg := RigidBody3D.new()
		seg.mass = 1.0
		seg.can_sleep = false
		seg.continuous_cd = true
		seg.linear_damp = 0.3
		seg.angular_damp = 1.5
		var cs := CollisionShape3D.new()
		var cap := CapsuleShape3D.new()
		cap.radius = 0.15
		cap.height = span
		cs.shape = cap
		seg.add_child(cs)
		seg.position = Vector3(0, 8 - span * 0.5 - i * span, 0)
		root.add_child(seg)
		segs.append(seg)

	for a in range(segs.size()):
		for b in range(a + 1, segs.size()):
			segs[a].add_collision_exception_with(segs[b])

	# Pin top segment to anchor.
	var pin := PinJoint3D.new()
	pin.position = Vector3(0, 8, 0)
	root.add_child(pin)
	pin.node_a = pin.get_path_to(anchor)
	pin.node_b = pin.get_path_to(segs[0])

	# Spine joints, BRCT-style.
	for j in range(segs.size() - 1):
		var joint := Generic6DOFJoint3D.new()
		joint.position = Vector3(0, 8 - (j + 1) * span, 0)
		root.add_child(joint)
		joint.node_a = joint.get_path_to(segs[j])
		joint.node_b = joint.get_path_to(segs[j + 1])
		for axis in ["x", "y", "z"]:
			joint.set("linear_limit_%s/lower_distance" % axis, 0.0)
			joint.set("linear_limit_%s/upper_distance" % axis, 0.0)
			joint.set("angular_limit_%s/lower_angle" % axis, deg_to_rad(-40.0))
			joint.set("angular_limit_%s/upper_angle" % axis, deg_to_rad(40.0))
			joint.set("angular_limit_%s/softness" % axis, 0.5)
			joint.set("angular_limit_%s/damping" % axis, 1.0)


func _process(_delta: float) -> bool:
	frames += 1
	if frames == 60:
		segs[2].apply_central_impulse(Vector3(6, 0, 0))
	if frames > 60:
		for i in range(segs.size() - 1):
			var rel: float = segs[i].global_basis.get_rotation_quaternion().angle_to(
				segs[i + 1].global_basis.get_rotation_quaternion())
			max_rel = maxf(max_rel, rel)
	if frames == 360:
		print("Max inter-segment angle: ", rad_to_deg(max_rel), " deg")
		print("Bottom seg pos: ", segs[2].global_position)
		if rad_to_deg(max_rel) > 20.0:
			print("RESULT: PASS - chain articulates under impulse")
		else:
			print("RESULT: FAIL - chain rigid (", rad_to_deg(max_rel), " deg)")
		quit()
	return false
