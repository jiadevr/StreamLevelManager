// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include"CoreMinimal.h"
#include"Subsystems/WorldSubsystem.h"
#include"MyStreamLevelManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwitchAllStreamLevelFinished);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrySwitchLevelFinished, bool, bIsLoadCommand);

USTRUCT()
struct FSwitchInfo
{
	GENERATED_BODY()
	TSet<FName> LevelToLoadInPreset;
	TSet<FName> LevelToLoadInCustom;
	TSet<FName> LevelToRemoveInPreset;
	TSet<FName> LevelToRemoveInCustom;
	UObject* NotificationReceiver;
};

UCLASS()
class STREAMLEVELMANAGER_API UMyStreamLevelManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//创建subsystem
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; };


	//加卸载完成委托
	UPROPERTY(BlueprintAssignable, Category="OnFinished")
	FOnTrySwitchLevelFinished OnTrySwitchLevelFinished;


	//单独加载、卸载关卡
	UFUNCTION(BlueprintCallable, Category="LoadExample")
	void TryLoadSomeLevels(const TSet<FName>& LevelNames, UObject* NotifyTarget);

	UFUNCTION(BlueprintCallable, Category="UnloadExample")
	void TryUnloadSomeLevels(const TSet<FName>& LevelNames, UObject* NotifyTarget);

protected:
	//加卸载通知对象
	UPROPERTY()
	UObject* LoadFinishNotifyTarget;

	UPROPERTY()
	UObject* UnLoadFinishNotifyTarget;

	void NotifyInvoker(bool bIsLoadCommand);

	//单独加载、卸载关卡
	UFUNCTION()
	void TryLoadCallBackTarget();

	UFUNCTION()
	void TryUnLoadCallBackTarget();

	int32 LoadCounter = 0;

	int32 UnloadCounter = 0;

	//整合函数通知对象
	UPROPERTY()
	UObject* OneFuncNotifyTarget = nullptr;

	//整合在一个函数中
	UFUNCTION()
	void CallBackForOneFunc();

	int32 CounterForSwitchInOneFunc = 0;
public:
	//整合
	UFUNCTION(BlueprintCallable, Category="StreamLevelsManager")
	void SwitchLevelsInOneFunc(const TSet<FName>& LevelToAdd, const TSet<FName>& LevelToRemove,
							   UObject* NotificationReceiver);

	//根据预设加载
public:
	//获取当前所有关卡TSet<FName>
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="StreamLevelsManager")
	void GetCurrentStreamLevels(TSet<FName>& LevelNames);

	//在预设基础上支持用户定义加卸载
	UFUNCTION(BlueprintCallable)
	void LoadWithPresetInfo(const TSet<FName>& PresetLoadLevels, const TSet<FName>& PresetRemoveLevels,
	                        const TSet<FName>& CustomLoadLevels, const TSet<FName>& CustomRemoveLevel,
	                        UObject* NotificationReceiver);


	//队列加卸载

protected:
	//交并集计算
	void GetStreamCommandResult(const TSet<FName>& PresetLoadLevels, const TSet<FName>& PresetRemoveLevels,
	                            const TSet<FName>& CustomLoadLevels, const TSet<FName>& CustomRemoveLevel,
	                            TSet<FName>& LoadLevelResult, TSet<FName>& UnLoadLevelResult);

	TQueue<FSwitchInfo> SwitchInfosQueue;
	//队列计数器
	int32 QueueElementCount=0;

	bool bIsSwitching = false;

	void SwitchLevelInternalFunc();

	int32 InternalCounter = 0;

	UFUNCTION()
	void SwitchCallbackInternal();

	void NotifyOnSwitchLoadFinish() const;

	FSwitchInfo CurrentInfo;



public:
	//队列加卸载蓝图调用
	UFUNCTION(BlueprintCallable)
	void SwitchStreamLevels(const TSet<FName>& PresetLoadLevels, const TSet<FName>& PresetRemoveLevels,
	                        const TSet<FName>& CustomLoadLevels, const TSet<FName>& CustomRemoveLevel,
	                        UObject* NotificationReceiver);
	//全部完成时调用委托
	UPROPERTY(BlueprintAssignable)
	FOnSwitchAllStreamLevelFinished SwitchAllStreamLevelFinished;


	////////////////////////////////////////////////////////////////////////
};
