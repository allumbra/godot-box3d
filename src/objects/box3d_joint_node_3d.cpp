#include "box3d_joint_node_3d.hpp"

#include "../servers/box3d_physics_server_3d.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/physics_body3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>

void Box3DJointNode3D::set_node_a(const NodePath& p_path) {
	if (node_a == p_path) {
		return;
	}
	node_a = p_path;
	_rebuild_if_inside_tree();
}

void Box3DJointNode3D::set_node_b(const NodePath& p_path) {
	if (node_b == p_path) {
		return;
	}
	node_b = p_path;
	_rebuild_if_inside_tree();
}

void Box3DJointNode3D::set_exclude_nodes_from_collision(bool p_exclude) {
	if (exclude_collision == p_exclude) {
		return;
	}
	exclude_collision = p_exclude;
	Box3DPhysicsServer3D* server = _get_server();
	if (server != nullptr && joint_rid.is_valid()) {
		server->_joint_disable_collisions_between_bodies(joint_rid, exclude_collision);
	}
}

Box3DPhysicsServer3D* Box3DJointNode3D::_get_server() const {
	return Box3DPhysicsServer3D::get_singleton();
}

void Box3DJointNode3D::_notification(int p_what) {
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

void Box3DJointNode3D::_build_joint() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (joint_rid.is_valid() || node_a.is_empty() || node_b.is_empty()) {
		return;
	}

	Box3DPhysicsServer3D* server = _get_server();
	if (server == nullptr) {
		WARN_PRINT_ONCE("Box3D joint nodes require the Box3D physics server to be active; joint not created.");
		return;
	}

	auto* body_a = Object::cast_to<PhysicsBody3D>(get_node_or_null(node_a));
	auto* body_b = Object::cast_to<PhysicsBody3D>(get_node_or_null(node_b));
	ERR_FAIL_NULL_MSG(body_a, "Box3D joint node: node_a must point to a PhysicsBody3D.");
	ERR_FAIL_NULL_MSG(body_b, "Box3D joint node: node_b must point to a PhysicsBody3D.");

	const Transform3D joint_xform = get_global_transform();
	const Transform3D frame_a = body_a->get_global_transform().affine_inverse() * joint_xform;
	const Transform3D frame_b = _derive_frame_b(joint_xform, body_b->get_global_transform());

	joint_rid = _create_joint(server, body_a->get_rid(), body_b->get_rid(), frame_a, frame_b);
	ERR_FAIL_COND(!joint_rid.is_valid());

	server->_joint_disable_collisions_between_bodies(joint_rid, exclude_collision);
	_apply_params(server);
}

void Box3DJointNode3D::_destroy_joint() {
	if (!joint_rid.is_valid()) {
		return;
	}
	Box3DPhysicsServer3D* server = _get_server();
	if (server != nullptr) {
		server->_free_rid(joint_rid);
	}
	joint_rid = RID();
}

void Box3DJointNode3D::_rebuild_if_inside_tree() {
	if (is_inside_tree()) {
		_destroy_joint();
		_build_joint();
	}
}

void Box3DJointNode3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_node_a", "path"), &Box3DJointNode3D::set_node_a);
	ClassDB::bind_method(D_METHOD("get_node_a"), &Box3DJointNode3D::get_node_a);
	ClassDB::bind_method(D_METHOD("set_node_b", "path"), &Box3DJointNode3D::set_node_b);
	ClassDB::bind_method(D_METHOD("get_node_b"), &Box3DJointNode3D::get_node_b);
	ClassDB::bind_method(D_METHOD("set_exclude_nodes_from_collision", "exclude"), &Box3DJointNode3D::set_exclude_nodes_from_collision);
	ClassDB::bind_method(D_METHOD("get_exclude_nodes_from_collision"), &Box3DJointNode3D::get_exclude_nodes_from_collision);
	ClassDB::bind_method(D_METHOD("get_joint_rid"), &Box3DJointNode3D::get_joint_rid);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "node_a", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "PhysicsBody3D"), "set_node_a", "get_node_a");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "node_b", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "PhysicsBody3D"), "set_node_b", "get_node_b");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "exclude_nodes_from_collision"), "set_exclude_nodes_from_collision", "get_exclude_nodes_from_collision");
}
