#include "PlayerControlsComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Component/TransformBus.h>
#include <MultiplayerCharacter/PebbleSpawnerComponentBus.h>
#include <Integration/AnimGraphComponentBus.h>
#include <MultiplayerCharacter/FootstepComponentBus.h>
#include <MultiplayerCharacter/MyCharacterBus.h>

using namespace AZ;
using namespace EMotionFX;
using namespace MultiplayerCharacter;

void PlayerControlsComponent::Activate()
{
    ServerPlayerControlsRequestBus::Handler::BusConnect( GetEntityId() );
    AZ::TickBus::Handler::BusConnect();
}

void PlayerControlsComponent::Deactivate()
{
    ServerPlayerControlsRequestBus::Handler::BusDisconnect();
    AZ::TickBus::Handler::BusDisconnect();
}

void PlayerControlsComponent::Reflect( AZ::ReflectContext* ref )
{
    auto sc = azrtti_cast<AZ::SerializeContext*>( ref );
    if ( !sc ) return;

    using Self = PlayerControlsComponent; // to shorten lines
    sc->Class<PlayerControlsComponent, Component>()
        ->Field( "Movement Speed", &Self::m_speed )
        ->Field( "Turning Speed", &Self::m_turnSpeed )
        ->Version( 1 );

    AZ::EditContext* ec = sc->GetEditContext();
    if ( !ec ) return;

    using namespace AZ::Edit::Attributes;
    using namespace AZ::Edit::UIHandlers;
    // reflection of this component for Lumberyard Editor
    ec->Class<PlayerControlsComponent>( "Player Controls",
        "[Implements various player controls]" )
        ->ClassElement( AZ::Edit::ClassElements::EditorData, "" )
        ->Attribute( AppearsInAddComponentMenu,
            AZ_CRC( "Game", 0x232b318c ) )
        ->Attribute( Category, "Multiplayer Character" )
        ->DataElement( Default, &Self::m_speed,
            "Movement Speed", "how fast the character moves" )
        ->DataElement( Default, &Self::m_turnSpeed,
            "Turning Speed", "how fast the character turns" );
}

void PlayerControlsComponent::MoveForward( ActionState state )
{
    m_isForward = state == ActionState::Started;
}

void PlayerControlsComponent::MoveBackward( ActionState state )
{
    m_isBackward = state == ActionState::Started;
}

void PlayerControlsComponent::StrafeLeft( ActionState state )
{
    m_isStrafingLeft = state == ActionState::Started;
}

void PlayerControlsComponent::StrafeRight( ActionState state )
{
    m_isStrafingRight = state == ActionState::Started;
}

void PlayerControlsComponent::Turn( float amount )
{
    m_rotZ = amount * m_turnSpeed;
    SetRotation();

    using AnimBus = Integration::AnimGraphComponentRequestBus;
    AnimBus::Event( GetEntityId(),
        &AnimBus::Events::SetNamedParameterFloat, "TurnSpeed",
        ( m_prevTurn - amount ) * 100 );

    m_prevTurn = amount;
}

void PlayerControlsComponent::Shoot( ActionState state )
{
    m_isShooting = state == ActionState::Started;
    if ( state != ActionState::Stopped ) return; // fire on release

    AZ::Transform myLocation;
    TransformBus::EventResult( myLocation, GetEntityId(),
        &TransformBus::Events::GetWorldTM );
    // place the pebble a little in front of the player
    const auto q = Quaternion::CreateFromTransform( myLocation );
    const Vector3 disp = q * AZ::Vector3::CreateAxisY( 1.f );
    myLocation.SetTranslation( myLocation.GetTranslation() + disp );

    PebbleSpawnerComponentBus::Broadcast(
        &PebbleSpawnerComponentBus::Events::SpawnPebbleAt,
        myLocation );
}

void PlayerControlsComponent::SetRotation()
{
    // Rotate the entity
    TransformBus::Event( GetEntityId(),
        &TransformBus::Events::SetLocalRotationQuaternion,
        Quaternion::CreateRotationZ( m_rotZ ) );
}

void PlayerControlsComponent::OnTick(
    float dt, AZ::ScriptTimePoint )
{
    static const Vector3 yUnit = Vector3::CreateAxisY( 1.f );
    static const Vector3 xUnit = Vector3::CreateAxisX( 1.f );

    // Figure out where the movement will take the entity
    AZ::Vector3 direction{ 0, 0, 0 };
    if ( m_isForward )
        direction += yUnit;
    if ( m_isBackward )
        direction -= yUnit;
    if ( m_isStrafingLeft )
        direction -= xUnit;
    if ( m_isStrafingRight )
        direction += xUnit;

    direction *= m_speed * dt /* Take frame time into account */;

#if defined(DEDICATED_SERVER)
    // Get the current orientation of the entity
    AZ::Quaternion q = Quaternion::CreateIdentity();
    TransformBus::EventResult( q, GetEntityId(), &TransformBus::Events::GetWorldRotationQuaternion );
    // Apply the orientation
    direction = q * direction;

    // Issue character move command
    MyCharacterBus::Event( GetEntityId(), &MyCharacterBus::Events::RequestVelocity, direction );
#endif

    using AnimBus = Integration::AnimGraphComponentRequestBus;
    AnimBus::Event( GetEntityId(), &AnimBus::Events::SetNamedParameterFloat, "Speed", direction.GetLengthSq() > 0 ? 10.f : 0.f );

    if ( direction.GetLengthSq() > 0 )
    {
        FootstepComponentBus::Event( GetEntityId(), &FootstepComponentBus::Events::TickFootstep, dt );
    }
}