extends Node3D

# Scene-level test for Box3DWeldJoint3D / Box3DDistanceJoint3D / Box3DMotorJoint3D
# nodes (R3): the node path derives joint frames from node transforms at READY.

var frames := 0
var weld_a: RigidBody3D
var weld_b: RigidBody3D
var bob: RigidBody3D
var anchor: StaticBody3D
var spinner: RigidBody3D
var dist_joint: Box3DDistanceJoint3D


func _ready() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	var ground := StaticBody3D.new()
	var gcs := CollisionShape3D.new()
	var gbox := BoxShape3D.new()
	gbox.size = Vector3(100, 1, 100)
	gcs.shape = gbox
	ground.add_child(gcs)
	ground.position = Vector3(0, -0.5, 0)
	add_child(ground)

	# Weld: pair at rest height, joint node placed between them.
	weld_a = _box(Vector3(-10, 0.31, 0))
	weld_b = _box(Vector3(-9.4, 0.31, 0))
	var weld := Box3DWeldJoint3D.new()
	weld.position = Vector3(-9.7, 0.31, 0)
	add_child(weld)
	weld.node_a = weld.get_path_to(weld_a)
	weld.node_b = weld.get_path_to(weld_b)

	# Distance: pendulum rod from a static anchor, joint node at the anchor.
	anchor = StaticBody3D.new()
	var acs := CollisionShape3D.new()
	var asph := SphereShape3D.new()
	asph.radius = 0.1
	acs.shape = asph
	anchor.add_child(acs)
	anchor.position = Vector3(0, 6, 0)
	add_child(anchor)
	bob = _sphere(Vector3(2, 6, 0))
	dist_joint = Box3DDistanceJoint3D.new()
	dist_joint.position = Vector3(0, 6, 0)
	dist_joint.length = 2.0
	add_child(dist_joint)
	dist_joint.node_a = dist_joint.get_path_to(anchor)
	dist_joint.node_b = dist_joint.get_path_to(bob)

	# Motor: angular-velocity drive on a floating body.
	var base := StaticBody3D.new()
	var bcs := CollisionShape3D.new()
	var bsph := SphereShape3D.new()
	bsph.radius = 0.1
	bcs.shape = bsph
	base.add_child(bcs)
	base.position = Vector3(10, 3, 0)
	add_child(base)
	spinner = _box(Vector3(10, 3, 0))
	spinner.gravity_scale = 0.0
	var motor := Box3DMotorJoint3D.new()
	motor.position = Vector3(10, 3, 0)
	motor.angular_velocity = Vector3(0, 4, 0)
	motor.max_velocity_torque = 100.0
	add_child(motor)
	motor.node_a = motor.get_path_to(base)
	motor.node_b = motor.get_path_to(spinner)


func _box(pos: Vector3) -> RigidBody3D:
	var body := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(0.6, 0.6, 0.6)
	cs.shape = box
	body.add_child(cs)
	body.position = pos
	add_child(body)
	return body


func _sphere(pos: Vector3) -> RigidBody3D:
	var body := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var sph := SphereShape3D.new()
	sph.radius = 0.3
	cs.shape = sph
	body.add_child(cs)
	body.position = pos
	add_child(body)
	return body


func _physics_process(_delta: float) -> void:
	frames += 1
	if frames != 300:
		return
	var sep: float = weld_a.global_position.distance_to(weld_b.global_position)
	print("Weld separation: ", sep)
	if absf(sep - 0.6) < 0.05:
		print("RESULT: PASS - Box3DWeldJoint3D node holds rigidly")
	else:
		print("RESULT: FAIL - weld node separation ", sep)

	var dist: float = anchor.global_position.distance_to(bob.global_position)
	print("Pendulum distance: ", dist, " readback: ", dist_joint.get_current_length())
	if absf(dist - 2.0) < 0.1:
		print("RESULT: PASS - Box3DDistanceJoint3D node holds rod length")
	else:
		print("RESULT: FAIL - distance node drifted to ", dist)

	print("Spinner angular velocity: ", spinner.angular_velocity)
	if spinner.angular_velocity.y > 2.0:
		print("RESULT: PASS - Box3DMotorJoint3D node drives angular velocity")
	else:
		print("RESULT: FAIL - motor node not driving (", spinner.angular_velocity.y, ")")
	get_tree().quit()
