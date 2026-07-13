extends SceneTree

# Regression test: a scaled CollisionShape3D must collide at its scaled size.
# Before the fix the basis scale was dropped (rotation+translation only), so a
# 0.5-scaled shape collided at full size (e.g. walls stopping cars short).

var frames := 0
var ball: RigidBody3D


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))

	# Platform: box shape sized 2x2x2 but SCALED to 0.5 => effective 1x1x1, top at y=1.
	var platform := StaticBody3D.new()
	var cs := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(2, 2, 2)
	cs.shape = box
	cs.scale = Vector3(0.5, 0.5, 0.5)
	cs.position = Vector3(0, 0.5, 0)
	platform.add_child(cs)
	root.add_child(platform)

	# Ball dropped on it: with correct scaling rests at y = 1 + 0.3 = 1.3;
	# with dropped scale (full-size 2x2x2 box) it would rest at y = 1.5 + 0.3 = 1.8.
	ball = _ball(Vector3(0, 3, 0))


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
		print("ball y: ", ball.global_position.y)
		if absf(ball.global_position.y - 1.3) < 0.1:
			print("RESULT: PASS - scaled box collides at its scaled size")
		else:
			print("RESULT: FAIL - ball rests at ", ball.global_position.y, " (expected ~1.3; 1.8 = scale dropped)")
		quit()
	return false
