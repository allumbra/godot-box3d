#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

using namespace godot;

struct b3RecPlayer;

// Script-facing wrapper over box3d's incremental replay player (b3RecPlayer_*):
// replays a recording produced by space_start_recording/space_stop_recording in a
// private box3d world, with frame stepping, seeking (keyframe ring), and per-frame
// state-hash divergence detection. The replayed world currently has no Godot-node
// mirror — this class is the validation/scrubbing surface; visual playback would
// need a body-state readback API (future work).
class Box3DReplayPlayer : public RefCounted {
	GDCLASS(Box3DReplayPlayer, RefCounted)

public:
	~Box3DReplayPlayer() override;

	// Creates the underlying player over a copy of the bytes. worker_count 1 matches a
	// serial recording; a different count turns the hash check into a cross-thread
	// determinism test. Returns false on a bad header/deserialization failure.
	bool open(const PackedByteArray& p_data, int p_worker_count);

	void close();

	bool is_open() const { return player != nullptr; }

	bool step_frame();

	void restart();

	void seek_frame(int p_frame);

	int get_frame() const;

	int get_frame_count() const;

	bool is_at_end() const;

	bool has_diverged() const;

protected:
	static void _bind_methods();

private:
	b3RecPlayer* player = nullptr;
};
