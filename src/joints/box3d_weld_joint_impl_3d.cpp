#include "box3d_weld_joint_impl_3d.hpp"

#include <box3d/box3d.h>

Box3DWeldJointImpl3D::Box3DWeldJointImpl3D(
		Box3DBodyImpl3D* p_body_a,
		Box3DBodyImpl3D* p_body_b,
		const Transform3D& p_local_frame_a,
		const Transform3D& p_local_frame_b) :
		Box3DJointImpl3D(p_body_a, p_body_b, p_local_frame_a, p_local_frame_b) {
}

b3JointId Box3DWeldJointImpl3D::_create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) {
	b3WeldJointDef def = b3DefaultWeldJointDef();
	def.base.bodyIdA = p_body_a;
	def.base.bodyIdB = p_body_b;
	def.base.localFrameA = p_local_frame_a;
	def.base.localFrameB = p_local_frame_b;
	def.linearHertz = (float)linear_hertz;
	def.linearDampingRatio = (float)linear_damping;
	def.angularHertz = (float)angular_hertz;
	def.angularDampingRatio = (float)angular_damping;

	return b3CreateWeldJoint(p_world_id, &def);
}

real_t Box3DWeldJointImpl3D::get_param(Param p_param) const {
	switch (p_param) {
		case PARAM_LINEAR_HERTZ:
			return linear_hertz;
		case PARAM_LINEAR_DAMPING:
			return linear_damping;
		case PARAM_ANGULAR_HERTZ:
			return angular_hertz;
		case PARAM_ANGULAR_DAMPING:
			return angular_damping;
		default:
			return 0.0;
	}
}

void Box3DWeldJointImpl3D::set_param(Param p_param, real_t p_value) {
	switch (p_param) {
		case PARAM_LINEAR_HERTZ:
			linear_hertz = p_value;
			if (has_joint_id()) {
				b3WeldJoint_SetLinearHertz(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_LINEAR_DAMPING:
			linear_damping = p_value;
			if (has_joint_id()) {
				b3WeldJoint_SetLinearDampingRatio(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_ANGULAR_HERTZ:
			angular_hertz = p_value;
			if (has_joint_id()) {
				b3WeldJoint_SetAngularHertz(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_ANGULAR_DAMPING:
			angular_damping = p_value;
			if (has_joint_id()) {
				b3WeldJoint_SetAngularDampingRatio(get_joint_id(), (float)p_value);
			}
			break;
		default:
			break;
	}
}
