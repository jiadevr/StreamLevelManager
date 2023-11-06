// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TrySwitchLevelAction.generated.h"

UINTERFACE()
class UTrySwitchLevelAction : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STREAMLEVELMANAGER_API ITrySwitchLevelAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void TrySwitchFinished(bool bIsLoadCommand);
};
