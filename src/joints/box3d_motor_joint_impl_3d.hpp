#pragma once

#include "box3d_joint_impl_3d.hpp"

// Box3D-native motor joint (b3MotorJointDef): drives a body's relative velocity and/or
// pose (kinematic-with-forces control). No Godot joint type matches, so it is exposed
// via motor_joint_* methods bound on Box3DPhysicsServer3D.
class Box3DMotorJointImpl3D final : public Box3DJointImpl3D {
public:
	enum Param {
		PARAM_MAX_VELOCITY_FORCE,
		PARAM_MAX_VELOCITY_TORQUE,
		PARAM_LINEAR_HERTZ,
		PARAM_LINEAR_DAMPING,
		PARAM_MAX_SPRING_FORCE,
		PARAM_ANGULAR_HERTZ,
		PARAM_ANGULAR_DAMPING,
		PARAM_MAX_SPRING_TORQUE,
		PARAM_MAX
	};

	Box3DMotorJointImpl3D(Box3DBodyImpl3D* p_body_a, Box3DBodyImpl3D* p_body_b, const Transform3D& p_local_frame_a, const Transform3D& p_local_frame_b);

	real_t get_param(Param p_param) const;

	void set_param(Param p_param, real_t p_value);

	Vector3 get_linear_velocity() const { return linear_velocity; }

	void set_linear_velocity(const Vector3& p_velocity);

	Vector3 get_angular_velocity() const { return angular_velocity; }

	void set_angular_velocity(const Vector3& p_velocity);

protected:
	b3JointId _create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) override;

private:
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
