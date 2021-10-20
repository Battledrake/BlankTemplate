#include "StdAfx.h"
#include "Player.h"

#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CryCore/StaticInstanceList.h>

namespace
{
	static void RegisterPlayerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CPlayerComponent));
		}
	}
	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterPlayerComponent);
}

void CPlayerComponent::Initialize()
{
	m_inputFlags.Clear();

	m_mouseDeltaRotation = ZERO;
	m_pCameraComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCameraComponent>();

	m_pAudioListenerComponent = m_pEntity->GetOrCreateComponent<Cry::Audio::DefaultComponents::CListenerComponent>();

	m_pInputComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CInputComponent>();

	m_pInputComponent->RegisterAction("player", "moveleft", [this](int activationMode, float value) { HandleInputFlagChange(EInputFlag::MoveLeft, (EActionActivationMode)activationMode); });
	m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, EKeyId::eKI_A);

	m_pInputComponent->RegisterAction("player", "moveright", [this](int activationMode, float value) { HandleInputFlagChange(EInputFlag::MoveRight, (EActionActivationMode)activationMode); });
	m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, EKeyId::eKI_D);

	m_pInputComponent->RegisterAction("player", "moveforward", [this](int activationMode, float value) { HandleInputFlagChange(EInputFlag::MoveForward, (EActionActivationMode)activationMode); });
	m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, EKeyId::eKI_W);

	m_pInputComponent->RegisterAction("player", "moveback", [this](int activationMode, float value) { HandleInputFlagChange(EInputFlag::MoveBack, (EActionActivationMode)activationMode); });
	m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, EKeyId::eKI_S);

	m_pInputComponent->RegisterAction("player", "mouse_rotateyaw", [this](int activationMode, float value) { m_mouseDeltaRotation.x -= value; });
	m_pInputComponent->BindAction("player", "mouse_rotateyaw", eAID_KeyboardMouse, EKeyId::eKI_MouseX);

	m_pInputComponent->RegisterAction("player", "mouse_rotatepitch", [this](int activationMode, float value) { m_mouseDeltaRotation.y -= value; });
	m_pInputComponent->BindAction("player", "mouse_rotatepitch", eAID_KeyboardMouse, EKeyId::eKI_MouseY);
}

Cry::Entity::EventFlags CPlayerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset;
}

void CPlayerComponent::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
		case Cry::Entity::EEvent::GameplayStarted:
		{
		}
		break;
		case Cry::Entity::EEvent::Update:
		{
			const float frameTime = event.fParam[0];

			const float moveSpeed = 20.5f;
			Vec3 velocity = ZERO;

			if (m_inputFlags & EInputFlag::MoveLeft)
			{
				velocity.x -= moveSpeed * frameTime;
			}
			if (m_inputFlags & EInputFlag::MoveRight)
			{
				velocity.x += moveSpeed * frameTime;
			}
			if (m_inputFlags & EInputFlag::MoveForward)
			{
				velocity.y += moveSpeed * frameTime;
			}
			if (m_inputFlags & EInputFlag::MoveBack)
			{
				velocity.y -= moveSpeed * frameTime;
			}

			Matrix34 transformation = m_pEntity->GetWorldTM();
			transformation.AddTranslation(transformation.TransformVector(velocity));

			Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(transformation));

			const float rotationSpeed = 0.002f;
			ypr.x += m_mouseDeltaRotation.x * rotationSpeed;
			ypr.y += m_mouseDeltaRotation.y * rotationSpeed;
			ypr.z = 0;

			transformation.SetRotation33(CCamera::CreateOrientationYPR(ypr));

			m_mouseDeltaRotation = ZERO;

			m_pEntity->SetWorldTM(transformation);
		}
		break;
		case Cry::Entity::EEvent::Reset:
		{
		}
		break;
	}
}

void CPlayerComponent::HandleInputFlagChange(const CEnumFlags<EInputFlag> flags, const CEnumFlags<EActionActivationMode> activationMode, const EInputFlagType type)
{
	switch (type)
	{
		case EInputFlagType::Hold:
		{
			if (activationMode == eAAM_OnRelease)
			{
				m_inputFlags &= ~flags;
			}
			else
			{
				m_inputFlags |= flags;
			}
		}
		break;
		case EInputFlagType::Toggle:
		{
			if (activationMode == eAAM_OnRelease)
			{
				m_inputFlags ^= flags;
			}
		}
		break;
	}
}