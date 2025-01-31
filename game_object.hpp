#pragma once

#include "model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace wind
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		float scale; 
		glm::vec3 rotation{};
		//object orientation in 3D is represented by angles on each of the 3D axis
		//look for quaternions encoding later (probly state of art atm)
		//here we will use tait-bryan Y X Z representation
		glm::mat4 mat4();
	};
	
	class LveGameObject
	{
		public:
			LveGameObject(const LveGameObject&) = delete;
			LveGameObject& operator=(const LveGameObject&) = delete;
			LveGameObject(LveGameObject &&) = default;
			LveGameObject& operator=(LveGameObject &&) = default;

			using id_t = unsigned int;
			static LveGameObject createGameObject()	
			{
				static id_t currentId = 0;
				return LveGameObject{currentId++};
			}

			const id_t getId() { return (id) ; }


			std::shared_ptr<LveModel> model{};
			glm::vec3 color{};
			TransformComponent transform{};

		private:
			LveGameObject(id_t objId) : id{objId} {}

			id_t	id;
	};
}