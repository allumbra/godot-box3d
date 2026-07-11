#include "box3d_distance_joint_impl_3d.hpp"

#include <box3d/box3d.h>

Box3DDistanceJointImpl3D::Box3DDistanceJointImpl3D(
		Box3DBodyImpl3D* p_body_a,
		Box3DBodyImpl3D* p_body_b,
		const Transform3D& p_local_frame_a,
		const Transform3D& p_local_frame_b) :
		Box3DJointImpl3D(p_body_a, p_body_b, p_local_frame_a, p_local_frame_b) {
}

b3JointId Box3DDistanceJointImpl3D::_create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) {
	b3DistanceJointDef def = b3DefaultDistanceJointDef();
	def.base.bodyIdA = p_body_a;
	def.base.bodyIdB = p_body_b;
	def.base.localFrameA = p_local_frame_a;
	def.base.localFrameB = p_local_frame_b;
	def.length = (float)length;
	def.enableSpring = enable_spring;
	def.lowerSpringForce = (float)lower_spring_force;
	def.upperSpringForce = (float)upper_spring_force;
	def.hertz = (float)hertz;
	def.dampingRatio = (float)damping;
	def.enableLimit = enable_limit;
	def.minLength = (float)min_length;
	def.maxLength = (float)max_length;
	def.enableMotor = enable_motor;
	def.maxMotorForce = (float)max_motor_force;
	def.motorSpeed = (float)motor_speed;

	return b3CreateDistanceJoint(p_world_id, &def);
}

real_t Box3DDistanceJointImpl3D::get_param(Param p_param) const {
	switch (p_param) {
		case PARAM_LENGTH:
			return length;
		case PARAM_LOWER_SPRING_FORCE:
			return lower_spring_force;
		case PARAM_UPPER_SPRING_FORCE:
			return upper_spring_force;
		case PARAM_HERTZ:
			return hertz;
		case PARAM_DAMPING:
			return damping;
		case PARAM_MIN_LENGTH:
			return min_length;
		case PARAM_MAX_LENGTH:
			return max_length;
		case PARAM_MOTOR_SPEED:
			return motor_speed;
		case PARAM_MAX_MOTOR_FORCE:
			return max_motor_force;
		default:
			return 0.0;
	}
}

void Box3DDistanceJointImpl3D::set_param(Param p_param, real_t p_value) {
	switch (p_param) {
		case PARAM_LENGTH:
			length = p_value;
			if (has_joint_id()) {
				b3DistanceJoint_SetLength(get_joint_id(), (float)length);
			}
			break;
		case PARAM_LOWER_SPRING_FORCE:
		case PARAM_UPPER_SPRING_FORCE:
			if (p_param == PARAM_LOWER_SPRING_FORCE) {
				lower_spring_force = p_value;
			} else {
				upper_spring_force = p_value;
			}
			if (has_joint_id()) {
				b3DistanceJoint_SetSpringForceRange(get_joint_id(), (float)lower_spring_force, (float)upper_spring_force);
			}
			break;
		case PARAM_HERTZ:
			hertz = p_value;
			if (has_joint_id()) {
				b3DistanceJoint_SetSpringHertz(get_joint_id(), (float)hertz);
			}
			break;
		case PARAM_DAMPING:
			damping = p_value;
			if (has_joint_id()) {
				b3DistanceJoint_SetSpringDampingRatio(get_joint_id(), (float)damping);
			}
			break;
		case PARAM_MIN_LENGTH:
		case PARAM_MAX_LENGTH:
			if (p_param == PARAM_MIN_LENGTH) {
				min_length = p_value;
			} else {
				max_length = p_value;
			}
			if (has_joint_id()) {
				b3DistanceJoint_SetLengthRange(get_joint_id(), (float)min_length, (float)max_length);
			}
			break;
		case PARAM_MOTOR_SPEED:
			motor_speed = p_value;
			if (has_joint_id()) {
				b3DistanceJoint_SetMotorSpeed(get_joint_id(), (float)motor_speed);
			}
			break;
		case PARAM_MAX_MOTOR_FORCE:
			max_motor_force = p_value;
			if (has_joint_id()) {
				b3DistanceJoint_SetMaxMotorForce(get_joint_id(), (float)max_motor_force);
			}
			break;
		default:
			break;
	}
}

bool Box3DDistanceJointImpl3D::get_flag(Flag p_flag) const {
	switch (p_flag) {
		case FLAG_ENABLE_SPRING:
			return enable_spring;
		case FLAG_ENABLE_LIMIT:
			return enable_limit;
		case FLAG_ENABLE_MOTOR:
			return enable_motor;
		default:
			return false;
	}
}

void Box3DDistanceJointImpl3D::set_flag(Flag p_flag, bool p_enabled) {
	switch (p_flag) {
		case FLAG_ENABLE_SPRING:
			enable_spring = p_enabled;
			if (has_joint_id()) {
				b3DistanceJoint_EnableSpring(get_joint_id(), enable_spring);
			}
			break;
		case FLAG_ENABLE_LIMIT:
			enable_limit = p_enabled;
			if (has_joint_id()) {
				b3DistanceJoint_EnableLimit(get_joint_id(), enable_limit);
			}
			break;
		case FLAG_ENABLE_MOTOR:
			enable_motor = p_enabled;
			if (has_joint_id()) {
				b3DistanceJoint_EnableMotor(get_joint_id(), enable_motor);
			}
			break;
		default:
			break;
	}
}

real_t Box3DDistanceJointImpl3D::get_current_length() const {
	return has_joint_id() ? (real_t)b3DistanceJoint_GetCurrentLength(get_joint_id()) : 0.0;
}

real_t Box3DDistanceJointImpl3D::get_motor_force() const {
	return has_joint_id() ? (real_t)b3DistanceJoint_GetMotorForce(get_joint_id()) : 0.0;
}
