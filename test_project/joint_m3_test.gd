extends SceneTree

# M3 tests: weld / distance / motor joints + generic constraint force readback,
# via the Box3D-specific server API (dynamic dispatch — see rolling_test.gd note).

var frames: int = 0
var server: Object

var weld_a: RigidBody3D
var weld_b: RigidBody3D
var weld_rid: RID

var anchor: StaticBody3D
var bob: RigidBody3D
var dist_rid: RID

var spinner: RigidBody3D
var spin_base: StaticBody3D
var motor_rid: RID

var impactor: RigidBody3D
var hit_events_seen: int = 0
var hit_speed_max: float = 0.0
var joint_events_seen: int = 0


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))
	server = PhysicsServer3D
	for m in ["weld_joint_create", "distance_joint_create", "motor_joint_create", "joint_get_constraint_force"]:
		if not server.has_method(m):
			print("RESULT: FAIL - ", m, " not exposed")
			quit()
			return

	var ground := StaticBody3D.new()
	var gcs := CollisionShape3D.new()
	var gbox := BoxShape3D.new()
	gbox.size = Vector3(100, 1, 100)
	gcs.shape = gbox
	ground.add_child(gcs)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)

	# --- Weld: two boxes welded side by side at rest height; must stay one rigid unit.
	weld_a = _box(Vector3(-10, 0.31, 0))
	weld_b = _box(Vector3(-9.4, 0.31, 0))
	# NOTE: global_transform reads identity for freshly added nodes inside
	# SceneTree._initialize, so local frames are computed arithmetically here
	# (bodies are unrotated: local origin = frame origin - body origin).
	var weld_origin := Vector3(-9.7, 0.31, 0)
	weld_rid = server.weld_joint_create(
		weld_a.get_rid(), weld_b.get_rid(),
		Transform3D(Basis(), weld_origin - weld_a.position),
		Transform3D(Basis(), weld_origin - weld_b.position))
	# Touching bodies: contacts would fight the weld (collide_connected is on by
	# default); the standard server method works on Box3D-specific joints too.
	PhysicsServer3D.joint_disable_collisions_between_bodies(weld_rid, true)

	# --- Distance: pendulum bob hanging on a rigid 2m rod from a static anchor.
	anchor = StaticBody3D.new()
	var acs := CollisionShape3D.new()
	var asph := SphereShape3D.new()
	asph.radius = 0.1
	acs.shape = asph
	anchor.add_child(acs)
	anchor.position = Vector3(0, 6, 0)
	root.add_child(anchor)
	bob = _sphere(Vector3(2, 6, 0)) # start horizontal: swings, rod must hold 2m
	dist_rid = server.distance_joint_create(
		anchor.get_rid(), bob.get_rid(),
		Transform3D(), Transform3D())
	server.distance_joint_set_param(dist_rid, Box3DPhysicsServer3D.DISTANCE_JOINT_PARAM_LENGTH, 2.0)

	# --- Motor: velocity-drive a floating body's angular velocity against a static base.
	spin_base = StaticBody3D.new()
	var scs := CollisionShape3D.new()
	var ssph := SphereShape3D.new()
	ssph.radius = 0.1
	scs.shape = ssph
	spin_base.add_child(scs)
	spin_base.position = Vector3(10, 3, 0)
	root.add_child(spin_base)
	spinner = _box(Vector3(10, 3, 0))
	spinner.gravity_scale = 0.0
	motor_rid = server.motor_joint_create(
		spin_base.get_rid(), spinner.get_rid(), Transform3D(), Transform3D())
	server.motor_joint_set_angular_velocity(motor_rid, Vector3(0, 4, 0))
	server.motor_joint_set_param(motor_rid, Box3DPhysicsServer3D.MOTOR_JOINT_PARAM_MAX_VELOCITY_TORQUE, 100.0)

	# --- Events: fast impactor for contact hit events; low force threshold on the
	# pendulum rod so its load fires joint force events.
	impactor = _sphere(Vector3(20, 8, 0))
	server.joint_set_force_threshold(dist_rid, 1.0)


func _box(pos: Vector3) -> RigidBody3D:
	var body := RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(0.6, 0.6, 0.6)
	cs.shape = box
	body.add_child(cs)
	body.position = pos
	root.add_child(body)
	return body


func _sphere(pos: Vector3) -> RigidBody3D:
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
	var space: RID = get_root().get_world_3d().space
	for event in server.space_get_contact_hit_events(space):
		hit_events_seen += 1
		hit_speed_max = maxf(hit_speed_max, event["approach_speed"])
	for joint in server.space_get_joint_force_events(space):
		if joint == dist_rid:
			joint_events_seen += 1
	if frames == 300:
		# Weld: separation must still be ~0.6 and both at same height.
		var sep: float = weld_a.global_position.distance_to(weld_b.global_position)
		var weld_force: Vector3 = server.joint_get_constraint_force(weld_rid)
		print("Weld A: ", weld_a.global_position, " B: ", weld_b.global_position)
		print("Weld separation: ", sep, "  constraint force: ", weld_force)
		if absf(sep - 0.6) < 0.05:
			print("RESULT: PASS - weld held the two boxes rigidly")
		else:
			print("RESULT: FAIL - weld separation drifted to ", sep)

		# Distance: bob must stay ~2m from anchor while swinging.
		var dist: float = anchor.global_position.distance_to(bob.global_position)
		var cur: float = server.distance_joint_get_current_length(dist_rid)
		print("Pendulum distance: ", dist, "  current_length readback: ", cur)
		if absf(dist - 2.0) < 0.1 and absf(cur - dist) < 0.1:
			print("RESULT: PASS - distance joint held rod length (readback live)")
		else:
			print("RESULT: FAIL - distance drifted")

		# Motor: spinner must have picked up angular velocity ~4 rad/s about Y.
		var av: Vector3 = spinner.angular_velocity
		print("Spinner angular velocity: ", av)
		if av.y > 2.0:
			print("RESULT: PASS - motor joint drives angular velocity")
		else:
			print("RESULT: FAIL - spinner not driven (", av.y, ")")

		print("Hit events: ", hit_events_seen, "  max approach speed: ", hit_speed_max)
		if hit_events_seen > 0 and hit_speed_max > 5.0:
			print("RESULT: PASS - contact hit events reported with approach speed")
		else:
			print("RESULT: FAIL - no contact hit events")

		print("Joint force events (rod): ", joint_events_seen)
		if joint_events_seen > 0:
			print("RESULT: PASS - joint force threshold events fire")
		else:
			print("RESULT: FAIL - no joint force events")
		quit()
	return false
