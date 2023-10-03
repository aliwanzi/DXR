#pragma once
#include "Common.h"
using namespace DirectX;

class Camera
{
public:
	static Camera& GetInstance();

	void SetPosition(float x, float y, float z);
	void SetLens(float fovY, float aspect, float zn, float zf);

	XMMATRIX GetView()const;
	XMMATRIX GetProj()const;

	void Strafe(float d);
	void Walk(float d);

	void Pitch(float angle);
	void RotateY(float angle);

	void UpdateViewMatrix();

	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnKeyboardInput();
	void Auto();
protected:
	Camera();
	~Camera() = default;
	Camera(const Camera&) = delete;
	const Camera& operator=(const Camera&) = delete;

private:

	POINT lastMousePos;

	XMFLOAT3 mPosition = { 20.0f, 1.f, 0.0f };
	XMFLOAT3 mRight = { 0.0f, 0.0f, 1.0f };
	XMFLOAT3 mUp = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 mLook = { -1.0f, 0.0f, 0.0f };
	XMFLOAT3 eyeAngle = { 0.0f, 0.0f, 1.0f };

	float mNearZ = 0.0f;
	float mFarZ = 0.0f;
	float mAspect = 0.0f;
	float mFovY = 0.0f;

	bool mViewDirty = true;

	XMFLOAT4X4 mView =
	{
		1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.0f,0.0f,0.0f,1.0f,
	};
	XMFLOAT4X4 mProj =
	{
		1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.0f,0.0f,0.0f,1.0f,
	};
};


