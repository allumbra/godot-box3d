#pragma once

#include "box3d_joint_impl_3d.hpp"

// Box3D-native wheel joint (b3WheelJointDef/b3CreateWheelJoint). Godot has no wheel
// joint type in PhysicsServer3D, so this is exposed through custom wheel_joint_*
// methods bound on Box3DPhysicsServer3D rather than the standard _joint_make_* path.
// Conventions (box3d types.h): body A is the chassis, body B is the wheel; the wheel
// spins about LOCAL Z of frame B, translates (suspension) along LOCAL X of frame A,
// and steers about LOCAL X of frame A.
class Box3DWheelJointImpl3D final : public Box3DJointImpl3D {
public:
	enum Param {
		PARAM_SUSPENSION_HERTZ,
		PARAM_SUSPENSION_DAMPING,
		PARAM_SUSPENSION_LOWER,
		PARAM_SUSPENSION_UPPER,
		PARAM_SPIN_MOTOR_SPEED,
		PARAM_MAX_SPIN_TORQUE,
		PARAM_STEERING_HERTZ,
		PARAM_STEERING_DAMPING,
		PARAM_TARGET_STEERING_ANGLE,
		PARAM_MAX_STEERING_TORQUE,
		PARAM_STEERING_LOWER,
		PARAM_STEERING_UPPER,
		PARAM_MAX
	};

	enum Flag {
		FLAG_ENABLE_SUSPENSION,
		FLAG_ENABLE_SUSPENSION_LIMIT,
		FLAG_ENABLE_SPIN_MOTOR,
		FLAG_ENABLE_STEERING,
		FLAG_ENABLE_STEERING_LIMIT,
		FLAG_MAX
	};

	Box3DWheelJointImpl3D(Box3DBodyImpl3D* p_body_a, Box3DBodyImpl3D* p_body_b, const Transform3D& p_local_frame_a, const Transform3D& p_local_frame_b);

	real_t get_param(Param p_param) const;

	void set_param(Param p_param, real_t p_value);

	bool get_flag(Flag p_flag) const;

	void set_flag(Flag p_flag, bool p_enabled);

	// Live read-backs (0.0 while the joint has no live b3JointId).
	real_t get_spin_speed() const;
	real_t get_spin_torque() const;
	real_t get_steering_angle() const;
	real_t get_steering_torque() const;

protected:
	b3JointId _create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) override;

private:
	real_t suspension_hertz = 4.0;
	real_t suspension_damping = 0.7;
	real_t suspension_lower = -0.25;
	real_t suspension_upper = 0.25;
	real_t spin_motor_speed = 0.0;
	real_t max_spin_torque = 0.0;
	real_t steering_hertz = 4.0;
	real_t steering_damping = 0.7;
	real_t target_steering_angle = 0.0;
	real_t max_steering_torque = 0.0;
	real_t steering_lower = -Math_PI * 0.25;
	real_t steering_upper = Math_PI * 0.25;

	bool enable_suspension = true;
	bool enable_suspension_limit = false;
	bool enable_spin_motor = false;
	bool enable_steering = false;
	bool enable_steering_limit = false;
};
