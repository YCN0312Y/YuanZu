// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/Rests/YuanZuItemType.h"
#include "YuanZuPickupItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USoundCue;
class UWidgetComponent;
class UYuanZuPickUp;

UCLASS()
class YUANZU_API AYuanZuPickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AYuanZuPickupItem();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	//控件组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent>PickupWidget;
	//控件
	UPROPERTY(EditAnywhere, Category = "YuanZu|UI")
	TSubclassOf<UYuanZuPickUp>Pickup;
	//重叠碰撞
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent>OverlapSphere;
	//拾取物模型
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent>PickupMesh;
	//拾取音效
	UPROPERTY(EditAnywhere, Category = "YuanZu|Sound")
	TObjectPtr<USoundCue>PickupSound;
	//物品名称

protected:
	//物品类型
	UPROPERTY(EditAnywhere, Category = "YuanZu|Item")
	EItemType ItemType;

public:
	//物品名称
	UPROPERTY(EditAnywhere, Category = "YuanZu|Sound")
	FText ItemName;

protected:
	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


public:
	void ShowItemWidget(bool bIsVisible);
	//获取物品类型
	FORCEINLINE EItemType GetItemType()const { return ItemType; }
};
