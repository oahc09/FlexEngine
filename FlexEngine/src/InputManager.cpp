#include "stdafx.hpp"

#include "InputManager.hpp"

#include <assert.h>

#include "Graphics/Renderer.hpp"
#include "Window/Window.hpp"

namespace flex
{
	namespace Input
	{
		const real Manager::MAX_JOYSTICK_ROTATION_SPEED = 15.0f;

		Manager::Manager()
		{
		}

		Manager::~Manager()
		{
		}

		void Manager::Initialize()
		{
			ImGuiIO& io = ImGui::GetIO();
			io.KeyMap[ImGuiKey_Tab] = (i32)KeyCode::KEY_TAB;
			io.KeyMap[ImGuiKey_LeftArrow] = (i32)KeyCode::KEY_LEFT;
			io.KeyMap[ImGuiKey_RightArrow] = (i32)KeyCode::KEY_RIGHT;
			io.KeyMap[ImGuiKey_UpArrow] = (i32)KeyCode::KEY_UP;
			io.KeyMap[ImGuiKey_DownArrow] = (i32)KeyCode::KEY_DOWN;
			io.KeyMap[ImGuiKey_PageUp] = (i32)KeyCode::KEY_PAGE_UP;
			io.KeyMap[ImGuiKey_PageDown] = (i32)KeyCode::KEY_PAGE_DOWN;
			io.KeyMap[ImGuiKey_Home] = (i32)KeyCode::KEY_HOME;
			io.KeyMap[ImGuiKey_End] = (i32)KeyCode::KEY_END;
			io.KeyMap[ImGuiKey_Insert] = (i32)KeyCode::KEY_INSERT;
			io.KeyMap[ImGuiKey_Delete] = (i32)KeyCode::KEY_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = (i32)KeyCode::KEY_BACKSPACE;
			io.KeyMap[ImGuiKey_Space] = (i32)KeyCode::KEY_SPACE;
			io.KeyMap[ImGuiKey_Enter] = (i32)KeyCode::KEY_ENTER;
			io.KeyMap[ImGuiKey_Escape] = (i32)KeyCode::KEY_ESCAPE;
			io.KeyMap[ImGuiKey_A] = (i32)KeyCode::KEY_A; // for text edit CTRL+A: select all
			io.KeyMap[ImGuiKey_C] = (i32)KeyCode::KEY_C; // for text edit CTRL+C: copy
			io.KeyMap[ImGuiKey_V] = (i32)KeyCode::KEY_V; // for text edit CTRL+V: paste
			io.KeyMap[ImGuiKey_X] = (i32)KeyCode::KEY_X; // for text edit CTRL+X: cut
			io.KeyMap[ImGuiKey_Y] = (i32)KeyCode::KEY_Y; // for text edit CTRL+Y: redo
			io.KeyMap[ImGuiKey_Z] = (i32)KeyCode::KEY_Z; // for text edit CTRL+Z: undo

			m_ImGuiIniFilepathStr = ROOT_LOCATION "saved/config/imgui.ini";
			io.IniFilename = m_ImGuiIniFilepathStr.c_str();

			g_InputBindings.resize((i32)Action::COUNT);
			g_InputBindingNames.resize((i32)Action::COUNT);
			g_InputBindings[(i32)Action::WALK_LEFT].keyCode = KeyCode::KEY_A;
			g_InputBindingNames[(i32)Action::WALK_LEFT] = "Walk left";
			g_InputBindings[(i32)Action::WALK_RIGHT].keyCode = KeyCode::KEY_D;
			g_InputBindingNames[(i32)Action::WALK_RIGHT] = "Walk right";
			g_InputBindings[(i32)Action::WALK_FORWARD].keyCode = KeyCode::KEY_W;
			g_InputBindingNames[(i32)Action::WALK_FORWARD] = "Walk forward";
			g_InputBindings[(i32)Action::WALK_BACKWARD].keyCode = KeyCode::KEY_S;
			g_InputBindingNames[(i32)Action::WALK_BACKWARD] = "Walk backward";
			g_InputBindings[(i32)Action::LOOK_H].gamepadAxis = GamepadAxis::LEFT_STICK_X;
			g_InputBindingNames[(i32)Action::LOOK_H] = "Look horizontal";
			g_InputBindings[(i32)Action::LOOK_V].gamepadAxis = GamepadAxis::LEFT_STICK_Y;
			g_InputBindingNames[(i32)Action::LOOK_V] = "Look vertical";

			ClearAllInputs();
		}

		void Manager::Update()
		{
			if (!g_Window->HasFocus())
			{
				ClearAllInputs();
				return;
			}

			// Keyboard keys
			for (auto& keyPair : m_Keys)
			{
				if (keyPair.second.down > 0)
				{
					++keyPair.second.down;
				}
			}

	#if 0 // Log mouse states
			Print("states:   ", false);
			for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
			{
				Print(std::string((m_MouseButtonStates & (1 << i)) != 0 ? "1" : "0") + ", ", false);
			}
			Logger::LogNewLine();

			Print("pressed:  ", false);
			for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
			{
				Print(std::string((m_MouseButtonsPressed & (1 << i)) != 0 ? "1" : "0") + ", ", false);
			}
			Logger::LogNewLine();

			Print("released: ", false);
			for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
			{
				Print(std::string((m_MouseButtonsReleased & (1 << i)) != 0 ? "1" : "0") + ", ", false);
			}
			Logger::LogNewLine();
			Logger::LogNewLine();
	#endif

			// Mouse buttons
			for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
			{
				m_MouseButtonsPressed  &= ~(1 << i);
				m_MouseButtonsReleased &= ~(1 << i);
				if (m_MouseButtonStates & (1 << i))
				{
					m_MouseButtonDrags[i].endLocation = m_MousePosition;
				}
			}

			// Gamepads
			for (i32 i = 0; i < 2; ++i)
			{
				GamepadState& gamepadState = m_GamepadStates[i];

				real joystickX = GetGamepadAxisValue(i, GamepadAxis::RIGHT_STICK_X);
				real joystickY = GetGamepadAxisValue(i, GamepadAxis::RIGHT_STICK_Y);

				i32 currentQuadrant = -1;

				real minimumExtensionLength = 0.35f;
				real extensionLength = glm::length(glm::vec2(joystickX, joystickY));
				if (extensionLength > minimumExtensionLength)
				{
					if (gamepadState.previousQuadrant != -1)
					{
						real currentAngle = atan2(joystickY, joystickX) + PI;
						real previousAngle = atan2(gamepadState.pJoystickY, gamepadState.pJoystickX) + PI;
						// Asymptote occurs on left
						if (joystickX < 0.0f)
						{
							if (gamepadState.pJoystickY < 0.0f && joystickY >= 0.0f)
							{
								// CCW
								currentAngle -= TWO_PI;
							}
							else if (gamepadState.pJoystickY >= 0.0f && joystickY < 0.0f)
							{
								// CW
								currentAngle += TWO_PI;
							}
						}

						real stickRotationSpeed = (currentAngle - previousAngle) / g_DeltaTime;
						stickRotationSpeed = glm::clamp(stickRotationSpeed,
														-MAX_JOYSTICK_ROTATION_SPEED,
														MAX_JOYSTICK_ROTATION_SPEED);

						gamepadState.averageRotationSpeeds.AddValue(stickRotationSpeed);
					}

					if (joystickX > 0.0f)
					{
						currentQuadrant = (joystickY > 0.0f ? 2 : 1);
					}
					else
					{
						currentQuadrant = (joystickY > 0.0f ? 3 : 0);
					}
				}
				else
				{
					gamepadState.averageRotationSpeeds.Reset();
				}

				gamepadState.previousQuadrant = currentQuadrant;

				gamepadState.pJoystickX = joystickX;
				gamepadState.pJoystickY = joystickY;
			}
		}

		void Manager::PostUpdate()
		{
			//glm::vec2i dPos = (m_MousePosition - m_PrevMousePosition);
			//Print("%i, %i\n", dPos.x, dPos.y);
			//i32 threshold = 300;
			//m_bMouseWrapped = abs(dPos.x) > threshold ||
			//	abs(dPos.y) > threshold;

			m_bMouseWrapped = false;
			m_PrevMousePosition = m_MousePosition;
			m_ScrollXOffset = 0.0f;
			m_ScrollYOffset = 0.0f;
		}

		void Manager::UpdateGamepadState(i32 gamepadIndex, real axes[6], u8 buttons[15])
		{
			assert(gamepadIndex == 0 || gamepadIndex == 1);

			m_pGamepadStates[gamepadIndex] = m_GamepadStates[gamepadIndex];

			for (i32 i = 0; i < 6; ++i)
			{
				m_GamepadStates[gamepadIndex].axes[i] = axes[i];
			}
			// Map triggers into range [0.0,1.0]
			m_GamepadStates[gamepadIndex].axes[4] = m_GamepadStates[gamepadIndex].axes[4] * 0.5f + 0.5f;
			m_GamepadStates[gamepadIndex].axes[5] = m_GamepadStates[gamepadIndex].axes[5] * 0.5f + 0.5f;

			HandleRadialDeadZone(&m_GamepadStates[gamepadIndex].axes[0], &m_GamepadStates[gamepadIndex].axes[1]);
			HandleRadialDeadZone(&m_GamepadStates[gamepadIndex].axes[2], &m_GamepadStates[gamepadIndex].axes[3]);

			u32 pStates = m_GamepadStates[gamepadIndex].buttonStates;

			m_GamepadStates[gamepadIndex].buttonStates = 0;
			for (i32 i = 0; i < GAMEPAD_BUTTON_COUNT; ++i)
			{
				m_GamepadStates[gamepadIndex].buttonStates = m_GamepadStates[gamepadIndex].buttonStates | (buttons[i] << i);
			}

			u32 changedButtons = m_GamepadStates[gamepadIndex].buttonStates ^ pStates;
			m_GamepadStates[gamepadIndex].buttonsPressed = changedButtons & m_GamepadStates[gamepadIndex].buttonStates;
			m_GamepadStates[gamepadIndex].buttonsReleased = changedButtons & (~m_GamepadStates[gamepadIndex].buttonStates);
		}

		GamepadState& Manager::GetGamepadState(i32 gamepadIndex)
		{
			assert(gamepadIndex == 0 ||
				   gamepadIndex == 1);
			return m_GamepadStates[gamepadIndex];
		}

		// http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
		void Manager::HandleRadialDeadZone(real* x, real* y)
		{
			real deadzone = 0.25f;

			glm::vec2 stick(*x, *y);
			real stickMagnitude = glm::length(stick);
			if (stickMagnitude < deadzone)
			{
				*x = 0.0f;
				*y = 0.0f;
			}
			else
			{
				glm::vec2 stickDir = (stick / stickMagnitude);
				glm::vec2 stickScaled = stickDir * ((stickMagnitude - deadzone) / (1.0f - deadzone));

				*x = stickScaled.x;
				*y = stickScaled.y;
			}
		}

		i32 Manager::GetKeyDown(KeyCode keyCode, bool bIgnoreImGui) const
		{
			if (!bIgnoreImGui && ImGui::GetIO().WantCaptureKeyboard)
			{
				return 0;
			}

			auto iter = m_Keys.find(keyCode);
			if (iter != m_Keys.end())
			{
				return m_Keys.at(keyCode).down;
			}

			return 0;
		}

		bool Manager::GetKeyPressed(KeyCode keyCode, bool bIgnoreImGui) const
		{
			return GetKeyDown(keyCode, bIgnoreImGui) == 1;
		}

		bool Manager::IsGamepadButtonDown(i32 gamepadIndex, GamepadButton button)
		{
			assert(gamepadIndex == 0 || gamepadIndex == 1);

			bool bDown = ((m_GamepadStates[gamepadIndex].buttonStates & (1 << (i32)button)) != 0);
			return bDown;
		}

		bool Manager::IsGamepadButtonPressed(i32 gamepadIndex, GamepadButton button)
		{
			assert(gamepadIndex == 0 || gamepadIndex == 1);

			bool bPressed = ((m_GamepadStates[gamepadIndex].buttonsPressed & (1 << (i32)button)) != 0);
			return bPressed;
		}

		bool Manager::IsGamepadButtonReleased(i32 gamepadIndex, GamepadButton button)
		{
			assert(gamepadIndex == 0 || gamepadIndex == 1);

			bool bReleased = ((m_GamepadStates[gamepadIndex].buttonsReleased & (1 << (i32)button)) != 0);
			return bReleased;
		}

		real Manager::GetGamepadAxisValue(i32 gamepadIndex, GamepadAxis axis)
		{
			assert(gamepadIndex == 0 || gamepadIndex == 1);

			real axisValue = m_GamepadStates[gamepadIndex].axes[(i32)axis];
			return axisValue;
		}

		bool Manager::HasGamepadAxisValueJustPassedThreshold(i32 gamepadIndex, GamepadAxis axis, real threshold)
		{
			assert(gamepadIndex == 0 || gamepadIndex == 1);

			return (m_GamepadStates[gamepadIndex].axes[(i32)axis] >= threshold &&
				m_pGamepadStates[gamepadIndex].axes[(i32)axis] < threshold);
		}

		void Manager::CursorPosCallback(double x, double y)
		{
			m_MousePosition = glm::vec2((real)x, (real)y);

			ImGuiIO& io = ImGui::GetIO();

			if (g_InputManager->IsAnyMouseButtonDown(true))
			{
				glm::vec2i frameBufferSize = g_Window->GetFrameBufferSize();
				if (m_MousePosition.x >= (real)(frameBufferSize.x - 1))
				{
					m_bMouseWrapped = true;
					m_MousePosition.x -= (frameBufferSize.x - 1);
					m_PrevMousePosition.x = m_MousePosition.x;
					io.MousePosPrev.x = m_MousePosition.x;
					for (MouseDrag& drag : m_MouseButtonDrags)
					{
						drag.startLocation -= glm::vec2(frameBufferSize.x - 1, 0.0f);
					}
					g_Window->SetCursorPos(m_MousePosition);
				}
				else if (m_MousePosition.x <= 0)
				{
					m_bMouseWrapped = true;
					m_MousePosition.x += (frameBufferSize.x - 1);
					m_PrevMousePosition.x = m_MousePosition.x;
					io.MousePosPrev.x = m_MousePosition.x;
					for (MouseDrag& drag : m_MouseButtonDrags)
					{
						drag.startLocation += glm::vec2(frameBufferSize.x - 1, 0.0f);
					}
					g_Window->SetCursorPos(m_MousePosition);
				}

				if (m_MousePosition.y >= (real)(frameBufferSize.y - 1))
				{
					m_bMouseWrapped = true;
					m_MousePosition.y -= (frameBufferSize.y - 1);
					m_PrevMousePosition.y = m_MousePosition.y;
					io.MousePosPrev.y = m_MousePosition.y;
					for (MouseDrag& drag : m_MouseButtonDrags)
					{
						drag.startLocation -= glm::vec2(0.0f, frameBufferSize.y - 1);
					}
					g_Window->SetCursorPos(m_MousePosition);
				}
				else if (m_MousePosition.y <= 0)
				{
					m_bMouseWrapped = true;
					m_MousePosition.y += (frameBufferSize.y - 1);
					m_PrevMousePosition.y = m_MousePosition.y;
					io.MousePosPrev.y = m_MousePosition.y;
					for (MouseDrag& drag : m_MouseButtonDrags)
					{
						drag.startLocation += glm::vec2(0.0f, frameBufferSize.y - 1);
					}
					g_Window->SetCursorPos(m_MousePosition);
				}
			}

			io.MousePos = m_MousePosition;
		}

		void Manager::MouseButtonCallback(MouseButton mouseButton, KeyAction action, i32 mods)
		{
			UNREFERENCED_PARAMETER(mods);

			assert((u32)mouseButton < MOUSE_BUTTON_COUNT);

			if (action == KeyAction::PRESS)
			{
				m_MouseButtonStates    |=  (1 << (u32)mouseButton);
				m_MouseButtonsPressed  |=  (1 << (u32)mouseButton);
				m_MouseButtonsReleased &= ~(1 << (u32)mouseButton);

				m_MouseButtonDrags[(i32)mouseButton].startLocation = m_MousePosition;
				m_MouseButtonDrags[(i32)mouseButton].endLocation = m_MousePosition;
			}
			else if (action == KeyAction::RELEASE)
			{
				m_MouseButtonStates    &= ~(1 << (u32)mouseButton);
				m_MouseButtonsPressed  &= ~(1 << (u32)mouseButton);
				m_MouseButtonsReleased |=  (1 << (u32)mouseButton);

				m_MouseButtonDrags[(i32)mouseButton].endLocation = m_MousePosition;
			}

			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[(i32)mouseButton] = (m_MouseButtonStates & (1 << (i32)mouseButton)) != 0;
		}

		void Manager::ScrollCallback(double xOffset, double yOffset)
		{
			m_ScrollXOffset = (real)xOffset;
			m_ScrollYOffset = (real)yOffset;

			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheelH += m_ScrollXOffset;
			io.MouseWheel += m_ScrollYOffset;
		}

		void Manager::KeyCallback(KeyCode keycode, KeyAction action, i32 mods)
		{
			UNREFERENCED_PARAMETER(mods);

			if (action == KeyAction::PRESS)
			{
				m_Keys[keycode].down = 1;
			}
			else if (action == KeyAction::REPEAT)
			{
				// Ignore repeat events (we're already counting how long the key is down for
			}
			else if (action == KeyAction::RELEASE)
			{
				m_Keys[keycode].down = 0;
			}

			ImGuiIO& io = ImGui::GetIO();
			io.KeysDown[(i32)keycode] = m_Keys[keycode].down > 0;

			io.KeyCtrl = GetKeyDown(KeyCode::KEY_LEFT_CONTROL, true) || GetKeyDown(KeyCode::KEY_RIGHT_CONTROL, true);
			io.KeyShift = GetKeyDown(KeyCode::KEY_LEFT_SHIFT, true) || GetKeyDown(KeyCode::KEY_RIGHT_SHIFT, true);
			io.KeyAlt = GetKeyDown(KeyCode::KEY_LEFT_ALT, true) || GetKeyDown(KeyCode::KEY_RIGHT_ALT, true);
			io.KeySuper = GetKeyDown(KeyCode::KEY_LEFT_SUPER, true) || GetKeyDown(KeyCode::KEY_RIGHT_SUPER, true);
		}

		void Manager::CharCallback(u32 character)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (character > 0 && character < 0x10000)
			{
				io.AddInputCharacter((ImWchar)character);
			}
		}

		bool Manager::DidMouseWrap() const
		{
			return m_bMouseWrapped;
		}

		glm::vec2 Manager::GetMousePosition() const
		{
			return m_MousePosition;
		}

		void Manager::ClearMouseMovement()
		{
			m_PrevMousePosition = m_MousePosition;
		}

		void Manager::SetMousePosition(glm::vec2 mousePos, bool bUpdatePreviousPos)
		{
			m_MousePosition = mousePos;

			if (bUpdatePreviousPos)
			{
				m_PrevMousePosition = m_MousePosition;
			}
		}

		glm::vec2 Manager::GetMouseMovement() const
		{
			if (ImGui::GetIO().WantCaptureMouse)
			{
				return glm::vec2(0, 0);
			}

			if (m_MousePosition == glm::vec2(-1.0f) ||
				m_PrevMousePosition == glm::vec2(-1.0f))
			{
				return VEC2_ZERO;
			}

			return m_MousePosition - m_PrevMousePosition;
		}

		void Manager::ClearMouseButton(MouseButton mouseButton)
		{
			m_MouseButtonStates &= ~(1 << ((i32)mouseButton));
			m_MouseButtonsPressed &= ~(1 << ((i32)mouseButton));
			m_MouseButtonsReleased &= ~(1 << ((i32)mouseButton));
		}

		bool Manager::IsAnyMouseButtonDown(bool bIgnoreImGui) const
		{
			if (!bIgnoreImGui && ImGui::GetIO().WantCaptureMouse)
			{
				return false;
			}

			for (i32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
			{
				if ((m_MouseButtonStates & (1 << i)) != 0)
				{
					return true;
				}
			}

			return false;
		}

		bool Manager::IsMouseButtonDown(MouseButton mouseButton) const
		{
			if (ImGui::GetIO().WantCaptureMouse)
			{
				return false;
			}

			assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

			return (m_MouseButtonStates & (1 << (i32)mouseButton)) != 0;
		}

		bool Manager::IsMouseButtonPressed(MouseButton mouseButton) const
		{
			if (ImGui::GetIO().WantCaptureMouse)
			{
				return false;
			}

			assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

			return (m_MouseButtonsPressed & (1 << (i32)mouseButton)) != 0;
		}

		bool Manager::IsMouseButtonReleased(MouseButton mouseButton) const
		{
			if (ImGui::GetIO().WantCaptureMouse)
			{
				return false;
			}

			assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

			return (m_MouseButtonsReleased & (1 << (i32)mouseButton)) != 0;
		}

		real Manager::GetVerticalScrollDistance() const
		{
			if (ImGui::GetIO().WantCaptureMouse)
			{
				return 0.0f;
			}

			return m_ScrollYOffset;
		}

		void Manager::ClearVerticalScrollDistance()
		{
			m_ScrollYOffset = 0;
		}

		bool Manager::IsMouseHoveringRect(const glm::vec2& posNorm, const glm::vec2& sizeNorm)
		{
			glm::vec2 posPixels;
			glm::vec2 sizePixels;
			g_Renderer->TransformRectToScreenSpace(posNorm, sizeNorm, posPixels, sizePixels);
			glm::vec2 halfSizePixels = sizePixels / 2.0f;

			bool bHoveringInArea = (m_MousePosition.x >= posPixels.x - halfSizePixels.x && m_MousePosition.x < posPixels.x + halfSizePixels.x &&
									m_MousePosition.y >= posPixels.y - halfSizePixels.y && m_MousePosition.y < posPixels.y + halfSizePixels.y);
			return bHoveringInArea;
		}

		glm::vec2 Manager::GetMouseDragDistance(MouseButton mouseButton)
		{
			assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

			return (m_MouseButtonDrags[(i32)mouseButton].endLocation - m_MouseButtonDrags[(i32)mouseButton].startLocation);
		}

		void Manager::ClearMouseDragDistance(MouseButton mouseButton)
		{
			assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

			m_MouseButtonDrags[(i32)mouseButton].startLocation = m_MouseButtonDrags[(i32)mouseButton].endLocation;
		}

		void Manager::ClearAllInputs()
		{
			ClearMouseInput();
			ClearKeyboadInput();
			ClearGampadInput(0);
			ClearGampadInput(1);
		}

		void Manager::ClearMouseInput()
		{
			m_PrevMousePosition = m_MousePosition = glm::vec2(-1.0f);
			m_ScrollXOffset = 0.0f;
			m_ScrollYOffset = 0.0f;

			m_MouseButtonStates = 0;
			m_MouseButtonsPressed = 0;
			m_MouseButtonsReleased = 0;

			for (MouseDrag& mouseDrag : m_MouseButtonDrags)
			{
				mouseDrag.startLocation = VEC2_ZERO;
				mouseDrag.endLocation = VEC2_ZERO;
			}
			g_Window->SetCursorMode(CursorMode::NORMAL);


			ImGuiIO& io = ImGui::GetIO();
			io.MousePos = m_MousePosition;
			io.MousePosPrev = m_PrevMousePosition;
			io.MouseWheel = m_ScrollYOffset;
		}

		void Manager::ClearKeyboadInput()
		{
			for (auto& keyPair : m_Keys)
			{
				keyPair.second.down = 0;
			}
		}

		void Manager::ClearGampadInput(i32 gamepadIndex)
		{
			assert(gamepadIndex == 0 || gamepadIndex == 1);

			GamepadState& gamepadState = m_GamepadStates[gamepadIndex];

			for (real& axis : gamepadState.axes)
			{
				axis = 0;
			}

			gamepadState.buttonStates = 0;
			gamepadState.buttonsPressed = 0;
			gamepadState.buttonsReleased = 0;

			gamepadState.averageRotationSpeeds = RollingAverage<real>(gamepadState.framesToAverageOver);

			m_pGamepadStates[gamepadIndex] = gamepadState;
		}
	} // namespace Input
} // namespace flex
