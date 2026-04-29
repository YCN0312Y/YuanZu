// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YuanZuBulletShell.generated.h"

class UStaticMeshComponent;
class USoundCue;

struct FTimerHandle;

UCLASS()
class YUANZU_API AYuanZuBulletShell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AYuanZuBulletShell();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//子弹壳
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BulletShellMesh;
	//子弹壳弹出力度
	UPROPERTY(EditAnywhere, Category = "YuanZu|Shell", meta = (ToolTip = "Used to set the ejection force of the cartridge case"))
	float ShellEjectionImpulse;
	UPROPERTY(EditAnywhere, Category = "YuanZu|Sound")
	USoundCue* ShellEjectionSound;

private:
	//延迟销毁子弹壳
	void DelayedDestroy();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);


};
