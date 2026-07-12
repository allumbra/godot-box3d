#include "box3d_g6dof_joint_impl_3d.hpp"

#include "../misc/type_conversions.hpp"

#include <box3d/box3d.h>

namespace {

// Basis whose local Z column is the given world-of-frame axis (for revolute), or
// whose local X column is it (for prismatic) — box3d's rotation/translation axes.
Basis _axis_to_z(int p_axis) {
	switch (p_axis) {
		case Vector3::AXIS_X:
			return Basis(Vector3(0, 0, -1), Vector3(0, 1, 0), Vector3(1, 0, 0));
		case Vector3::AXIS_Y:
			return Basis(Vector3(1, 0, 0), Vector3(0, 0, -1), Vector3(0, 1, 0));
		default:
			return Basis();
	}
}

Basis _axis_to_x(int p_axis) {
	switch (p_axis) {
		case Vector3::AXIS_Y:
			return Basis(Vector3(0, 1, 0), Vector3(-1, 0, 0), Vector3(0, 0, 1));
		case Vector3::AXIS_Z:
			return Basis(Vector3(0, 0, 1), Vector3(0, 1, 0), Vector3(-1, 0, 0));
		default:
			return Basis();
	}
}

} // namespace

Box3DGeneric6DOFJointImpl3D::Box3DGeneric6DOFJointImpl3D(
		Box3DBodyImpl3D* p_body_a,
		Box3DBodyImpl3D* p_body_b,
		const Transform3D& p_local_frame_a,
		const Transform3D& p_local_frame_b) :
		Box3DJointImpl3D(p_body_a, p_body_b, p_local_frame_a, p_local_frame_b) {
	// Godot's Generic6DOFJoint3D defaults: every axis limited with lower == upper == 0,
	// i.e. fully locked (a weld) until the user frees axes.
	for (int axis = 0; axis < 3; axis++) {
		flags[axis][PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_LINEAR_LIMIT] = true;
		flags[axis][PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_ANGULAR_LIMIT] = true;
		params[axis][PhysicsServer3D::G6DOF_JOINT_ANGULAR_DAMPING] = 1.0;
		params[axis][PhysicsServer3D::G6DOF_JOINT_LINEAR_DAMPING] = 1.0;
		params[axis][PhysicsServer3D::G6DOF_JOINT_LINEAR_LIMIT_SOFTNESS] = 0.7;
		params[axis][PhysicsServer3D::G6DOF_JOINT_ANGULAR_LIMIT_SOFTNESS] = 0.5;
	}
}

bool Box3DGeneric6DOFJointImpl3D::_linear_locked(int p_axis) const {
	return flags[p_axis][PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_LINEAR_LIMIT] &&
			params[p_axis][PhysicsServer3D::G6DOF_JOINT_LINEAR_LOWER_LIMIT] >=
					params[p_axis][PhysicsServer3D::G6DOF_JOINT_LINEAR_UPPER_LIMIT];
}

bool Box3DGeneric6DOFJointImpl3D::_angular_locked(int p_axis) const {
	return flags[p_axis][PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_ANGULAR_LIMIT] &&
			params[p_axis][PhysicsServer3D::G6DOF_JOINT_ANGULAR_LOWER_LIMIT] >=
					params[p_axis][PhysicsServer3D::G6DOF_JOINT_ANGULAR_UPPER_LIMIT];
}

b3JointId Box3DGeneric6DOFJointImpl3D::_create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) {
	const int linear_locked_count = (int)_linear_locked(0) + (int)_linear_locked(1) + (int)_linear_locked(2);
	const int angular_locked_count = (int)_angular_locked(0) + (int)_angular_locked(1) + (int)_angular_locked(2);

	// All six axes locked: weld.
	if (linear_locked_count == 3 && angular_locked_count == 3) {
		b3WeldJointDef def = b3DefaultWeldJointDef();
		def.base.bodyIdA = p_body_a;
		def.base.bodyIdB = p_body_b;
		def.base.localFrameA = p_local_frame_a;
		def.base.localFrameB = p_local_frame_b;
		return b3CreateWeldJoint(p_world_id, &def);
	}

	// Position locked, exactly one rotation axis usable: revolute about that axis.
	if (linear_locked_count == 3 && angular_locked_count == 2) {
		int free_axis = 0;
		for (int axis = 0; axis < 3; axis++) {
			if (!_angular_locked(axis)) {
				free_axis = axis;
			}
		}
		const Basis remap = _axis_to_z(free_axis);
		const Transform3D frame_a = b3_to_godot(p_local_frame_a) * Transform3D(remap);
		const Transform3D frame_b = b3_to_godot(p_local_frame_b) * Transform3D(remap);

		b3RevoluteJointDef def = b3DefaultRevoluteJointDef();
		def.base.bodyIdA = p_body_a;
		def.base.bodyIdB = p_body_b;
		def.base.localFrameA = godot_to_b3_transform(frame_a);
		def.base.localFrameB = godot_to_b3_transform(frame_b);
		if (flags[free_axis][PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_ANGULAR_LIMIT]) {
			def.enableLimit = true;
			def.lowerAngle = (float)params[free_axis][PhysicsServer3D::G6DOF_JOINT_ANGULAR_LOWER_LIMIT];
			def.upperAngle = (float)params[free_axis][PhysicsServer3D::G6DOF_JOINT_ANGULAR_UPPER_LIMIT];
		}
		return b3CreateRevoluteJoint(p_world_id, &def);
	}

	// One translation axis usable, rotation locked: prismatic along that axis.
	if (linear_locked_count == 2 && angular_locked_count == 3) {
		int free_axis = 0;
		for (int axis = 0; axis < 3; axis++) {
			if (!_linear_locked(axis)) {
				free_axis = axis;
			}
		}
		const Basis remap = _axis_to_x(free_axis);
		const Transform3D frame_a = b3_to_godot(p_local_frame_a) * Transform3D(remap);
		const Transform3D frame_b = b3_to_godot(p_local_frame_b) * Transform3D(remap);

		b3PrismaticJointDef def = b3DefaultPrismaticJointDef();
		def.base.bodyIdA = p_body_a;
		def.base.bodyIdB = p_body_b;
		def.base.localFrameA = godot_to_b3_transform(frame_a);
		def.base.localFrameB = godot_to_b3_transform(frame_b);
		if (flags[free_axis][PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_LINEAR_LIMIT]) {
			def.enableLimit = true;
			def.lowerTranslation = (float)params[free_axis][PhysicsServer3D::G6DOF_JOINT_LINEAR_LOWER_LIMIT];
			def.upperTranslation = (float)params[free_axis][PhysicsServer3D::G6DOF_JOINT_LINEAR_UPPER_LIMIT];
		}
		return b3CreatePrismaticJoint(p_world_id, &def);
	}

	// Everything else (incl. the common ragdoll case: linear locked, angular free or
	// limited on 2-3 axes): spherical, approximating enabled angular limits with a
	// cone around frame A's Z plus twist limits from the X axis.
	if (linear_locked_count != 3) {
		WARN_PRINT_ONCE("Box3D: Generic6DOFJoint3D with multiple free linear axes is not representable; approximating with a ball joint (linear locked).");
	}

	b3SphericalJointDef def = b3DefaultSphericalJointDef();
	def.base.bodyIdA = p_body_a;
	def.base.bodyIdB = p_body_b;
	def.base.localFrameA = p_local_frame_a;
	def.base.localFrameB = p_local_frame_b;

	// Swing limits (Y/Z axes) -> cone angle; use the largest enabled span so nothing
	// is over-constrained. Twist (X axis) -> twist limits.
	float cone_angle = 0.0f;
	bool cone_enabled = false;
	for (int axis = 1; axis < 3; axis++) {
		if (flags[axis][PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_ANGULAR_LIMIT] && !_angular_locked(axis)) {
			cone_enabled = true;
			cone_angle = MAX(cone_angle, (float)Math::abs(params[axis][PhysicsServer3D::G6DOF_JOINT_ANGULAR_UPPER_LIMIT]));
			cone_angle = MAX(cone_angle, (float)Math::abs(params[axis][PhysicsServer3D::G6DOF_JOINT_ANGULAR_LOWER_LIMIT]));
		}
	}
	if (cone_enabled) {
		def.enableConeLimit = true;
		def.coneAngle = cone_angle;
	}
	if (flags[0][PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_ANGULAR_LIMIT] && !_angular_locked(0)) {
		def.enableTwistLimit = true;
		def.lowerTwistAngle = (float)params[0][PhysicsServer3D::G6DOF_JOINT_ANGULAR_LOWER_LIMIT];
		def.upperTwistAngle = (float)params[0][PhysicsServer3D::G6DOF_JOINT_ANGULAR_UPPER_LIMIT];
	} else if (_angular_locked(0)) {
		def.enableTwistLimit = true;
		def.lowerTwistAngle = 0.0f;
		def.upperTwistAngle = 0.0f;
	}

	return b3CreateSphericalJoint(p_world_id, &def);
}

real_t Box3DGeneric6DOFJointImpl3D::get_param(Vector3::Axis p_axis, Param p_param) const {
	ERR_FAIL_INDEX_V((int)p_param, PhysicsServer3D::G6DOF_JOINT_MAX, 0.0);
	return params[(int)p_axis][(int)p_param];
}

void Box3DGeneric6DOFJointImpl3D::set_param(Vector3::Axis p_axis, Param p_param, real_t p_value) {
	ERR_FAIL_INDEX((int)p_param, PhysicsServer3D::G6DOF_JOINT_MAX);
	if (params[(int)p_axis][(int)p_param] == p_value) {
		return;
	}
	params[(int)p_axis][(int)p_param] = p_value;
	switch (p_param) {
		case PhysicsServer3D::G6DOF_JOINT_LINEAR_LOWER_LIMIT:
		case PhysicsServer3D::G6DOF_JOINT_LINEAR_UPPER_LIMIT:
		case PhysicsServer3D::G6DOF_JOINT_ANGULAR_LOWER_LIMIT:
		case PhysicsServer3D::G6DOF_JOINT_ANGULAR_UPPER_LIMIT:
			// Limits can change the decomposition (locked vs free axis): rebuild.
			rebuild();
			break;
		default:
			// Softness/damping/motor/spring params have no direct box3d equivalent in
			// the decomposed representation; cached for round-tripping only.
			break;
	}
}

bool Box3DGeneric6DOFJointImpl3D::get_flag(Vector3::Axis p_axis, Flag p_flag) const {
	ERR_FAIL_INDEX_V((int)p_flag, PhysicsServer3D::G6DOF_JOINT_FLAG_MAX, false);
	return flags[(int)p_axis][(int)p_flag];
}

void Box3DGeneric6DOFJointImpl3D::set_flag(Vector3::Axis p_axis, Flag p_flag, bool p_enabled) {
	ERR_FAIL_INDEX((int)p_flag, PhysicsServer3D::G6DOF_JOINT_FLAG_MAX);
	if (flags[(int)p_axis][(int)p_flag] == p_enabled) {
		return;
	}
	flags[(int)p_axis][(int)p_flag] = p_enabled;
	switch (p_flag) {
		case PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_LINEAR_LIMIT:
		case PhysicsServer3D::G6DOF_JOINT_FLAG_ENABLE_ANGULAR_LIMIT:
			rebuild();
			break;
		default:
			WARN_PRINT_ONCE("Box3D: Generic6DOFJoint3D springs/motors are not supported by the decomposed mapping and are ignored.");
			break;
	}
}
