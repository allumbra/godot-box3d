#pragma once

#include "box3d_joint_node_3d.hpp"

// Scene-facing weld joint (b3WeldJointDef): rigidly locks node_b to node_a with
// optional soft response (hertz 0 = maximally stiff).
class Box3DWeldJoint3D : public Box3DJointNode3D {
	GDCLASS(Box3DWeldJoint3D, Box3DJointNode3D)

public:
	real_t get_linear_hertz() const { return linear_hertz; }
	void set_linear_hertz(real_t p_value);

	real_t get_linear_damping_ratio() const { return linear_damping; }
	void set_linear_damping_ratio(real_t p_value);

	real_t get_angular_hertz() const { return angular_hertz; }
	void set_angular_hertz(real_t p_value);

	real_t get_angular_damping_ratio() const { return angular_damping; }
	void set_angular_damping_ratio(real_t p_value);

protected:
	static void _bind_methods();

	RID _create_joint(Box3DPhysicsServer3D* p_server, const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) override;

	void _apply_params(Box3DPhysicsServer3D* p_server) override;

private:
	void _set_param(int p_param, real_t p_value);

	real_t linear_hertz = 0.0;
	real_t linear_damping = 1.0;
	real_t angular_hertz = 0.0;
	real_t angular_damping = 1.0;
};
