#pragma once

#include "box3d_joint_impl_3d.hpp"

// Generic6DOFJoint3D -> decomposed onto the closest box3d joint by axis
// configuration. Box3d has no true 6DOF joint, but the configurations games
// actually use map cleanly:
//   - all linear + all angular locked            -> weld
//   - linear locked, angular free/limited        -> spherical (cone/twist limits)
//   - linear locked, one angular axis free       -> revolute about that axis
//   - one linear axis free, angular locked       -> prismatic along that axis
// Anything else falls back to spherical with a one-time warning. Changing a
// flag/limit that alters the configuration rebuilds the underlying joint.
class Box3DGeneric6DOFJointImpl3D final : public Box3DJointImpl3D {
public:
	using Param = PhysicsServer3D::G6DOFJointAxisParam;
	using Flag = PhysicsServer3D::G6DOFJointAxisFlag;

	Box3DGeneric6DOFJointImpl3D(Box3DBodyImpl3D* p_body_a, Box3DBodyImpl3D* p_body_b, const Transform3D& p_local_frame_a, const Transform3D& p_local_frame_b);

	PhysicsServer3D::JointType get_type() const override { return PhysicsServer3D::JOINT_TYPE_6DOF; }

	real_t get_param(Vector3::Axis p_axis, Param p_param) const;

	void set_param(Vector3::Axis p_axis, Param p_param, real_t p_value);

	bool get_flag(Vector3::Axis p_axis, Flag p_flag) const;

	void set_flag(Vector3::Axis p_axis, Flag p_flag, bool p_enabled);

protected:
	b3JointId _create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) override;

private:
	bool _linear_locked(int p_axis) const;
	bool _angular_locked(int p_axis) const;

	real_t params[3][PhysicsServer3D::G6DOF_JOINT_MAX] = {};
	bool flags[3][PhysicsServer3D::G6DOF_JOINT_FLAG_MAX] = {};
};
