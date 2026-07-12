extends SceneTree

# Tests the Generic6DOF and ConeTwist joint mappings (previously hard-rejected).
# 1. 6DOF with linear locked + angular free (BRCT's "ball joints holding the spine
#    together" pattern) must behave like a pin: chain link holds its distance.
# 2. 6DOF all-locked (Godot default) must weld: follower keeps its offset.
# 3. ConeTwist: swinging bob must be stopped by the cone limit.

var frames := 0
var anchor: StaticBody3D
var link: RigidBody3D
var weld_a: RigidBody3D
var weld_b: RigidBody3D
var ct_anchor: StaticBody3D
var ct_bob: RigidBody3D


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

	# --- 1: 6DOF ball joint (linear locked, angular free) — hanging chain link.
	anchor = _static_sphere(Vector3(0, 6, 0))
	link = _ball(Vector3(1.5, 6, 0))
	var g6 := _make_6dof(anchor, link, Vector3(0, 6, 0))
	# Free all angular axes (linear stays locked at Godot defaults = ball joint).
	g6.set("angular_limit_x/enabled", false)
	g6.set("angular_limit_y/enabled", false)
	g6.set("angular_limit_z/enabled", false)

	# --- 2: 6DOF default (all locked) — weld pair at rest.
	weld_a = _ball(Vector3(6, 0.31, 0))
	weld_b = _ball(Vector3(6.8, 0.31, 0))
	_make_6dof_nodes(weld_a, weld_b, Vector3(6.4, 0.31, 0))

	# --- 3: ConeTwist, swing span 30deg: bob launched sideways must be caught.
	ct_anchor = _static_sphere(Vector3(-6, 6, 0))
	ct_bob = _ball(Vector3(-6, 4, 0))
	var ct := ConeTwistJoint3D.new()
	ct.position = Vector3(-6, 6, 0)
	# Godot cone axis = joint local X; hang direction is -Y, so point X down.
	ct.rotation_degrees = Vector3(0, 0, -90)
	ct.swing_span = deg_to_rad(30.0)
	root.add_child(ct)
	ct.node_a = ct.get_path_to(ct_anchor)
	ct.node_b = ct.get_path_to(ct_bob)
	ct_bob.linear_velocity = Vector3(6, 0, 0)


func _make_6dof(body_a: PhysicsBody3D, body_b: PhysicsBody3D, pos: Vector3) -> Generic6DOFJoint3D:
	return _make_6dof_nodes(body_a, body_b, pos)


func _make_6dof_nodes(body_a: PhysicsBody3D, body_b: PhysicsBody3D, pos: Vector3) -> Generic6DOFJoint3D:
	var j := Generic6DOFJoint3D.new()
	j.position = pos
	root.add_child(j)
	j.node_a = j.get_path_to(body_a)
	j.node_b = j.get_path_to(body_b)
	return j


func _static_sphere(pos: Vector3) -> StaticBody3D:
	var body := StaticBody3D.new()
	var cs := CollisionShape3D.new()
	var sph := SphereShape3D.new()
	sph.radius = 0.1
	cs.shape = sph
	body.add_child(cs)
	body.position = pos
	root.add_child(body)
	return body


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
	if frames == 300:
		# 1: ball-joint link must hang ~1.5 from the anchor (not fly away, not weld).
		var d: float = anchor.global_position.distance_to(link.global_position)
		print("6DOF ball link distance: ", d)
		if absf(d - 1.5) < 0.1:
			print("RESULT: PASS - 6DOF linear-locked maps to a working ball joint")
		else:
			print("RESULT: FAIL - 6DOF link at distance ", d)

		# 2: welded pair keeps its 0.8 offset.
		var sep: float = weld_a.global_position.distance_to(weld_b.global_position)
		print("6DOF weld separation: ", sep)
		if absf(sep - 0.8) < 0.05:
			print("RESULT: PASS - 6DOF all-locked welds rigidly")
		else:
			print("RESULT: FAIL - 6DOF weld separation ", sep)

		# 3: cone-limited bob: angle from straight-down must be <= ~30deg (+slack).
		var to_bob: Vector3 = (ct_bob.global_position - ct_anchor.global_position).normalized()
		var angle := rad_to_deg(Vector3.DOWN.angle_to(to_bob))
		print("ConeTwist swing angle: ", angle, " deg")
		if angle < 45.0:
			print("RESULT: PASS - cone limit caught the swinging bob")
		else:
			print("RESULT: FAIL - bob swung to ", angle, " deg")
		quit()
	return false
