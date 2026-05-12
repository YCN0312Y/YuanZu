// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/YuanZuPickupItemSpawnPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Items/YuanZuPickupItem.h"

// Sets default values
AYuanZuPickupItemSpawnPoint::AYuanZuPickupItemSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

}

// Called when the game starts or when spawned
void AYuanZuPickupItemSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		SpawnItem();
	}
}

// Called every frame
void AYuanZuPickupItemSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AYuanZuPickupItemSpawnPoint::SpawnItem()
{
	if (ItemListClass.IsEmpty())return;

	int32 RandomItem = FMath::RandRange(0, ItemListClass.Num() - 1);
	Item = GetWorld()->SpawnActor<AYuanZuPickupItem>(ItemListClass[RandomItem], GetActorTransform());
	if (Item && HasAuthority())
	{
		Item->OnDestroyed.AddDynamic(this, &AYuanZuPickupItemSpawnPoint::SpawnItemTimer);
	}
}

void AYuanZuPickupItemSpawnPoint::SpawnItemTimer(AActor* DestroyedActor)
{
	float RandomTime = FMath::RandRange(SpawnMinTime, SpawnMaxTime);
	GetWorldTimerManager().SetTimer(SpawnItemTimerHandle, this, &AYuanZuPickupItemSpawnPoint::SpawnItem, RandomTime);
}

