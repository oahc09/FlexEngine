#pragma once

namespace flex
{
	class BezierCurve;
	class Player;

	// The player controller is responsible for setting the player's
	// transform based on either player input, or an AI behavior
	class PlayerController final
	{
	public:
		PlayerController();
		~PlayerController();

		void Initialize(Player* player);
		void Update();
		void Destroy();

		void ResetTransformAndVelocities();
		void UpdateIsPossessed();

		BezierCurve* GetRailRiding() const;

	private:
		void SnapPosToRail();

		real m_MoveAcceleration = 140.0f;
		real m_MaxMoveSpeed = 20.0f;
		real m_RotateHSpeed = 6.0f;
		real m_RotateVSpeed = 2.0f;
		real m_RotateFriction = 0.03f;
		// How quickly to turn towards direction of movement
		real m_RotationSnappiness = 80.0f;
		// If the player has a velocity magnitude of this value or lower, their
		// rotation speed will linearly decrease as their velocity approaches 0
		real m_MaxSlowDownRotationSpeedVel = 10.0f;

		enum class Mode
		{
			THIRD_PERSON,
			FIRST_PERSON
		};

		Mode m_Mode = Mode::FIRST_PERSON;

		i32 m_PlayerIndex = -1;

		Player* m_Player = nullptr;

		bool m_bGrounded = false;

		bool m_bPossessed = false;

		BezierCurve* m_RailRiding = nullptr;
		real m_DistAlongRail = 0.0f;
		real m_RailMoveSpeed = 1.0f;
		real m_RailAttachMinDist = 4.0f;

		AudioSourceID m_SoundRailAttachID;
		AudioSourceID m_SoundRailDetachID;

	};
} // namespace flex
