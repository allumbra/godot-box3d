extends SceneTree

# Headless test for Box3DWheelJoint3D / the wheel_joint_* server API (M2).
# Chassis + one driven wheel on the ground: the spin motor must move the pair
# along +Z (wheel spins about its local Z axis = world X here, ground contact
# converts spin to forward travel), and readbacks must be live.

var frames: int = 0
var chassis: RigidBody3D
var wheel: RigidBody3D
var joint_rid: RID
var server: Object


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	var ground := StaticBody3D.new()
	var ground_shape := CollisionShape3D.new()
	var ground_box := BoxShape3D.new()
	ground_box.size = Vector3(200, 1, 200)
	ground_shape.shape = ground_box
	ground.add_child(ground_shape)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)

	chassis = RigidBody3D.new()
	var chassis_shape := CollisionShape3D.new()
	var chassis_box := BoxShape3D.new()
	chassis_box.size = Vector3(1.0, 0.4, 2.0)
	chassis_shape.shape = chassis_box
	chassis.add_child(chassis_shape)
	chassis.mass = 10.0
	chassis.position = Vector3(0, 1.0, 0)
	root.add_child(chassis)

	wheel = RigidBody3D.new()
	var wheel_shape := CollisionShape3D.new()
	var sphere := SphereShape3D.new()
	sphere.radius = 0.4
	wheel_shape.shape = sphere
	wheel.add_child(wheel_shape)
	wheel.mass = 1.0
	wheel.position = Vector3(0.7, 0.4, 0)
	root.add_child(wheel)

	server = PhysicsServer3D
	if not server.has_method("wheel_joint_create"):
		print("RESULT: FAIL - wheel_joint_create not exposed on PhysicsServer3D")
		quit()
		return

	# Created during _initialize deliberately: regression test for the
	# rebuild-on-space-attach fix (joints created before the bodies' b3BodyIds are
	# final used to stay dead forever).
	_create_joint()


func _create_joint() -> void:
	# Joint frames: suspension/steer axis = local X (point it up: +Y world),
	# spin axis = local Z of frame B (wheel rolls forward along world Z when
	# spinning about world X — so frame B local Z maps to world X).
	# Joint origin at the wheel center.
	# global_transform reads identity inside SceneTree._initialize (Godot quirk),
	# so local frames are computed arithmetically (bodies are unrotated).
	var joint_basis := Basis(Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(1, 0, 0))
	var frame_a := Transform3D(joint_basis, wheel.position - chassis.position)
	var frame_b := Transform3D(joint_basis, Vector3())

	joint_rid = server.wheel_joint_create(
		chassis.get_rid(), wheel.get_rid(), frame_a, frame_b)
	server.wheel_joint_set_flag(joint_rid, Box3DPhysicsServer3D.WHEEL_JOINT_FLAG_ENABLE_SUSPENSION, true)
	server.wheel_joint_set_param(joint_rid, Box3DPhysicsServer3D.WHEEL_JOINT_PARAM_SUSPENSION_HERTZ, 4.0)
	server.wheel_joint_set_param(joint_rid, Box3DPhysicsServer3D.WHEEL_JOINT_PARAM_SUSPENSION_DAMPING, 0.7)
	server.wheel_joint_set_flag(joint_rid, Box3DPhysicsServer3D.WHEEL_JOINT_FLAG_ENABLE_SPIN_MOTOR, true)
	server.wheel_joint_set_param(joint_rid, Box3DPhysicsServer3D.WHEEL_JOINT_PARAM_MAX_SPIN_TORQUE, 50.0)
	server.wheel_joint_set_param(joint_rid, Box3DPhysicsServer3D.WHEEL_JOINT_PARAM_SPIN_MOTOR_SPEED, 20.0)


func _process(_delta: float) -> bool:
	frames += 1
	if frames == 300:
		var spin: float = server.wheel_joint_get_spin_speed(joint_rid)
		print("Wheel pos: ", wheel.global_position)
		print("Chassis pos: ", chassis.global_position)
		print("Spin speed readback: ", spin)
		print("Spin torque readback: ", server.wheel_joint_get_spin_torque(joint_rid))
		var moved: float = wheel.global_position.z
		if absf(spin) > 1.0:
			print("RESULT: PASS - spin motor is spinning the wheel (readback live)")
		else:
			print("RESULT: FAIL - spin speed readback is ", spin)
		if absf(moved) > 0.3:
			print("RESULT: PASS - motorized wheel propelled the pair (z=", moved, ")")
		else:
			print("RESULT: FAIL - no forward travel (z=", moved, ")")
		quit()
	return false
