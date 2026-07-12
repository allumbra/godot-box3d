#include "box3d_motor_joint_node_3d.hpp"

#include "../joints/box3d_motor_joint_impl_3d.hpp"
#include "../servers/box3d_physics_server_3d.hpp"

#include <godot_cpp/classes/physics_direct_space_state3d.hpp>

using Impl = Box3DMotorJointImpl3D;

RID Box3DMotorJoint3D::_create_joint(Box3DPhysicsServer3D* p_server, const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) {
	return p_server->motor_joint_create(p_body_a, p_body_b, p_frame_a, p_frame_b);
}

void Box3DMotorJoint3D::_apply_params(Box3DPhysicsServer3D* p_server) {
	p_server->motor_joint_set_linear_velocity(joint_rid, linear_velocity);
	p_server->motor_joint_set_angular_velocity(joint_rid, angular_velocity);
	p_server->motor_joint_set_param(joint_rid, Impl::PARAM_MAX_VELOCITY_FORCE, max_velocity_force);
	p_server->motor_joint_set_param(joint_rid, Impl::PARAM_MAX_VELOCITY_TORQUE, max_velocity_torque);
	p_server->motor_joint_set_param(joint_rid, Impl::PARAM_LINEAR_HERTZ, linear_hertz);
	p_server->motor_joint_set_param(joint_rid, Impl::PARAM_LINEAR_DAMPING, linear_damping);
	p_server->motor_joint_set_param(joint_rid, Impl::PARAM_MAX_SPRING_FORCE, max_spring_force);
	p_server->motor_joint_set_param(joint_rid, Impl::PARAM_ANGULAR_HERTZ, angular_hertz);
	p_server->motor_joint_set_param(joint_rid, Impl::PARAM_ANGULAR_DAMPING, angular_damping);
	p_server->motor_joint_set_param(joint_rid, Impl::PARAM_MAX_SPRING_TORQUE, max_spring_torque);
}

void Box3DMotorJoint3D::_set_param(int p_param, real_t p_value) {
	Box3DPhysicsServer3D* server = _get_server();
	if (server != nullptr && joint_rid.is_valid()) {
		server->motor_joint_set_param(joint_rid, p_param, p_value);
	}
}

void Box3DMotorJoint3D::set_linear_velocity(const Vector3& p_velocity) {
	linear_velocity = p_velocity;
	Box3DPhysicsServer3D* server = _get_server();
	if (server != nullptr && joint_rid.is_valid()) {
		server->motor_joint_set_linear_velocity(joint_rid, p_velocity);
	}
}

void Box3DMotorJoint3D::set_angular_velocity(const Vector3& p_velocity) {
	angular_velocity = p_velocity;
	Box3DPhysicsServer3D* server = _get_server();
	if (server != nullptr && joint_rid.is_valid()) {
		server->motor_joint_set_angular_velocity(joint_rid, p_velocity);
	}
}

void Box3DMotorJoint3D::set_max_velocity_force(real_t p_value) {
	max_velocity_force = p_value;
	_set_param(Impl::PARAM_MAX_VELOCITY_FORCE, p_value);
}

void Box3DMotorJoint3D::set_max_velocity_torque(real_t p_value) {
	max_velocity_torque = p_value;
	_set_param(Impl::PARAM_MAX_VELOCITY_TORQUE, p_value);
}

void Box3DMotorJoint3D::set_linear_hertz(real_t p_value) {
	linear_hertz = p_value;
	_set_param(Impl::PARAM_LINEAR_HERTZ, p_value);
}

void Box3DMotorJoint3D::set_linear_damping_ratio(real_t p_value) {
	linear_damping = p_value;
	_set_param(Impl::PARAM_LINEAR_DAMPING, p_value);
}

void Box3DMotorJoint3D::set_max_spring_force(real_t p_value) {
	max_spring_force = p_value;
	_set_param(Impl::PARAM_MAX_SPRING_FORCE, p_value);
}

void Box3DMotorJoint3D::set_angular_hertz(real_t p_value) {
	angular_hertz = p_value;
	_set_param(Impl::PARAM_ANGULAR_HERTZ, p_value);
}

void Box3DMotorJoint3D::set_angular_damping_ratio(real_t p_value) {
	angular_damping = p_value;
	_set_param(Impl::PARAM_ANGULAR_DAMPING, p_value);
}

void Box3DMotorJoint3D::set_max_spring_torque(real_t p_value) {
	max_spring_torque = p_value;
	_set_param(Impl::PARAM_MAX_SPRING_TORQUE, p_value);
}

void Box3DMotorJoint3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "velocity"), &Box3DMotorJoint3D::set_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &Box3DMotorJoint3D::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_angular_velocity", "velocity"), &Box3DMotorJoint3D::set_angular_velocity);
	ClassDB::bind_method(D_METHOD("get_angular_velocity"), &Box3DMotorJoint3D::get_angular_velocity);
	ClassDB::bind_method(D_METHOD("set_max_velocity_force", "newtons"), &Box3DMotorJoint3D::set_max_velocity_force);
	ClassDB::bind_method(D_METHOD("get_max_velocity_force"), &Box3DMotorJoint3D::get_max_velocity_force);
	ClassDB::bind_method(D_METHOD("set_max_velocity_torque", "newton_meters"), &Box3DMotorJoint3D::set_max_velocity_torque);
	ClassDB::bind_method(D_METHOD("get_max_velocity_torque"), &Box3DMotorJoint3D::get_max_velocity_torque);
	ClassDB::bind_method(D_METHOD("set_linear_hertz", "hertz"), &Box3DMotorJoint3D::set_linear_hertz);
	ClassDB::bind_method(D_METHOD("get_linear_hertz"), &Box3DMotorJoint3D::get_linear_hertz);
	ClassDB::bind_method(D_METHOD("set_linear_damping_ratio", "ratio"), &Box3DMotorJoint3D::set_linear_damping_ratio);
	ClassDB::bind_method(D_METHOD("get_linear_damping_ratio"), &Box3DMotorJoint3D::get_linear_damping_ratio);
	ClassDB::bind_method(D_METHOD("set_max_spring_force", "newtons"), &Box3DMotorJoint3D::set_max_spring_force);
	ClassDB::bind_method(D_METHOD("get_max_spring_force"), &Box3DMotorJoint3D::get_max_spring_force);
	ClassDB::bind_method(D_METHOD("set_angular_hertz", "hertz"), &Box3DMotorJoint3D::set_angular_hertz);
	ClassDB::bind_method(D_METHOD("get_angular_hertz"), &Box3DMotorJoint3D::get_angular_hertz);
	ClassDB::bind_method(D_METHOD("set_angular_damping_ratio", "ratio"), &Box3DMotorJoint3D::set_angular_damping_ratio);
	ClassDB::bind_method(D_METHOD("get_angular_damping_ratio"), &Box3DMotorJoint3D::get_angular_damping_ratio);
	ClassDB::bind_method(D_METHOD("set_max_spring_torque", "newton_meters"), &Box3DMotorJoint3D::set_max_spring_torque);
	ClassDB::bind_method(D_METHOD("get_max_spring_torque"), &Box3DMotorJoint3D::get_max_spring_torque);

	ADD_GROUP("Velocity Drive", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_linear_velocity", "get_linear_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "angular_velocity", PROPERTY_HINT_NONE, "suffix:rad/s"), "set_angular_velocity", "get_angular_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_velocity_force", PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"), "set_max_velocity_force", "get_max_velocity_force");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_velocity_torque", PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"), "set_max_velocity_torque", "get_max_velocity_torque");
	ADD_GROUP("Position Spring", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_hertz", PROPERTY_HINT_RANGE, "0,120,0.1"), "set_linear_hertz", "get_linear_hertz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_damping_ratio", PROPERTY_HINT_RANGE, "0,5,0.01"), "set_linear_damping_ratio", "get_linear_damping_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_spring_force", PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"), "set_max_spring_force", "get_max_spring_force");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_hertz", PROPERTY_HINT_RANGE, "0,120,0.1"), "set_angular_hertz", "get_angular_hertz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_damping_ratio", PROPERTY_HINT_RANGE, "0,5,0.01"), "set_angular_damping_ratio", "get_angular_damping_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_spring_torque", PROPERTY_HINT_RANGE, "0,10000,0.1,or_greater"), "set_max_spring_torque", "get_max_spring_torque");
}
