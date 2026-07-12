#include "box3d_distance_joint_node_3d.hpp"

#include "../joints/box3d_distance_joint_impl_3d.hpp"
#include "../servers/box3d_physics_server_3d.hpp"

#include <godot_cpp/classes/physics_direct_space_state3d.hpp>

using Impl = Box3DDistanceJointImpl3D;

RID Box3DDistanceJoint3D::_create_joint(Box3DPhysicsServer3D* p_server, const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) {
	return p_server->distance_joint_create(p_body_a, p_body_b, p_frame_a, p_frame_b);
}

void Box3DDistanceJoint3D::_apply_params(Box3DPhysicsServer3D* p_server) {
	p_server->distance_joint_set_param(joint_rid, Impl::PARAM_LENGTH, length);
	p_server->distance_joint_set_param(joint_rid, Impl::PARAM_HERTZ, hertz);
	p_server->distance_joint_set_param(joint_rid, Impl::PARAM_DAMPING, damping);
	p_server->distance_joint_set_param(joint_rid, Impl::PARAM_MIN_LENGTH, min_length);
	p_server->distance_joint_set_param(joint_rid, Impl::PARAM_MAX_LENGTH, max_length);
	p_server->distance_joint_set_param(joint_rid, Impl::PARAM_MOTOR_SPEED, motor_speed);
	p_server->distance_joint_set_param(joint_rid, Impl::PARAM_MAX_MOTOR_FORCE, motor_max_force);
	p_server->distance_joint_set_flag(joint_rid, Impl::FLAG_ENABLE_SPRING, spring_enabled);
	p_server->distance_joint_set_flag(joint_rid, Impl::FLAG_ENABLE_LIMIT, limit_enabled);
	p_server->distance_joint_set_flag(joint_rid, Impl::FLAG_ENABLE_MOTOR, motor_enabled);
}

void Box3DDistanceJoint3D::_set_param(int p_param, real_t p_value) {
	Box3DPhysicsServer3D* server = _get_server();
	if (server != nullptr && joint_rid.is_valid()) {
		server->distance_joint_set_param(joint_rid, p_param, p_value);
	}
}

void Box3DDistanceJoint3D::_set_flag(int p_flag, bool p_enabled) {
	Box3DPhysicsServer3D* server = _get_server();
	if (server != nullptr && joint_rid.is_valid()) {
		server->distance_joint_set_flag(joint_rid, p_flag, p_enabled);
	}
}

void Box3DDistanceJoint3D::set_length(real_t p_value) {
	length = p_value;
	_set_param(Impl::PARAM_LENGTH, p_value);
}

void Box3DDistanceJoint3D::set_spring_enabled(bool p_enabled) {
	spring_enabled = p_enabled;
	_set_flag(Impl::FLAG_ENABLE_SPRING, p_enabled);
}

void Box3DDistanceJoint3D::set_spring_hertz(real_t p_value) {
	hertz = p_value;
	_set_param(Impl::PARAM_HERTZ, p_value);
}

void Box3DDistanceJoint3D::set_spring_damping_ratio(real_t p_value) {
	damping = p_value;
	_set_param(Impl::PARAM_DAMPING, p_value);
}

void Box3DDistanceJoint3D::set_limit_enabled(bool p_enabled) {
	limit_enabled = p_enabled;
	_set_flag(Impl::FLAG_ENABLE_LIMIT, p_enabled);
}

void Box3DDistanceJoint3D::set_min_length(real_t p_value) {
	min_length = p_value;
	_set_param(Impl::PARAM_MIN_LENGTH, p_value);
}

void Box3DDistanceJoint3D::set_max_length(real_t p_value) {
	max_length = p_value;
	_set_param(Impl::PARAM_MAX_LENGTH, p_value);
}

void Box3DDistanceJoint3D::set_motor_enabled(bool p_enabled) {
	motor_enabled = p_enabled;
	_set_flag(Impl::FLAG_ENABLE_MOTOR, p_enabled);
}

void Box3DDistanceJoint3D::set_motor_speed(real_t p_value) {
	motor_speed = p_value;
	_set_param(Impl::PARAM_MOTOR_SPEED, p_value);
}

void Box3DDistanceJoint3D::set_motor_max_force(real_t p_value) {
	motor_max_force = p_value;
	_set_param(Impl::PARAM_MAX_MOTOR_FORCE, p_value);
}

real_t Box3DDistanceJoint3D::get_current_length() const {
	Box3DPhysicsServer3D* server = _get_server();
	return (server != nullptr && joint_rid.is_valid()) ? server->distance_joint_get_current_length(joint_rid) : 0.0;
}

real_t Box3DDistanceJoint3D::get_motor_force() const {
	Box3DPhysicsServer3D* server = _get_server();
	return (server != nullptr && joint_rid.is_valid()) ? server->distance_joint_get_motor_force(joint_rid) : 0.0;
}

void Box3DDistanceJoint3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_length", "meters"), &Box3DDistanceJoint3D::set_length);
	ClassDB::bind_method(D_METHOD("get_length"), &Box3DDistanceJoint3D::get_length);
	ClassDB::bind_method(D_METHOD("set_spring_enabled", "enabled"), &Box3DDistanceJoint3D::set_spring_enabled);
	ClassDB::bind_method(D_METHOD("get_spring_enabled"), &Box3DDistanceJoint3D::get_spring_enabled);
	ClassDB::bind_method(D_METHOD("set_spring_hertz", "hertz"), &Box3DDistanceJoint3D::set_spring_hertz);
	ClassDB::bind_method(D_METHOD("get_spring_hertz"), &Box3DDistanceJoint3D::get_spring_hertz);
	ClassDB::bind_method(D_METHOD("set_spring_damping_ratio", "ratio"), &Box3DDistanceJoint3D::set_spring_damping_ratio);
	ClassDB::bind_method(D_METHOD("get_spring_damping_ratio"), &Box3DDistanceJoint3D::get_spring_damping_ratio);
	ClassDB::bind_method(D_METHOD("set_limit_enabled", "enabled"), &Box3DDistanceJoint3D::set_limit_enabled);
	ClassDB::bind_method(D_METHOD("get_limit_enabled"), &Box3DDistanceJoint3D::get_limit_enabled);
	ClassDB::bind_method(D_METHOD("set_min_length", "meters"), &Box3DDistanceJoint3D::set_min_length);
	ClassDB::bind_method(D_METHOD("get_min_length"), &Box3DDistanceJoint3D::get_min_length);
	ClassDB::bind_method(D_METHOD("set_max_length", "meters"), &Box3DDistanceJoint3D::set_max_length);
	ClassDB::bind_method(D_METHOD("get_max_length"), &Box3DDistanceJoint3D::get_max_length);
	ClassDB::bind_method(D_METHOD("set_motor_enabled", "enabled"), &Box3DDistanceJoint3D::set_motor_enabled);
	ClassDB::bind_method(D_METHOD("get_motor_enabled"), &Box3DDistanceJoint3D::get_motor_enabled);
	ClassDB::bind_method(D_METHOD("set_motor_speed", "meters_per_second"), &Box3DDistanceJoint3D::set_motor_speed);
	ClassDB::bind_method(D_METHOD("get_motor_speed"), &Box3DDistanceJoint3D::get_motor_speed);
	ClassDB::bind_method(D_METHOD("set_motor_max_force", "newtons"), &Box3DDistanceJoint3D::set_motor_max_force);
	ClassDB::bind_method(D_METHOD("get_motor_max_force"), &Box3DDistanceJoint3D::get_motor_max_force);
	ClassDB::bind_method(D_METHOD("get_current_length"), &Box3DDistanceJoint3D::get_current_length);
	ClassDB::bind_method(D_METHOD("get_motor_force"), &Box3DDistanceJoint3D::get_motor_force);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater,suffix:m"), "set_length", "get_length");
	ADD_GROUP("Spring", "spring_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "spring_enabled"), "set_spring_enabled", "get_spring_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spring_hertz", PROPERTY_HINT_RANGE, "0,30,0.1"), "set_spring_hertz", "get_spring_hertz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spring_damping_ratio", PROPERTY_HINT_RANGE, "0,5,0.01"), "set_spring_damping_ratio", "get_spring_damping_ratio");
	ADD_GROUP("Limit", "limit_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "limit_enabled"), "set_limit_enabled", "get_limit_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_length", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater,suffix:m"), "set_min_length", "get_min_length");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_length", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater,suffix:m"), "set_max_length", "get_max_length");
	ADD_GROUP("Motor", "motor_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "motor_enabled"), "set_motor_enabled", "get_motor_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "motor_speed", PROPERTY_HINT_NONE, "suffix:m/s"), "set_motor_speed", "get_motor_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "motor_max_force", PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"), "set_motor_max_force", "get_motor_max_force");
}
