extends SceneTree

# Regression test: per-query RID exclusion (PhysicsRayQueryParameters3D.exclude) must
# be honored. Before the fix the wrapper never consulted is_body_excluded_from_query,
# so e.g. vehicle suspension rays hit the vehicle's own chassis at point-blank range
# (cars launched skyward with "tire tracks in the air").

var frames := 0
var body: RigidBody3D


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

	body = RigidBody3D.new()
	body.freeze = true
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(2, 1, 2)
	cs.shape = box
	body.add_child(cs)
	body.position = Vector3(0, 2, 0)
	root.add_child(body)


func _process(_delta: float) -> bool:
	frames += 1
	if frames == 10:
		var space := root.get_world_3d().direct_space_state
		# Ray from inside the box straight down, hit_from_inside enabled.
		var q := PhysicsRayQueryParameters3D.create(Vector3(0, 2, 0), Vector3(0, -3, 0))
		q.hit_from_inside = true
		var hit_no_exclude: Dictionary = space.intersect_ray(q)

		q.exclude = [body.get_rid()]
		var hit_excluded: Dictionary = space.intersect_ray(q)

		print("no-exclude hit: ", hit_no_exclude.get("rid", RID()) == body.get_rid(),
			"  excluded hit y: ", hit_excluded.get("position", Vector3(9, 9, 9)).y)
		if not hit_no_exclude.is_empty() and hit_no_exclude["rid"] == body.get_rid():
			print("RESULT: PASS - unexcluded ray hits the body")
		else:
			print("RESULT: FAIL - unexcluded ray missed the body")
		if not hit_excluded.is_empty() and hit_excluded["rid"] != body.get_rid() and absf(hit_excluded["position"].y) < 0.1:
			print("RESULT: PASS - excluded ray passes through to the ground")
		else:
			print("RESULT: FAIL - exclusion not honored (", hit_excluded, ")")
		quit()
	return false
