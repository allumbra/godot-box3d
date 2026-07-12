#pragma once

#include "box3d_joint_node_3d.hpp"

// Scene-facing distance joint (b3DistanceJointDef): rope/rod/spring between two
// anchor points, with optional limit and linear motor, plus live readbacks.
class Box3DDistanceJoint3D : public Box3DJointNode3D {
	GDCLASS(Box3DDistanceJoint3D, Box3DJointNode3D)

public:
	real_t get_length() const { return length; }
	void set_length(real_t p_value);

	bool get_spring_enabled() const { return spring_enabled; }
	void set_spring_enabled(bool p_enabled);

	real_t get_spring_hertz() const { return hertz; }
	void set_spring_hertz(real_t p_value);

	real_t get_spring_damping_ratio() const { return damping; }
	void set_spring_damping_ratio(real_t p_value);

	bool get_limit_enabled() const { return limit_enabled; }
	void set_limit_enabled(bool p_enabled);

	real_t get_min_length() const { return min_length; }
	void set_min_length(real_t p_value);

	real_t get_max_length() const { return max_length; }
	void set_max_length(real_t p_value);

	bool get_motor_enabled() const { return motor_enabled; }
	void set_motor_enabled(bool p_enabled);

	real_t get_motor_speed() const { return motor_speed; }
	void set_motor_speed(real_t p_value);

	real_t get_motor_max_force() const { return motor_max_force; }
	void set_motor_max_force(real_t p_value);

	real_t get_current_length() const;

	real_t get_motor_force() const;

protected:
	static void _bind_methods();

	RID _create_joint(Box3DPhysicsServer3D* p_server, const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) override;

	void _apply_params(Box3DPhysicsServer3D* p_server) override;

	// A distance joint needs two distinct anchors: this node marks the anchor on
	// body A; body B attaches at its own origin.
	Transform3D _derive_frame_b(const Transform3D& p_joint_xform, const Transform3D& p_body_b_xform) const override {
		return Transform3D();
	}

private:
	void _set_param(int p_param, real_t p_value);

	void _set_flag(int p_flag, bool p_enabled);

	real_t length = 1.0;
	real_t hertz = 4.0;
	real_t damping = 0.7;
	real_t min_length = 0.0;
	real_t max_length = 1000.0;
	real_t motor_speed = 0.0;
	real_t motor_max_force = 0.0;
	bool spring_enabled = false;
	bool limit_enabled = false;
	bool motor_enabled = false;
};
