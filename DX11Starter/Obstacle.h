#pragma once
#include "Entity.h"
class Obstacle :
	public Entity
{
public:
	Obstacle(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~Obstacle();
};

