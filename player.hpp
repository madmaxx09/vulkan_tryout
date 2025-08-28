#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "game_object.hpp"

typedef struct s_projectiles
{
	glm::vec3 position;
	int identifier;
} t_projectiles;

namespace wind
{
	class Player
	{
		public:
			Player(LveGameObject &entity);
			~Player();

			
			LveGameObject physicalEntity;
		private:
			glm::vec3 position;
			std::vector<t_projectiles> projectiles;

	};
}