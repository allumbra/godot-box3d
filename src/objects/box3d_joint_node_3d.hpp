#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/rid.hpp>

using namespace godot;

class Box3DPhysicsServer3D;

// Shared base for scene-facing Box3D joint nodes (weld/distance/motor). Handles the
// node_a/node_b wiring, joint-frame derivation from this node's transform at ready
// time, lifecycle (build on READY, destroy on EXIT_TREE), and rebuild on path change.
// Subclasses implement _create_joint (server call) and _apply_params (push cached
// properties to the live joint).
class Box3DJointNode3D : public Node3D {
	GDCLASS(Box3DJointNode3D, Node3D)

public:
	NodePath get_node_a() const { return node_a; }
	void set_node_a(const NodePath& p_path);

	NodePath get_node_b() const { return node_b; }
	void set_node_b(const NodePath& p_path);

	bool get_exclude_nodes_from_collision() const { return exclude_collision; }
	void set_exclude_nodes_from_collision(bool p_exclude);

	RID get_joint_rid() const { return joint_rid; }

protected:
	static void _bind_methods();

	void _notification(int p_what);

	// Creates the joint on the Box3D server and returns its RID (invalid RID on failure).
	virtual RID _create_joint(Box3DPhysicsServer3D* p_server, const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) { return RID(); }

	// Pushes every cached property to the live joint after creation.
	virtual void _apply_params(Box3DPhysicsServer3D* p_server) {}

	// Local joint frame on body B. Default: this node's transform expressed in body B's
	// space (coincident frames — right for weld/motor/wheel). Distance-style joints
	// override to anchor at body B's own origin instead.
	virtual Transform3D _derive_frame_b(const Transform3D& p_joint_xform, const Transform3D& p_body_b_xform) const {
		return p_body_b_xform.affine_inverse() * p_joint_xform;
	}

	Box3DPhysicsServer3D* _get_server() const;

	void _build_joint();

	void _destroy_joint();

	void _rebuild_if_inside_tree();

	RID joint_rid;

private:
	NodePath node_a;
	NodePath node_b;
	bool exclude_collision = true;
};
