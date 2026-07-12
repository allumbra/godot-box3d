extends SceneTree

# R1 test: kinematic character mover (space_collide_mover / solve_mover_planes /
# clip_mover_velocity), modeled on box3d samples/sample_character.cpp. A capsule
# walks +X across the floor and must climb a 0.3 m step without a physics body.

const RADIUS := 0.4
const HALF_SEGMENT := 0.5 # capsule core half-length; bottom = pos.y - HALF_SEGMENT - RADIUS

var frames := 0
var pos := Vector3(0, 0.91, 0) # capsule center; bottom just above the floor
var velocity := Vector3.ZERO
var planes_seen := 0
var max_bottom_on_step := -1.0
var server: Object


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))
	server = PhysicsServer3D
	if not server.has_method("space_collide_mover"):
		print("RESULT: FAIL - mover API not exposed")
		quit()
		return

	var ground := StaticBody3D.new()
	var gcs := CollisionShape3D.new()
	var gbox := BoxShape3D.new()
	gbox.size = Vector3(40, 1, 40)
	gcs.shape = gbox
	ground.add_child(gcs)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)

	# 0.3 m step at x = 2.
	var step := StaticBody3D.new()
	var scs := CollisionShape3D.new()
	var sbox := BoxShape3D.new()
	sbox.size = Vector3(2.0, 0.3, 4.0)
	scs.shape = sbox
	step.add_child(scs)
	step.position = Vector3(3.0, 0.15, 0)
	root.add_child(step)


func _process(delta: float) -> bool:
	if not server.has_method("space_collide_mover"):
		return true
	frames += 1
	var space: RID = root.get_world_3d().space

	velocity.x = 2.0
	velocity.y -= 9.8 * delta

	var c1 := pos + Vector3(0, -HALF_SEGMENT, 0)
	var c2 := pos + Vector3(0, HALF_SEGMENT, 0)
	var planes: Array = server.space_collide_mover(space, c1, c2, RADIUS, 0xFFFFFFFF)
	planes_seen += planes.size()
	var solved: Dictionary = server.solve_mover_planes(velocity * delta, planes)
	pos += solved["delta"]
	velocity = server.clip_mover_velocity(velocity, solved["planes"])

	# Track how high the capsule bottom rides while over the step (x in [2.5, 3.5]).
	if pos.x > 2.5 and pos.x < 3.5:
		max_bottom_on_step = maxf(max_bottom_on_step, pos.y - HALF_SEGMENT - RADIUS)

	if frames == 240:
		print("Final pos: ", pos, "  velocity: ", velocity, "  planes seen: ", planes_seen)
		if pos.x > 3.0 and planes_seen > 100:
			print("RESULT: PASS - mover walked forward with plane contact")
		else:
			print("RESULT: FAIL - x=", pos.x, " planes=", planes_seen)
		# While over the step the capsule bottom must have ridden at ~0.3 (climbed,
		# not tunneled through).
		print("Max bottom while over step: ", max_bottom_on_step)
		if absf(max_bottom_on_step - 0.3) < 0.08:
			print("RESULT: PASS - mover climbed the 0.3 m step (bottom=", max_bottom_on_step, ")")
		else:
			print("RESULT: FAIL - bottom over step was ", max_bottom_on_step, " (expected ~0.3)")
		quit()
	return false
