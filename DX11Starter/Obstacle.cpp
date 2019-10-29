#include "Obstacle.h"

Obstacle::Obstacle(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material):Entity(mesh, material)
{
	tag = "Obstacle";
}

Obstacle::~Obstacle()
{
}
