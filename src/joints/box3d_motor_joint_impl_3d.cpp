#include "box3d_motor_joint_impl_3d.hpp"

#include "../misc/type_conversions.hpp"

#include <box3d/box3d.h>

Box3DMotorJointImpl3D::Box3DMotorJointImpl3D(
		Box3DBodyImpl3D* p_body_a,
		Box3DBodyImpl3D* p_body_b,
		const Transform3D& p_local_frame_a,
		const Transform3D& p_local_frame_b) :
		Box3DJointImpl3D(p_body_a, p_body_b, p_local_frame_a, p_local_frame_b) {
}

b3JointId Box3DMotorJointImpl3D::_create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) {
	b3MotorJointDef def = b3DefaultMotorJointDef();
	def.base.bodyIdA = p_body_a;
	def.base.bodyIdB = p_body_b;
	def.base.localFrameA = p_local_frame_a;
	def.base.localFrameB = p_local_frame_b;
	def.linearVelocity = godot_to_b3(linear_velocity);
	def.maxVelocityForce = (float)max_velocity_force;
	def.angularVelocity = godot_to_b3(angular_velocity);
	def.maxVelocityTorque = (float)max_velocity_torque;
	def.linearHertz = (float)linear_hertz;
	def.linearDampingRatio = (float)linear_damping;
	def.maxSpringForce = (float)max_spring_force;
	def.angularHertz = (float)angular_hertz;
	def.angularDampingRatio = (float)angular_damping;
	def.maxSpringTorque = (float)max_spring_torque;

	return b3CreateMotorJoint(p_world_id, &def);
}

real_t Box3DMotorJointImpl3D::get_param(Param p_param) const {
	switch (p_param) {
		case PARAM_MAX_VELOCITY_FORCE:
			return max_velocity_force;
		case PARAM_MAX_VELOCITY_TORQUE:
			return max_velocity_torque;
		case PARAM_LINEAR_HERTZ:
			return linear_hertz;
		case PARAM_LINEAR_DAMPING:
			return linear_damping;
		case PARAM_MAX_SPRING_FORCE:
			return max_spring_force;
		case PARAM_ANGULAR_HERTZ:
			return angular_hertz;
		case PARAM_ANGULAR_DAMPING:
			return angular_damping;
		case PARAM_MAX_SPRING_TORQUE:
			return max_spring_torque;
		default:
			return 0.0;
	}
}

void Box3DMotorJointImpl3D::set_param(Param p_param, real_t p_value) {
	switch (p_param) {
		case PARAM_MAX_VELOCITY_FORCE:
			max_velocity_force = p_value;
			if (has_joint_id()) {
				b3MotorJoint_SetMaxVelocityForce(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_MAX_VELOCITY_TORQUE:
			max_velocity_torque = p_value;
			if (has_joint_id()) {
				b3MotorJoint_SetMaxVelocityTorque(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_LINEAR_HERTZ:
			linear_hertz = p_value;
			if (has_joint_id()) {
				b3MotorJoint_SetLinearHertz(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_LINEAR_DAMPING:
			linear_damping = p_value;
			if (has_joint_id()) {
				b3MotorJoint_SetLinearDampingRatio(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_MAX_SPRING_FORCE:
			max_spring_force = p_value;
			if (has_joint_id()) {
				b3MotorJoint_SetMaxSpringForce(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_ANGULAR_HERTZ:
			angular_hertz = p_value;
			if (has_joint_id()) {
				b3MotorJoint_SetAngularHertz(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_ANGULAR_DAMPING:
			angular_damping = p_value;
			if (has_joint_id()) {
				b3MotorJoint_SetAngularDampingRatio(get_joint_id(), (float)p_value);
			}
			break;
		case PARAM_MAX_SPRING_TORQUE:
			max_spring_torque = p_value;
			if (has_joint_id()) {
				b3MotorJoint_SetMaxSpringTorque(get_joint_id(), (float)p_value);
			}
			break;
		default:
			break;
	}
}

void Box3DMotorJointImpl3D::set_linear_velocity(const Vector3& p_velocity) {
	linear_velocity = p_velocity;
	if (has_joint_id()) {
		b3MotorJoint_SetLinearVelocity(get_joint_id(), godot_to_b3(linear_velocity));
	}
}

void Box3DMotorJointImpl3D::set_angular_velocity(const Vector3& p_velocity) {
	angular_velocity = p_velocity;
	if (has_joint_id()) {
		b3MotorJoint_SetAngularVelocity(get_joint_id(), godot_to_b3(angular_velocity));
	}
}
