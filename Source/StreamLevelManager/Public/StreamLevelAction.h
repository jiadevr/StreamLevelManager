// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StreamLevelAction.generated.h"


UINTERFACE()
class UStreamLevelAction : public UInterface
{
	GENERATED_BODY()
};

class STREAMLEVELMANAGER_API IStreamLevelAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void LevelSwitchFinished(bool bFinishAll);
	
};
