#include "SandboxECS.h"

#include "imgui/imgui.h"

#include <box2d/b2_body.h>

using namespace OverEngine;

SandboxECS::SandboxECS()
	: Layer("SandboxECS"), m_CameraMovementDirection(0.0f)
{
	OE_PROFILE_FUNCTION();

#pragma region Input
	auto actionMap = InputActionMap::Create();

	InputAction CameraMovement(InputActionType::Button, {
		{
			{KeyCode::A}, {KeyCode::D},
			{KeyCode::S}, {KeyCode::W}
		},
		{
			{KeyCode::Left}, {KeyCode::Right},
			{KeyCode::Down}, {KeyCode::Up}
		}
	});
	CameraMovement.AddCallBack([&](InputAction::TriggerInfo& info) {
		m_CameraMovementDirection = info.ReadValue<Vector2>();
	});
	actionMap->AddAction(CameraMovement);

	InputAction CameraRotation(InputActionType::Button, {
		{ {KeyCode::Q}, {KeyCode::E} }
	});
	CameraRotation.AddCallBack([&](InputAction::TriggerInfo& info) {
		m_CameraRotationDirection = -info.x;
	});
	actionMap->AddAction(CameraRotation);

	InputAction EscapeKeyAction(InputActionType::Button, {
		{ {KeyCode::Escape, true, false} }
	});
	EscapeKeyAction.AddCallBack([&](InputAction::TriggerInfo& info) {
		m_MainCameraTransform->SetPosition({ 0.0f, 0.0f, 0.0f });
		m_MainCameraTransform->SetRotation(QuaternionEuler({ 0.0f, 0.0f, 0.0f }));
		m_MainCameraCameraHandle->SetOrthographicSize(10.0f);
	});
	actionMap->AddAction(EscapeKeyAction);
#pragma endregion

#pragma region Textures
	m_CheckerBoardTexture = Texture2D::MasterFromFile("assets/textures/Checkerboard.png");
	m_CheckerBoardTexture->SetSWrapping(TextureWrapping::ClampToBorder);
	m_CheckerBoardTexture->SetTWrapping(TextureWrapping::ClampToBorder);
	m_CheckerBoardTexture->SetBorderColor({ 0.0f, 1.0f, 1.0f, 1.0f });
	m_CheckerBoardTexture->SetFilter(TextureFiltering::Nearest);

	m_OELogoTexture = Texture2D::MasterFromFile("assets/textures/OELogo.png");

	m_SpriteSheet = Texture2D::MasterFromFile("assets/textures/platformPack_tilesheet@2.png");
	m_SpriteSheet->SetSWrapping(TextureWrapping::ClampToBorder);
	m_SpriteSheet->SetTWrapping(TextureWrapping::ClampToBorder);
	m_SpriteSheet->SetBorderColor({ 0.0f, 1.0f, 1.0f, 1.0f });
	m_SpriteSheet->SetFilter(TextureFiltering::Nearest);

	m_Sprite = Texture2D::SubTextureFromExistingOne(m_SpriteSheet, { 128 * 4, 128 * 2, 128, 128 });
	m_ObstacleSprite = Texture2D::SubTextureFromExistingOne(m_SpriteSheet, { 128 * 0, 128 * 0, 128, 128 });
#pragma endregion

#pragma region ECS
	m_Scene = CreateRef<Scene>();

	////////////////////////////////////////////////////////////////
	// Player //////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	m_Player = m_Scene->CreateEntity("Player");

	// SpriteRenderer
	m_Player.AddComponent<SpriteRendererComponent>(m_Sprite);

	// PhysicsBody2D
	PhysicsBodyProps props;
	props.Type = PhysicsBodyType::Dynamic;
	m_Player.AddComponent<PhysicsBody2DComponent>(props);

	// PhysicsCollider2D
	auto& playerColliderList = m_Player.AddComponent<PhysicsColliders2DComponent>();
	auto playerCollider = CreateRef<PhysicsCollider2D>();
	playerCollider->GetShape()->SetAsBox({ 1.0f, 1.0f });
	playerCollider->GetMaterial().Bounciness = 0.9f;
	playerColliderList.AddCollider(playerCollider);

	////////////////////////////////////////////////////////////////
	// Obstacle ////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	Entity obstacle = m_Scene->CreateEntity("Obstacle");

	// SpriteRenderer
	auto& spriteRenderer = obstacle.AddComponent<SpriteRendererComponent>(m_ObstacleSprite);
	spriteRenderer.TilingFactorX = 4.0f;
	spriteRenderer.OverrideSWrapping = TextureWrapping::Repeat;
	spriteRenderer.OverrideTWrapping = TextureWrapping::Repeat;

	// PhysicsBody2D
	PhysicsBodyProps obstacleBodyProps;
	obstacleBodyProps.Type = PhysicsBodyType::Static;
	obstacle.AddComponent<PhysicsBody2DComponent>(obstacleBodyProps);

	auto& obstacleTransform = obstacle.GetComponent<TransformComponent>();
	obstacleTransform.SetPosition({ 0.0f, -2.0f, 0.0f });
	obstacleTransform.SetScale({ 4.0f, 1.0f, 1.0f });
	obstacleTransform.SetRotation(QuaternionEuler({ 0.0f, 0.0f, 45.0f }));

	// PhysicsCollider2D
	auto& colliderList = obstacle.AddComponent<PhysicsColliders2DComponent>();
	auto collider = CreateRef<PhysicsCollider2D>();
	collider->GetShape()->SetAsBox({ 4.0f, 1.0f });
	colliderList.AddCollider(collider);

	////////////////////////////////////////////////////////////////
	// Obstacle2 ///////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	Entity obstacle2 = m_Scene->CreateEntity("Obstacle2");

	// SpriteRenderer
	auto& spriteRenderer2 = obstacle2.AddComponent<SpriteRendererComponent>(m_ObstacleSprite);
	spriteRenderer2.TilingFactorX = 40.0f;
	spriteRenderer2.OverrideSWrapping = TextureWrapping::Repeat;
	spriteRenderer2.OverrideTWrapping = TextureWrapping::Repeat;

	// PhysicsBody2D
	PhysicsBodyProps obstacle2BodyProps;
	obstacle2BodyProps.Type = PhysicsBodyType::Static;
	obstacle2.AddComponent<PhysicsBody2DComponent>(obstacle2BodyProps);


	auto& obstacle2Transform = obstacle2.GetComponent<TransformComponent>();
	obstacle2Transform.SetScale({ 40.0f, 1.0f, 1.0f });
	obstacle2Transform.SetPosition({ 0.0f, -8.0f, 0.0f });
	//obstacle2Transform.SetRotation(QuaternionEuler({ 0.0f, 0.0f, 90.0f }));

	// PhysicsCollider2D
	auto& colliderList2 = obstacle2.AddComponent<PhysicsColliders2DComponent>();
	auto collider2 = CreateRef<PhysicsCollider2D>();
	collider2->GetShape()->SetAsBox({ 40.0f, 1.0f });
	colliderList2.AddCollider(collider2);

	////////////////////////////////////////////////////////////////
	// Main Camera /////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	m_MainCamera = m_Scene->CreateEntity("MainCamera");
	auto& app = Application::Get();
	float aspectRatio = (float)app.GetMainWindow().GetWidth() / (float)app.GetMainWindow().GetHeight();
	m_MainCameraCameraHandle = &m_MainCamera.AddComponent<CameraComponent>(Camera(CameraType::Orthographic, 10.0f, aspectRatio, -1.0f, 1.0f)).Camera;
	m_MainCameraTransform = &m_MainCamera.GetComponent<TransformComponent>();
#pragma endregion
}

static Vector<float> s_FPSSamples(200);
static bool VSync = true;
static char fpsText[32];

void SandboxECS::OnUpdate(TimeStep DeltaTime)
{
	OE_PROFILE_FUNCTION();

	m_MainCameraCameraHandle = &m_MainCamera.GetComponent<CameraComponent>().Camera;
	m_MainCameraTransform = &m_MainCamera.GetComponent<TransformComponent>();

	// Update
	Vector3 offset(m_CameraMovementDirection, 0.0f);
	offset = offset * (m_CameraSpeed * DeltaTime * m_MainCameraCameraHandle->GetOrthographicSize());
	Mat4x4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(QuaternionEulerAngles(m_MainCameraTransform->GetRotation()).z), Vector3(0, 0, 1));
	m_MainCameraTransform->SetPosition(Vector4(m_MainCameraTransform->GetPosition(), 0.0f) + (rotationMatrix * Vector4(offset, 1.0f)));

	m_MainCameraTransform->SetRotation(QuaternionEuler({ 0.0f, 0.0f, QuaternionEulerAngles(m_MainCameraTransform->GetRotation()).z + m_CameraRotationDirection * DeltaTime * 80.0f }));

	for (uint32_t i = 0; i < (uint32_t)((int)s_FPSSamples.size() - 1); i++)
	{
		s_FPSSamples[i] = s_FPSSamples[i + (uint32_t)1];
	}

	s_FPSSamples[(int)s_FPSSamples.size() - 1] = 1 / DeltaTime;
	if (s_FPSSamples[(int)s_FPSSamples.size() - 1] > 10000)
		s_FPSSamples[(int)s_FPSSamples.size() - 1] = 0;
	
	sprintf_s(fpsText, 32, "%i", (int)s_FPSSamples[(int)s_FPSSamples.size() - 1]);

	
	m_Scene->OnUpdate(DeltaTime, {m_MainCameraCameraHandle->GetAspectRatio(), 1});
}

void SandboxECS::OnImGuiRender()
{
	OE_PROFILE_FUNCTION();

	ImGui::Begin("Camera");
	ImGui::DragFloat2("Position", glm::value_ptr(const_cast<Vector3&>(m_MainCameraTransform->GetPosition())), m_MainCameraCameraHandle->GetOrthographicSize() / 20);
	// ImGui::DragFloat("Rotation", glm::value_ptr(const_cast<Vector3&>(m_Camera.GetRotation())));
	// ImGui::DragFloat("Size", &(const_cast<float&>(m_Camera.GetOrthographicSize())));
	ImGui::End();

	/*ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	ImGui::Begin("Texture Debugger1");

	uint32_t cellSize = 8;
	for (int x = 0; x < ImGui::GetContentRegionAvail().x / cellSize + 1; x++)
	{
		for (int y = 0; y < ImGui::GetContentRegionAvail().y / cellSize + 1; y++)
		{
			float colorTint = (x % 2 == 0 && y % 2 != 0) || (x % 2 != 0 && y % 2 == 0) ? 0.8f : 1.0f;
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2({ ImGui::GetCursorScreenPos().x + x * cellSize, ImGui::GetCursorScreenPos().y + y * cellSize }),
				ImVec2({ ImGui::GetCursorScreenPos().x + (x + 1) * cellSize, ImGui::GetCursorScreenPos().y + (y + 1) * cellSize }),
				ImGui::ColorConvertFloat4ToU32(ImVec4(colorTint, colorTint, colorTint, 1.0f))
			);

		}
	}

	ImGui::Image((void*)1, ImGui::GetContentRegionAvail());
	ImGui::End();

	ImGui::Begin("Texture Debugger2");

	for (int x = 0; x < ImGui::GetContentRegionAvail().x / cellSize + 1; x++)
	{
		for (int y = 0; y < ImGui::GetContentRegionAvail().y / cellSize + 1; y++)
		{
			float colorTint = (x % 2 == 0 && y % 2 != 0) || (x % 2 != 0 && y % 2 == 0) ? 0.8f : 1.0f;
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2({ ImGui::GetCursorScreenPos().x + x * cellSize, ImGui::GetCursorScreenPos().y + y * cellSize }),
				ImVec2({ ImGui::GetCursorScreenPos().x + (x + 1) * cellSize, ImGui::GetCursorScreenPos().y + (y + 1) * cellSize }),
				ImGui::ColorConvertFloat4ToU32(ImVec4(colorTint, colorTint, colorTint, 1.0f))
			);

		}
	}

	ImGui::Image((void*)2, ImGui::GetContentRegionAvail());
	ImGui::End();

	ImGui::PopStyleVar();*/

	ImGui::Begin("Renderer2D Statistics");
	
	ImGui::PlotLines("FPS", s_FPSSamples.data(), (int)s_FPSSamples.size(), 0, fpsText, FLT_MAX, FLT_MAX, ImVec2{ 0, 80 });

	ImGui::Text("DrawCalls : %i", Renderer2D::GetStatistics().DrawCalls);
	ImGui::Text("QuadCount : %i", Renderer2D::GetStatistics().QuadCount);
	ImGui::Text("IndexCount : %i", Renderer2D::GetStatistics().GetIndexCount());
	ImGui::Text("VertexCount : %i", Renderer2D::GetStatistics().GetVertexCount());

	ImGui::End();

	ImGui::Begin("Stuff");

	if (ImGui::Checkbox("V-Sync", &VSync))
	{
		Application::Get().GetMainWindow().SetVSync(VSync);
	}

	ImGui::End();
}

void SandboxECS::OnEvent(Event& event)
{
	OE_PROFILE_FUNCTION();

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<WindowResizeEvent>(OE_BIND_EVENT_FN(SandboxECS::OnWindowResizeEvent));
	dispatcher.Dispatch<MouseScrolledEvent>(OE_BIND_EVENT_FN(SandboxECS::OnMouseScrolledEvent));
}

bool SandboxECS::OnWindowResizeEvent(WindowResizeEvent& event)
{
	OE_PROFILE_FUNCTION();

	m_MainCameraCameraHandle->SetAspectRatio((float)event.GetWidth() / (float)event.GetHeight());
	return false;
}

bool SandboxECS::OnMouseScrolledEvent(MouseScrolledEvent& event)
{
	OE_PROFILE_FUNCTION();

	float newSize = m_MainCameraCameraHandle->GetOrthographicSize() - (float)event.GetYOffset() / 4.0f;
	if (newSize > 0)
		m_MainCameraCameraHandle->SetOrthographicSize(newSize);
	return false;
}