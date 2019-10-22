#include "Ship.h"

Ship::Ship(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material):Entity(mesh,material)
{
	XMStoreFloat4(&originalRotation, XMQuaternionIdentity());
	health = 5;
	tag = "Player";
}

Ship::~Ship()
{
}

void Ship::Update(float deltaTime)
{
	position.z += 2 * deltaTime;
	SetPosition(position);

	GetInput(deltaTime);

	auto curRot = XMLoadFloat4(&rotation);
	auto originalRot = XMLoadFloat4(&originalRotation);
	auto springRot = XMQuaternionSlerp(curRot, originalRot, 2.0f * deltaTime);

	XMStoreFloat4(&rotation, springRot);
	SetRotation(rotation);
}

void Ship::GetInput(float deltaTime)
{


	if (GetAsyncKeyState('W') & 0x8000)
	{
		XMVECTOR newRotationTemp = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), 3.14159f / 6 * deltaTime);
		position.y += 2 * deltaTime;
		XMFLOAT4 newRot;
		XMStoreFloat4(&newRot, newRotationTemp);

		auto curRot = this->rotation;

		auto finalRot = XMQuaternionMultiply(newRotationTemp, XMLoadFloat4(&curRot));
		XMStoreFloat4(&curRot, finalRot);
		SetRotation(curRot);
	}

	//move left
	if (GetAsyncKeyState('S') & 0x8000)
	{
		XMVECTOR newRotationTemp = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), -3.14159f / 6 * deltaTime);
		XMFLOAT4 newRot;
		position.y -= 2 * deltaTime;
		XMStoreFloat4(&newRot, newRotationTemp);

		auto curRot = this->rotation;

		auto finalRot = XMQuaternionMultiply(newRotationTemp, XMLoadFloat4(&curRot));
		XMStoreFloat4(&curRot, finalRot);
		SetRotation(curRot);
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		XMVECTOR newRotationTemp = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), 3.14159f / 6 * deltaTime);
		XMFLOAT4 newRot;
		position.x += 2 * deltaTime;
		XMStoreFloat4(&newRot, newRotationTemp);

		auto curRot = this->rotation;

		auto finalRot = XMQuaternionMultiply(newRotationTemp, XMLoadFloat4(&curRot));
		XMStoreFloat4(&curRot, finalRot);
		SetRotation(curRot);
	}

	//move left
	if (GetAsyncKeyState('A') & 0x8000)
	{
		XMVECTOR newRotationTemp = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), -3.14159f / 6 * deltaTime);
		XMFLOAT4 newRot;
		position.x -= 2 * deltaTime;
		XMStoreFloat4(&newRot, newRotationTemp);

		auto curRot = this->rotation;

		auto finalRot = XMQuaternionMultiply(newRotationTemp, XMLoadFloat4(&curRot));
		XMStoreFloat4(&curRot, finalRot);
		SetRotation(curRot);
	}
}

bool Ship::IsColliding(Entity* other)
{
	if (other->GetTag() == "Obstacle")
	{
		health += 1;
		return true;
	}

	return false;
}

void Ship::SetOriginalRotation(XMFLOAT4 originalRotation)
{
	this->originalRotation = originalRotation;
}
