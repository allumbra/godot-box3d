#include "box3d_replay_player.hpp"

#include <box3d/box3d.h>

Box3DReplayPlayer::~Box3DReplayPlayer() {
	close();
}

bool Box3DReplayPlayer::open(const PackedByteArray& p_data, int p_worker_count) {
	close();
	ERR_FAIL_COND_V(p_data.is_empty(), false);
	player = b3RecPlayer_Create(p_data.ptr(), p_data.size(), MAX(1, p_worker_count));
	return player != nullptr;
}

void Box3DReplayPlayer::close() {
	if (player != nullptr) {
		b3RecPlayer_Destroy(player);
		player = nullptr;
	}
}

bool Box3DReplayPlayer::step_frame() {
	ERR_FAIL_NULL_V(player, false);
	return b3RecPlayer_StepFrame(player);
}

void Box3DReplayPlayer::restart() {
	ERR_FAIL_NULL(player);
	b3RecPlayer_Restart(player);
}

void Box3DReplayPlayer::seek_frame(int p_frame) {
	ERR_FAIL_NULL(player);
	b3RecPlayer_SeekFrame(player, p_frame);
}

int Box3DReplayPlayer::get_frame() const {
	ERR_FAIL_NULL_V(player, 0);
	return b3RecPlayer_GetFrame(player);
}

int Box3DReplayPlayer::get_frame_count() const {
	ERR_FAIL_NULL_V(player, 0);
	return b3RecPlayer_GetFrameCount(player);
}

bool Box3DReplayPlayer::is_at_end() const {
	ERR_FAIL_NULL_V(player, true);
	return b3RecPlayer_IsAtEnd(player);
}

bool Box3DReplayPlayer::has_diverged() const {
	ERR_FAIL_NULL_V(player, false);
	return b3RecPlayer_HasDiverged(player);
}

void Box3DReplayPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("open", "data", "worker_count"), &Box3DReplayPlayer::open);
	ClassDB::bind_method(D_METHOD("close"), &Box3DReplayPlayer::close);
	ClassDB::bind_method(D_METHOD("is_open"), &Box3DReplayPlayer::is_open);
	ClassDB::bind_method(D_METHOD("step_frame"), &Box3DReplayPlayer::step_frame);
	ClassDB::bind_method(D_METHOD("restart"), &Box3DReplayPlayer::restart);
	ClassDB::bind_method(D_METHOD("seek_frame", "frame"), &Box3DReplayPlayer::seek_frame);
	ClassDB::bind_method(D_METHOD("get_frame"), &Box3DReplayPlayer::get_frame);
	ClassDB::bind_method(D_METHOD("get_frame_count"), &Box3DReplayPlayer::get_frame_count);
	ClassDB::bind_method(D_METHOD("is_at_end"), &Box3DReplayPlayer::is_at_end);
	ClassDB::bind_method(D_METHOD("has_diverged"), &Box3DReplayPlayer::has_diverged);
}
