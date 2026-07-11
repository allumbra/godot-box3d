extends Node3D

# Drivable 4-wheel cart on Box3DWheelJoint3D (M2 deliverable).
# Controls: W/S (or Up/Down) = drive, A/D (or Left/Right) = steer, Space = brake.
# AUTO_DRIVE makes it drive a gentle S-curve on its own (for headless/visual checks).

const AUTO_DRIVE := true
const DRIVE_SPEED := 25.0 # wheel spin speed, rad/s
const DRIVE_TORQUE := 60.0
const STEER_ANGLE := 0.45 # rad
const WHEEL_RADIUS := 0.35

var chassis: RigidBody3D
var rear_joints: Array = []
var front_joints: Array = []
var time_alive := 0.0


func _ready() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	var ground := StaticBody3D.new()
	var ground_cs := CollisionShape3D.new()
	var ground_shape := BoxShape3D.new()
	ground_shape.size = Vector3(400, 1, 400)
	ground_cs.shape = ground_shape
	ground.add_child(ground_cs)
	ground.position = Vector3(0, -0.5, 0)
	var ground_mesh := MeshInstance3D.new()
	var plane := PlaneMesh.new()
	plane.size = Vector2(400, 400)
	ground_mesh.mesh = plane
	ground_mesh.position = Vector3(0, 0.501, 0)
	ground.add_child(ground_mesh)
	add_child(ground)

	chassis = RigidBody3D.new()
	chassis.name = "Chassis"
	var chassis_cs := CollisionShape3D.new()
	var chassis_shape := BoxShape3D.new()
	chassis_shape.size = Vector3(1.2, 0.35, 2.4)
	chassis_cs.shape = chassis_shape
	chassis.add_child(chassis_cs)
	chassis.mass = 20.0
	chassis.position = Vector3(0, 0.8, 0)
	var chassis_mesh := MeshInstance3D.new()
	var box_mesh := BoxMesh.new()
	box_mesh.size = chassis_shape.size
	chassis_mesh.mesh = box_mesh
	chassis.add_child(chassis_mesh)
	add_child(chassis)

	# Wheel corners: x = axle side, z = front(-)/rear(+).
	var corners := [
		{ "pos": Vector3(-0.75, 0.45, -0.9), "front": true },
		{ "pos": Vector3(0.75, 0.45, -0.9), "front": true },
		{ "pos": Vector3(-0.75, 0.45, 0.9), "front": false },
		{ "pos": Vector3(0.75, 0.45, 0.9), "front": false },
	]
	for corner in corners:
		_add_wheel(corner["pos"], corner["front"])

	var camera := Camera3D.new()
	camera.name = "ChaseCamera"
	camera.position = Vector3(0, 4, 8)
	camera.rotation_degrees = Vector3(-20, 0, 0)
	add_child(camera)

	var light := DirectionalLight3D.new()
	light.rotation_degrees = Vector3(-55, -30, 0)
	add_child(light)


func _add_wheel(local_pos: Vector3, is_front: bool) -> void:
	var wheel := RigidBody3D.new()
	wheel.name = "Wheel_%s%s" % [("F" if is_front else "R"), ("L" if local_pos.x < 0 else "R")]
	var cs := CollisionShape3D.new()
	var sphere := SphereShape3D.new()
	sphere.radius = WHEEL_RADIUS
	cs.shape = sphere
	wheel.add_child(cs)
	wheel.mass = 2.0
	wheel.position = local_pos
	var mesh := MeshInstance3D.new()
	var sphere_mesh := SphereMesh.new()
	sphere_mesh.radius = WHEEL_RADIUS
	sphere_mesh.height = WHEEL_RADIUS * 2.0
	mesh.mesh = sphere_mesh
	wheel.add_child(mesh)
	# Spin marker so wheel rotation is visible.
	var marker := MeshInstance3D.new()
	var marker_mesh := BoxMesh.new()
	marker_mesh.size = Vector3(0.05, 0.05, WHEEL_RADIUS * 1.9)
	marker.mesh = marker_mesh
	wheel.add_child(marker)
	add_child(wheel)

	var joint := Box3DWheelJoint3D.new()
	joint.name = wheel.name + "_Joint"
	# Joint frame: local X = up (suspension/steer axis), local Z = axle (world X).
	joint.transform = Transform3D(
		Basis(Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(1, 0, 0)), local_pos)
	joint.suspension_enabled = true
	joint.suspension_hertz = 4.0
	joint.suspension_damping_ratio = 0.7
	joint.suspension_limit_enabled = true
	joint.suspension_limit_lower = -0.15
	joint.suspension_limit_upper = 0.15
	if is_front:
		joint.steering_enabled = true
		joint.steering_hertz = 8.0
		joint.steering_damping_ratio = 1.0
		joint.steering_max_torque = 200.0
		joint.steering_limit_enabled = true
		joint.steering_limit_lower = -STEER_ANGLE
		joint.steering_limit_upper = STEER_ANGLE
	else:
		joint.motor_enabled = true
		joint.motor_max_torque = DRIVE_TORQUE
		joint.motor_speed = 0.0
	add_child(joint)
	joint.node_a = joint.get_path_to(chassis)
	joint.node_b = joint.get_path_to(wheel)

	if is_front:
		front_joints.append(joint)
	else:
		rear_joints.append(joint)


func _physics_process(delta: float) -> void:
	time_alive += delta

	var drive := Input.get_axis("ui_down", "ui_up")
	var steer := Input.get_axis("ui_right", "ui_left")
	if AUTO_DRIVE and absf(drive) < 0.01 and absf(steer) < 0.01:
		drive = 1.0 if time_alive > 1.0 else 0.0
		steer = sin(time_alive * 0.6) * 0.5

	for joint in rear_joints:
		joint.motor_speed = -drive * DRIVE_SPEED
	for joint in front_joints:
		joint.steering_target_angle = steer * STEER_ANGLE

	var camera: Camera3D = get_node("ChaseCamera")
	var target: Vector3 = chassis.global_position
	camera.global_position = camera.global_position.lerp(
		target + Vector3(0, 3.5, 7.0), minf(1.0, delta * 3.0))
	camera.look_at(target)

	if Engine.get_physics_frames() % 120 == 0:
		print("t=%.1fs chassis=%s speed=%.2f spin=%.2f steer=%.2f" % [
			time_alive, chassis.global_position, chassis.linear_velocity.length(),
			rear_joints[0].get_spin_speed(), front_joints[0].get_steering_angle()])
