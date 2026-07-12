#include "box3d_joint_impl_3d.hpp"

#include "../misc/type_conversions.hpp"
#include "../objects/box3d_body_impl_3d.hpp"
#include "../spaces/box3d_space_3d.hpp"

#include <box3d/box3d.h>

Box3DJointImpl3D::Box3DJointImpl3D(
		Box3DBodyImpl3D* p_body_a,
		Box3DBodyImpl3D* p_body_b,
		const Transform3D& p_local_frame_a,
		const Transform3D& p_local_frame_b) :
		local_frame_a(p_local_frame_a),
		local_frame_b(p_local_frame_b),
		body_a(p_body_a),
		body_b(p_body_b) {
	if (body_a != nullptr) {
		body_a->register_joint(this);
	}
	if (body_b != nullptr) {
		body_b->register_joint(this);
	}
}

Box3DJointImpl3D::~Box3DJointImpl3D() {
	if (body_a != nullptr) {
		body_a->unregister_joint(this);
	}
	if (body_b != nullptr) {
		body_b->unregister_joint(this);
	}
	_destroy_joint_id();
}

void Box3DJointImpl3D::on_body_destroyed(Box3DBodyImpl3D* p_body) {
	// The body (and with it, via b3DestroyBody, any live box3d joint) is going away;
	// drop our reference so rebuild()/destructor never touch freed state.
	if (body_a == p_body) {
		body_a = nullptr;
	}
	if (body_b == p_body) {
		body_b = nullptr;
	}
	_destroy_joint_id();
}

void Box3DJointImpl3D::set_collision_disabled(bool p_disabled) {
	collision_disabled = p_disabled;
	if (has_joint_id()) {
		b3Joint_SetCollideConnected(joint_id, !p_disabled);
	}
}

void Box3DJointImpl3D::rebuild() {
	_destroy_joint_id();

	if (body_a == nullptr || body_b == nullptr) {
		return;
	}
	if (!body_a->has_body_id() || !body_b->has_body_id()) {
		return;
	}
	if (body_a->get_space() == nullptr) {
		return;
	}

	const b3Transform frame_a = godot_to_b3_transform(local_frame_a);
	const b3Transform frame_b = godot_to_b3_transform(local_frame_b);
	const b3WorldId world_id = body_a->get_space()->get_world_id();

	joint_id = _create_joint_id(world_id, body_a->get_body_id(), body_b->get_body_id(), frame_a, frame_b);

	if (has_joint_id()) {
		b3Joint_SetCollideConnected(joint_id, !collision_disabled);
		// The server maps joint events (see space_get_joint_force_events) back to this
		// wrapper through the box3d user data pointer.
		b3Joint_SetUserData(joint_id, this);
		b3Joint_SetForceThreshold(joint_id, (float)force_threshold);
		b3Joint_SetTorqueThreshold(joint_id, (float)torque_threshold);
	}
}

void Box3DJointImpl3D::set_force_threshold(real_t p_threshold) {
	force_threshold = p_threshold;
	if (has_joint_id()) {
		b3Joint_SetForceThreshold(joint_id, (float)p_threshold);
	}
}

void Box3DJointImpl3D::set_torque_threshold(real_t p_threshold) {
	torque_threshold = p_threshold;
	if (has_joint_id()) {
		b3Joint_SetTorqueThreshold(joint_id, (float)p_threshold);
	}
}

void Box3DJointImpl3D::set_local_frame_a(const Transform3D& p_frame) {
	local_frame_a = p_frame;
	if (has_joint_id()) {
		b3Joint_SetLocalFrameA(joint_id, godot_to_b3_transform(local_frame_a));
		_wake_bodies();
	}
}

void Box3DJointImpl3D::set_local_frame_b(const Transform3D& p_frame) {
	local_frame_b = p_frame;
	if (has_joint_id()) {
		b3Joint_SetLocalFrameB(joint_id, godot_to_b3_transform(local_frame_b));
		_wake_bodies();
	}
}

void Box3DJointImpl3D::_wake_bodies() {
	// Frame mutation does not wake sleeping bodies inside box3d; without this a moved
	// anchor has no visible effect until something else wakes the pair.
	if (body_a != nullptr && body_a->has_body_id()) {
		b3Body_SetAwake(body_a->get_body_id(), true);
	}
	if (body_b != nullptr && body_b->has_body_id()) {
		b3Body_SetAwake(body_b->get_body_id(), true);
	}
}

void Box3DJointImpl3D::_destroy_joint_id() {
	// b3DestroyBody destroys attached joints inside box3d, so a cached joint_id can be
	// stale by the time we get here (e.g. a body left its space) — validate first.
	if (has_joint_id() && b3Joint_IsValid(joint_id)) {
		b3DestroyJoint(joint_id, true);
	}
	joint_id = b3_nullJointId;
}
