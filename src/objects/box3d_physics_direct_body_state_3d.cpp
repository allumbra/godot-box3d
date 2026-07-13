#include "box3d_physics_direct_body_state_3d.hpp"

#include "../misc/type_conversions.hpp"
#include "../shapes/box3d_shape_instance_3d.hpp"
#include "../spaces/box3d_physics_direct_space_state_3d.hpp"
#include "../spaces/box3d_space_3d.hpp"
#include "box3d_body_impl_3d.hpp"
#include "box3d_shaped_object_impl_3d.hpp"

#include <godot_cpp/core/object.hpp>

#include <box3d/box3d.h>

Vector3 Box3DPhysicsDirectBodyState3D::_get_total_gravity() const {
	if (body->get_space() != nullptr) {
		return b3_to_godot(b3World_GetGravity(body->get_space()->get_world_id())) * (float)body->get_gravity_scale();
	}
	return Vector3();
}

double Box3DPhysicsDirectBodyState3D::_get_total_angular_damp() const {
	return body->get_angular_damping();
}

double Box3DPhysicsDirectBodyState3D::_get_total_linear_damp() const {
	return body->get_linear_damping();
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_center_of_mass() const {
	return body->get_transform().xform(body->get_center_of_mass());
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_center_of_mass_local() const {
	return body->get_center_of_mass();
}

Basis Box3DPhysicsDirectBodyState3D::_get_principal_inertia_axes() const {
	return Basis();
}

double Box3DPhysicsDirectBodyState3D::_get_inverse_mass() const {
	const real_t mass = body->get_mass();
	return mass > 0.0 ? 1.0 / mass : 0.0;
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_inverse_inertia() const {
	const Vector3 inertia = body->get_inertia();
	return Vector3(
			inertia.x > 0.0 ? 1.0 / inertia.x : 0.0,
			inertia.y > 0.0 ? 1.0 / inertia.y : 0.0,
			inertia.z > 0.0 ? 1.0 / inertia.z : 0.0);
}

Basis Box3DPhysicsDirectBodyState3D::_get_inverse_inertia_tensor() const {
	const Vector3 inv_inertia = _get_inverse_inertia();
	Basis basis;
	basis.set_column(0, Vector3(inv_inertia.x, 0, 0));
	basis.set_column(1, Vector3(0, inv_inertia.y, 0));
	basis.set_column(2, Vector3(0, 0, inv_inertia.z));
	return basis;
}

void Box3DPhysicsDirectBodyState3D::_set_linear_velocity(const Vector3& p_velocity) {
	body->set_linear_velocity(p_velocity);
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_linear_velocity() const {
	return body->get_linear_velocity();
}

void Box3DPhysicsDirectBodyState3D::_set_angular_velocity(const Vector3& p_velocity) {
	body->set_angular_velocity(p_velocity);
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_angular_velocity() const {
	return body->get_angular_velocity();
}

void Box3DPhysicsDirectBodyState3D::_set_transform(const Transform3D& p_transform) {
	body->set_transform(p_transform);
}

Transform3D Box3DPhysicsDirectBodyState3D::_get_transform() const {
	return body->get_transform();
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_velocity_at_local_position(const Vector3& p_local_position) const {
	if (!body->has_body_id()) {
		return Vector3();
	}
	const Vector3 world_point = body->get_transform().xform(p_local_position);
	return b3_to_godot(b3Body_GetWorldPointVelocity(body->get_body_id(), godot_to_b3(world_point)));
}

void Box3DPhysicsDirectBodyState3D::_apply_central_impulse(const Vector3& p_impulse) {
	body->apply_central_impulse(p_impulse);
}

void Box3DPhysicsDirectBodyState3D::_apply_impulse(const Vector3& p_impulse, const Vector3& p_position) {
	body->apply_impulse(p_impulse, p_position);
}

void Box3DPhysicsDirectBodyState3D::_apply_torque_impulse(const Vector3& p_impulse) {
	body->apply_torque_impulse(p_impulse);
}

void Box3DPhysicsDirectBodyState3D::_apply_central_force(const Vector3& p_force) {
	body->apply_central_force(p_force);
}

void Box3DPhysicsDirectBodyState3D::_apply_force(const Vector3& p_force, const Vector3& p_position) {
	body->apply_force(p_force, p_position);
}

void Box3DPhysicsDirectBodyState3D::_apply_torque(const Vector3& p_torque) {
	body->apply_torque(p_torque);
}

void Box3DPhysicsDirectBodyState3D::_add_constant_central_force(const Vector3& p_force) {
	body->add_constant_central_force(p_force);
}

void Box3DPhysicsDirectBodyState3D::_add_constant_force(const Vector3& p_force, const Vector3& p_position) {
	body->add_constant_force(p_force, p_position);
}

void Box3DPhysicsDirectBodyState3D::_add_constant_torque(const Vector3& p_torque) {
	body->add_constant_torque(p_torque);
}

void Box3DPhysicsDirectBodyState3D::_set_constant_force(const Vector3& p_force) {
	body->set_constant_force(p_force);
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_constant_force() const {
	return body->get_constant_force();
}

void Box3DPhysicsDirectBodyState3D::_set_constant_torque(const Vector3& p_torque) {
	body->set_constant_torque(p_torque);
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_constant_torque() const {
	return body->get_constant_torque();
}

void Box3DPhysicsDirectBodyState3D::_set_sleep_state(bool p_enabled) {
	body->set_sleeping(p_enabled);
}

bool Box3DPhysicsDirectBodyState3D::_is_sleeping() const {
	return body->is_sleeping();
}

void Box3DPhysicsDirectBodyState3D::_refresh_contacts() const {
	contacts.clear();
	if (!body->has_body_id()) {
		return;
	}
	const b3BodyId our_id = body->get_body_id();
	const int capacity = b3Body_GetContactCapacity(our_id);
	if (capacity <= 0) {
		return;
	}

	LocalVector<b3ContactData> data;
	data.resize(capacity);
	const int count = b3Body_GetContactData(our_id, data.ptr(), capacity);

	const Vector3 our_com = b3_to_godot(b3Body_GetWorldCenterOfMass(our_id));
	const Vector3 our_lin = b3_to_godot(b3Body_GetLinearVelocity(our_id));
	const Vector3 our_ang = b3_to_godot(b3Body_GetAngularVelocity(our_id));

	for (int c = 0; c < count; c++) {
		const b3ContactData& contact = data[c];
		const b3BodyId body_a = b3Shape_GetBody(contact.shapeIdA);
		const bool we_are_a = B3_ID_EQUALS(body_a, our_id);
		const b3ShapeId our_shape = we_are_a ? contact.shapeIdA : contact.shapeIdB;
		const b3ShapeId other_shape = we_are_a ? contact.shapeIdB : contact.shapeIdA;
		const b3BodyId other_id = b3Shape_GetBody(other_shape);

		auto* other_object = static_cast<Box3DShapedObjectImpl3D*>(b3Body_GetUserData(other_id));
		if (other_object == nullptr) {
			continue;
		}
		const Vector3 other_com = b3_to_godot(b3Body_GetWorldCenterOfMass(other_id));
		const Vector3 other_lin = b3_to_godot(b3Body_GetLinearVelocity(other_id));
		const Vector3 other_ang = b3_to_godot(b3Body_GetAngularVelocity(other_id));

		auto* our_instance = static_cast<Box3DShapeInstance3D*>(b3Shape_GetUserData(our_shape));
		auto* other_instance = static_cast<Box3DShapeInstance3D*>(b3Shape_GetUserData(other_shape));

		for (int m = 0; m < contact.manifoldCount; m++) {
			const b3Manifold& manifold = contact.manifolds[m];
			// Manifold normal points from shape A to shape B; Godot's contact normal
			// points toward this body.
			const Vector3 normal_toward_us = b3_to_godot(manifold.normal) * (we_are_a ? -1.0f : 1.0f);
			for (int p = 0; p < manifold.pointCount; p++) {
				const b3ManifoldPoint& point = manifold.points[p];
				// Skip speculative points that never interacted this step.
				if (point.totalNormalImpulse <= 0.0f && point.separation > 0.0f) {
					continue;
				}
				const Vector3 our_anchor = b3_to_godot(we_are_a ? point.anchorA : point.anchorB);
				const Vector3 other_anchor = b3_to_godot(we_are_a ? point.anchorB : point.anchorA);

				CachedContact cached;
				cached.position = our_com + our_anchor;
				cached.normal = normal_toward_us;
				cached.impulse = normal_toward_us * point.totalNormalImpulse;
				cached.local_velocity = our_lin + our_ang.cross(our_anchor);
				cached.collider_position = other_com + other_anchor;
				cached.collider_velocity = other_lin + other_ang.cross(other_anchor);
				cached.collider_rid = other_object->get_rid();
				cached.collider_instance_id = other_object->get_instance_id();
				cached.local_shape = our_instance != nullptr ? (int32_t)our_instance->get_index() : 0;
				cached.collider_shape = other_instance != nullptr ? (int32_t)other_instance->get_index() : 0;
				contacts.push_back(cached);
			}
		}
	}
}

int32_t Box3DPhysicsDirectBodyState3D::_get_contact_count() const {
	_refresh_contacts();
	return (int32_t)contacts.size();
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_contact_local_position(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), Vector3());
	return contacts[p_index].position;
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_contact_local_normal(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), Vector3());
	return contacts[p_index].normal;
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_contact_impulse(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), Vector3());
	return contacts[p_index].impulse;
}

int32_t Box3DPhysicsDirectBodyState3D::_get_contact_local_shape(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), 0);
	return contacts[p_index].local_shape;
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_contact_local_velocity_at_position(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), Vector3());
	return contacts[p_index].local_velocity;
}

RID Box3DPhysicsDirectBodyState3D::_get_contact_collider(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), RID());
	return contacts[p_index].collider_rid;
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_contact_collider_position(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), Vector3());
	return contacts[p_index].collider_position;
}

uint64_t Box3DPhysicsDirectBodyState3D::_get_contact_collider_id(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), 0);
	return contacts[p_index].collider_instance_id;
}

Object* Box3DPhysicsDirectBodyState3D::_get_contact_collider_object(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), nullptr);
	return ObjectDB::get_instance(ObjectID(contacts[p_index].collider_instance_id));
}

int32_t Box3DPhysicsDirectBodyState3D::_get_contact_collider_shape(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), 0);
	return contacts[p_index].collider_shape;
}

Vector3 Box3DPhysicsDirectBodyState3D::_get_contact_collider_velocity_at_position(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, (int32_t)contacts.size(), Vector3());
	return contacts[p_index].collider_velocity;
}

double Box3DPhysicsDirectBodyState3D::_get_step() const {
	if (body->get_space() != nullptr) {
		return body->get_space()->get_last_step();
	}
	return 0.0;
}

void Box3DPhysicsDirectBodyState3D::_integrate_forces() {
	// Default implementation: PhysicsServer3DExtension calls the script-side
	// _integrate_forces() separately; nothing additional needed here for v1.
}

PhysicsDirectSpaceState3D* Box3DPhysicsDirectBodyState3D::_get_space_state() {
	if (body->get_space() != nullptr) {
		return body->get_space()->get_direct_state();
	}
	return nullptr;
}
