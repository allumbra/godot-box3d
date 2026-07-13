extends SceneTree

# Regression test: PhysicsDirectBodyState3D contact reporting (count/position/normal/
# impulse/collider). Previously ALL contact accessors were stubs returning zeros, so
# contact monitors and impulse-based damage models saw nothing on box3d.

var frames := 0
var ball: RigidBody3D
var ground: StaticBody3D
var peak_impulse := 0.0
var saw_ground_collider := false
var contact_pos_ok := false


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	ground = StaticBody3D.new()
	var gcs := CollisionShape3D.new()
	var gbox := BoxShape3D.new()
	gbox.size = Vector3(40, 1, 40)
	gcs.shape = gbox
	ground.add_child(gcs)
	ground.position = Vector3(0, -0.5, 0)
	root.add_child(ground)

	ball = RigidBody3D.new()
	var cs := CollisionShape3D.new()
	var sph := SphereShape3D.new()
	sph.radius = 0.5
	cs.shape = sph
	ball.add_child(cs)
	ball.position = Vector3(0, 3, 0)
	root.add_child(ball)


func _process(_delta: float) -> bool:
	frames += 1
	var state := PhysicsServer3D.body_get_direct_state(ball.get_rid())
	if state != null:
		for i in state.get_contact_count():
			var imp: Vector3 = state.get_contact_impulse(i)
			peak_impulse = maxf(peak_impulse, imp.length())
			if state.get_contact_collider(i) == ground.get_rid():
				saw_ground_collider = true
			var pos: Vector3 = state.get_contact_local_position(i)
			if absf(pos.y) < 0.15 and pos.y > -0.15:
				contact_pos_ok = true
	if frames == 200:
		print("peak impulse: ", peak_impulse, "  ground collider seen: ", saw_ground_collider,
			"  contact pos ok: ", contact_pos_ok)
		if peak_impulse > 2.0:
			print("RESULT: PASS - contact impulses are reported (peak ", peak_impulse, ")")
		else:
			print("RESULT: FAIL - impulse never exceeded 2.0 (", peak_impulse, ")")
		if saw_ground_collider and contact_pos_ok:
			print("RESULT: PASS - collider RID and contact position are reported")
		else:
			print("RESULT: FAIL - collider=", saw_ground_collider, " pos=", contact_pos_ok)
		quit()
	return false
