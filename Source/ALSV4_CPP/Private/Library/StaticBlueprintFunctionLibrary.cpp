// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/StaticBlueprintFunctionLibrary.h"

#include "Character/ALSBaseCharacter.h"

void UStaticBlueprintFunctionLibrary::DebugAuthorityClientFloat(AActor* Actor, float Time, int KeyPosition, FString StringVar, float FloatVar)
{
	int32 UniqueID = -1;
	ENetRole NNetRole;
	if (Actor)
	{
		AALSBaseCharacter* ALSBaseCharacter = Cast<AALSBaseCharacter>(Actor);
		if (ALSBaseCharacter)
		{
			NNetRole = ALSBaseCharacter->GetLocalRole();
			FString ControllerName = FString(" NoController: ");
			if (ALSBaseCharacter->GetController())
			{
				UniqueID = ALSBaseCharacter->GetController()->GetUniqueID();
			}
			else
			{
				//UniqueID = ALSBaseCharacter->GetUniqueID();
			}
		}
	}
	if (KeyPosition != -1)
	{
		if (NNetRole == ROLE_Authority)
		{
			GEngine->AddOnScreenDebugMessage(KeyPosition, Time, FColor::Red, FString::Printf(TEXT("Auth: %f %s %f"), float(UniqueID), *StringVar, float (FloatVar)));
			UE_LOG(LogTemp, Warning, TEXT("Auth: %f %s %f"), float(UniqueID), *StringVar, float(FloatVar));
		}
		if (NNetRole < ROLE_Authority)
		{
			GEngine->AddOnScreenDebugMessage(KeyPosition + 1, Time, FColor::Green, FString::Printf(TEXT("Client: %f %s %f"), float(UniqueID), *StringVar, float(FloatVar)));
			UE_LOG(LogTemp, Warning, TEXT("Client: %f %s %f"), float(UniqueID), *StringVar, float(FloatVar));
		}
	}
	else
	{
		if (NNetRole == ROLE_Authority)
		{
			GEngine->AddOnScreenDebugMessage(-1, Time, FColor::Red, FString::Printf(TEXT("Auth: %f %s %f"), float(UniqueID), *StringVar, float(FloatVar)));
			UE_LOG(LogTemp, Warning, TEXT("Auth: %f %s %f"), float(UniqueID), *StringVar, float(FloatVar));
		}
		if (NNetRole < ROLE_Authority)
		{
			GEngine->AddOnScreenDebugMessage(-1, Time, FColor::Green, FString::Printf(TEXT("Client: %f %s %f"), float(UniqueID), *StringVar, float(FloatVar)));
			UE_LOG(LogTemp, Warning, TEXT("Client: %f %s %f"), float(UniqueID), *StringVar, float(FloatVar));
		}
	}
}
