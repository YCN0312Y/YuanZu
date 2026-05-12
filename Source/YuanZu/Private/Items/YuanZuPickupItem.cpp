// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/YuanZuPickupItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/WidgetComponent.h"
#include "Character/YuanZuCharacterBase.h"
#include "Components/YuanZuCombatComponent.h"
#include "UI/Weapon/YuanZuPickUp.h"

AYuanZuPickupItem::AYuanZuPickupItem()
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapSphere->SetSphereRadius(64.f);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(OverlapSphere);
	PickupWidget->SetWidgetSpace(EWidgetSpace::Screen);
	PickupWidget->SetDrawAtDesiredSize(true);
	PickupWidget->SetVisibility(false);

}

void AYuanZuPickupItem::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && OverlapSphere)
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AYuanZuPickupItem::OnSphereBeginOverlap);
		OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &AYuanZuPickupItem::OnSphereEndOverlap);
	}
	if (Pickup && PickupWidget)
	{
		PickupWidget->SetWidgetClass(Pickup);
		UYuanZuPickUp* NewPickUp = Cast<UYuanZuPickUp>(PickupWidget->GetUserWidgetObject());
		if (NewPickUp)
		{
			NewPickUp->SetItem(this);
		}
	}
}

void AYuanZuPickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AYuanZuPickupItem::Destroyed()
{
	Super::Destroyed();

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
}

void AYuanZuPickupItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AYuanZuCharacterBase* YZC = Cast<AYuanZuCharacterBase>(OtherActor);
	if (YZC)
	{
		YZC->SetOVerlappingItem(this);
	}
}

void AYuanZuPickupItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AYuanZuCharacterBase* YZC = Cast<AYuanZuCharacterBase>(OtherActor);
	if (YZC)
	{
		YZC->SetOVerlappingItem(nullptr);
	}
}

void AYuanZuPickupItem::ShowItemWidget(bool bIsVisible)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bIsVisible);
	}
}

