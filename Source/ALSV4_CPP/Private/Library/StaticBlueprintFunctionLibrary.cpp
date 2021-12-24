// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/StaticBlueprintFunctionLibrary.h"

#include "Character/ALSBaseCharacter.h"

void UStaticBlueprintFunctionLibrary::DebugAuthorityClientFloat(AActor* Actor, float Time, int KeyPosition, FString StringVar, float FloatVar, FVector Vector1, FRotator Rotator1)
{
	FString Prefix;
	UObject* Object = Cast<UObject>(Actor);
	if (Object)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(Object, EGetWorldErrorMode::ReturnNull);
		if (World)
		{
			if (World->WorldType == EWorldType::PIE)
			{
				switch (World->GetNetMode())
				{
				case NM_Client:
					// GPlayInEditorID 0 is always the server, so 1 will be first client.
					// You want to keep this logic in sync with GeneratePIEViewportWindowTitle and UpdatePlayInEditorWorldDebugString
					Prefix = FString::Printf(TEXT("Client %d: "), GPlayInEditorID);
					break;
				case NM_DedicatedServer:
				case NM_ListenServer:
					Prefix = FString::Printf(TEXT("Server: "));
					break;
				case NM_Standalone:
					break;
				}
			}
		}
	}
	int32 UniqueID = -1;
	if (Actor)
	{
		AALSBaseCharacter* ALSBaseCharacter = Cast<AALSBaseCharacter>(Actor);
		if (ALSBaseCharacter)
		{
			/*if (ALSBaseCharacter->GetController())
			{
				UniqueID = ALSBaseCharacter->GetController()->GetUniqueID();
			}
			else*/
			{
				UniqueID = ALSBaseCharacter->GetUniqueID();
			}
		}
	}
	if (KeyPosition != -1)
	{
		KeyPosition += GPlayInEditorID;
		GEngine->AddOnScreenDebugMessage(KeyPosition, Time, FColor::Green, FString::Printf(TEXT("%s %f %s %f %s %s"), *Prefix , float(UniqueID), *StringVar, float (FloatVar), *Vector1.ToString(), *Rotator1.ToString()));
		//UE_LOG(LogTemp, Warning, TEXT("%s %f %s %f %s %s"), *Prefix, float(UniqueID), *StringVar, float(FloatVar), *Vector1.ToString(), *Rotator1.ToString());
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, Time, FColor::Green, FString::Printf(TEXT("%s %f %s %f %s %s"), *Prefix, float(UniqueID), *StringVar, float(FloatVar), *Vector1.ToString(), *Rotator1.ToString()));
		//UE_LOG(LogTemp, Warning, TEXT("%s %f %s %f %s %s"), *Prefix, float(UniqueID), *StringVar, float(FloatVar), *Vector1.ToString(), *Rotator1.ToString());
	}
}
