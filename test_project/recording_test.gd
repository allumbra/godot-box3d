extends SceneTree

# M5 test: record 120 frames of a small pile, stop, get bytes, validate the replay.

var frames: int = 0
var server: Object


func _initialize() -> void:
	print("Active physics engine setting: ", ProjectSettings.get_setting("physics/3d/physics_engine"))
	server = PhysicsServer3D
	if not server.has_method("space_start_recording"):
		print("RESULT: FAIL - recording API not exposed")
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

	var shape := BoxShape3D.new()
	shape.size = Vector3(0.5, 0.5, 0.5)
	for i in range(20):
		var body := RigidBody3D.new()
		var cs := CollisionShape3D.new()
		cs.shape = shape
		body.add_child(cs)
		body.position = Vector3(float(i % 4) * 0.6 - 0.9, 1.0 + float(i / 4) * 0.6, 0.0)
		root.add_child(body)

	server.space_start_recording(get_root().get_world_3d().space)
	print("Recording active: ", server.space_is_recording(get_root().get_world_3d().space))


func _process(_delta: float) -> bool:
	frames += 1
	if frames == 120:
		var space: RID = get_root().get_world_3d().space
		var bytes: PackedByteArray = server.space_stop_recording(space)
		print("Recording size: ", bytes.size(), " bytes")
		if bytes.size() > 0:
			print("RESULT: PASS - recording captured bytes")
		else:
			print("RESULT: FAIL - empty recording")
		var valid: bool = server.validate_replay(bytes, 1)
		print("Replay valid: ", valid)
		if valid:
			print("RESULT: PASS - recording replays and validates deterministically")
		else:
			print("RESULT: FAIL - replay validation failed")

		# Replay player: step through all frames, seek backward, check divergence.
		var player := Box3DReplayPlayer.new()
		if not player.open(bytes, 1):
			print("RESULT: FAIL - replay player could not open recording")
			quit()
			return true
		var stepped := 0
		while player.step_frame():
			stepped += 1
		print("Player stepped ", stepped, "/", player.get_frame_count(),
			" frames, diverged: ", player.has_diverged())
		player.seek_frame(10)
		var at_10: int = player.get_frame()
		if stepped == player.get_frame_count() and not player.has_diverged() and at_10 == 10:
			print("RESULT: PASS - replay player steps, seeks (frame ", at_10, "), no divergence")
		else:
			print("RESULT: FAIL - player stepped=", stepped, " seek=", at_10,
				" diverged=", player.has_diverged())
		quit()
	return false
