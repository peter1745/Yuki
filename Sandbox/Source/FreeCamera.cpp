#include "FreeCamera.hpp"

#include <Yuki/Core/Logging.hpp>
#include <Yuki/Math/Math.hpp>

FreeCamera::FreeCamera(Yuki::GenericWindow* InInputWindow)
	: m_InputWindow(InInputWindow)
{
	m_ViewMatrix.SetIdentity();

	m_Position = { 0.0f, 0.0f, 0.0f };
	m_Forward = { 0.0f, 0.0f, -1.0f };
	m_Right = { 1.0f, 0.0f, 0.0f };
	m_Up = { 0.0f, 1.0f, 0.0f };

	m_Yaw = 0.0f;
	m_Pitch = 0.0f;

	UpdateViewMatrix();
}

void FreeCamera::Update(float InDeltaTime)
{
	if (m_InputWindow->IsMouseButtonPressed(Yuki::MouseButton::Right))
	{
		m_InputWindow->SetCursorState(Yuki::CursorState::Locked);
	}
	else
	{
		(void)m_InputWindow->GetRawMouseDeltaX();
		(void)m_InputWindow->GetRawMouseDeltaY();
		m_InputWindow->SetCursorState(Yuki::CursorState::Normal);
		return;
	}

	if (m_InputWindow->IsKeyPressed(Yuki::KeyCode::W))
		m_Position -= m_Forward * m_MovementSpeed * InDeltaTime;

	if (m_InputWindow->IsKeyPressed(Yuki::KeyCode::S))
		m_Position += m_Forward * m_MovementSpeed * InDeltaTime;

	if (m_InputWindow->IsKeyPressed(Yuki::KeyCode::D))
		m_Position -= m_Right * m_MovementSpeed * InDeltaTime;

	if (m_InputWindow->IsKeyPressed(Yuki::KeyCode::A))
		m_Position += m_Right * m_MovementSpeed * InDeltaTime;

	if (m_InputWindow->IsKeyPressed(Yuki::KeyCode::LeftShift))
		m_Position -= m_Up * m_MovementSpeed * InDeltaTime;

	if (m_InputWindow->IsKeyPressed(Yuki::KeyCode::Space))
		m_Position += m_Up * m_MovementSpeed * InDeltaTime;

	m_Yaw += m_InputWindow->GetRawMouseDeltaX() * 0.001f;
	m_Pitch += m_InputWindow->GetRawMouseDeltaY() * 0.001f;

	if (m_Pitch > Yuki::Math::Radians(89.0f))
		m_Pitch = Yuki::Math::Radians(89.0f);
	else if (m_Pitch < Yuki::Math::Radians(-89.0f))
		m_Pitch = Yuki::Math::Radians(-89.0f);

	UpdateViewMatrix();
}

void FreeCamera::UpdateViewMatrix()
{
	m_Forward.X = Yuki::Math::Cos(m_Yaw) * Yuki::Math::Cos(m_Pitch);
	m_Forward.Y = Yuki::Math::Sin(m_Pitch);
	m_Forward.Z = Yuki::Math::Sin(m_Yaw) * Yuki::Math::Cos(m_Pitch);
	m_Forward.Normalize();

	m_Right = m_Forward.Cross({ 0.0f, 1.0f, 0.0f }).Normalized();
	m_Up = m_Right.Cross(m_Forward).Normalized();

	m_ViewMatrix = Yuki::Math::Mat4::LookAt(m_Position, m_Position + m_Forward, m_Up);
}
