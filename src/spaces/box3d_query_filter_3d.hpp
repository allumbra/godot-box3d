#pragma once

#include <godot_cpp/classes/physics_direct_space_state3d_extension.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/rid.hpp>

#include <box3d/types.h>

using namespace godot;

// Builds a b3QueryFilter (direct 32-bit zero-extended layer/mask pass-through, per the
// plan) plus a side-channel RID exclude-set, since Box3D's own filter has no native
// per-query RID-exclude list. Callers check should_exclude() from inside their
// b3*ResultFcn/b3CastResultFcn callback alongside whatever the callback itself needs.
struct Box3DQueryFilter3D {
	b3QueryFilter filter = b3DefaultQueryFilter();
	HashSet<RID> exclude;
	bool collide_with_bodies = true;
	bool collide_with_areas = false;
	// When set, per-query engine-side exclusions (PhysicsRayQueryParameters3D.exclude
	// et al.) are honored: Godot never passes the exclude array into the extension
	// query virtuals — it holds the list and the extension must ask back per candidate.
	PhysicsDirectSpaceState3DExtension* state = nullptr;

	Box3DQueryFilter3D() = default;

	Box3DQueryFilter3D(uint32_t p_collision_mask, bool p_collide_with_bodies, bool p_collide_with_areas) :
			collide_with_bodies(p_collide_with_bodies), collide_with_areas(p_collide_with_areas) {
		filter = b3DefaultQueryFilter();
		filter.maskBits = (uint64_t)p_collision_mask;
	}

	bool should_exclude(const RID& p_rid) const {
		if (exclude.has(p_rid)) {
			return true;
		}
		return state != nullptr && state->is_body_excluded_from_query(p_rid);
	}
};
