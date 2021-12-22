// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StaticBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API UStaticBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Debug")
	static void DebugAuthorityClientFloat(AActor* Actor, float Time, int KeyPosition, FString StringVar, float FloatVar);

	
};
