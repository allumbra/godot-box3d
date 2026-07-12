#include "box3d_pin_joint_impl_3d.hpp"

#include <box3d/box3d.h>

Box3DPinJointImpl3D::Box3DPinJointImpl3D(
		Box3DBodyImpl3D* p_body_a,
		Box3DBodyImpl3D* p_body_b,
		const Transform3D& p_local_frame_a,
		const Transform3D& p_local_frame_b) :
		Box3DJointImpl3D(p_body_a, p_body_b, p_local_frame_a, p_local_frame_b) {
}

b3JointId Box3DPinJointImpl3D::_create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) {
	b3SphericalJointDef def = b3DefaultSphericalJointDef();
	def.base.bodyIdA = p_body_a;
	def.base.bodyIdB = p_body_b;
	def.base.localFrameA = p_local_frame_a;
	def.base.localFrameB = p_local_frame_b;
	// No spring: a Godot pin joint is a free ball-socket. Mapping BIAS/DAMPING onto
	// box3d's rotational spring made every pin joint hold its spawn pose (e.g.
	// ragdolls falling in a rigid plank). Those params are solver error-correction
	// knobs in Godot with no box3d equivalent — cached but ignored, like the hinge's.
	return b3CreateSphericalJoint(p_world_id, &def);
}

real_t Box3DPinJointImpl3D::get_param(Param p_param) const {
	switch (p_param) {
		case PhysicsServer3D::PIN_JOINT_DAMPING:
			return damping;
		case PhysicsServer3D::PIN_JOINT_BIAS:
			return bias;
		case PhysicsServer3D::PIN_JOINT_IMPULSE_CLAMP:
			return impulse_clamp;
		default:
			return 0.0;
	}
}

void Box3DPinJointImpl3D::set_param(Param p_param, real_t p_value) {
	switch (p_param) {
		case PhysicsServer3D::PIN_JOINT_DAMPING:
			damping = p_value;
			WARN_PRINT_ONCE("Box3D: PinJoint3D's DAMPING parameter has no Box3D equivalent and is ignored.");
			break;
		case PhysicsServer3D::PIN_JOINT_BIAS:
			bias = p_value;
			WARN_PRINT_ONCE("Box3D: PinJoint3D's BIAS parameter has no Box3D equivalent and is ignored.");
			break;
		case PhysicsServer3D::PIN_JOINT_IMPULSE_CLAMP:
			impulse_clamp = p_value;
			WARN_PRINT_ONCE("Box3D: PinJoint3D's IMPULSE_CLAMP parameter has no Box3D equivalent and is ignored.");
			break;
		default:
			break;
	}
}
