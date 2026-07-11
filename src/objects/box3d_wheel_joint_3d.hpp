#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/rid.hpp>

using namespace godot;

// Scene-facing wheel joint node (Godot has no built-in wheel joint). Wraps the
// wheel_joint_* API bound on Box3DPhysicsServer3D. Node A is the chassis, node B
// the wheel. This node's own transform defines the joint frames at ready time:
// its LOCAL X is the suspension/steering axis (point it up for a car), its LOCAL Z
// is the wheel spin axis (point it along the axle). Only functional when the
// active physics server is Box3D.
class Box3DWheelJoint3D : public Node3D {
	GDCLASS(Box3DWheelJoint3D, Node3D)

public:
	NodePath get_node_a() const { return node_a; }
	void set_node_a(const NodePath& p_path);

	NodePath get_node_b() const { return node_b; }
	void set_node_b(const NodePath& p_path);

	bool get_suspension_enabled() const { return suspension_enabled; }
	void set_suspension_enabled(bool p_enabled);

	real_t get_suspension_hertz() const { return suspension_hertz; }
	void set_suspension_hertz(real_t p_value);

	real_t get_suspension_damping_ratio() const { return suspension_damping_ratio; }
	void set_suspension_damping_ratio(real_t p_value);

	bool get_suspension_limit_enabled() const { return suspension_limit_enabled; }
	void set_suspension_limit_enabled(bool p_enabled);

	real_t get_suspension_limit_lower() const { return suspension_limit_lower; }
	void set_suspension_limit_lower(real_t p_value);

	real_t get_suspension_limit_upper() const { return suspension_limit_upper; }
	void set_suspension_limit_upper(real_t p_value);

	bool get_motor_enabled() const { return motor_enabled; }
	void set_motor_enabled(bool p_enabled);

	real_t get_motor_speed() const { return motor_speed; }
	void set_motor_speed(real_t p_value);

	real_t get_motor_max_torque() const { return motor_max_torque; }
	void set_motor_max_torque(real_t p_value);

	bool get_steering_enabled() const { return steering_enabled; }
	void set_steering_enabled(bool p_enabled);

	real_t get_steering_hertz() const { return steering_hertz; }
	void set_steering_hertz(real_t p_value);

	real_t get_steering_damping_ratio() const { return steering_damping_ratio; }
	void set_steering_damping_ratio(real_t p_value);

	real_t get_target_steering_angle() const { return target_steering_angle; }
	void set_target_steering_angle(real_t p_value);

	real_t get_steering_max_torque() const { return steering_max_torque; }
	void set_steering_max_torque(real_t p_value);

	bool get_steering_limit_enabled() const { return steering_limit_enabled; }
	void set_steering_limit_enabled(bool p_enabled);

	real_t get_steering_limit_lower() const { return steering_limit_lower; }
	void set_steering_limit_lower(real_t p_value);

	real_t get_steering_limit_upper() const { return steering_limit_upper; }
	void set_steering_limit_upper(real_t p_value);

	// Live read-backs (0.0 when the joint is not live).
	real_t get_spin_speed() const;
	real_t get_spin_torque() const;
	real_t get_steering_angle() const;
	real_t get_steering_torque() const;

	RID get_joint_rid() const { return joint_rid; }

protected:
	static void _bind_methods();

	void _notification(int p_what);

private:
	void _build_joint();

	void _destroy_joint();

	void _set_param(int p_param, real_t p_value);

	void _set_flag(int p_flag, bool p_enabled);

	NodePath node_a;
	NodePath node_b;

	bool suspension_enabled = true;
	real_t suspension_hertz = 4.0;
	real_t suspension_damping_ratio = 0.7;
	bool suspension_limit_enabled = false;
	real_t suspension_limit_lower = -0.25;
	real_t suspension_limit_upper = 0.25;

	bool motor_enabled = false;
	real_t motor_speed = 0.0;
	real_t motor_max_torque = 0.0;

	bool steering_enabled = false;
	real_t steering_hertz = 4.0;
	real_t steering_damping_ratio = 0.7;
	real_t target_steering_angle = 0.0;
	real_t steering_max_torque = 0.0;
	bool steering_limit_enabled = false;
	real_t steering_limit_lower = -Math_PI * 0.25;
	real_t steering_limit_upper = Math_PI * 0.25;

	RID joint_rid;
};
