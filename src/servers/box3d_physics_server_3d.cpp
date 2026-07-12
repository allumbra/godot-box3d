#include "box3d_physics_server_3d.hpp"

#include "../joints/box3d_hinge_joint_impl_3d.hpp"
#include "../joints/box3d_joint_impl_3d.hpp"
#include "../joints/box3d_pin_joint_impl_3d.hpp"
#include "../joints/box3d_distance_joint_impl_3d.hpp"
#include "../joints/box3d_motor_joint_impl_3d.hpp"
#include "../joints/box3d_slider_joint_impl_3d.hpp"
#include "../joints/box3d_weld_joint_impl_3d.hpp"
#include "../joints/box3d_wheel_joint_impl_3d.hpp"
#include "../misc/type_conversions.hpp"
#include "../objects/box3d_area_impl_3d.hpp"
#include "../objects/box3d_body_impl_3d.hpp"
#include "../objects/box3d_physics_direct_body_state_3d.hpp"
#include "../objects/box3d_shaped_object_impl_3d.hpp"
#include "../shapes/box3d_box_shape_impl_3d.hpp"
#include "../shapes/box3d_capsule_shape_impl_3d.hpp"
#include "../shapes/box3d_concave_polygon_shape_impl_3d.hpp"
#include "../shapes/box3d_convex_polygon_shape_impl_3d.hpp"
#include "../shapes/box3d_heightmap_shape_impl_3d.hpp"
#include "../shapes/box3d_shape_impl_3d.hpp"
#include "../shapes/box3d_sphere_shape_impl_3d.hpp"
#include "../shapes/box3d_world_boundary_shape_impl_3d.hpp"
#include "../spaces/box3d_physics_direct_space_state_3d.hpp"
#include "../spaces/box3d_space_3d.hpp"

#include <box3d/box3d.h>
#include <box3d/collision.h>

#include <godot_cpp/templates/local_vector.hpp>

Box3DPhysicsServer3D* Box3DPhysicsServer3D::singleton = nullptr;

Box3DPhysicsServer3D::Box3DPhysicsServer3D() {
	singleton = this;
}

Box3DPhysicsServer3D::~Box3DPhysicsServer3D() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

Box3DShapedObjectImpl3D* Box3DPhysicsServer3D::_get_shaped_object(const RID& p_rid) const {
	if (Box3DBodyImpl3D* body = body_owner.get_or_null(p_rid)) {
		return body;
	}
	if (Box3DAreaImpl3D* area = area_owner.get_or_null(p_rid)) {
		return area;
	}
	return nullptr;
}

// --- Shapes ---

RID Box3DPhysicsServer3D::_world_boundary_shape_create() {
	auto* shape = memnew(Box3DWorldBoundaryShapeImpl3D);
	const RID rid = shape_owner.make_rid(shape);
	shape->set_rid(rid);
	return rid;
}

RID Box3DPhysicsServer3D::_separation_ray_shape_create() {
	ERR_FAIL_V_MSG(RID(), "Box3D: SeparationRayShape3D is not supported in this version of the Box3D extension.");
}

RID Box3DPhysicsServer3D::_sphere_shape_create() {
	auto* shape = memnew(Box3DSphereShapeImpl3D);
	const RID rid = shape_owner.make_rid(shape);
	shape->set_rid(rid);
	return rid;
}

RID Box3DPhysicsServer3D::_box_shape_create() {
	auto* shape = memnew(Box3DBoxShapeImpl3D);
	const RID rid = shape_owner.make_rid(shape);
	shape->set_rid(rid);
	return rid;
}

RID Box3DPhysicsServer3D::_capsule_shape_create() {
	auto* shape = memnew(Box3DCapsuleShapeImpl3D);
	const RID rid = shape_owner.make_rid(shape);
	shape->set_rid(rid);
	return rid;
}

RID Box3DPhysicsServer3D::_cylinder_shape_create() {
	ERR_FAIL_V_MSG(RID(), "Box3D: CylinderShape3D is not supported in this version of the Box3D extension.");
}

RID Box3DPhysicsServer3D::_convex_polygon_shape_create() {
	auto* shape = memnew(Box3DConvexPolygonShapeImpl3D);
	const RID rid = shape_owner.make_rid(shape);
	shape->set_rid(rid);
	return rid;
}

RID Box3DPhysicsServer3D::_concave_polygon_shape_create() {
	auto* shape = memnew(Box3DConcavePolygonShapeImpl3D);
	const RID rid = shape_owner.make_rid(shape);
	shape->set_rid(rid);
	return rid;
}

RID Box3DPhysicsServer3D::_heightmap_shape_create() {
	auto* shape = memnew(Box3DHeightMapShapeImpl3D);
	const RID rid = shape_owner.make_rid(shape);
	shape->set_rid(rid);
	return rid;
}

RID Box3DPhysicsServer3D::_custom_shape_create() {
	ERR_FAIL_V_MSG(RID(), "Box3D: custom shapes are not supported.");
}

void Box3DPhysicsServer3D::_shape_set_data(const RID& p_shape, const Variant& p_data) {
	Box3DShapeImpl3D* shape = shape_owner.get_or_null(p_shape);
	ERR_FAIL_NULL(shape);
	shape->set_data(p_data);
}

void Box3DPhysicsServer3D::_shape_set_custom_solver_bias(const RID& p_shape, double p_bias) {
	// No Box3D equivalent.
}

void Box3DPhysicsServer3D::_shape_set_margin(const RID& p_shape, double p_margin) {
	// No Box3D equivalent (Box3D shapes have no configurable collision margin).
}

double Box3DPhysicsServer3D::_shape_get_margin(const RID& p_shape) const {
	return 0.0;
}

PhysicsServer3D::ShapeType Box3DPhysicsServer3D::_shape_get_type(const RID& p_shape) const {
	Box3DShapeImpl3D* shape = shape_owner.get_or_null(p_shape);
	ERR_FAIL_NULL_V(shape, PhysicsServer3D::SHAPE_CUSTOM);
	return shape->get_type();
}

Variant Box3DPhysicsServer3D::_shape_get_data(const RID& p_shape) const {
	Box3DShapeImpl3D* shape = shape_owner.get_or_null(p_shape);
	ERR_FAIL_NULL_V(shape, Variant());
	return shape->get_data();
}

double Box3DPhysicsServer3D::_shape_get_custom_solver_bias(const RID& p_shape) const {
	return 0.0;
}

// --- Space ---

RID Box3DPhysicsServer3D::_space_create() {
	auto* space = memnew(Box3DSpace3D);
	const RID rid = space_owner.make_rid(space);
	space->set_rid(rid);

	const RID default_area_rid = _area_create();
	Box3DAreaImpl3D* default_area = area_owner.get_or_null(default_area_rid);
	ERR_FAIL_NULL_V(default_area, rid);
	default_area->set_default_area(true);
	default_area->set_space(space);
	space->set_default_area(default_area);

	return rid;
}

void Box3DPhysicsServer3D::_space_set_active(const RID& p_space, bool p_active) {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL(space);
	space->set_active(p_active);
	if (p_active) {
		active_spaces.insert(space);
	} else {
		active_spaces.erase(space);
	}
}

bool Box3DPhysicsServer3D::_space_is_active(const RID& p_space) const {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, false);
	return space->is_active();
}

void Box3DPhysicsServer3D::_space_set_param(const RID& p_space, PhysicsServer3D::SpaceParameter p_param, double p_value) {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL(space);
	space->set_param(p_param, p_value);
}

double Box3DPhysicsServer3D::_space_get_param(const RID& p_space, PhysicsServer3D::SpaceParameter p_param) const {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, 0.0);
	return space->get_param(p_param);
}

PhysicsDirectSpaceState3D* Box3DPhysicsServer3D::_space_get_direct_state(const RID& p_space) {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, nullptr);
	return space->get_direct_state();
}

void Box3DPhysicsServer3D::_space_set_debug_contacts(const RID& p_space, int32_t p_max_contacts) {
	// Not implemented in v1 (no debug draw).
}

PackedVector3Array Box3DPhysicsServer3D::_space_get_contacts(const RID& p_space) const {
	return PackedVector3Array();
}

int32_t Box3DPhysicsServer3D::_space_get_contact_count(const RID& p_space) const {
	return 0;
}

// --- Areas ---

RID Box3DPhysicsServer3D::_area_create() {
	auto* area = memnew(Box3DAreaImpl3D);
	const RID rid = area_owner.make_rid(area);
	area->set_rid(rid);
	return rid;
}

void Box3DPhysicsServer3D::_area_set_space(const RID& p_area, const RID& p_space) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	Box3DSpace3D* old_space = area->get_space();
	if (old_space != nullptr) {
		old_space->unregister_area(area);
	}
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	area->set_space(space);
	if (space != nullptr) {
		space->register_area(area);
	}
}

RID Box3DPhysicsServer3D::_area_get_space(const RID& p_area) const {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL_V(area, RID());
	Box3DSpace3D* space = area->get_space();
	return space != nullptr ? space->get_rid() : RID();
}

void Box3DPhysicsServer3D::_area_add_shape(const RID& p_area, const RID& p_shape, const Transform3D& p_transform, bool p_disabled) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	Box3DShapeImpl3D* shape = shape_owner.get_or_null(p_shape);
	ERR_FAIL_NULL(shape);
	shape->add_owner(area);
	area->add_shape(shape, p_transform, p_disabled);
}

void Box3DPhysicsServer3D::_area_set_shape(const RID& p_area, int32_t p_shape_idx, const RID& p_shape) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	Box3DShapeImpl3D* shape = shape_owner.get_or_null(p_shape);
	ERR_FAIL_NULL(shape);
	shape->add_owner(area);
	area->set_shape(p_shape_idx, shape);
}

void Box3DPhysicsServer3D::_area_set_shape_transform(const RID& p_area, int32_t p_shape_idx, const Transform3D& p_transform) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_shape_transform(p_shape_idx, p_transform);
}

void Box3DPhysicsServer3D::_area_set_shape_disabled(const RID& p_area, int32_t p_shape_idx, bool p_disabled) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_shape_disabled(p_shape_idx, p_disabled);
}

int32_t Box3DPhysicsServer3D::_area_get_shape_count(const RID& p_area) const {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL_V(area, 0);
	return area->get_shape_count();
}

RID Box3DPhysicsServer3D::_area_get_shape(const RID& p_area, int32_t p_shape_idx) const {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL_V(area, RID());
	Box3DShapeImpl3D* shape = area->get_shape(p_shape_idx);
	return shape != nullptr ? shape->get_rid() : RID();
}

Transform3D Box3DPhysicsServer3D::_area_get_shape_transform(const RID& p_area, int32_t p_shape_idx) const {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL_V(area, Transform3D());
	return area->get_shape_transform(p_shape_idx);
}

void Box3DPhysicsServer3D::_area_remove_shape(const RID& p_area, int32_t p_shape_idx) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->remove_shape(p_shape_idx);
}

void Box3DPhysicsServer3D::_area_clear_shapes(const RID& p_area) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->clear_shapes();
}

void Box3DPhysicsServer3D::_area_attach_object_instance_id(const RID& p_area, uint64_t p_id) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_instance_id(p_id);
}

uint64_t Box3DPhysicsServer3D::_area_get_object_instance_id(const RID& p_area) const {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL_V(area, 0);
	return area->get_instance_id();
}

void Box3DPhysicsServer3D::_area_set_param(const RID& p_area, PhysicsServer3D::AreaParameter p_param, const Variant& p_value) {
	// Godot core addresses a space's default area by the SPACE RID (e.g. World3D
	// pushing project gravity settings) — mirror the built-in servers' translation.
	Box3DAreaImpl3D* area = nullptr;
	if (Box3DSpace3D* space = space_owner.get_or_null(p_area)) {
		area = space->get_default_area();
	} else {
		area = area_owner.get_or_null(p_area);
	}
	ERR_FAIL_NULL(area);
	area->set_param(p_param, p_value);
}

void Box3DPhysicsServer3D::_area_set_transform(const RID& p_area, const Transform3D& p_transform) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_transform(p_transform);
}

Variant Box3DPhysicsServer3D::_area_get_param(const RID& p_area, PhysicsServer3D::AreaParameter p_param) const {
	// See _area_set_param: a space RID addresses that space's default area.
	Box3DAreaImpl3D* area = nullptr;
	if (Box3DSpace3D* space = space_owner.get_or_null(p_area)) {
		area = space->get_default_area();
	} else {
		area = area_owner.get_or_null(p_area);
	}
	ERR_FAIL_NULL_V(area, Variant());
	return area->get_param(p_param);
}

Transform3D Box3DPhysicsServer3D::_area_get_transform(const RID& p_area) const {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL_V(area, Transform3D());
	return area->get_transform();
}

void Box3DPhysicsServer3D::_area_set_collision_layer(const RID& p_area, uint32_t p_layer) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_collision_layer(p_layer);
}

uint32_t Box3DPhysicsServer3D::_area_get_collision_layer(const RID& p_area) const {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL_V(area, 0);
	return area->get_collision_layer();
}

void Box3DPhysicsServer3D::_area_set_collision_mask(const RID& p_area, uint32_t p_mask) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_collision_mask(p_mask);
}

uint32_t Box3DPhysicsServer3D::_area_get_collision_mask(const RID& p_area) const {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL_V(area, 0);
	return area->get_collision_mask();
}

void Box3DPhysicsServer3D::_area_set_monitorable(const RID& p_area, bool p_monitorable) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_monitorable(p_monitorable);
}

void Box3DPhysicsServer3D::_area_set_ray_pickable(const RID& p_area, bool p_enable) {
	// Not implemented in v1 (no editor picking support).
}

void Box3DPhysicsServer3D::_area_set_monitor_callback(const RID& p_area, const Callable& p_callback) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_body_monitor_callback(p_callback);
}

void Box3DPhysicsServer3D::_area_set_area_monitor_callback(const RID& p_area, const Callable& p_callback) {
	Box3DAreaImpl3D* area = area_owner.get_or_null(p_area);
	ERR_FAIL_NULL(area);
	area->set_area_monitor_callback(p_callback);
}

// --- Bodies ---

RID Box3DPhysicsServer3D::_body_create() {
	auto* body = memnew(Box3DBodyImpl3D);
	const RID rid = body_owner.make_rid(body);
	body->set_rid(rid);
	return rid;
}

void Box3DPhysicsServer3D::_body_set_space(const RID& p_body, const RID& p_space) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	Box3DSpace3D* old_space = body->get_space();
	if (old_space != nullptr) {
		old_space->unregister_body(body);
	}
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	body->set_space(space);
	if (space != nullptr) {
		space->register_body(body);
	}
}

RID Box3DPhysicsServer3D::_body_get_space(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, RID());
	Box3DSpace3D* space = body->get_space();
	return space != nullptr ? space->get_rid() : RID();
}

void Box3DPhysicsServer3D::_body_set_mode(const RID& p_body, PhysicsServer3D::BodyMode p_mode) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_mode(p_mode);
}

PhysicsServer3D::BodyMode Box3DPhysicsServer3D::_body_get_mode(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, PhysicsServer3D::BODY_MODE_STATIC);
	return body->get_mode();
}

void Box3DPhysicsServer3D::_body_add_shape(const RID& p_body, const RID& p_shape, const Transform3D& p_transform, bool p_disabled) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	Box3DShapeImpl3D* shape = shape_owner.get_or_null(p_shape);
	ERR_FAIL_NULL(shape);
	shape->add_owner(body);
	body->add_shape(shape, p_transform, p_disabled);
}

void Box3DPhysicsServer3D::_body_set_shape(const RID& p_body, int32_t p_shape_idx, const RID& p_shape) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	Box3DShapeImpl3D* shape = shape_owner.get_or_null(p_shape);
	ERR_FAIL_NULL(shape);
	shape->add_owner(body);
	body->set_shape(p_shape_idx, shape);
}

void Box3DPhysicsServer3D::_body_set_shape_transform(const RID& p_body, int32_t p_shape_idx, const Transform3D& p_transform) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_shape_transform(p_shape_idx, p_transform);
}

void Box3DPhysicsServer3D::_body_set_shape_disabled(const RID& p_body, int32_t p_shape_idx, bool p_disabled) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_shape_disabled(p_shape_idx, p_disabled);
}

int32_t Box3DPhysicsServer3D::_body_get_shape_count(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, 0);
	return body->get_shape_count();
}

RID Box3DPhysicsServer3D::_body_get_shape(const RID& p_body, int32_t p_shape_idx) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, RID());
	Box3DShapeImpl3D* shape = body->get_shape(p_shape_idx);
	return shape != nullptr ? shape->get_rid() : RID();
}

Transform3D Box3DPhysicsServer3D::_body_get_shape_transform(const RID& p_body, int32_t p_shape_idx) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, Transform3D());
	return body->get_shape_transform(p_shape_idx);
}

void Box3DPhysicsServer3D::_body_remove_shape(const RID& p_body, int32_t p_shape_idx) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->remove_shape(p_shape_idx);
}

void Box3DPhysicsServer3D::_body_clear_shapes(const RID& p_body) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->clear_shapes();
}

void Box3DPhysicsServer3D::_body_attach_object_instance_id(const RID& p_body, uint64_t p_id) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_instance_id(p_id);
}

uint64_t Box3DPhysicsServer3D::_body_get_object_instance_id(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, 0);
	return body->get_instance_id();
}

void Box3DPhysicsServer3D::_body_set_enable_continuous_collision_detection(const RID& p_body, bool p_enable) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_ccd_enabled(p_enable);
}

bool Box3DPhysicsServer3D::_body_is_continuous_collision_detection_enabled(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, false);
	return body->is_ccd_enabled();
}

void Box3DPhysicsServer3D::_body_set_collision_layer(const RID& p_body, uint32_t p_layer) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_collision_layer(p_layer);
}

uint32_t Box3DPhysicsServer3D::_body_get_collision_layer(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, 0);
	return body->get_collision_layer();
}

void Box3DPhysicsServer3D::_body_set_collision_mask(const RID& p_body, uint32_t p_mask) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_collision_mask(p_mask);
}

uint32_t Box3DPhysicsServer3D::_body_get_collision_mask(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, 0);
	return body->get_collision_mask();
}

void Box3DPhysicsServer3D::_body_set_collision_priority(const RID& p_body, double p_priority) {
	// No Box3D equivalent (collision priority affects only the built-in solver's
	// impulse weighting, which Box3D does not expose).
}

double Box3DPhysicsServer3D::_body_get_collision_priority(const RID& p_body) const {
	return 1.0;
}

void Box3DPhysicsServer3D::_body_set_user_flags(const RID& p_body, uint32_t p_flags) {
	// Not used by Box3D.
}

uint32_t Box3DPhysicsServer3D::_body_get_user_flags(const RID& p_body) const {
	return 0;
}

void Box3DPhysicsServer3D::_body_set_param(const RID& p_body, PhysicsServer3D::BodyParameter p_param, const Variant& p_value) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	switch (p_param) {
		case PhysicsServer3D::BODY_PARAM_BOUNCE:
			body->set_bounce(p_value);
			break;
		case PhysicsServer3D::BODY_PARAM_FRICTION:
			body->set_friction(p_value);
			break;
		case PhysicsServer3D::BODY_PARAM_MASS:
			body->set_mass(p_value);
			break;
		case PhysicsServer3D::BODY_PARAM_INERTIA:
			body->set_inertia(p_value);
			break;
		case PhysicsServer3D::BODY_PARAM_CENTER_OF_MASS:
			body->set_center_of_mass(p_value);
			break;
		case PhysicsServer3D::BODY_PARAM_GRAVITY_SCALE:
			body->set_gravity_scale(p_value);
			break;
		case PhysicsServer3D::BODY_PARAM_LINEAR_DAMP_MODE:
		case PhysicsServer3D::BODY_PARAM_ANGULAR_DAMP_MODE:
			// Box3D always applies damping as a simple replace mode; COMBINE mode has no
			// direct equivalent and is treated the same as REPLACE.
			break;
		case PhysicsServer3D::BODY_PARAM_LINEAR_DAMP:
			body->set_linear_damping(p_value);
			break;
		case PhysicsServer3D::BODY_PARAM_ANGULAR_DAMP:
			body->set_angular_damping(p_value);
			break;
		default:
			break;
	}
}

Variant Box3DPhysicsServer3D::_body_get_param(const RID& p_body, PhysicsServer3D::BodyParameter p_param) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, Variant());
	switch (p_param) {
		case PhysicsServer3D::BODY_PARAM_BOUNCE:
			return body->get_bounce();
		case PhysicsServer3D::BODY_PARAM_FRICTION:
			return body->get_friction();
		case PhysicsServer3D::BODY_PARAM_MASS:
			return body->get_mass();
		case PhysicsServer3D::BODY_PARAM_INERTIA:
			return body->get_inertia();
		case PhysicsServer3D::BODY_PARAM_CENTER_OF_MASS:
			return body->get_center_of_mass();
		case PhysicsServer3D::BODY_PARAM_GRAVITY_SCALE:
			return body->get_gravity_scale();
		case PhysicsServer3D::BODY_PARAM_LINEAR_DAMP_MODE:
		case PhysicsServer3D::BODY_PARAM_ANGULAR_DAMP_MODE:
			return PhysicsServer3D::BODY_DAMP_MODE_COMBINE;
		case PhysicsServer3D::BODY_PARAM_LINEAR_DAMP:
			return body->get_linear_damping();
		case PhysicsServer3D::BODY_PARAM_ANGULAR_DAMP:
			return body->get_angular_damping();
		default:
			return Variant();
	}
}

void Box3DPhysicsServer3D::_body_reset_mass_properties(const RID& p_body) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->apply_mass_from_shapes();
}

void Box3DPhysicsServer3D::_body_set_state(const RID& p_body, PhysicsServer3D::BodyState p_state, const Variant& p_value) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	switch (p_state) {
		case PhysicsServer3D::BODY_STATE_TRANSFORM:
			body->set_transform(p_value);
			break;
		case PhysicsServer3D::BODY_STATE_LINEAR_VELOCITY:
			body->set_linear_velocity(p_value);
			break;
		case PhysicsServer3D::BODY_STATE_ANGULAR_VELOCITY:
			body->set_angular_velocity(p_value);
			break;
		case PhysicsServer3D::BODY_STATE_SLEEPING:
			body->set_sleeping(p_value);
			break;
		case PhysicsServer3D::BODY_STATE_CAN_SLEEP:
			body->set_sleep_enabled(p_value);
			break;
		default:
			break;
	}
}

Variant Box3DPhysicsServer3D::_body_get_state(const RID& p_body, PhysicsServer3D::BodyState p_state) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, Variant());
	switch (p_state) {
		case PhysicsServer3D::BODY_STATE_TRANSFORM:
			return body->get_transform();
		case PhysicsServer3D::BODY_STATE_LINEAR_VELOCITY:
			return body->get_linear_velocity();
		case PhysicsServer3D::BODY_STATE_ANGULAR_VELOCITY:
			return body->get_angular_velocity();
		case PhysicsServer3D::BODY_STATE_SLEEPING:
			return body->is_sleeping();
		case PhysicsServer3D::BODY_STATE_CAN_SLEEP:
			return body->is_sleep_enabled();
		default:
			return Variant();
	}
}

void Box3DPhysicsServer3D::_body_apply_central_impulse(const RID& p_body, const Vector3& p_impulse) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->apply_central_impulse(p_impulse);
}

void Box3DPhysicsServer3D::_body_apply_impulse(const RID& p_body, const Vector3& p_impulse, const Vector3& p_position) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->apply_impulse(p_impulse, p_position);
}

void Box3DPhysicsServer3D::_body_apply_torque_impulse(const RID& p_body, const Vector3& p_impulse) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->apply_torque_impulse(p_impulse);
}

void Box3DPhysicsServer3D::_body_apply_central_force(const RID& p_body, const Vector3& p_force) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->apply_central_force(p_force);
}

void Box3DPhysicsServer3D::_body_apply_force(const RID& p_body, const Vector3& p_force, const Vector3& p_position) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->apply_force(p_force, p_position);
}

void Box3DPhysicsServer3D::_body_apply_torque(const RID& p_body, const Vector3& p_torque) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->apply_torque(p_torque);
}

void Box3DPhysicsServer3D::_body_add_constant_central_force(const RID& p_body, const Vector3& p_force) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->add_constant_central_force(p_force);
}

void Box3DPhysicsServer3D::_body_add_constant_force(const RID& p_body, const Vector3& p_force, const Vector3& p_position) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->add_constant_force(p_force, p_position);
}

void Box3DPhysicsServer3D::_body_add_constant_torque(const RID& p_body, const Vector3& p_torque) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->add_constant_torque(p_torque);
}

void Box3DPhysicsServer3D::_body_set_constant_force(const RID& p_body, const Vector3& p_force) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_constant_force(p_force);
}

Vector3 Box3DPhysicsServer3D::_body_get_constant_force(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, Vector3());
	return body->get_constant_force();
}

void Box3DPhysicsServer3D::_body_set_constant_torque(const RID& p_body, const Vector3& p_torque) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_constant_torque(p_torque);
}

Vector3 Box3DPhysicsServer3D::_body_get_constant_torque(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, Vector3());
	return body->get_constant_torque();
}

void Box3DPhysicsServer3D::_body_set_axis_velocity(const RID& p_body, const Vector3& p_axis_velocity) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	const Vector3 axis = p_axis_velocity.normalized();
	Vector3 velocity = body->get_linear_velocity();
	velocity -= axis * axis.dot(velocity);
	velocity += p_axis_velocity;
	body->set_linear_velocity(velocity);
}

void Box3DPhysicsServer3D::_body_set_axis_lock(const RID& p_body, PhysicsServer3D::BodyAxis p_axis, bool p_lock) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_axis_lock(p_axis, p_lock);
}

bool Box3DPhysicsServer3D::_body_is_axis_locked(const RID& p_body, PhysicsServer3D::BodyAxis p_axis) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, false);
	return body->get_axis_lock(p_axis);
}

void Box3DPhysicsServer3D::_body_add_collision_exception(const RID& p_body, const RID& p_excepted_body) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	Box3DBodyImpl3D* excepted = body_owner.get_or_null(p_excepted_body);
	ERR_FAIL_NULL(body);
	ERR_FAIL_NULL(excepted);
	// Backed by a box3d filter joint between the pair (b3CreateFilterJoint).
	body->add_collision_exception(p_excepted_body, excepted);
}

void Box3DPhysicsServer3D::_body_remove_collision_exception(const RID& p_body, const RID& p_excepted_body) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->remove_collision_exception(p_excepted_body);
}

TypedArray<RID> Box3DPhysicsServer3D::_body_get_collision_exceptions(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, TypedArray<RID>());
	return body->get_collision_exceptions();
}

void Box3DPhysicsServer3D::_body_set_max_contacts_reported(const RID& p_body, int32_t p_amount) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_max_contacts_reported(p_amount);
}

int32_t Box3DPhysicsServer3D::_body_get_max_contacts_reported(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, 0);
	return body->get_max_contacts_reported();
}

void Box3DPhysicsServer3D::_body_set_contacts_reported_depth_threshold(const RID& p_body, double p_threshold) {
	// No Box3D equivalent.
}

double Box3DPhysicsServer3D::_body_get_contacts_reported_depth_threshold(const RID& p_body) const {
	return 0.0;
}

void Box3DPhysicsServer3D::_body_set_omit_force_integration(const RID& p_body, bool p_enable) {
	// Not applicable: Box3D has no separate "omit force integration" concept; the
	// force_integration_callback (when set) is simply the sole source of forces for v1.
}

bool Box3DPhysicsServer3D::_body_is_omitting_force_integration(const RID& p_body) const {
	return false;
}

void Box3DPhysicsServer3D::_body_set_state_sync_callback(const RID& p_body, const Callable& p_callable) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_state_sync_callback(p_callable);
}

void Box3DPhysicsServer3D::_body_set_force_integration_callback(const RID& p_body, const Callable& p_callable, const Variant& p_userdata) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_force_integration_callback(p_callable, p_userdata);
}

void Box3DPhysicsServer3D::_body_set_ray_pickable(const RID& p_body, bool p_enable) {
	// Not implemented in v1 (no editor picking support).
}

bool Box3DPhysicsServer3D::_body_test_motion(
		const RID& p_body,
		const Transform3D& p_from,
		const Vector3& p_motion,
		double p_margin,
		int32_t p_max_collisions,
		bool p_collide_separation_ray,
		bool p_recovery_as_collision,
		PhysicsServer3DExtensionMotionResult* p_result) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, false);
	Box3DSpace3D* space = body->get_space();
	ERR_FAIL_NULL_V(space, false);

	return space->get_direct_state()->test_body_motion(*body, p_from, p_motion, p_margin, p_max_collisions, p_recovery_as_collision, p_result);
}

PhysicsDirectBodyState3D* Box3DPhysicsServer3D::_body_get_direct_state(const RID& p_body) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, nullptr);
	if (!body->has_body_id()) {
		return nullptr;
	}
	return body->get_direct_state_or_null();
}

// --- Joints ---

RID Box3DPhysicsServer3D::_joint_create() {
	// Placeholder RID: the concrete Box3DJointImpl3D is only constructed once
	// _joint_make_pin/_joint_make_hinge/_joint_make_slider is called (see below), since
	// only then are the joint type and both body RIDs known.
	const RID rid = joint_owner.make_rid(nullptr);
	return rid;
}

void Box3DPhysicsServer3D::_joint_clear(const RID& p_joint) {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	if (joint != nullptr) {
		memdelete(joint);
		joint_owner.replace(p_joint, nullptr);
	}
}

void Box3DPhysicsServer3D::_joint_make_pin(const RID& p_joint, const RID& p_body_a, const Vector3& p_local_a, const RID& p_body_b, const Vector3& p_local_b) {
	_joint_clear(p_joint);

	Box3DBodyImpl3D* body_a = body_owner.get_or_null(p_body_a);
	Box3DBodyImpl3D* body_b = body_owner.get_or_null(p_body_b);
	ERR_FAIL_NULL(body_a);
	ERR_FAIL_NULL(body_b);

	auto* joint = memnew(Box3DPinJointImpl3D(body_a, body_b, Transform3D(Basis(), p_local_a), Transform3D(Basis(), p_local_b)));
	joint->set_rid(p_joint);
	joint_owner.replace(p_joint, joint);
	joint->rebuild();
}

void Box3DPhysicsServer3D::_pin_joint_set_param(const RID& p_joint, PhysicsServer3D::PinJointParam p_param, double p_value) {
	auto* joint = dynamic_cast<Box3DPinJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	joint->set_param(p_param, p_value);
}

double Box3DPhysicsServer3D::_pin_joint_get_param(const RID& p_joint, PhysicsServer3D::PinJointParam p_param) const {
	auto* joint = dynamic_cast<Box3DPinJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_param(p_param);
}

void Box3DPhysicsServer3D::_pin_joint_set_local_a(const RID& p_joint, const Vector3& p_local_a) {
	auto* joint = dynamic_cast<Box3DPinJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	Transform3D frame = joint->get_local_frame_a();
	frame.origin = p_local_a;
	joint->set_local_frame_a(frame);
}

Vector3 Box3DPhysicsServer3D::_pin_joint_get_local_a(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DPinJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, Vector3());
	return joint->get_local_frame_a().origin;
}

void Box3DPhysicsServer3D::_pin_joint_set_local_b(const RID& p_joint, const Vector3& p_local_b) {
	auto* joint = dynamic_cast<Box3DPinJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	Transform3D frame = joint->get_local_frame_b();
	frame.origin = p_local_b;
	joint->set_local_frame_b(frame);
}

Vector3 Box3DPhysicsServer3D::_pin_joint_get_local_b(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DPinJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, Vector3());
	return joint->get_local_frame_b().origin;
}

void Box3DPhysicsServer3D::_joint_make_hinge(const RID& p_joint, const RID& p_body_a, const Transform3D& p_hinge_a, const RID& p_body_b, const Transform3D& p_hinge_b) {
	_joint_clear(p_joint);

	Box3DBodyImpl3D* body_a = body_owner.get_or_null(p_body_a);
	Box3DBodyImpl3D* body_b = body_owner.get_or_null(p_body_b);
	ERR_FAIL_NULL(body_a);
	ERR_FAIL_NULL(body_b);

	// Box3D's revolute joint rotates about the LOCAL Z axis of the joint frames; Godot's
	// HingeJoint3D convention uses local Z as the hinge axis too (the incoming p_hinge_a/b
	// transforms are the joint frames as Godot core builds them for HingeJoint3D), so no
	// remap is required here.
	auto* joint = memnew(Box3DHingeJointImpl3D(body_a, body_b, p_hinge_a, p_hinge_b));
	joint->set_rid(p_joint);
	joint_owner.replace(p_joint, joint);
	joint->rebuild();
}

void Box3DPhysicsServer3D::_joint_make_hinge_simple(const RID& p_joint, const RID& p_body_a, const Vector3& p_pivot_a, const Vector3& p_axis_a, const RID& p_body_b, const Vector3& p_pivot_b, const Vector3& p_axis_b) {
	_joint_clear(p_joint);

	Box3DBodyImpl3D* body_a = body_owner.get_or_null(p_body_a);
	Box3DBodyImpl3D* body_b = body_owner.get_or_null(p_body_b);
	ERR_FAIL_NULL(body_a);
	ERR_FAIL_NULL(body_b);

	// Build a joint-frame basis that maps local Z to the given hinge axis (shortest-arc
	// rotation from +Z), since Box3D's revolute joint always rotates about local Z.
	const Vector3 z_axis(0, 0, 1);
	const Quaternion rotation_a = Quaternion(z_axis, p_axis_a.normalized());
	const Quaternion rotation_b = Quaternion(z_axis, p_axis_b.normalized());

	const Transform3D frame_a(Basis(rotation_a), p_pivot_a);
	const Transform3D frame_b(Basis(rotation_b), p_pivot_b);

	auto* joint = memnew(Box3DHingeJointImpl3D(body_a, body_b, frame_a, frame_b));
	joint->set_rid(p_joint);
	joint_owner.replace(p_joint, joint);
	joint->rebuild();
}

RID Box3DPhysicsServer3D::wheel_joint_create(const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) {
	Box3DBodyImpl3D* body_a = body_owner.get_or_null(p_body_a);
	Box3DBodyImpl3D* body_b = body_owner.get_or_null(p_body_b);
	ERR_FAIL_NULL_V(body_a, RID());
	ERR_FAIL_NULL_V(body_b, RID());

	const RID rid = joint_owner.make_rid(nullptr);
	auto* joint = memnew(Box3DWheelJointImpl3D(body_a, body_b, p_frame_a, p_frame_b));
	joint->set_rid(rid);
	joint_owner.replace(rid, joint);
	joint->rebuild();
	return rid;
}

void Box3DPhysicsServer3D::wheel_joint_set_param(const RID& p_joint, int p_param, double p_value) {
	auto* joint = dynamic_cast<Box3DWheelJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	ERR_FAIL_INDEX(p_param, Box3DWheelJointImpl3D::PARAM_MAX);
	joint->set_param((Box3DWheelJointImpl3D::Param)p_param, p_value);
}

double Box3DPhysicsServer3D::wheel_joint_get_param(const RID& p_joint, int p_param) const {
	auto* joint = dynamic_cast<Box3DWheelJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	ERR_FAIL_INDEX_V(p_param, Box3DWheelJointImpl3D::PARAM_MAX, 0.0);
	return joint->get_param((Box3DWheelJointImpl3D::Param)p_param);
}

void Box3DPhysicsServer3D::wheel_joint_set_flag(const RID& p_joint, int p_flag, bool p_enabled) {
	auto* joint = dynamic_cast<Box3DWheelJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	ERR_FAIL_INDEX(p_flag, Box3DWheelJointImpl3D::FLAG_MAX);
	joint->set_flag((Box3DWheelJointImpl3D::Flag)p_flag, p_enabled);
}

bool Box3DPhysicsServer3D::wheel_joint_get_flag(const RID& p_joint, int p_flag) const {
	auto* joint = dynamic_cast<Box3DWheelJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, false);
	ERR_FAIL_INDEX_V(p_flag, Box3DWheelJointImpl3D::FLAG_MAX, false);
	return joint->get_flag((Box3DWheelJointImpl3D::Flag)p_flag);
}

double Box3DPhysicsServer3D::wheel_joint_get_spin_speed(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DWheelJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_spin_speed();
}

double Box3DPhysicsServer3D::wheel_joint_get_spin_torque(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DWheelJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_spin_torque();
}

double Box3DPhysicsServer3D::wheel_joint_get_steering_angle(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DWheelJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_steering_angle();
}

double Box3DPhysicsServer3D::wheel_joint_get_steering_torque(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DWheelJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_steering_torque();
}

RID Box3DPhysicsServer3D::weld_joint_create(const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) {
	Box3DBodyImpl3D* body_a = body_owner.get_or_null(p_body_a);
	Box3DBodyImpl3D* body_b = body_owner.get_or_null(p_body_b);
	ERR_FAIL_NULL_V(body_a, RID());
	ERR_FAIL_NULL_V(body_b, RID());
	const RID rid = joint_owner.make_rid(nullptr);
	auto* joint = memnew(Box3DWeldJointImpl3D(body_a, body_b, p_frame_a, p_frame_b));
	joint->set_rid(rid);
	joint_owner.replace(rid, joint);
	joint->rebuild();
	return rid;
}

void Box3DPhysicsServer3D::weld_joint_set_param(const RID& p_joint, int p_param, double p_value) {
	auto* joint = dynamic_cast<Box3DWeldJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	ERR_FAIL_INDEX(p_param, Box3DWeldJointImpl3D::PARAM_MAX);
	joint->set_param((Box3DWeldJointImpl3D::Param)p_param, p_value);
}

double Box3DPhysicsServer3D::weld_joint_get_param(const RID& p_joint, int p_param) const {
	auto* joint = dynamic_cast<Box3DWeldJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	ERR_FAIL_INDEX_V(p_param, Box3DWeldJointImpl3D::PARAM_MAX, 0.0);
	return joint->get_param((Box3DWeldJointImpl3D::Param)p_param);
}

RID Box3DPhysicsServer3D::distance_joint_create(const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) {
	Box3DBodyImpl3D* body_a = body_owner.get_or_null(p_body_a);
	Box3DBodyImpl3D* body_b = body_owner.get_or_null(p_body_b);
	ERR_FAIL_NULL_V(body_a, RID());
	ERR_FAIL_NULL_V(body_b, RID());
	const RID rid = joint_owner.make_rid(nullptr);
	auto* joint = memnew(Box3DDistanceJointImpl3D(body_a, body_b, p_frame_a, p_frame_b));
	joint->set_rid(rid);
	joint_owner.replace(rid, joint);
	joint->rebuild();
	return rid;
}

void Box3DPhysicsServer3D::distance_joint_set_param(const RID& p_joint, int p_param, double p_value) {
	auto* joint = dynamic_cast<Box3DDistanceJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	ERR_FAIL_INDEX(p_param, Box3DDistanceJointImpl3D::PARAM_MAX);
	joint->set_param((Box3DDistanceJointImpl3D::Param)p_param, p_value);
}

double Box3DPhysicsServer3D::distance_joint_get_param(const RID& p_joint, int p_param) const {
	auto* joint = dynamic_cast<Box3DDistanceJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	ERR_FAIL_INDEX_V(p_param, Box3DDistanceJointImpl3D::PARAM_MAX, 0.0);
	return joint->get_param((Box3DDistanceJointImpl3D::Param)p_param);
}

void Box3DPhysicsServer3D::distance_joint_set_flag(const RID& p_joint, int p_flag, bool p_enabled) {
	auto* joint = dynamic_cast<Box3DDistanceJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	ERR_FAIL_INDEX(p_flag, Box3DDistanceJointImpl3D::FLAG_MAX);
	joint->set_flag((Box3DDistanceJointImpl3D::Flag)p_flag, p_enabled);
}

bool Box3DPhysicsServer3D::distance_joint_get_flag(const RID& p_joint, int p_flag) const {
	auto* joint = dynamic_cast<Box3DDistanceJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, false);
	ERR_FAIL_INDEX_V(p_flag, Box3DDistanceJointImpl3D::FLAG_MAX, false);
	return joint->get_flag((Box3DDistanceJointImpl3D::Flag)p_flag);
}

double Box3DPhysicsServer3D::distance_joint_get_current_length(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DDistanceJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_current_length();
}

double Box3DPhysicsServer3D::distance_joint_get_motor_force(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DDistanceJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_motor_force();
}

RID Box3DPhysicsServer3D::motor_joint_create(const RID& p_body_a, const RID& p_body_b, const Transform3D& p_frame_a, const Transform3D& p_frame_b) {
	Box3DBodyImpl3D* body_a = body_owner.get_or_null(p_body_a);
	Box3DBodyImpl3D* body_b = body_owner.get_or_null(p_body_b);
	ERR_FAIL_NULL_V(body_a, RID());
	ERR_FAIL_NULL_V(body_b, RID());
	const RID rid = joint_owner.make_rid(nullptr);
	auto* joint = memnew(Box3DMotorJointImpl3D(body_a, body_b, p_frame_a, p_frame_b));
	joint->set_rid(rid);
	joint_owner.replace(rid, joint);
	joint->rebuild();
	return rid;
}

void Box3DPhysicsServer3D::motor_joint_set_param(const RID& p_joint, int p_param, double p_value) {
	auto* joint = dynamic_cast<Box3DMotorJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	ERR_FAIL_INDEX(p_param, Box3DMotorJointImpl3D::PARAM_MAX);
	joint->set_param((Box3DMotorJointImpl3D::Param)p_param, p_value);
}

double Box3DPhysicsServer3D::motor_joint_get_param(const RID& p_joint, int p_param) const {
	auto* joint = dynamic_cast<Box3DMotorJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	ERR_FAIL_INDEX_V(p_param, Box3DMotorJointImpl3D::PARAM_MAX, 0.0);
	return joint->get_param((Box3DMotorJointImpl3D::Param)p_param);
}

void Box3DPhysicsServer3D::motor_joint_set_linear_velocity(const RID& p_joint, const Vector3& p_velocity) {
	auto* joint = dynamic_cast<Box3DMotorJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	joint->set_linear_velocity(p_velocity);
}

Vector3 Box3DPhysicsServer3D::motor_joint_get_linear_velocity(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DMotorJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, Vector3());
	return joint->get_linear_velocity();
}

void Box3DPhysicsServer3D::motor_joint_set_angular_velocity(const RID& p_joint, const Vector3& p_velocity) {
	auto* joint = dynamic_cast<Box3DMotorJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	joint->set_angular_velocity(p_velocity);
}

Vector3 Box3DPhysicsServer3D::motor_joint_get_angular_velocity(const RID& p_joint) const {
	auto* joint = dynamic_cast<Box3DMotorJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, Vector3());
	return joint->get_angular_velocity();
}

Vector3 Box3DPhysicsServer3D::joint_get_constraint_force(const RID& p_joint) const {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	ERR_FAIL_NULL_V(joint, Vector3());
	if (!joint->has_joint_id()) {
		return Vector3();
	}
	return b3_to_godot(b3Joint_GetConstraintForce(joint->get_joint_id()));
}

Vector3 Box3DPhysicsServer3D::joint_get_constraint_torque(const RID& p_joint) const {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	ERR_FAIL_NULL_V(joint, Vector3());
	if (!joint->has_joint_id()) {
		return Vector3();
	}
	return b3_to_godot(b3Joint_GetConstraintTorque(joint->get_joint_id()));
}

void Box3DPhysicsServer3D::joint_set_force_threshold(const RID& p_joint, double p_threshold) {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	ERR_FAIL_NULL(joint);
	joint->set_force_threshold(p_threshold);
}

double Box3DPhysicsServer3D::joint_get_force_threshold(const RID& p_joint) const {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_force_threshold();
}

void Box3DPhysicsServer3D::joint_set_torque_threshold(const RID& p_joint, double p_threshold) {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	ERR_FAIL_NULL(joint);
	joint->set_torque_threshold(p_threshold);
}

double Box3DPhysicsServer3D::joint_get_torque_threshold(const RID& p_joint) const {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_torque_threshold();
}

Array Box3DPhysicsServer3D::space_get_contact_hit_events(const RID& p_space) const {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, Array());
	return space->get_contact_hit_events();
}

Array Box3DPhysicsServer3D::space_get_joint_force_events(const RID& p_space) const {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, Array());
	return space->get_joint_force_events();
}

namespace {

struct MoverPlaneCollector {
	Vector3 origin;
	Array planes;
};

bool _mover_plane_result(b3ShapeId p_shape, const b3PlaneResult* p_results, int p_count, void* p_context) {
	auto* collector = static_cast<MoverPlaneCollector*>(p_context);
	for (int i = 0; i < p_count; i++) {
		Dictionary plane;
		plane["normal"] = b3_to_godot(p_results[i].plane.normal);
		plane["offset"] = p_results[i].plane.offset;
		plane["point"] = b3_to_godot(p_results[i].point) + collector->origin;
		plane["push_limit"] = FLT_MAX;
		plane["clip_velocity"] = true;
		plane["push"] = 0.0;
		collector->planes.push_back(plane);
	}
	return true;
}

// Rebuilds the box3d plane array from the script-facing dictionaries.
LocalVector<b3CollisionPlane> _planes_from_array(const Array& p_planes) {
	LocalVector<b3CollisionPlane> planes;
	planes.reserve(p_planes.size());
	for (int i = 0; i < p_planes.size(); i++) {
		const Dictionary plane_dict = p_planes[i];
		b3CollisionPlane plane;
		plane.plane.normal = godot_to_b3((Vector3)plane_dict.get("normal", Vector3()));
		plane.plane.offset = (float)(double)plane_dict.get("offset", 0.0);
		plane.pushLimit = (float)(double)plane_dict.get("push_limit", (double)FLT_MAX);
		plane.push = (float)(double)plane_dict.get("push", 0.0);
		plane.clipVelocity = (bool)plane_dict.get("clip_velocity", true);
		planes.push_back(plane);
	}
	return planes;
}

} // namespace

Array Box3DPhysicsServer3D::space_collide_mover(const RID& p_space, const Vector3& p_c1, const Vector3& p_c2, double p_radius, uint32_t p_collision_mask) const {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, Array());

	const Vector3 origin = (p_c1 + p_c2) * 0.5;
	b3Capsule mover;
	mover.center1 = godot_to_b3(p_c1 - origin);
	mover.center2 = godot_to_b3(p_c2 - origin);
	mover.radius = (float)p_radius;

	b3QueryFilter filter = b3DefaultQueryFilter();
	filter.maskBits = (uint64_t)p_collision_mask;

	MoverPlaneCollector collector;
	collector.origin = origin;
	b3World_CollideMover(space->get_world_id(), godot_to_b3(origin), &mover, filter, _mover_plane_result, &collector);
	return collector.planes;
}

double Box3DPhysicsServer3D::space_cast_mover(const RID& p_space, const Vector3& p_c1, const Vector3& p_c2, double p_radius, const Vector3& p_translation, uint32_t p_collision_mask) const {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, 1.0);

	const Vector3 origin = (p_c1 + p_c2) * 0.5;
	b3Capsule mover;
	mover.center1 = godot_to_b3(p_c1 - origin);
	mover.center2 = godot_to_b3(p_c2 - origin);
	mover.radius = (float)p_radius;

	b3QueryFilter filter = b3DefaultQueryFilter();
	filter.maskBits = (uint64_t)p_collision_mask;

	return b3World_CastMover(space->get_world_id(), godot_to_b3(origin), &mover, godot_to_b3(p_translation), filter, nullptr, nullptr);
}

Dictionary Box3DPhysicsServer3D::solve_mover_planes(const Vector3& p_target_delta, const Array& p_planes) const {
	LocalVector<b3CollisionPlane> planes = _planes_from_array(p_planes);
	const b3PlaneSolverResult result = b3SolvePlanes(godot_to_b3(p_target_delta), planes.ptr(), planes.size());

	// b3SolvePlanes writes each plane's computed push; hand the updated planes back so
	// clip_mover_velocity can skip zero-push planes like the box3d samples do.
	Array updated = p_planes.duplicate(true);
	for (int i = 0; i < updated.size(); i++) {
		Dictionary plane_dict = updated[i];
		plane_dict["push"] = planes[i].push;
	}

	Dictionary solved;
	solved["delta"] = b3_to_godot(result.delta);
	solved["iterations"] = result.iterationCount;
	solved["planes"] = updated;
	return solved;
}

Vector3 Box3DPhysicsServer3D::clip_mover_velocity(const Vector3& p_velocity, const Array& p_planes) const {
	LocalVector<b3CollisionPlane> planes = _planes_from_array(p_planes);
	return b3_to_godot(b3ClipVector(godot_to_b3(p_velocity), planes.ptr(), planes.size()));
}

void Box3DPhysicsServer3D::space_explode(const RID& p_space, const Vector3& p_position, double p_radius, double p_falloff, double p_impulse_per_area, uint32_t p_collision_mask) {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL(space);

	b3ExplosionDef def = b3DefaultExplosionDef();
	def.maskBits = (uint64_t)p_collision_mask;
	def.position = godot_to_b3(p_position);
	def.radius = (float)p_radius;
	def.falloff = (float)p_falloff;
	def.impulsePerArea = (float)p_impulse_per_area;
	b3World_Explode(space->get_world_id(), &def);
}

void Box3DPhysicsServer3D::space_start_recording(const RID& p_space) {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL(space);
	space->start_recording();
}

PackedByteArray Box3DPhysicsServer3D::space_stop_recording(const RID& p_space) {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, PackedByteArray());
	return space->stop_recording();
}

bool Box3DPhysicsServer3D::space_is_recording(const RID& p_space) const {
	Box3DSpace3D* space = space_owner.get_or_null(p_space);
	ERR_FAIL_NULL_V(space, false);
	return space->is_recording();
}

bool Box3DPhysicsServer3D::validate_replay(const PackedByteArray& p_data, int p_worker_count) const {
	ERR_FAIL_COND_V(p_data.is_empty(), false);
	return b3ValidateReplay(p_data.ptr(), p_data.size(), MAX(1, p_worker_count));
}

void Box3DPhysicsServer3D::body_set_rolling_resistance(const RID& p_body, double p_value) {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL(body);
	body->set_rolling_resistance(p_value);
}

double Box3DPhysicsServer3D::body_get_rolling_resistance(const RID& p_body) const {
	Box3DBodyImpl3D* body = body_owner.get_or_null(p_body);
	ERR_FAIL_NULL_V(body, 0.0);
	return body->get_rolling_resistance();
}

void Box3DPhysicsServer3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("wheel_joint_create", "body_a", "body_b", "frame_a", "frame_b"), &Box3DPhysicsServer3D::wheel_joint_create);
	ClassDB::bind_method(D_METHOD("wheel_joint_set_param", "joint", "param", "value"), &Box3DPhysicsServer3D::wheel_joint_set_param);
	ClassDB::bind_method(D_METHOD("wheel_joint_get_param", "joint", "param"), &Box3DPhysicsServer3D::wheel_joint_get_param);
	ClassDB::bind_method(D_METHOD("wheel_joint_set_flag", "joint", "flag", "enabled"), &Box3DPhysicsServer3D::wheel_joint_set_flag);
	ClassDB::bind_method(D_METHOD("wheel_joint_get_flag", "joint", "flag"), &Box3DPhysicsServer3D::wheel_joint_get_flag);
	ClassDB::bind_method(D_METHOD("wheel_joint_get_spin_speed", "joint"), &Box3DPhysicsServer3D::wheel_joint_get_spin_speed);
	ClassDB::bind_method(D_METHOD("wheel_joint_get_spin_torque", "joint"), &Box3DPhysicsServer3D::wheel_joint_get_spin_torque);
	ClassDB::bind_method(D_METHOD("wheel_joint_get_steering_angle", "joint"), &Box3DPhysicsServer3D::wheel_joint_get_steering_angle);
	ClassDB::bind_method(D_METHOD("wheel_joint_get_steering_torque", "joint"), &Box3DPhysicsServer3D::wheel_joint_get_steering_torque);
	ClassDB::bind_method(D_METHOD("body_set_rolling_resistance", "body", "value"), &Box3DPhysicsServer3D::body_set_rolling_resistance);
	ClassDB::bind_method(D_METHOD("body_get_rolling_resistance", "body"), &Box3DPhysicsServer3D::body_get_rolling_resistance);

	ClassDB::bind_method(D_METHOD("weld_joint_create", "body_a", "body_b", "frame_a", "frame_b"), &Box3DPhysicsServer3D::weld_joint_create);
	ClassDB::bind_method(D_METHOD("weld_joint_set_param", "joint", "param", "value"), &Box3DPhysicsServer3D::weld_joint_set_param);
	ClassDB::bind_method(D_METHOD("weld_joint_get_param", "joint", "param"), &Box3DPhysicsServer3D::weld_joint_get_param);

	ClassDB::bind_method(D_METHOD("distance_joint_create", "body_a", "body_b", "frame_a", "frame_b"), &Box3DPhysicsServer3D::distance_joint_create);
	ClassDB::bind_method(D_METHOD("distance_joint_set_param", "joint", "param", "value"), &Box3DPhysicsServer3D::distance_joint_set_param);
	ClassDB::bind_method(D_METHOD("distance_joint_get_param", "joint", "param"), &Box3DPhysicsServer3D::distance_joint_get_param);
	ClassDB::bind_method(D_METHOD("distance_joint_set_flag", "joint", "flag", "enabled"), &Box3DPhysicsServer3D::distance_joint_set_flag);
	ClassDB::bind_method(D_METHOD("distance_joint_get_flag", "joint", "flag"), &Box3DPhysicsServer3D::distance_joint_get_flag);
	ClassDB::bind_method(D_METHOD("distance_joint_get_current_length", "joint"), &Box3DPhysicsServer3D::distance_joint_get_current_length);
	ClassDB::bind_method(D_METHOD("distance_joint_get_motor_force", "joint"), &Box3DPhysicsServer3D::distance_joint_get_motor_force);

	ClassDB::bind_method(D_METHOD("motor_joint_create", "body_a", "body_b", "frame_a", "frame_b"), &Box3DPhysicsServer3D::motor_joint_create);
	ClassDB::bind_method(D_METHOD("motor_joint_set_param", "joint", "param", "value"), &Box3DPhysicsServer3D::motor_joint_set_param);
	ClassDB::bind_method(D_METHOD("motor_joint_get_param", "joint", "param"), &Box3DPhysicsServer3D::motor_joint_get_param);
	ClassDB::bind_method(D_METHOD("motor_joint_set_linear_velocity", "joint", "velocity"), &Box3DPhysicsServer3D::motor_joint_set_linear_velocity);
	ClassDB::bind_method(D_METHOD("motor_joint_get_linear_velocity", "joint"), &Box3DPhysicsServer3D::motor_joint_get_linear_velocity);
	ClassDB::bind_method(D_METHOD("motor_joint_set_angular_velocity", "joint", "velocity"), &Box3DPhysicsServer3D::motor_joint_set_angular_velocity);
	ClassDB::bind_method(D_METHOD("motor_joint_get_angular_velocity", "joint"), &Box3DPhysicsServer3D::motor_joint_get_angular_velocity);

	ClassDB::bind_method(D_METHOD("joint_get_constraint_force", "joint"), &Box3DPhysicsServer3D::joint_get_constraint_force);
	ClassDB::bind_method(D_METHOD("joint_get_constraint_torque", "joint"), &Box3DPhysicsServer3D::joint_get_constraint_torque);
	ClassDB::bind_method(D_METHOD("joint_set_force_threshold", "joint", "threshold"), &Box3DPhysicsServer3D::joint_set_force_threshold);
	ClassDB::bind_method(D_METHOD("joint_get_force_threshold", "joint"), &Box3DPhysicsServer3D::joint_get_force_threshold);
	ClassDB::bind_method(D_METHOD("joint_set_torque_threshold", "joint", "threshold"), &Box3DPhysicsServer3D::joint_set_torque_threshold);
	ClassDB::bind_method(D_METHOD("joint_get_torque_threshold", "joint"), &Box3DPhysicsServer3D::joint_get_torque_threshold);
	ClassDB::bind_method(D_METHOD("space_get_contact_hit_events", "space"), &Box3DPhysicsServer3D::space_get_contact_hit_events);
	ClassDB::bind_method(D_METHOD("space_get_joint_force_events", "space"), &Box3DPhysicsServer3D::space_get_joint_force_events);
	ClassDB::bind_method(D_METHOD("space_collide_mover", "space", "capsule_start", "capsule_end", "radius", "collision_mask"), &Box3DPhysicsServer3D::space_collide_mover);
	ClassDB::bind_method(D_METHOD("space_cast_mover", "space", "capsule_start", "capsule_end", "radius", "translation", "collision_mask"), &Box3DPhysicsServer3D::space_cast_mover);
	ClassDB::bind_method(D_METHOD("solve_mover_planes", "target_delta", "planes"), &Box3DPhysicsServer3D::solve_mover_planes);
	ClassDB::bind_method(D_METHOD("clip_mover_velocity", "velocity", "planes"), &Box3DPhysicsServer3D::clip_mover_velocity);
	ClassDB::bind_method(D_METHOD("space_explode", "space", "position", "radius", "falloff", "impulse_per_area", "collision_mask"), &Box3DPhysicsServer3D::space_explode);
	ClassDB::bind_method(D_METHOD("space_start_recording", "space"), &Box3DPhysicsServer3D::space_start_recording);
	ClassDB::bind_method(D_METHOD("space_stop_recording", "space"), &Box3DPhysicsServer3D::space_stop_recording);
	ClassDB::bind_method(D_METHOD("space_is_recording", "space"), &Box3DPhysicsServer3D::space_is_recording);
	ClassDB::bind_method(D_METHOD("validate_replay", "data", "worker_count"), &Box3DPhysicsServer3D::validate_replay);

	const StringName cls = get_class_static();
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_SUSPENSION_HERTZ", Box3DWheelJointImpl3D::PARAM_SUSPENSION_HERTZ);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_SUSPENSION_DAMPING", Box3DWheelJointImpl3D::PARAM_SUSPENSION_DAMPING);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_SUSPENSION_LOWER", Box3DWheelJointImpl3D::PARAM_SUSPENSION_LOWER);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_SUSPENSION_UPPER", Box3DWheelJointImpl3D::PARAM_SUSPENSION_UPPER);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_SPIN_MOTOR_SPEED", Box3DWheelJointImpl3D::PARAM_SPIN_MOTOR_SPEED);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_MAX_SPIN_TORQUE", Box3DWheelJointImpl3D::PARAM_MAX_SPIN_TORQUE);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_STEERING_HERTZ", Box3DWheelJointImpl3D::PARAM_STEERING_HERTZ);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_STEERING_DAMPING", Box3DWheelJointImpl3D::PARAM_STEERING_DAMPING);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_TARGET_STEERING_ANGLE", Box3DWheelJointImpl3D::PARAM_TARGET_STEERING_ANGLE);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_MAX_STEERING_TORQUE", Box3DWheelJointImpl3D::PARAM_MAX_STEERING_TORQUE);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_STEERING_LOWER", Box3DWheelJointImpl3D::PARAM_STEERING_LOWER);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_PARAM_STEERING_UPPER", Box3DWheelJointImpl3D::PARAM_STEERING_UPPER);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_FLAG_ENABLE_SUSPENSION", Box3DWheelJointImpl3D::FLAG_ENABLE_SUSPENSION);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_FLAG_ENABLE_SUSPENSION_LIMIT", Box3DWheelJointImpl3D::FLAG_ENABLE_SUSPENSION_LIMIT);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_FLAG_ENABLE_SPIN_MOTOR", Box3DWheelJointImpl3D::FLAG_ENABLE_SPIN_MOTOR);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_FLAG_ENABLE_STEERING", Box3DWheelJointImpl3D::FLAG_ENABLE_STEERING);
	ClassDB::bind_integer_constant(cls, "", "WHEEL_JOINT_FLAG_ENABLE_STEERING_LIMIT", Box3DWheelJointImpl3D::FLAG_ENABLE_STEERING_LIMIT);

	ClassDB::bind_integer_constant(cls, "", "WELD_JOINT_PARAM_LINEAR_HERTZ", Box3DWeldJointImpl3D::PARAM_LINEAR_HERTZ);
	ClassDB::bind_integer_constant(cls, "", "WELD_JOINT_PARAM_LINEAR_DAMPING", Box3DWeldJointImpl3D::PARAM_LINEAR_DAMPING);
	ClassDB::bind_integer_constant(cls, "", "WELD_JOINT_PARAM_ANGULAR_HERTZ", Box3DWeldJointImpl3D::PARAM_ANGULAR_HERTZ);
	ClassDB::bind_integer_constant(cls, "", "WELD_JOINT_PARAM_ANGULAR_DAMPING", Box3DWeldJointImpl3D::PARAM_ANGULAR_DAMPING);

	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_LENGTH", Box3DDistanceJointImpl3D::PARAM_LENGTH);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_LOWER_SPRING_FORCE", Box3DDistanceJointImpl3D::PARAM_LOWER_SPRING_FORCE);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_UPPER_SPRING_FORCE", Box3DDistanceJointImpl3D::PARAM_UPPER_SPRING_FORCE);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_HERTZ", Box3DDistanceJointImpl3D::PARAM_HERTZ);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_DAMPING", Box3DDistanceJointImpl3D::PARAM_DAMPING);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_MIN_LENGTH", Box3DDistanceJointImpl3D::PARAM_MIN_LENGTH);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_MAX_LENGTH", Box3DDistanceJointImpl3D::PARAM_MAX_LENGTH);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_MOTOR_SPEED", Box3DDistanceJointImpl3D::PARAM_MOTOR_SPEED);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_PARAM_MAX_MOTOR_FORCE", Box3DDistanceJointImpl3D::PARAM_MAX_MOTOR_FORCE);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_FLAG_ENABLE_SPRING", Box3DDistanceJointImpl3D::FLAG_ENABLE_SPRING);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_FLAG_ENABLE_LIMIT", Box3DDistanceJointImpl3D::FLAG_ENABLE_LIMIT);
	ClassDB::bind_integer_constant(cls, "", "DISTANCE_JOINT_FLAG_ENABLE_MOTOR", Box3DDistanceJointImpl3D::FLAG_ENABLE_MOTOR);

	ClassDB::bind_integer_constant(cls, "", "MOTOR_JOINT_PARAM_MAX_VELOCITY_FORCE", Box3DMotorJointImpl3D::PARAM_MAX_VELOCITY_FORCE);
	ClassDB::bind_integer_constant(cls, "", "MOTOR_JOINT_PARAM_MAX_VELOCITY_TORQUE", Box3DMotorJointImpl3D::PARAM_MAX_VELOCITY_TORQUE);
	ClassDB::bind_integer_constant(cls, "", "MOTOR_JOINT_PARAM_LINEAR_HERTZ", Box3DMotorJointImpl3D::PARAM_LINEAR_HERTZ);
	ClassDB::bind_integer_constant(cls, "", "MOTOR_JOINT_PARAM_LINEAR_DAMPING", Box3DMotorJointImpl3D::PARAM_LINEAR_DAMPING);
	ClassDB::bind_integer_constant(cls, "", "MOTOR_JOINT_PARAM_MAX_SPRING_FORCE", Box3DMotorJointImpl3D::PARAM_MAX_SPRING_FORCE);
	ClassDB::bind_integer_constant(cls, "", "MOTOR_JOINT_PARAM_ANGULAR_HERTZ", Box3DMotorJointImpl3D::PARAM_ANGULAR_HERTZ);
	ClassDB::bind_integer_constant(cls, "", "MOTOR_JOINT_PARAM_ANGULAR_DAMPING", Box3DMotorJointImpl3D::PARAM_ANGULAR_DAMPING);
	ClassDB::bind_integer_constant(cls, "", "MOTOR_JOINT_PARAM_MAX_SPRING_TORQUE", Box3DMotorJointImpl3D::PARAM_MAX_SPRING_TORQUE);
}

void Box3DPhysicsServer3D::_hinge_joint_set_param(const RID& p_joint, PhysicsServer3D::HingeJointParam p_param, double p_value) {
	auto* joint = dynamic_cast<Box3DHingeJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	joint->set_param(p_param, p_value);
}

double Box3DPhysicsServer3D::_hinge_joint_get_param(const RID& p_joint, PhysicsServer3D::HingeJointParam p_param) const {
	auto* joint = dynamic_cast<Box3DHingeJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_param(p_param);
}

void Box3DPhysicsServer3D::_hinge_joint_set_flag(const RID& p_joint, PhysicsServer3D::HingeJointFlag p_flag, bool p_enabled) {
	auto* joint = dynamic_cast<Box3DHingeJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	joint->set_flag(p_flag, p_enabled);
}

bool Box3DPhysicsServer3D::_hinge_joint_get_flag(const RID& p_joint, PhysicsServer3D::HingeJointFlag p_flag) const {
	auto* joint = dynamic_cast<Box3DHingeJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, false);
	return joint->get_flag(p_flag);
}

void Box3DPhysicsServer3D::_joint_make_slider(const RID& p_joint, const RID& p_body_a, const Transform3D& p_local_ref_a, const RID& p_body_b, const Transform3D& p_local_ref_b) {
	_joint_clear(p_joint);

	Box3DBodyImpl3D* body_a = body_owner.get_or_null(p_body_a);
	Box3DBodyImpl3D* body_b = body_owner.get_or_null(p_body_b);
	ERR_FAIL_NULL(body_a);
	ERR_FAIL_NULL(body_b);

	// Box3D's prismatic joint slides along the LOCAL X axis of local frame A, matching
	// Godot's SliderJoint3D convention (which also slides along local X), so no axis
	// remap is required here.
	auto* joint = memnew(Box3DSliderJointImpl3D(body_a, body_b, p_local_ref_a, p_local_ref_b));
	joint->set_rid(p_joint);
	joint_owner.replace(p_joint, joint);
	joint->rebuild();
}

void Box3DPhysicsServer3D::_slider_joint_set_param(const RID& p_joint, PhysicsServer3D::SliderJointParam p_param, double p_value) {
	auto* joint = dynamic_cast<Box3DSliderJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL(joint);
	joint->set_param(p_param, p_value);
}

double Box3DPhysicsServer3D::_slider_joint_get_param(const RID& p_joint, PhysicsServer3D::SliderJointParam p_param) const {
	auto* joint = dynamic_cast<Box3DSliderJointImpl3D*>(joint_owner.get_or_null(p_joint));
	ERR_FAIL_NULL_V(joint, 0.0);
	return joint->get_param(p_param);
}

void Box3DPhysicsServer3D::_joint_make_cone_twist(const RID& p_joint, const RID& p_body_a, const Transform3D& p_local_ref_a, const RID& p_body_b, const Transform3D& p_local_ref_b) {
	ERR_FAIL_MSG("Box3D: ConeTwistJoint3D is not supported in this version of the Box3D extension.");
}

void Box3DPhysicsServer3D::_cone_twist_joint_set_param(const RID& p_joint, PhysicsServer3D::ConeTwistJointParam p_param, double p_value) {
}

double Box3DPhysicsServer3D::_cone_twist_joint_get_param(const RID& p_joint, PhysicsServer3D::ConeTwistJointParam p_param) const {
	return 0.0;
}

void Box3DPhysicsServer3D::_joint_make_generic_6dof(const RID& p_joint, const RID& p_body_a, const Transform3D& p_local_ref_a, const RID& p_body_b, const Transform3D& p_local_ref_b) {
	ERR_FAIL_MSG("Box3D: Generic6DOFJoint3D is not supported in this version of the Box3D extension.");
}

void Box3DPhysicsServer3D::_generic_6dof_joint_set_param(const RID& p_joint, Vector3::Axis p_axis, PhysicsServer3D::G6DOFJointAxisParam p_param, double p_value) {
}

double Box3DPhysicsServer3D::_generic_6dof_joint_get_param(const RID& p_joint, Vector3::Axis p_axis, PhysicsServer3D::G6DOFJointAxisParam p_param) const {
	return 0.0;
}

void Box3DPhysicsServer3D::_generic_6dof_joint_set_flag(const RID& p_joint, Vector3::Axis p_axis, PhysicsServer3D::G6DOFJointAxisFlag p_flag, bool p_enable) {
}

bool Box3DPhysicsServer3D::_generic_6dof_joint_get_flag(const RID& p_joint, Vector3::Axis p_axis, PhysicsServer3D::G6DOFJointAxisFlag p_flag) const {
	return false;
}

PhysicsServer3D::JointType Box3DPhysicsServer3D::_joint_get_type(const RID& p_joint) const {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	if (joint == nullptr) {
		return PhysicsServer3D::JOINT_TYPE_MAX;
	}
	return joint->get_type();
}

void Box3DPhysicsServer3D::_joint_set_solver_priority(const RID& p_joint, int32_t p_priority) {
	// No Box3D equivalent.
}

int32_t Box3DPhysicsServer3D::_joint_get_solver_priority(const RID& p_joint) const {
	return 0;
}

void Box3DPhysicsServer3D::_joint_disable_collisions_between_bodies(const RID& p_joint, bool p_disable) {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	ERR_FAIL_NULL(joint);
	joint->set_collision_disabled(p_disable);
}

bool Box3DPhysicsServer3D::_joint_is_disabled_collisions_between_bodies(const RID& p_joint) const {
	Box3DJointImpl3D* joint = joint_owner.get_or_null(p_joint);
	ERR_FAIL_NULL_V(joint, false);
	return joint->is_collision_disabled();
}

// --- Soft bodies (non-goal) ---

RID Box3DPhysicsServer3D::_soft_body_create() {
	ERR_FAIL_V_MSG(RID(), "Box3D: SoftBody3D is not supported in this version of the Box3D extension.");
}

void Box3DPhysicsServer3D::_soft_body_update_rendering_server(const RID& p_body, PhysicsServer3DRenderingServerHandler* p_rendering_server_handler) {
}

void Box3DPhysicsServer3D::_soft_body_set_space(const RID& p_body, const RID& p_space) {
}

RID Box3DPhysicsServer3D::_soft_body_get_space(const RID& p_body) const {
	return RID();
}

void Box3DPhysicsServer3D::_soft_body_set_ray_pickable(const RID& p_body, bool p_enable) {
}

void Box3DPhysicsServer3D::_soft_body_set_collision_layer(const RID& p_body, uint32_t p_layer) {
}

uint32_t Box3DPhysicsServer3D::_soft_body_get_collision_layer(const RID& p_body) const {
	return 0;
}

void Box3DPhysicsServer3D::_soft_body_set_collision_mask(const RID& p_body, uint32_t p_mask) {
}

uint32_t Box3DPhysicsServer3D::_soft_body_get_collision_mask(const RID& p_body) const {
	return 0;
}

void Box3DPhysicsServer3D::_soft_body_add_collision_exception(const RID& p_body, const RID& p_excepted_body) {
}

void Box3DPhysicsServer3D::_soft_body_remove_collision_exception(const RID& p_body, const RID& p_excepted_body) {
}

TypedArray<RID> Box3DPhysicsServer3D::_soft_body_get_collision_exceptions(const RID& p_body) const {
	return TypedArray<RID>();
}

void Box3DPhysicsServer3D::_soft_body_set_state(const RID& p_body, PhysicsServer3D::BodyState p_state, const Variant& p_variant) {
}

Variant Box3DPhysicsServer3D::_soft_body_get_state(const RID& p_body, PhysicsServer3D::BodyState p_state) const {
	return Variant();
}

void Box3DPhysicsServer3D::_soft_body_set_transform(const RID& p_body, const Transform3D& p_transform) {
}

void Box3DPhysicsServer3D::_soft_body_set_simulation_precision(const RID& p_body, int32_t p_simulation_precision) {
}

int32_t Box3DPhysicsServer3D::_soft_body_get_simulation_precision(const RID& p_body) const {
	return 0;
}

void Box3DPhysicsServer3D::_soft_body_set_total_mass(const RID& p_body, double p_total_mass) {
}

double Box3DPhysicsServer3D::_soft_body_get_total_mass(const RID& p_body) const {
	return 0.0;
}

void Box3DPhysicsServer3D::_soft_body_set_linear_stiffness(const RID& p_body, double p_stiffness) {
}

double Box3DPhysicsServer3D::_soft_body_get_linear_stiffness(const RID& p_body) const {
	return 0.0;
}

void Box3DPhysicsServer3D::_soft_body_set_pressure_coefficient(const RID& p_body, double p_pressure_coefficient) {
}

double Box3DPhysicsServer3D::_soft_body_get_pressure_coefficient(const RID& p_body) const {
	return 0.0;
}

void Box3DPhysicsServer3D::_soft_body_set_damping_coefficient(const RID& p_body, double p_damping_coefficient) {
}

double Box3DPhysicsServer3D::_soft_body_get_damping_coefficient(const RID& p_body) const {
	return 0.0;
}

void Box3DPhysicsServer3D::_soft_body_set_drag_coefficient(const RID& p_body, double p_drag_coefficient) {
}

double Box3DPhysicsServer3D::_soft_body_get_drag_coefficient(const RID& p_body) const {
	return 0.0;
}

void Box3DPhysicsServer3D::_soft_body_set_mesh(const RID& p_body, const RID& p_mesh) {
}

AABB Box3DPhysicsServer3D::_soft_body_get_bounds(const RID& p_body) const {
	return AABB();
}

void Box3DPhysicsServer3D::_soft_body_move_point(const RID& p_body, int32_t p_point_index, const Vector3& p_global_position) {
}

Vector3 Box3DPhysicsServer3D::_soft_body_get_point_global_position(const RID& p_body, int32_t p_point_index) const {
	return Vector3();
}

void Box3DPhysicsServer3D::_soft_body_remove_all_pinned_points(const RID& p_body) {
}

void Box3DPhysicsServer3D::_soft_body_pin_point(const RID& p_body, int32_t p_point_index, bool p_pin) {
}

bool Box3DPhysicsServer3D::_soft_body_is_point_pinned(const RID& p_body, int32_t p_point_index) const {
	return false;
}

// --- Lifecycle ---

void Box3DPhysicsServer3D::_free_rid(const RID& p_rid) {
	// Joints and shapes free before bodies/areas that reference them; bodies/areas free
	// before shapes they hold (mirrors JoltPhysicsServer3DExtension::_free_rid's ordering).
	if (Box3DJointImpl3D* joint = joint_owner.get_or_null(p_rid)) {
		memdelete(joint);
		joint_owner.free(p_rid);
		return;
	}

	if (Box3DBodyImpl3D* body = body_owner.get_or_null(p_rid)) {
		if (Box3DSpace3D* space = body->get_space()) {
			space->unregister_body(body);
		}
		memdelete(body);
		body_owner.free(p_rid);
		return;
	}

	if (Box3DAreaImpl3D* area = area_owner.get_or_null(p_rid)) {
		if (Box3DSpace3D* space = area->get_space()) {
			space->unregister_area(area);
		}
		memdelete(area);
		area_owner.free(p_rid);
		return;
	}

	if (Box3DShapeImpl3D* shape = shape_owner.get_or_null(p_rid)) {
		memdelete(shape);
		shape_owner.free(p_rid);
		return;
	}

	if (Box3DSpace3D* space = space_owner.get_or_null(p_rid)) {
		Box3DAreaImpl3D* default_area = space->get_default_area();
		if (default_area != nullptr) {
			const RID default_area_rid = default_area->get_rid();
			space->unregister_area(default_area);
			memdelete(default_area);
			area_owner.free(default_area_rid);
		}
		active_spaces.erase(space);
		memdelete(space);
		space_owner.free(p_rid);
		return;
	}
}

void Box3DPhysicsServer3D::_set_active(bool p_active) {
	active = p_active;
}

void Box3DPhysicsServer3D::_init() {
}

void Box3DPhysicsServer3D::_step(double p_step) {
	if (!active) {
		return;
	}
	for (Box3DSpace3D* space : active_spaces) {
		space->step((float)p_step);
	}
}

void Box3DPhysicsServer3D::_sync() {
}

void Box3DPhysicsServer3D::_flush_queries() {
	if (!active) {
		return;
	}
	for (Box3DSpace3D* space : active_spaces) {
		space->flush_queries();
	}
}

void Box3DPhysicsServer3D::_end_sync() {
}

void Box3DPhysicsServer3D::_finish() {
}

bool Box3DPhysicsServer3D::_is_flushing_queries() const {
	for (Box3DSpace3D* space : active_spaces) {
		if (space->is_flushing_queries()) {
			return true;
		}
	}
	return false;
}

int32_t Box3DPhysicsServer3D::_get_process_info(PhysicsServer3D::ProcessInfo p_process_info) {
	return 0;
}
