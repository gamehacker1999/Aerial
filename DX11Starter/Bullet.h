#pragma once
#include "Entity.h"
#include<DirectXMath.h>
#define MAX_LIFETIME 4
using namespace DirectX;
class Bullet :
	public Entity
{
private:
	float lifeTime;
public:
	bool isActive;
	Bullet(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~Bullet();
	void Update(float deltaTime) override;
	void Reset();
};

