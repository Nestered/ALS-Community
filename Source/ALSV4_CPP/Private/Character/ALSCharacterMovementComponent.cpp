// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Haziq Fadhil
// Contributors:    Doğa Can Yanıkoğlu


#include "Character/ALSCharacterMovementComponent.h"
#include "Character/ALSBaseCharacter.h"

#include "Curves/CurveVector.h"
#include "Net/UnrealNetwork.h"

UALSCharacterMovementComponent::UALSCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UALSCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
                                                       const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);
	// Moved to UGSCharacterMovementComponent
	//if (!CharacterOwner)
	//{
	//	return;
	//}

	//// Set Movement Settings
	//if (bRequestMovementSettingsChange)
	//{
	//	const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait); // SavedAllowedGait
	//	MaxWalkSpeed = UpdateMaxWalkSpeed;
	//	MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
	//	if (PawnOwner->GetLocalRole() == ROLE_Authority)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("Authority: AllowedGait %f UpdateMaxWalkSpeed %f"), float(AllowedGait), float(UpdateMaxWalkSpeed));
	//	}
	//	if (PawnOwner->GetLocalRole() < ROLE_Authority)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("Client: AllowedGait %f UpdateMaxWalkSpeed %f"), float(AllowedGait), float(UpdateMaxWalkSpeed));
	//	}
	//	bRequestMovementSettingsChange = false;
	//}
}

void UALSCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	if (CurrentMovementSettings.MovementCurve)
	{
		// Update the Ground Friction using the Movement Curve.
		// This allows for fine control over movement behavior at each speed.
		GroundFriction = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Z;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

float UALSCharacterMovementComponent::GetMaxAcceleration() const
{
	// Update the Acceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxAcceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).X;
}

float UALSCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	// Update the Deceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxBrakingDeceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Y;
}

void UALSCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) // Client only
{
	Super::UpdateFromCompressedFlags(Flags);

	bRequestMovementSettingsChange = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

	bWantsStaminaChange = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

class FNetworkPredictionData_Client* UALSCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UALSCharacterMovementComponent* MutableThis = const_cast<UALSCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
		// Maximum distance character is allowed to lag behind server location when interpolating between updates.
		//MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 292.f;
		// Maximum distance beyond which character is teleported to the new server location without any smoothing.
		//MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 500.f;
	}

	return ClientPredictionData;
}

void UALSCharacterMovementComponent::FSavedMove_My::Clear()
{
	Super::Clear();

	bSavedRequestMovementSettingsChange = false;
	SavedAllowedGait = EALSGait::Walking;
	bSavedWantsStaminaChange = false;
	SavedStamina = 0.f;
}

uint8 UALSCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRequestMovementSettingsChange)
	{
		Result |= FLAG_Custom_0;
	}

	if (bSavedWantsStaminaChange)
	{
		Result |= FLAG_Custom_1;
	}
	
	return Result;
}

void UALSCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime,
                                                               FVector const& NewAccel,
                                                               class FNetworkPredictionData_Client_Character&
                                                               ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UALSCharacterMovementComponent* CharacterMovement = Cast<UALSCharacterMovementComponent>(Character->GetCharacterMovement());
	AALSBaseCharacter* ALSBaseCharacter = Cast<AALSBaseCharacter>(Character);
	if (CharacterMovement && ALSBaseCharacter)
	{
		bSavedRequestMovementSettingsChange = CharacterMovement->bRequestMovementSettingsChange;
		SavedAllowedGait = CharacterMovement->AllowedGait;

		//This is literally just the exact opposite of UpdateFromCompressed flags. We're taking the input
		//from the player and storing it in the saved move.
		bSavedWantsStaminaChange = CharacterMovement->bWantsStaminaChange;
		SavedStamina = CharacterMovement->Stamina;
		/*if (ALSBaseCharacter->GetLocalRole() == ROLE_Authority)
		{
			UE_LOG(LogTemp, Warning, TEXT("Authority: SetMoveFor SavedStamina %f"), float(SavedStamina));
		}
		if (ALSBaseCharacter->GetLocalRole() < ROLE_Authority)
		{
			UE_LOG(LogTemp, Warning, TEXT("Client: SetMoveFor SavedStamina %f"), float(SavedStamina));
		}*/
	}
}

void UALSCharacterMovementComponent::FSavedMove_My::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UALSCharacterMovementComponent* CharacterMovement = Cast<UALSCharacterMovementComponent>(Character->GetCharacterMovement());
	AALSBaseCharacter* ALSBaseCharacter = Cast<AALSBaseCharacter>(Character);
	if (CharacterMovement && ALSBaseCharacter)
	{
		CharacterMovement->AllowedGait = SavedAllowedGait;

		//This is just the exact opposite of SetMoveFor. It copies the state from the saved move to the movement
		//component before a correction is made to a client.
		CharacterMovement->Stamina = SavedStamina;
		/*if (ALSBaseCharacter->GetLocalRole() == ROLE_Authority)
		{
			UE_LOG(LogTemp, Log, TEXT("Authority: PrepMoveFor CharacterMovement->Stamina %f"), float(CharacterMovement->Stamina));
		}
		if (ALSBaseCharacter->GetLocalRole() < ROLE_Authority)
		{
			UE_LOG(LogTemp, Log, TEXT("Client: PrepMoveFor CharacterMovement->Stamina %f"), float(CharacterMovement->Stamina));
		}*/
	}
}

bool UALSCharacterMovementComponent::FSavedMove_My::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character,
	float MaxDelta) const
{
	if (bSavedRequestMovementSettingsChange != ((FSavedMove_My*)&NewMove)->bSavedRequestMovementSettingsChange)
	{
		return false;
	}

	if (SavedStamina != ((FSavedMove_My*)&NewMove)->SavedStamina)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

UALSCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UALSCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return MakeShared<FSavedMove_My>();
}

void UALSCharacterMovementComponent::Server_SetAllowedGait_Implementation(const EALSGait NewAllowedGait)
{
	AllowedGait = NewAllowedGait;
}

float UALSCharacterMovementComponent::GetMappedSpeed() const
{
	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed, and 3 = the Sprint Speed.
	// This allows us to vary the movement speeds but still use the mapped range in calculations for consistent results

	const float Speed = Velocity.Size2D();
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurrentMovementSettings.RunSpeed;
	const float LocSprintSpeed = CurrentMovementSettings.SprintSpeed;

	if (Speed > LocRunSpeed)
	{
		return FMath::GetMappedRangeValueClamped({LocRunSpeed, LocSprintSpeed}, {2.0f, 3.0f}, Speed);
	}

	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped({LocWalkSpeed, LocRunSpeed}, {1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped({0.0f, LocWalkSpeed}, {0.0f, 1.0f}, Speed);
}

void UALSCharacterMovementComponent::SetMovementSettings(FALSMovementSettings NewMovementSettings)
{
	// Set the current movement settings from the owner
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}

void UALSCharacterMovementComponent::SetAllowedGait(EALSGait NewAllowedGait)
{
	//UE_LOG(LogTemp, Warning, TEXT("SetAllowedGait"));
	if (AllowedGait != NewAllowedGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			//UE_LOG(LogTemp, Warning, TEXT("SetAllowedGait()PawnOwner->IsLocallyControlled()"));
			AllowedGait = NewAllowedGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				//UE_LOG(LogTemp, Warning, TEXT("SetAllowedGait()GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy"));
				Server_SetAllowedGait(NewAllowedGait);
			}
			bRequestMovementSettingsChange = true;
			//return;
		}
		//if (!PawnOwner->HasAuthority())
		//{
		//	//const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
		//	//MaxWalkSpeed = UpdateMaxWalkSpeed;
		//	//MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
		//}
	}
}

void UALSCharacterMovementComponent::SetForcedAllowedGait(EALSGait NewAllowedGait)
{
	//UE_LOG(LogTemp, Warning, TEXT("SetForcedAllowedGait"));
	if (AllowedGait == NewAllowedGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			//UE_LOG(LogTemp, Warning, TEXT("SetForcedAllowedGait()PawnOwner->IsLocallyControlled()"));
			AllowedGait = NewAllowedGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				//UE_LOG(LogTemp, Warning, TEXT("SetForcedAllowedGait()GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy"));
				Server_SetAllowedGait(NewAllowedGait);
				/*if (PawnOwner->GetLocalRole() == ROLE_Authority)
				{
					UE_LOG(LogTemp, Warning, TEXT("AuthorityAutonomousProxy:Server_SetAllowedGait(NewAllowedGait)"));
				}
				if (PawnOwner->GetLocalRole() < ROLE_Authority)
				{
					UE_LOG(LogTemp, Warning, TEXT("ClientAutonomousProxy: Server_SetAllowedGait(NewAllowedGait)"));
				}*/
			}
			bRequestMovementSettingsChange = true;
			//return;
		}
	}
	//if (!PawnOwner->HasAuthority())
	//{
	//	//const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
	//	//MaxWalkSpeed = UpdateMaxWalkSpeed;
	//	//MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
	//}
}

void UALSCharacterMovementComponent::ServerSetStamina_Implementation(const float& SendStamina)
{
	Stamina = SendStamina;
	//if (PawnOwner->GetLocalRole() == ROLE_Authority)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Authority: ServerSetStamina"));
	//}
	//if (PawnOwner->GetLocalRole() < ROLE_Authority)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Client: ServerSetStamina"));
	//}
	//UE_LOG(LogTemp, Warning, TEXT("ServerSetStamina"));
}

bool UALSCharacterMovementComponent::ServerSetStamina_Validate(const float& SendStamina)
{
	return true;
}

void UALSCharacterMovementComponent::DoStaminaChange()
{
	bWantsStaminaChange = true;
}

//void UALSCharacterMovementComponent::Client_SetAllowedGait_Implementation()
//{
//	// UNUSED???
//	UE_LOG(LogTemp, Warning, TEXT("UNUSED???"));
//	SetForcedAllowedGait(AllowedGait);
//	//if (PawnOwner->GetLocalRole() == ROLE_Authority)
//	//{
//	//	UE_LOG(LogTemp, Warning, TEXT("Authority: Client_SetAllowedGait_Implementation()"));
//	//}
//	//if (PawnOwner->GetLocalRole() < ROLE_Authority)
//	//{
//	//	UE_LOG(LogTemp, Warning, TEXT("Client: Client_SetAllowedGait_Implementation()"));
//	//}
//}