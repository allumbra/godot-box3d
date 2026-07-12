#pragma once

#include "box3d_joint_impl_3d.hpp"

// ConeTwistJoint3D -> b3SphericalJoint with cone + twist limits. Godot's cone is
// centered on the joint frame's local X axis; box3d's on frame A's local Z — the
// frames passed to box3d are remapped (X->Z) at creation. SOFTNESS/BIAS/RELAXATION
// have no box3d equivalent (cached, warned, ignored).
class Box3DConeTwistJointImpl3D final : public Box3DJointImpl3D {
public:
	using Param = PhysicsServer3D::ConeTwistJointParam;

	Box3DConeTwistJointImpl3D(Box3DBodyImpl3D* p_body_a, Box3DBodyImpl3D* p_body_b, const Transform3D& p_local_frame_a, const Transform3D& p_local_frame_b);

	PhysicsServer3D::JointType get_type() const override { return PhysicsServer3D::JOINT_TYPE_CONE_TWIST; }

	real_t get_param(Param p_param) const;

	void set_param(Param p_param, real_t p_value);

protected:
	b3JointId _create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) override;

private:
	real_t swing_span = Math_PI * 0.25;
	real_t twist_span = Math_PI;
	real_t bias = 0.3;
	real_t softness = 0.8;
	real_t relaxation = 1.0;
};
