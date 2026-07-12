#pragma once

#include "box3d_joint_node_3d.hpp"

// Scene-facing motor joint (b3MotorJointDef): drives node_b's velocity and/or pose
// relative to node_a (kinematic-with-forces control).
class Box3DMotorJoint3D : public Box3DJointNode3D {
	GDCLASS(Box3DMotorJoint3D, Box3DJointNode3D)

public:
	Vector3 get_linear_velocity() const { return linear_velocity; }
	void set_linear_velocity(const Vector3& p_velocity);

	Vector3 get_angular_velocity() const { return angular_velocity; }
	void set_angular_velocity(const Vector3& p_velocity);

	real_t get_max_velocity_force() const { return max_velocity_force; }
	void set_max_velocity_force(real_t p_value);

	real_t get_max_velocity_torque() const { return max_velocity_torque; }
	void set_max_velocity_torque(real_t p_value);

	real_t get_linear_hertz() const { return linear_hertz; }
	void set_linear_hertz(real_t p_value);

	real_t get_linear_damping_ratio() const { return linear_damping; }
	void set_linear_damping_ratio(real_t p_value);

	real_t get_max_spring_force() const { return max_spring_force; }
	void set_max_spring_force(real_t p_value);

	real_t get_angular_hertz() const { return angular_hertz; }
	void set_angular_hertz(real_t p_value);

	real_t get_angular_damping_ratio() const { return angular_damping; }
	void set_angular_damping_ratio(real_t p_value);

	real_t get_max_spring_torque() const { return max_spring_torque; }
	void set_max_spring_torque(real_t p_value);

protected:
	static void _bind_methods();

	RID _create_joint(Box3DPhysicsServer3D* p_server, const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) override;

	void _apply_params(Box3DPhysicsServer3D* p_server) override;

private:
	void _set_param(int p_param, real_t p_value);

	Vector3 linear_velocity;
	Vector3 angular_velocity;
	real_t max_velocity_force = 0.0;
	real_t max_velocity_torque = 0.0;
	real_t linear_hertz = 0.0;
	real_t linear_damping = 1.0;
	real_t max_spring_force = 0.0;
	real_t angular_hertz = 0.0;
	real_t angular_damping = 1.0;
	real_t max_spring_torque = 0.0;
};
