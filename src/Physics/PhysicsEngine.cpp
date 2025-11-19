#include "PhysicsEngine.h"

void PhysicsEngine::Update(double delta) {
	for (auto& e : _entities) {

		e->Update(delta);
		
		//if (e->CollidesWithVoxels)
		ResolveVoxelCollisions(delta, e.get());
		//else
		//	e->SetPosition(e->GetPosition() + e->Velocity * (float)delta);
	}
}

void PhysicsEngine::ResolveVoxelCollisions(double delta, Entity* e) {
	e->IsOnFloor = false;
	//Z Pass
	e->SetPosition(e->GetPosition() + glm::vec3(0.0f, 0.0f, e->Velocity.z * delta));
	glm::vec3 hitboxPos = e->Shape->Origin + e->GetPosition();

	int startX = (int)std::floor(hitboxPos.x - e->Shape->Size.x / 2.0f + 0.001f);
	int endX = (int)std::ceil(hitboxPos.x + e->Shape->Size.x / 2.0f - 0.001f) - 1;

	int startY = (int)std::floor(hitboxPos.y - e->Shape->Size.y / 2.0f + 0.001f);
	int endY = (int)std::ceil(hitboxPos.y + e->Shape->Size.y / 2.0f - 0.001f) - 1;

	//adding a tiny error margin to account for floating point precision errors
	int startZ = (int)std::floor(hitboxPos.z - e->Shape->Size.z / 2.0f + 0.001f);
	int endZ = (int)std::ceil(hitboxPos.z + e->Shape->Size.z / 2.0f - 0.001f) - 1;
	for (int x = startX; x <= endX; x++) {
		for (int y = startY; y <= endY; y++) {
			for (int z = startZ; z <= endZ; z++) {
				if (_chunkManager->GetGlobalBlock(glm::vec3(x, y, z)) == 0)
					continue;
				glm::vec3 position = e->GetPosition();
				glm::vec3 hitbox = position + e->Shape->Origin;
				float halfZ = e->Shape->Size.z * 0.5f;
				bool intersectsZ = (hitbox.z - halfZ) < (z + 1.0f) && (z) < (hitbox.z + halfZ);
				if (!intersectsZ)
					continue;

				double distanceZ = (double)hitbox.z - (z + 0.5);
				double penetrationZ = (double)(e->Shape->Size.z / 2.0) + 0.5f - std::abs(distanceZ);
				penetrationZ = penetrationZ <= 0 ? 0.0f : distanceZ < 0 ? -penetrationZ : penetrationZ;
				if (penetrationZ < 0) {
					position.z = z - e->Shape->Size.z / 2.0f - e->Shape->Origin.z;
				}
				else if (penetrationZ > 0) {
					position.z = z + 1.0 + e->Shape->Size.z / 2.0f - e->Shape->Origin.z;
				}
				e->SetPosition(position);
				e->IsOnFloor = true;
				e->Velocity.z = 0.0f;
			}
		}
	}

	//Y pass
	e->CollisionY = false;
	e->SetPosition(e->GetPosition() + glm::vec3(0.0f, e->Velocity.y * delta, 0.0f));
	hitboxPos = e->Shape->Origin + e->GetPosition();

	startX = (int)std::floor(hitboxPos.x - e->Shape->Size.x / 2.0f + 0.001f);
	endX = (int)std::ceil(hitboxPos.x + e->Shape->Size.x / 2.0f - 0.001f) - 1;

	startY = (int)std::floor(hitboxPos.y - e->Shape->Size.y / 2.0f + 0.001f);
	endY = (int)std::ceil(hitboxPos.y + e->Shape->Size.y / 2.0f - 0.001f) - 1;

	//adding a tiny error margin to account for floating point precision errors
	startZ = (int)std::floor(hitboxPos.z - e->Shape->Size.z / 2.0f + 0.001f);
	endZ = (int)std::ceil(hitboxPos.z + e->Shape->Size.z / 2.0f - 0.001f) - 1;
	for (int x = startX; x <= endX; x++) {
		for (int y = startY; y <= endY; y++) {
			for (int z = startZ; z <= endZ; z++) {
				if (_chunkManager->GetGlobalBlock(glm::vec3(x, y, z)) == 0)
					continue;

				glm::vec3 position = e->GetPosition();
				glm::vec3 hitbox = position + e->Shape->Origin;
				float halfY = e->Shape->Size.y * 0.5f;
				bool intersectsY = (hitbox.y - halfY) < (y + 1.0f) && (y) < (hitbox.y + halfY);
				if (!intersectsY)
					continue;

				double distanceY = (double)hitbox.y - (y + 0.5);
				double penetrationY = (e->Shape->Size.y / 2.0f) + 0.5f - std::abs(distanceY);
				penetrationY = penetrationY <= 0 ? 0.0f : distanceY < 0 ? -penetrationY : penetrationY;
				if (penetrationY < 0) {
					position.y = y - e->Shape->Size.y / 2.0f;
				}
				else if (penetrationY > 0) {
					position.y = y + 1 + e->Shape->Size.y / 2.0f;
				}
				e->SetPosition(position);
				e->Velocity.y = 0.0f;
				e->CollisionY = true;
			}
		}
	}

	//X Pass
	e->CollisionX = false;
	e->SetPosition(e->GetPosition() + glm::vec3(e->Velocity.x * delta, 0.0f, 0.0f));
	hitboxPos = e->Shape->Origin + e->GetPosition();

	startX = (int)std::floor(hitboxPos.x - e->Shape->Size.x / 2.0f + 0.001f);
	endX = (int)std::ceil(hitboxPos.x + e->Shape->Size.x / 2.0f - 0.001f) - 1;

	startY = (int)std::floor(hitboxPos.y - e->Shape->Size.y / 2.0f + 0.001f);
	endY = (int)std::ceil(hitboxPos.y + e->Shape->Size.y / 2.0f - 0.001f) - 1;

	//adding a tiny error margin to account for floating point precision errors
	startZ = (int)std::floor(hitboxPos.z - e->Shape->Size.z / 2.0f + 0.001f);
	endZ = (int)std::ceil(hitboxPos.z + e->Shape->Size.z / 2.0f - 0.001f) - 1;
	for (int x = startX; x <= endX; x++) {
		for (int y = startY; y <= endY; y++) {
			for (int z = startZ; z <= endZ; z++) {
				if (_chunkManager->GetGlobalBlock(glm::vec3(x, y, z)) == 0)
					continue;
				glm::vec3 position = e->GetPosition();
				glm::vec3 hitbox = position + e->Shape->Origin;
				float halfX = e->Shape->Size.x * 0.5f;
				bool intersectsX = (hitbox.x - halfX) < (x + 1.0f) && (x) < (hitbox.x + halfX);
				if (!intersectsX)
					continue;

				double distanceX = (double)hitbox.x - (x + 0.5);
				double penetrationX = (double)(e->Shape->Size.x / 2.0) + 0.5 - std::abs(distanceX);
				penetrationX = penetrationX <= 0 ? 0.0f : distanceX < 0 ? -penetrationX : penetrationX;
				if (penetrationX < 0) {
					position.x = x - e->Shape->Size.x / 2.0f;
				}
				else if (penetrationX > 0) {
					position.x = x + 1 + e->Shape->Size.x / 2.0f;
					position.x = x + 1 + e->Shape->Size.x / 2.0f;
				}
				e->SetPosition(position);
				e->Velocity.x = 0.0f;
				e->CollisionX = true;
			}
		}
	}
}

void PhysicsEngine::ResolveEntityCollisions(double delta, Entity* e1, Entity* e2) {
	return;
}