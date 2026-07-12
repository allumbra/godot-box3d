#pragma once

#include "box3d_joint_impl_3d.hpp"

// Box3D filter joint (b3CreateFilterJoint): disables collision between exactly two
// bodies, nothing else. Backs Godot's per-body collision exceptions
// (body_add_collision_exception); owned by the body the exception was added to, not by
// a joint RID.
class Box3DFilterJointImpl3D final : public Box3DJointImpl3D {
public:
	Box3DFilterJointImpl3D(Box3DBodyImpl3D* p_body_a, Box3DBodyImpl3D* p_body_b) :
			Box3DJointImpl3D(p_body_a, p_body_b, Transform3D(), Transform3D()) {
		// The entire purpose of a filter joint is collideConnected=false; without this,
		// the base rebuild() would re-enable collision on the pair.
		set_collision_disabled(true);
	}

protected:
	b3JointId _create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) override;
};
