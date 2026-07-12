#include "box3d_concave_polygon_shape_impl_3d.hpp"

#include "../misc/type_conversions.hpp"

#include <box3d/collision.h>

Box3DConcavePolygonShapeImpl3D::~Box3DConcavePolygonShapeImpl3D() {
	if (mesh != nullptr) {
		b3DestroyMesh(mesh);
		mesh = nullptr;
	}
	for (b3MeshData* transformed : transformed_meshes) {
		b3DestroyMesh(transformed);
	}
	transformed_meshes.clear();
}

Variant Box3DConcavePolygonShapeImpl3D::get_data() const {
	Dictionary data;
	data["faces"] = faces;
	data["backface_collision"] = false;
	return data;
}

void Box3DConcavePolygonShapeImpl3D::set_data(const Variant& p_data) {
	if (p_data.get_type() == Variant::DICTIONARY) {
		const Dictionary data = p_data;
		ERR_FAIL_COND(!data.has("faces"));
		faces = data["faces"];
	} else {
		ERR_FAIL_COND(p_data.get_type() != Variant::PACKED_VECTOR3_ARRAY);
		faces = p_data;
	}
	_rebuild_mesh();
}

void Box3DConcavePolygonShapeImpl3D::_rebuild_mesh() {
	if (mesh != nullptr) {
		b3DestroyMesh(mesh);
		mesh = nullptr;
	}
	for (b3MeshData* transformed : transformed_meshes) {
		b3DestroyMesh(transformed);
	}
	transformed_meshes.clear();

	const int face_count = faces.size();
	if (face_count < 3 || face_count % 3 != 0) {
		return;
	}

	Vector3 min_point = faces[0];
	Vector3 max_point = faces[0];
	for (int i = 0; i < face_count; i++) {
		min_point = min_point.min(faces[i]);
		max_point = max_point.max(faces[i]);
	}
	aabb = AABB(min_point, max_point - min_point);

	mesh = _cook_mesh(Transform3D());
}

b3MeshData* Box3DConcavePolygonShapeImpl3D::_cook_mesh(const Transform3D& p_transform) const {
	const int face_count = faces.size();
	if (face_count < 3 || face_count % 3 != 0) {
		return nullptr;
	}

	const int triangle_count = face_count / 3;

	LocalVector<b3Vec3> vertices;
	vertices.resize(face_count);

	LocalVector<int32_t> indices;
	indices.resize(face_count);

	for (int i = 0; i < face_count; i++) {
		vertices[i] = godot_to_b3(p_transform.xform(faces[i]));
		indices[i] = i;
	}

	b3MeshDef def = {};
	def.vertices = vertices.ptr();
	def.indices = indices.ptr();
	def.materialIndices = nullptr;
	def.weldTolerance = 0.0f;
	def.vertexCount = face_count;
	def.triangleCount = triangle_count;
	def.weldVertices = false;
	def.useMedianSplit = false;
	def.identifyEdges = false;

	return b3CreateMesh(&def, nullptr, 0);
}

const b3MeshData* Box3DConcavePolygonShapeImpl3D::get_mesh_transformed(const Transform3D& p_local) {
	if (p_local.origin == Vector3() && p_local.basis.is_equal_approx(Basis())) {
		return mesh;
	}
	b3MeshData* transformed = _cook_mesh(p_local);
	if (transformed != nullptr) {
		transformed_meshes.push_back(transformed);
	}
	return transformed;
}
