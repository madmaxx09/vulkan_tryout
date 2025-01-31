#include "game_object.hpp"

namespace wind
{
	glm::mat4 TransformComponent::mat4() 
	{ //fourth dimension is for homegeneous coordinate
		auto transform = glm::translate(glm::mat4{1.f}, translation);

		transform = glm::rotate(transform, rotation.y, {0.f, 1.f, 0.f});
		transform = glm::rotate(transform, rotation.x, {1.f, 0.f, 0.f});
		transform = glm::rotate(transform, rotation.z, {0.f, 0.f, 1.f});

		transform = glm::scale(transform, glm::vec3(scale));
		return transform;
	}
}