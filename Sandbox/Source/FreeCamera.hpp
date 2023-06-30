#include <Yuki/Math/Vec3.hpp>
#include <Yuki/Math/Mat4.hpp>

#include <Yuki/Core/GenericWindow.hpp>

class FreeCamera
{
public:
    FreeCamera(Yuki::GenericWindow* InInputWindow);

    void Update(float InDeltaTime);

	const Yuki::Math::Mat4& GetViewMatrix() const { return m_ViewMatrix; }

	float& GetMovementSpeed() { return m_MovementSpeed; }

private:
    void UpdateViewMatrix();

private:
	Yuki::GenericWindow* m_InputWindow = nullptr;

	float m_MovementSpeed = 10.0f;

    Yuki::Math::Vec3 m_Position;
    Yuki::Math::Vec3 m_Forward;
    Yuki::Math::Vec3 m_Right;
    Yuki::Math::Vec3 m_Up;

    float m_Yaw;
    float m_Pitch;

    Yuki::Math::Mat4 m_ViewMatrix;
};
