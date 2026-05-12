// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YuanZuPickupItemSpawnPoint.generated.h"

class AYuanZuPickupItem;

UCLASS()
class YUANZU_API AYuanZuPickupItemSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AYuanZuPickupItemSpawnPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//生成的物品
	UPROPERTY()
	TObjectPtr<AYuanZuPickupItem>Item;
	//物品列表
	UPROPERTY(EditAnywhere, Category = "YuanZu|Item")
	TArray<TSubclassOf<AYuanZuPickupItem>>ItemListClass;
	//最小时间
	UPROPERTY(EditAnywhere, Category = "YuanZu|Item")
	int32 SpawnMinTime;
	//最大时间
	UPROPERTY(EditAnywhere, Category = "YuanZu|Item")
	int32 SpawnMaxTime;

	FTimerHandle SpawnItemTimerHandle;
private:
	//生成物品
	void SpawnItem();
	//根据定时器时间生成物品
	UFUNCTION()
	void SpawnItemTimer(AActor* DestroyedActor);
};
