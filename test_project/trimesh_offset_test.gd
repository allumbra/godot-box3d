extends SceneTree

# Regression test: a ConcavePolygonShape3D on an OFFSET CollisionShape3D must collide
# at the offset location. Before the fix the instance transform was silently dropped —
# trimesh colliders sat at the body origin (invisible obstacles / fall-through floors).

var frames := 0
var ball_on_mesh: RigidBody3D
var ball_off_mesh: RigidBody3D


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	# Static body at origin; trimesh platform (4x4 quad at y=0 local) OFFSET to x=+5, y=+2.
	var holder := StaticBody3D.new()
	var cs := CollisionShape3D.new()
	var tri := ConcavePolygonShape3D.new()
	tri.set_faces(PackedVector3Array([
		Vector3(-2, 0, -2), Vector3(2, 0, 2), Vector3(2, 0, -2),
		Vector3(-2, 0, -2), Vector3(-2, 0, 2), Vector3(2, 0, 2),
	]))
	cs.shape = tri
	cs.position = Vector3(5, 2, 0)
	holder.add_child(cs)
	root.add_child(holder)

	# Ball dropped over the OFFSET platform: must land on it (y ~= 2.3).
	ball_on_mesh = _ball(Vector3(5, 4, 0))
	# Ball dropped over the body ORIGIN: nothing there, must fall past y=0.
	ball_off_mesh = _ball(Vector3(0, 4, 0))


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
	if frames == 400:
		print("on-mesh ball y: ", ball_on_mesh.global_position.y,
			"  off-mesh ball y: ", ball_off_mesh.global_position.y)
		if absf(ball_on_mesh.global_position.y - 2.3) < 0.1:
			print("RESULT: PASS - offset trimesh collides at its offset location")
		else:
			print("RESULT: FAIL - ball over offset mesh at y=", ball_on_mesh.global_position.y)
		if ball_off_mesh.global_position.y < -1.0:
			print("RESULT: PASS - no phantom collider at the body origin")
		else:
			print("RESULT: FAIL - phantom collision at origin (y=", ball_off_mesh.global_position.y, ")")
		quit()
	return false
