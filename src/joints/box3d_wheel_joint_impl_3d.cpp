#include "box3d_wheel_joint_impl_3d.hpp"

#include <box3d/box3d.h>

Box3DWheelJointImpl3D::Box3DWheelJointImpl3D(
		Box3DBodyImpl3D* p_body_a,
		Box3DBodyImpl3D* p_body_b,
		const Transform3D& p_local_frame_a,
		const Transform3D& p_local_frame_b) :
		Box3DJointImpl3D(p_body_a, p_body_b, p_local_frame_a, p_local_frame_b) {
}

b3JointId Box3DWheelJointImpl3D::_create_joint_id(b3WorldId p_world_id, b3BodyId p_body_a, b3BodyId p_body_b, b3Transform p_local_frame_a, b3Transform p_local_frame_b) {
	b3WheelJointDef def = b3DefaultWheelJointDef();
	def.base.bodyIdA = p_body_a;
	def.base.bodyIdB = p_body_b;
	def.base.localFrameA = p_local_frame_a;
	def.base.localFrameB = p_local_frame_b;
	def.enableSuspensionSpring = enable_suspension;
	def.suspensionHertz = (float)suspension_hertz;
	def.suspensionDampingRatio = (float)suspension_damping;
	def.enableSuspensionLimit = enable_suspension_limit;
	def.lowerSuspensionLimit = (float)suspension_lower;
	def.upperSuspensionLimit = (float)suspension_upper;
	def.enableSpinMotor = enable_spin_motor;
	def.maxSpinTorque = (float)max_spin_torque;
	def.spinSpeed = (float)spin_motor_speed;
	def.enableSteering = enable_steering;
	def.steeringHertz = (float)steering_hertz;
	def.steeringDampingRatio = (float)steering_damping;
	def.targetSteeringAngle = (float)target_steering_angle;
	def.maxSteeringTorque = (float)max_steering_torque;
	def.enableSteeringLimit = enable_steering_limit;
	def.lowerSteeringLimit = (float)steering_lower;
	def.upperSteeringLimit = (float)steering_upper;

	return b3CreateWheelJoint(p_world_id, &def);
}

real_t Box3DWheelJointImpl3D::get_param(Param p_param) const {
	switch (p_param) {
		case PARAM_SUSPENSION_HERTZ:
			return suspension_hertz;
		case PARAM_SUSPENSION_DAMPING:
			return suspension_damping;
		case PARAM_SUSPENSION_LOWER:
			return suspension_lower;
		case PARAM_SUSPENSION_UPPER:
			return suspension_upper;
		case PARAM_SPIN_MOTOR_SPEED:
			return spin_motor_speed;
		case PARAM_MAX_SPIN_TORQUE:
			return max_spin_torque;
		case PARAM_STEERING_HERTZ:
			return steering_hertz;
		case PARAM_STEERING_DAMPING:
			return steering_damping;
		case PARAM_TARGET_STEERING_ANGLE:
			return target_steering_angle;
		case PARAM_MAX_STEERING_TORQUE:
			return max_steering_torque;
		case PARAM_STEERING_LOWER:
			return steering_lower;
		case PARAM_STEERING_UPPER:
			return steering_upper;
		default:
			return 0.0;
	}
}

void Box3DWheelJointImpl3D::set_param(Param p_param, real_t p_value) {
	switch (p_param) {
		case PARAM_SUSPENSION_HERTZ:
			suspension_hertz = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSuspensionHertz(get_joint_id(), (float)suspension_hertz);
			}
			break;
		case PARAM_SUSPENSION_DAMPING:
			suspension_damping = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSuspensionDampingRatio(get_joint_id(), (float)suspension_damping);
			}
			break;
		case PARAM_SUSPENSION_LOWER:
			suspension_lower = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSuspensionLimits(get_joint_id(), (float)suspension_lower, (float)suspension_upper);
			}
			break;
		case PARAM_SUSPENSION_UPPER:
			suspension_upper = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSuspensionLimits(get_joint_id(), (float)suspension_lower, (float)suspension_upper);
			}
			break;
		case PARAM_SPIN_MOTOR_SPEED:
			spin_motor_speed = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSpinMotorSpeed(get_joint_id(), (float)spin_motor_speed);
			}
			break;
		case PARAM_MAX_SPIN_TORQUE:
			max_spin_torque = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetMaxSpinTorque(get_joint_id(), (float)max_spin_torque);
			}
			break;
		case PARAM_STEERING_HERTZ:
			steering_hertz = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSteeringHertz(get_joint_id(), (float)steering_hertz);
			}
			break;
		case PARAM_STEERING_DAMPING:
			steering_damping = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSteeringDampingRatio(get_joint_id(), (float)steering_damping);
			}
			break;
		case PARAM_TARGET_STEERING_ANGLE:
			target_steering_angle = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetTargetSteeringAngle(get_joint_id(), (float)target_steering_angle);
			}
			break;
		case PARAM_MAX_STEERING_TORQUE:
			max_steering_torque = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetMaxSteeringTorque(get_joint_id(), (float)max_steering_torque);
			}
			break;
		case PARAM_STEERING_LOWER:
			steering_lower = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSteeringLimits(get_joint_id(), (float)steering_lower, (float)steering_upper);
			}
			break;
		case PARAM_STEERING_UPPER:
			steering_upper = p_value;
			if (has_joint_id()) {
				b3WheelJoint_SetSteeringLimits(get_joint_id(), (float)steering_lower, (float)steering_upper);
			}
			break;
		default:
			break;
	}
}

bool Box3DWheelJointImpl3D::get_flag(Flag p_flag) const {
	switch (p_flag) {
		case FLAG_ENABLE_SUSPENSION:
			return enable_suspension;
		case FLAG_ENABLE_SUSPENSION_LIMIT:
			return enable_suspension_limit;
		case FLAG_ENABLE_SPIN_MOTOR:
			return enable_spin_motor;
		case FLAG_ENABLE_STEERING:
			return enable_steering;
		case FLAG_ENABLE_STEERING_LIMIT:
			return enable_steering_limit;
		default:
			return false;
	}
}

void Box3DWheelJointImpl3D::set_flag(Flag p_flag, bool p_enabled) {
	switch (p_flag) {
		case FLAG_ENABLE_SUSPENSION:
			enable_suspension = p_enabled;
			if (has_joint_id()) {
				b3WheelJoint_EnableSuspension(get_joint_id(), enable_suspension);
			}
			break;
		case FLAG_ENABLE_SUSPENSION_LIMIT:
			enable_suspension_limit = p_enabled;
			if (has_joint_id()) {
				b3WheelJoint_EnableSuspensionLimit(get_joint_id(), enable_suspension_limit);
			}
			break;
		case FLAG_ENABLE_SPIN_MOTOR:
			enable_spin_motor = p_enabled;
			if (has_joint_id()) {
				b3WheelJoint_EnableSpinMotor(get_joint_id(), enable_spin_motor);
			}
			break;
		case FLAG_ENABLE_STEERING:
			enable_steering = p_enabled;
			if (has_joint_id()) {
				b3WheelJoint_EnableSteering(get_joint_id(), enable_steering);
			}
			break;
		case FLAG_ENABLE_STEERING_LIMIT:
			enable_steering_limit = p_enabled;
			if (has_joint_id()) {
				b3WheelJoint_EnableSteeringLimit(get_joint_id(), enable_steering_limit);
			}
			break;
		default:
			break;
	}
}

real_t Box3DWheelJointImpl3D::get_spin_speed() const {
	return has_joint_id() ? (real_t)b3WheelJoint_GetSpinSpeed(get_joint_id()) : 0.0;
}

real_t Box3DWheelJointImpl3D::get_spin_torque() const {
	return has_joint_id() ? (real_t)b3WheelJoint_GetSpinTorque(get_joint_id()) : 0.0;
}

real_t Box3DWheelJointImpl3D::get_steering_angle() const {
	return has_joint_id() ? (real_t)b3WheelJoint_GetSteeringAngle(get_joint_id()) : 0.0;
}

real_t Box3DWheelJointImpl3D::get_steering_torque() const {
	return has_joint_id() ? (real_t)b3WheelJoint_GetSteeringTorque(get_joint_id()) : 0.0;
}
