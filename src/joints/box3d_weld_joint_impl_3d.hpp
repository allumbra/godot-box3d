#pragma once

#include "box3d_joint_impl_3d.hpp"

// Box3D-native weld joint (b3WeldJointDef): rigidly locks two bodies together, with
// optional soft response (hertz 0 = maximally stiff). No Godot joint type matches, so
// it is exposed via weld_joint_* methods bound on Box3DPhysicsServer3D.
class Box3DWeldJointImpl3D final : public Box3DJointImpl3D {
public:
	enum Param {
		PARAM_LINEAR_HERTZ,
		PARAM_LINEAR_DAMPING,
		PARAM_ANGULAR_HERTZ,
		PARAM_ANGULAR_DAMPING,
		PARAM_MAX
	};

	Box3DWeldJointImpl3D(Box3DBodyImpl3D* p_body_a, Box3DBodyImpl3D* p_body_b, const Transform3D& p_local_frame_a, const Transform3D& p_local_frame_b);

	real_t get_param(Param p_param) const;

	void set_param(Param p_param, real_t p_value);

protected:
	b3JointId _create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) override;

private:
	real_t linear_hertz = 0.0;
	real_t linear_damping = 1.0;
	real_t angular_hertz = 0.0;
	real_t angular_damping = 1.0;
};
