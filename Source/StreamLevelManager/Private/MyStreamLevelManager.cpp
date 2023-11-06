// Fill out your copyright notice in the Description page of Project Settings.


#include "MyStreamLevelManager.h"
#include"StreamLevelAction.h"
#include "TrySwitchLevelAction.h"
#include"Engine/LevelStreaming.h"
#include"Kismet/GameplayStatics.h"

void UMyStreamLevelManager::TryLoadSomeLevels(const TSet<FName>& LevelNames, UObject* NotifyTarget)
{
	FLatentActionInfo LatentForTryLoadLevel = FLatentActionInfo(1, 1,TEXT("TryLoadCallBackTarget"), this);
	LoadCounter = LevelNames.Num();
	LoadFinishNotifyTarget = NotifyTarget;
	for (const auto& CurrentName : LevelNames)
	{
		LatentForTryLoadLevel.UUID = FMath::Rand();
		UGameplayStatics::LoadStreamLevel(this, CurrentName, true, false, LatentForTryLoadLevel);
	}
}

void UMyStreamLevelManager::TryUnloadSomeLevels(const TSet<FName>& LevelNames, UObject* NotifyTarget)
{
	FLatentActionInfo LatentForTryUnLoadLevel = FLatentActionInfo(2, 1,TEXT("TryUnLoadCallBackTarget"), this);
	UnloadCounter = LevelNames.Num();
	UnLoadFinishNotifyTarget = NotifyTarget;
	for (const auto& CurrentName : LevelNames)
	{
		LatentForTryUnLoadLevel.UUID = FMath::Rand();
		UGameplayStatics::UnloadStreamLevel(this, CurrentName, LatentForTryUnLoadLevel, false);
	}
}

void UMyStreamLevelManager::TryLoadCallBackTarget()
{
	UE_LOG(LogTemp, Display, TEXT("Load Level Finished!"))
	LoadCounter--;
	if (LoadCounter <= 0)
	{
		UE_LOG(LogTemp, Display, TEXT("All Load Level Done! Call Interface And Delegate"));
		NotifyInvoker(true);
	}
}

void UMyStreamLevelManager::TryUnLoadCallBackTarget()
{
	UE_LOG(LogTemp, Display, TEXT("UnLoad Level Finished!"));
	UnloadCounter--;
	if (UnloadCounter <= 0)
	{
		UE_LOG(LogTemp, Display, TEXT("All Unload Level Done! Call Interface And Delegate"));
		NotifyInvoker(false);
	}
}

void UMyStreamLevelManager::NotifyInvoker(bool bIsLoadCommand)
{
	UObject* FinishNotifyTarget = bIsLoadCommand ? LoadFinishNotifyTarget : UnLoadFinishNotifyTarget;
	if (FinishNotifyTarget && FinishNotifyTarget->GetClass()->ImplementsInterface(UTrySwitchLevelAction::StaticClass()))
	{
		ITrySwitchLevelAction::Execute_TrySwitchFinished(FinishNotifyTarget, bIsLoadCommand);
	}
	if (OnTrySwitchLevelFinished.IsBound())
	{
		OnTrySwitchLevelFinished.Broadcast(bIsLoadCommand);
	}
}

void UMyStreamLevelManager::GetCurrentStreamLevels(TSet<FName>& LevelNames)
{
	UWorld* CurrentWorld = GetWorld();
	TArray<ULevelStreaming*> StreamingLevels = CurrentWorld->GetStreamingLevels();
	LevelNames.Empty();
	for (const auto& StreamingLevel : StreamingLevels)
	{
		UE_LOG(LogTemp, Display, TEXT("LevelName: %s, State: %s"), *UWorld::RemovePIEPrefix(
			       FPackageName::GetShortName(StreamingLevel->GetWorldAssetPackageFName())),
		       ULevelStreaming::EnumToString(StreamingLevel->GetCurrentState()));
		if (ULevelStreaming::ECurrentState::LoadedVisible != StreamingLevel->GetCurrentState())
		{
			continue;
		}
		//去除前缀(搞虚幻的风行者(https://zhuanlan.zhihu.com/p/552324547))
		///Game/LevelSwitcher/UEDPIE_0_StreamLevel8 => StreamLevel8
		FString LevelCleanName = UWorld::RemovePIEPrefix(
			FPackageName::GetShortName(StreamingLevel->GetWorldAssetPackageFName()));

		LevelNames.Add(FName(*LevelCleanName));
	}
	return;
}

void UMyStreamLevelManager::LoadWithPresetInfo(const TSet<FName>& PresetLoadLevels,
                                               const TSet<FName>& PresetRemoveLevels,
                                               const TSet<FName>& CustomLoadLevels,
                                               const TSet<FName>& CustomRemoveLevel, UObject* NotificationReceiver)
{
	TSet<FName> LevelsLoadRequiredResult;
	TSet<FName> LevelsUnLoadRequiredResult;
	GetStreamCommandResult(PresetLoadLevels, PresetRemoveLevels, CustomLoadLevels, CustomRemoveLevel,
	                       LevelsLoadRequiredResult, LevelsUnLoadRequiredResult);

	//和用户传入的取并集
	SwitchLevelsInOneFunc(LevelsLoadRequiredResult, LevelsUnLoadRequiredResult, NotificationReceiver);
}

void UMyStreamLevelManager::SwitchLevelsInOneFunc(const TSet<FName>& LevelToAdd, const TSet<FName>& LevelToRemove,
                                                  UObject* NotificationReceiver)
{
	//检测是否有重叠关卡，如果有则不加载
	const TSet<FName> OverlappedLevelName = LevelToAdd.Intersect(LevelToRemove);
	TSet<FName> LevelToAddResultInOneFunc = LevelToAdd;
	TSet<FName> LevelToRemoveResultInOneFunc = LevelToRemove;
	if (!OverlappedLevelName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Overlapped %d Level(s) Detected! Those Level Will Not Be Loaded"),
		       OverlappedLevelName.Num());
		LevelToAddResultInOneFunc = LevelToAddResultInOneFunc.Difference(OverlappedLevelName);
	}
	if (LevelToAddResultInOneFunc.Contains("None"))
	{
		LevelToAddResultInOneFunc.Remove("None");
	}
	if (LevelToRemoveResultInOneFunc.Contains("None"))
	{
		LevelToRemoveResultInOneFunc.Remove("None");
	}
	CounterForSwitchInOneFunc = LevelToAddResultInOneFunc.Num() + LevelToRemoveResultInOneFunc.Num();
	OneFuncNotifyTarget = NotificationReceiver;
	// TryLoadSomeLevels(LevelToAddResultInOneFunc,NotificationReceiver);
	// TryUnloadSomeLevels(LevelToRemoveResultInOneFunc,NotificationReceiver);
	//转化成下面的内容
	FLatentActionInfo LatentActionInOneFunc = FLatentActionInfo(3, 1,TEXT("CallBackForOneFunc"), this);
	if (!LevelToAddResultInOneFunc.IsEmpty())
	{
		for (const auto& CurrentName : LevelToAddResultInOneFunc)
		{
			LatentActionInOneFunc.UUID = FMath::Rand();
			UGameplayStatics::LoadStreamLevel(this, CurrentName, true, false, LatentActionInOneFunc);
		}
	}
	if (!LevelToRemoveResultInOneFunc.IsEmpty())
	{
		for (const auto& CurrentName : LevelToRemoveResultInOneFunc)
		{
			LatentActionInOneFunc.UUID = FMath::Rand();
			UGameplayStatics::UnloadStreamLevel(this, CurrentName, LatentActionInOneFunc, false);
		}
	}
}

void UMyStreamLevelManager::GetStreamCommandResult(const TSet<FName>& PresetLoadLevels,
                                                   const TSet<FName>& PresetRemoveLevels,
                                                   const TSet<FName>& CustomLoadLevels,
                                                   const TSet<FName>& CustomRemoveLevel,
                                                   TSet<FName>& LoadLevelResult, TSet<FName>& UnLoadLevelResult)
{
	TSet<FName> CurrentLoadedLevels;
	GetCurrentStreamLevels(CurrentLoadedLevels);
	//计算当前需要加载但不在当前已加载的关卡
	TSet<FName> LevelsLoadRequired = PresetLoadLevels.Union(CustomLoadLevels);
	LevelsLoadRequired = LevelsLoadRequired.Difference(CustomRemoveLevel);
	LoadLevelResult = LevelsLoadRequired.Difference(CurrentLoadedLevels);
	//计算需要卸载的且当前已经加载的关卡
	TSet<FName> LevelsUnLoadRequired = PresetRemoveLevels.Union(CustomRemoveLevel);
	LevelsUnLoadRequired = LevelsUnLoadRequired.Difference(CustomLoadLevels);
	UnLoadLevelResult = LevelsUnLoadRequired.Intersect(CurrentLoadedLevels);
}

void UMyStreamLevelManager::CallBackForOneFunc()
{
	CounterForSwitchInOneFunc--;
	if (CounterForSwitchInOneFunc <= 0)
	{
		//因为整合，下面传递了True参数
		if (OneFuncNotifyTarget && OneFuncNotifyTarget->GetClass()->ImplementsInterface(
			UTrySwitchLevelAction::StaticClass()))
		{
			ITrySwitchLevelAction::Execute_TrySwitchFinished(OneFuncNotifyTarget, true);
		}
		if (OnTrySwitchLevelFinished.IsBound())
		{
			OnTrySwitchLevelFinished.Broadcast(true);
		}
	}
}

void UMyStreamLevelManager::SwitchStreamLevels(const TSet<FName>& PresetLoadLevels,
                                               const TSet<FName>& PresetRemoveLevels,
                                               const TSet<FName>& CustomLoadLevels,
                                               const TSet<FName>& CustomRemoveLevels,
                                               UObject* NotificationReceiver)
{
	FSwitchInfo SwitchCommandInfo;
	SwitchCommandInfo.LevelToLoadInPreset = PresetLoadLevels;
	SwitchCommandInfo.LevelToRemoveInPreset = PresetRemoveLevels;
	SwitchCommandInfo.LevelToLoadInCustom = CustomLoadLevels;
	SwitchCommandInfo.LevelToRemoveInCustom = CustomRemoveLevels;
	SwitchCommandInfo.NotificationReceiver = NotificationReceiver;
	SwitchInfosQueue.Enqueue(SwitchCommandInfo);
	QueueElementCount++;
	if (bIsSwitching)
	{
		UE_LOG(LogTemp, Display, TEXT("Switching, This Command Was Enqueued,Current QueneLength:%d"),
		       QueueElementCount);
		return;
	}
	SwitchLevelInternalFunc();
}

void UMyStreamLevelManager::SwitchLevelInternalFunc()
{
	bIsSwitching = true;
	//从队列中取值
	if (SwitchInfosQueue.IsEmpty())
	{
		QueueElementCount = 0;
		bIsSwitching = false;
		return;
	}
	SwitchInfosQueue.Dequeue(CurrentInfo);
	QueueElementCount--;
	TSet<FName> LoadResult;
	TSet<FName> UnloadResult;
	GetStreamCommandResult(CurrentInfo.LevelToLoadInPreset, CurrentInfo.LevelToRemoveInPreset,
	                       CurrentInfo.LevelToLoadInCustom, CurrentInfo.LevelToRemoveInCustom, LoadResult,
	                       UnloadResult);
	//检测是否有重叠关卡，如果有则不加载
	const TSet<FName> OverlappedLevelName = LoadResult.Intersect(UnloadResult);
	if (!OverlappedLevelName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Overlapped %d Level(s) Detected! Those Level Will Not Be Loaded"),
		       OverlappedLevelName.Num());
		LoadResult = LoadResult.Difference(OverlappedLevelName);
	}
	if (LoadResult.Contains("None"))
	{
		LoadResult.Remove("None");
	}
	if (UnloadResult.Contains("None"))
	{
		UnloadResult.Remove("None");
	}
	InternalCounter = LoadResult.Num() + UnloadResult.Num();
	FLatentActionInfo LatentActionInOneFunc = FLatentActionInfo(4, 1,TEXT("SwitchCallbackInternal"), this);
	if (!LoadResult.IsEmpty())
	{
		for (const auto& CurrentName : LoadResult)
		{
			LatentActionInOneFunc.UUID = FMath::Rand();
			UGameplayStatics::LoadStreamLevel(this, CurrentName, true, false, LatentActionInOneFunc);
		}
	}
	if (!UnloadResult.IsEmpty())
	{
		for (const auto& CurrentName : UnloadResult)
		{
			LatentActionInOneFunc.UUID = FMath::Rand();
			UGameplayStatics::UnloadStreamLevel(this, CurrentName, LatentActionInOneFunc, false);
		}
	}
}

void UMyStreamLevelManager::SwitchCallbackInternal()
{
	InternalCounter--;
	if (InternalCounter > 0)
	{
		return;
	}
	NotifyOnSwitchLoadFinish();
	SwitchLevelInternalFunc();
}

void UMyStreamLevelManager::NotifyOnSwitchLoadFinish() const
{
	const bool bIsAllCommandFinished = SwitchInfosQueue.IsEmpty();
	UE_LOG(LogTemp,Display,TEXT("Finish One Of Switch Command,Current Command Queue Length: %d"),QueueElementCount);
	if (CurrentInfo.NotificationReceiver && CurrentInfo.NotificationReceiver->GetClass()->ImplementsInterface(
		UStreamLevelAction::StaticClass()))
	{
		IStreamLevelAction::Execute_LevelSwitchFinished(CurrentInfo.NotificationReceiver, bIsAllCommandFinished);
	}
	if (bIsAllCommandFinished && SwitchAllStreamLevelFinished.IsBound())
	{
		SwitchAllStreamLevelFinished.Broadcast();
	}
}



