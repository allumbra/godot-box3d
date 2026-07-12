#pragma once

#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include <box3d/id.h>

#include <cfloat>

using namespace godot;

class Box3DBodyImpl3D;

// Base for pin/hinge/slider joint wrappers. Box3D joints are created eagerly against two
// live b3BodyIds (unlike Godot's PhysicsServer3D API, which allows _joint_make_* to be
// called with bodies that may not yet be attached to a space) so, similar to
// Box3DShapedObjectImpl3D, the b3JointId is only created once both bodies have a valid
// b3BodyId; until then all construction state is cached here.
class Box3DJointImpl3D {
public:
	virtual ~Box3DJointImpl3D();

	virtual PhysicsServer3D::JointType get_type() const { return PhysicsServer3D::JOINT_TYPE_MAX; }

	RID get_rid() const { return rid; }

	void set_rid(const RID& p_rid) { rid = p_rid; }

	Box3DBodyImpl3D* get_body_a() const { return body_a; }

	Box3DBodyImpl3D* get_body_b() const { return body_b; }

	b3JointId get_joint_id() const { return joint_id; }

	bool has_joint_id() const { return B3_IS_NON_NULL(joint_id); }

	bool is_collision_disabled() const { return collision_disabled; }

	void set_collision_disabled(bool p_disabled);

	// Called by the server once both bodies (or either body's space attachment) may have
	// changed, to (re)build the live b3JointId if both bodies now have a b3BodyId.
	void rebuild();

	// Called by Box3DBodyImpl3D after its b3BodyId was destroyed/recreated (space change):
	// the old b3JointId is implicitly gone (box3d destroys joints with their body), so the
	// joint must be rebuilt against the new body ids.
	void on_body_id_changed() { rebuild(); }

	// Called from Box3DBodyImpl3D's destructor for every joint still attached to it.
	void on_body_destroyed(Box3DBodyImpl3D* p_body);

	// Force/torque event thresholds (b3JointDef.forceThreshold/torqueThreshold): when
	// exceeded during a step, box3d emits a joint event, surfaced by the server as
	// space_get_joint_force_events(). FLT_MAX (default) never fires.
	real_t get_force_threshold() const { return force_threshold; }

	void set_force_threshold(real_t p_threshold);

	real_t get_torque_threshold() const { return torque_threshold; }

	void set_torque_threshold(real_t p_threshold);

	// Post-creation joint frame mutation (b3Joint_SetLocalFrameA/B) — enables e.g.
	// changing a pin joint's anchors after creation.
	Transform3D get_local_frame_a() const { return local_frame_a; }

	void set_local_frame_a(const Transform3D& p_frame);

	Transform3D get_local_frame_b() const { return local_frame_b; }

	void set_local_frame_b(const Transform3D& p_frame);

protected:
	Box3DJointImpl3D(Box3DBodyImpl3D* p_body_a, Box3DBodyImpl3D* p_body_b, const Transform3D& p_local_frame_a, const Transform3D& p_local_frame_b);

	virtual b3JointId _create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) = 0;

	void _destroy_joint_id();

	void _wake_bodies();

	Transform3D local_frame_a;
	Transform3D local_frame_b;

private:
	RID rid;
	Box3DBodyImpl3D* body_a = nullptr;
	Box3DBodyImpl3D* body_b = nullptr;
	b3JointId joint_id = b3_nullJointId;
	real_t force_threshold = FLT_MAX;
	real_t torque_threshold = FLT_MAX;
	bool collision_disabled = false;
};
