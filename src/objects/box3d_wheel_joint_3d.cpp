#include "box3d_wheel_joint_3d.hpp"

#include "../joints/box3d_wheel_joint_impl_3d.hpp"
#include "../servers/box3d_physics_server_3d.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/physics_body3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>

void Box3DWheelJoint3D::set_node_a(const NodePath& p_path) {
	if (node_a == p_path) {
		return;
	}
	node_a = p_path;
	if (is_inside_tree()) {
		_destroy_joint();
		_build_joint();
	}
}

void Box3DWheelJoint3D::set_node_b(const NodePath& p_path) {
	if (node_b == p_path) {
		return;
	}
	node_b = p_path;
	if (is_inside_tree()) {
		_destroy_joint();
		_build_joint();
	}
}

void Box3DWheelJoint3D::set_suspension_enabled(bool p_enabled) {
	suspension_enabled = p_enabled;
	_set_flag(Box3DWheelJointImpl3D::FLAG_ENABLE_SUSPENSION, p_enabled);
}

void Box3DWheelJoint3D::set_suspension_hertz(real_t p_value) {
	suspension_hertz = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_SUSPENSION_HERTZ, p_value);
}

void Box3DWheelJoint3D::set_suspension_damping_ratio(real_t p_value) {
	suspension_damping_ratio = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_SUSPENSION_DAMPING, p_value);
}

void Box3DWheelJoint3D::set_suspension_limit_enabled(bool p_enabled) {
	suspension_limit_enabled = p_enabled;
	_set_flag(Box3DWheelJointImpl3D::FLAG_ENABLE_SUSPENSION_LIMIT, p_enabled);
}

void Box3DWheelJoint3D::set_suspension_limit_lower(real_t p_value) {
	suspension_limit_lower = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_SUSPENSION_LOWER, p_value);
}

void Box3DWheelJoint3D::set_suspension_limit_upper(real_t p_value) {
	suspension_limit_upper = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_SUSPENSION_UPPER, p_value);
}

void Box3DWheelJoint3D::set_motor_enabled(bool p_enabled) {
	motor_enabled = p_enabled;
	_set_flag(Box3DWheelJointImpl3D::FLAG_ENABLE_SPIN_MOTOR, p_enabled);
}

void Box3DWheelJoint3D::set_motor_speed(real_t p_value) {
	motor_speed = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_SPIN_MOTOR_SPEED, p_value);
}

void Box3DWheelJoint3D::set_motor_max_torque(real_t p_value) {
	motor_max_torque = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_MAX_SPIN_TORQUE, p_value);
}

void Box3DWheelJoint3D::set_steering_enabled(bool p_enabled) {
	steering_enabled = p_enabled;
	_set_flag(Box3DWheelJointImpl3D::FLAG_ENABLE_STEERING, p_enabled);
}

void Box3DWheelJoint3D::set_steering_hertz(real_t p_value) {
	steering_hertz = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_STEERING_HERTZ, p_value);
}

void Box3DWheelJoint3D::set_steering_damping_ratio(real_t p_value) {
	steering_damping_ratio = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_STEERING_DAMPING, p_value);
}

void Box3DWheelJoint3D::set_target_steering_angle(real_t p_value) {
	target_steering_angle = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_TARGET_STEERING_ANGLE, p_value);
}

void Box3DWheelJoint3D::set_steering_max_torque(real_t p_value) {
	steering_max_torque = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_MAX_STEERING_TORQUE, p_value);
}

void Box3DWheelJoint3D::set_steering_limit_enabled(bool p_enabled) {
	steering_limit_enabled = p_enabled;
	_set_flag(Box3DWheelJointImpl3D::FLAG_ENABLE_STEERING_LIMIT, p_enabled);
}

void Box3DWheelJoint3D::set_steering_limit_lower(real_t p_value) {
	steering_limit_lower = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_STEERING_LOWER, p_value);
}

void Box3DWheelJoint3D::set_steering_limit_upper(real_t p_value) {
	steering_limit_upper = p_value;
	_set_param(Box3DWheelJointImpl3D::PARAM_STEERING_UPPER, p_value);
}

real_t Box3DWheelJoint3D::get_spin_speed() const {
	Box3DPhysicsServer3D* server = Box3DPhysicsServer3D::get_singleton();
	return (server != nullptr && joint_rid.is_valid()) ? server->wheel_joint_get_spin_speed(joint_rid) : 0.0;
}

real_t Box3DWheelJoint3D::get_spin_torque() const {
	Box3DPhysicsServer3D* server = Box3DPhysicsServer3D::get_singleton();
	return (server != nullptr && joint_rid.is_valid()) ? server->wheel_joint_get_spin_torque(joint_rid) : 0.0;
}

real_t Box3DWheelJoint3D::get_steering_angle() const {
	Box3DPhysicsServer3D* server = Box3DPhysicsServer3D::get_singleton();
	return (server != nullptr && joint_rid.is_valid()) ? server->wheel_joint_get_steering_angle(joint_rid) : 0.0;
}

real_t Box3DWheelJoint3D::get_steering_torque() const {
	Box3DPhysicsServer3D* server = Box3DPhysicsServer3D::get_singleton();
	return (server != nullptr && joint_rid.is_valid()) ? server->wheel_joint_get_steering_torque(joint_rid) : 0.0;
}

void Box3DWheelJoint3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_build_joint();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_destroy_joint();
		} break;
		default: {
		} break;
	}
}

void Box3DWheelJoint3D::_build_joint() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (joint_rid.is_valid() || node_a.is_empty() || node_b.is_empty()) {
		return;
	}

	Box3DPhysicsServer3D* server = Box3DPhysicsServer3D::get_singleton();
	if (server == nullptr) {
		WARN_PRINT_ONCE("Box3DWheelJoint3D requires the Box3D physics server to be active; joint not created.");
		return;
	}

	auto* body_a = Object::cast_to<PhysicsBody3D>(get_node_or_null(node_a));
	auto* body_b = Object::cast_to<PhysicsBody3D>(get_node_or_null(node_b));
	ERR_FAIL_NULL_MSG(body_a, "Box3DWheelJoint3D: node_a must point to a PhysicsBody3D (the chassis).");
	ERR_FAIL_NULL_MSG(body_b, "Box3DWheelJoint3D: node_b must point to a PhysicsBody3D (the wheel).");

	const Transform3D joint_xform = get_global_transform();
	const Transform3D frame_a = body_a->get_global_transform().affine_inverse() * joint_xform;
	const Transform3D frame_b = body_b->get_global_transform().affine_inverse() * joint_xform;

	joint_rid = server->wheel_joint_create(body_a->get_rid(), body_b->get_rid(), frame_a, frame_b);
	ERR_FAIL_COND(!joint_rid.is_valid());

	server->wheel_joint_set_flag(joint_rid, Box3DWheelJointImpl3D::FLAG_ENABLE_SUSPENSION, suspension_enabled);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_SUSPENSION_HERTZ, suspension_hertz);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_SUSPENSION_DAMPING, suspension_damping_ratio);
	server->wheel_joint_set_flag(joint_rid, Box3DWheelJointImpl3D::FLAG_ENABLE_SUSPENSION_LIMIT, suspension_limit_enabled);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_SUSPENSION_LOWER, suspension_limit_lower);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_SUSPENSION_UPPER, suspension_limit_upper);
	server->wheel_joint_set_flag(joint_rid, Box3DWheelJointImpl3D::FLAG_ENABLE_SPIN_MOTOR, motor_enabled);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_SPIN_MOTOR_SPEED, motor_speed);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_MAX_SPIN_TORQUE, motor_max_torque);
	server->wheel_joint_set_flag(joint_rid, Box3DWheelJointImpl3D::FLAG_ENABLE_STEERING, steering_enabled);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_STEERING_HERTZ, steering_hertz);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_STEERING_DAMPING, steering_damping_ratio);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_TARGET_STEERING_ANGLE, target_steering_angle);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_MAX_STEERING_TORQUE, steering_max_torque);
	server->wheel_joint_set_flag(joint_rid, Box3DWheelJointImpl3D::FLAG_ENABLE_STEERING_LIMIT, steering_limit_enabled);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_STEERING_LOWER, steering_limit_lower);
	server->wheel_joint_set_param(joint_rid, Box3DWheelJointImpl3D::PARAM_STEERING_UPPER, steering_limit_upper);
}

void Box3DWheelJoint3D::_destroy_joint() {
	if (!joint_rid.is_valid()) {
		return;
	}
	Box3DPhysicsServer3D* server = Box3DPhysicsServer3D::get_singleton();
	if (server != nullptr) {
		server->_free_rid(joint_rid);
	}
	joint_rid = RID();
}

void Box3DWheelJoint3D::_set_param(int p_param, real_t p_value) {
	if (!joint_rid.is_valid()) {
		return;
	}
	Box3DPhysicsServer3D* server = Box3DPhysicsServer3D::get_singleton();
	if (server != nullptr) {
		server->wheel_joint_set_param(joint_rid, p_param, p_value);
	}
}

void Box3DWheelJoint3D::_set_flag(int p_flag, bool p_enabled) {
	if (!joint_rid.is_valid()) {
		return;
	}
	Box3DPhysicsServer3D* server = Box3DPhysicsServer3D::get_singleton();
	if (server != nullptr) {
		server->wheel_joint_set_flag(joint_rid, p_flag, p_enabled);
	}
}

void Box3DWheelJoint3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_node_a", "path"), &Box3DWheelJoint3D::set_node_a);
	ClassDB::bind_method(D_METHOD("get_node_a"), &Box3DWheelJoint3D::get_node_a);
	ClassDB::bind_method(D_METHOD("set_node_b", "path"), &Box3DWheelJoint3D::set_node_b);
	ClassDB::bind_method(D_METHOD("get_node_b"), &Box3DWheelJoint3D::get_node_b);

	ClassDB::bind_method(D_METHOD("set_suspension_enabled", "enabled"), &Box3DWheelJoint3D::set_suspension_enabled);
	ClassDB::bind_method(D_METHOD("get_suspension_enabled"), &Box3DWheelJoint3D::get_suspension_enabled);
	ClassDB::bind_method(D_METHOD("set_suspension_hertz", "hertz"), &Box3DWheelJoint3D::set_suspension_hertz);
	ClassDB::bind_method(D_METHOD("get_suspension_hertz"), &Box3DWheelJoint3D::get_suspension_hertz);
	ClassDB::bind_method(D_METHOD("set_suspension_damping_ratio", "ratio"), &Box3DWheelJoint3D::set_suspension_damping_ratio);
	ClassDB::bind_method(D_METHOD("get_suspension_damping_ratio"), &Box3DWheelJoint3D::get_suspension_damping_ratio);
	ClassDB::bind_method(D_METHOD("set_suspension_limit_enabled", "enabled"), &Box3DWheelJoint3D::set_suspension_limit_enabled);
	ClassDB::bind_method(D_METHOD("get_suspension_limit_enabled"), &Box3DWheelJoint3D::get_suspension_limit_enabled);
	ClassDB::bind_method(D_METHOD("set_suspension_limit_lower", "meters"), &Box3DWheelJoint3D::set_suspension_limit_lower);
	ClassDB::bind_method(D_METHOD("get_suspension_limit_lower"), &Box3DWheelJoint3D::get_suspension_limit_lower);
	ClassDB::bind_method(D_METHOD("set_suspension_limit_upper", "meters"), &Box3DWheelJoint3D::set_suspension_limit_upper);
	ClassDB::bind_method(D_METHOD("get_suspension_limit_upper"), &Box3DWheelJoint3D::get_suspension_limit_upper);

	ClassDB::bind_method(D_METHOD("set_motor_enabled", "enabled"), &Box3DWheelJoint3D::set_motor_enabled);
	ClassDB::bind_method(D_METHOD("get_motor_enabled"), &Box3DWheelJoint3D::get_motor_enabled);
	ClassDB::bind_method(D_METHOD("set_motor_speed", "radians_per_second"), &Box3DWheelJoint3D::set_motor_speed);
	ClassDB::bind_method(D_METHOD("get_motor_speed"), &Box3DWheelJoint3D::get_motor_speed);
	ClassDB::bind_method(D_METHOD("set_motor_max_torque", "torque"), &Box3DWheelJoint3D::set_motor_max_torque);
	ClassDB::bind_method(D_METHOD("get_motor_max_torque"), &Box3DWheelJoint3D::get_motor_max_torque);

	ClassDB::bind_method(D_METHOD("set_steering_enabled", "enabled"), &Box3DWheelJoint3D::set_steering_enabled);
	ClassDB::bind_method(D_METHOD("get_steering_enabled"), &Box3DWheelJoint3D::get_steering_enabled);
	ClassDB::bind_method(D_METHOD("set_steering_hertz", "hertz"), &Box3DWheelJoint3D::set_steering_hertz);
	ClassDB::bind_method(D_METHOD("get_steering_hertz"), &Box3DWheelJoint3D::get_steering_hertz);
	ClassDB::bind_method(D_METHOD("set_steering_damping_ratio", "ratio"), &Box3DWheelJoint3D::set_steering_damping_ratio);
	ClassDB::bind_method(D_METHOD("get_steering_damping_ratio"), &Box3DWheelJoint3D::get_steering_damping_ratio);
	ClassDB::bind_method(D_METHOD("set_target_steering_angle", "radians"), &Box3DWheelJoint3D::set_target_steering_angle);
	ClassDB::bind_method(D_METHOD("get_target_steering_angle"), &Box3DWheelJoint3D::get_target_steering_angle);
	ClassDB::bind_method(D_METHOD("set_steering_max_torque", "torque"), &Box3DWheelJoint3D::set_steering_max_torque);
	ClassDB::bind_method(D_METHOD("get_steering_max_torque"), &Box3DWheelJoint3D::get_steering_max_torque);
	ClassDB::bind_method(D_METHOD("set_steering_limit_enabled", "enabled"), &Box3DWheelJoint3D::set_steering_limit_enabled);
	ClassDB::bind_method(D_METHOD("get_steering_limit_enabled"), &Box3DWheelJoint3D::get_steering_limit_enabled);
	ClassDB::bind_method(D_METHOD("set_steering_limit_lower", "radians"), &Box3DWheelJoint3D::set_steering_limit_lower);
	ClassDB::bind_method(D_METHOD("get_steering_limit_lower"), &Box3DWheelJoint3D::get_steering_limit_lower);
	ClassDB::bind_method(D_METHOD("set_steering_limit_upper", "radians"), &Box3DWheelJoint3D::set_steering_limit_upper);
	ClassDB::bind_method(D_METHOD("get_steering_limit_upper"), &Box3DWheelJoint3D::get_steering_limit_upper);

	ClassDB::bind_method(D_METHOD("get_spin_speed"), &Box3DWheelJoint3D::get_spin_speed);
	ClassDB::bind_method(D_METHOD("get_spin_torque"), &Box3DWheelJoint3D::get_spin_torque);
	ClassDB::bind_method(D_METHOD("get_steering_angle"), &Box3DWheelJoint3D::get_steering_angle);
	ClassDB::bind_method(D_METHOD("get_steering_torque"), &Box3DWheelJoint3D::get_steering_torque);
	ClassDB::bind_method(D_METHOD("get_joint_rid"), &Box3DWheelJoint3D::get_joint_rid);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "node_a", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "PhysicsBody3D"), "set_node_a", "get_node_a");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "node_b", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "PhysicsBody3D"), "set_node_b", "get_node_b");

	ADD_GROUP("Suspension", "suspension_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "suspension_enabled"), "set_suspension_enabled", "get_suspension_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "suspension_hertz", PROPERTY_HINT_RANGE, "0,30,0.1"), "set_suspension_hertz", "get_suspension_hertz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "suspension_damping_ratio", PROPERTY_HINT_RANGE, "0,5,0.01"), "set_suspension_damping_ratio", "get_suspension_damping_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "suspension_limit_enabled"), "set_suspension_limit_enabled", "get_suspension_limit_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "suspension_limit_lower", PROPERTY_HINT_RANGE, "-10,0,0.001,suffix:m"), "set_suspension_limit_lower", "get_suspension_limit_lower");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "suspension_limit_upper", PROPERTY_HINT_RANGE, "0,10,0.001,suffix:m"), "set_suspension_limit_upper", "get_suspension_limit_upper");

	ADD_GROUP("Motor", "motor_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "motor_enabled"), "set_motor_enabled", "get_motor_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "motor_speed", PROPERTY_HINT_NONE, "suffix:rad/s"), "set_motor_speed", "get_motor_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "motor_max_torque", PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"), "set_motor_max_torque", "get_motor_max_torque");

	ADD_GROUP("Steering", "steering_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "steering_enabled"), "set_steering_enabled", "get_steering_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "steering_hertz", PROPERTY_HINT_RANGE, "0,30,0.1"), "set_steering_hertz", "get_steering_hertz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "steering_damping_ratio", PROPERTY_HINT_RANGE, "0,5,0.01"), "set_steering_damping_ratio", "get_steering_damping_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "steering_target_angle", PROPERTY_HINT_RANGE, "-180,180,0.1,radians_as_degrees"), "set_target_steering_angle", "get_target_steering_angle");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "steering_max_torque", PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"), "set_steering_max_torque", "get_steering_max_torque");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "steering_limit_enabled"), "set_steering_limit_enabled", "get_steering_limit_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "steering_limit_lower", PROPERTY_HINT_RANGE, "-180,0,0.1,radians_as_degrees"), "set_steering_limit_lower", "get_steering_limit_lower");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "steering_limit_upper", PROPERTY_HINT_RANGE, "0,180,0.1,radians_as_degrees"), "set_steering_limit_upper", "get_steering_limit_upper");
}
