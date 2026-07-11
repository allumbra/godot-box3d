#pragma once

#include "box3d_joint_impl_3d.hpp"

#include <cfloat>

// Box3D-native distance joint (b3DistanceJointDef): rope/rod/spring between two anchor
// points, with optional limit and linear motor. No Godot joint type matches, so it is
// exposed via distance_joint_* methods bound on Box3DPhysicsServer3D.
class Box3DDistanceJointImpl3D final : public Box3DJointImpl3D {
public:
	enum Param {
		PARAM_LENGTH,
		PARAM_LOWER_SPRING_FORCE,
		PARAM_UPPER_SPRING_FORCE,
		PARAM_HERTZ,
		PARAM_DAMPING,
		PARAM_MIN_LENGTH,
		PARAM_MAX_LENGTH,
		PARAM_MOTOR_SPEED,
		PARAM_MAX_MOTOR_FORCE,
		PARAM_MAX
	};

	enum Flag {
		FLAG_ENABLE_SPRING,
		FLAG_ENABLE_LIMIT,
		FLAG_ENABLE_MOTOR,
		FLAG_MAX
	};

	Box3DDistanceJointImpl3D(Box3DBodyImpl3D* p_body_a, Box3DBodyImpl3D* p_body_b, const Transform3D& p_local_frame_a, const Transform3D& p_local_frame_b);

	real_t get_param(Param p_param) const;

	void set_param(Param p_param, real_t p_value);

	bool get_flag(Flag p_flag) const;

	void set_flag(Flag p_flag, bool p_enabled);

	// Live read-backs (0.0 while not live).
	real_t get_current_length() const;
	real_t get_motor_force() const;

protected:
	b3JointId _create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) override;

private:
	real_t length = 1.0;
	real_t lower_spring_force = -FLT_MAX;
	real_t upper_spring_force = FLT_MAX;
	real_t hertz = 4.0;
	real_t damping = 0.7;
	real_t min_length = 0.0;
	real_t max_length = FLT_MAX;
	real_t motor_speed = 0.0;
	real_t max_motor_force = 0.0;

	bool enable_spring = false;
	bool enable_limit = false;
	bool enable_motor = false;
};
