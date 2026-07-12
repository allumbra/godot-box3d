#include "box3d_weld_joint_node_3d.hpp"

#include "../joints/box3d_weld_joint_impl_3d.hpp"
#include "../servers/box3d_physics_server_3d.hpp"

#include <godot_cpp/classes/physics_direct_space_state3d.hpp>

RID Box3DWeldJoint3D::_create_joint(Box3DPhysicsServer3D* p_server, const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) {
	return p_server->weld_joint_create(p_body_a, p_body_b, p_frame_a, p_frame_b);
}

void Box3DWeldJoint3D::_apply_params(Box3DPhysicsServer3D* p_server) {
	p_server->weld_joint_set_param(joint_rid, Box3DWeldJointImpl3D::PARAM_LINEAR_HERTZ, linear_hertz);
	p_server->weld_joint_set_param(joint_rid, Box3DWeldJointImpl3D::PARAM_LINEAR_DAMPING, linear_damping);
	p_server->weld_joint_set_param(joint_rid, Box3DWeldJointImpl3D::PARAM_ANGULAR_HERTZ, angular_hertz);
	p_server->weld_joint_set_param(joint_rid, Box3DWeldJointImpl3D::PARAM_ANGULAR_DAMPING, angular_damping);
}

void Box3DWeldJoint3D::_set_param(int p_param, real_t p_value) {
	Box3DPhysicsServer3D* server = _get_server();
	if (server != nullptr && joint_rid.is_valid()) {
		server->weld_joint_set_param(joint_rid, p_param, p_value);
	}
}

void Box3DWeldJoint3D::set_linear_hertz(real_t p_value) {
	linear_hertz = p_value;
	_set_param(Box3DWeldJointImpl3D::PARAM_LINEAR_HERTZ, p_value);
}

void Box3DWeldJoint3D::set_linear_damping_ratio(real_t p_value) {
	linear_damping = p_value;
	_set_param(Box3DWeldJointImpl3D::PARAM_LINEAR_DAMPING, p_value);
}

void Box3DWeldJoint3D::set_angular_hertz(real_t p_value) {
	angular_hertz = p_value;
	_set_param(Box3DWeldJointImpl3D::PARAM_ANGULAR_HERTZ, p_value);
}

void Box3DWeldJoint3D::set_angular_damping_ratio(real_t p_value) {
	angular_damping = p_value;
	_set_param(Box3DWeldJointImpl3D::PARAM_ANGULAR_DAMPING, p_value);
}

void Box3DWeldJoint3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linear_hertz", "hertz"), &Box3DWeldJoint3D::set_linear_hertz);
	ClassDB::bind_method(D_METHOD("get_linear_hertz"), &Box3DWeldJoint3D::get_linear_hertz);
	ClassDB::bind_method(D_METHOD("set_linear_damping_ratio", "ratio"), &Box3DWeldJoint3D::set_linear_damping_ratio);
	ClassDB::bind_method(D_METHOD("get_linear_damping_ratio"), &Box3DWeldJoint3D::get_linear_damping_ratio);
	ClassDB::bind_method(D_METHOD("set_angular_hertz", "hertz"), &Box3DWeldJoint3D::set_angular_hertz);
	ClassDB::bind_method(D_METHOD("get_angular_hertz"), &Box3DWeldJoint3D::get_angular_hertz);
	ClassDB::bind_method(D_METHOD("set_angular_damping_ratio", "ratio"), &Box3DWeldJoint3D::set_angular_damping_ratio);
	ClassDB::bind_method(D_METHOD("get_angular_damping_ratio"), &Box3DWeldJoint3D::get_angular_damping_ratio);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_hertz", PROPERTY_HINT_RANGE, "0,120,0.1"), "set_linear_hertz", "get_linear_hertz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_damping_ratio", PROPERTY_HINT_RANGE, "0,5,0.01"), "set_linear_damping_ratio", "get_linear_damping_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_hertz", PROPERTY_HINT_RANGE, "0,120,0.1"), "set_angular_hertz", "get_angular_hertz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_damping_ratio", PROPERTY_HINT_RANGE, "0,5,0.01"), "set_angular_damping_ratio", "get_angular_damping_ratio");
}
