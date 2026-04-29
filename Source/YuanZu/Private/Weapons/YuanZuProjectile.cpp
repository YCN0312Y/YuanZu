// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/YuanZuProjectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Character/YuanZuCharacterBase.h"
#include "YuanZu/YuanZu.h"

AYuanZuProjectile::AYuanZuProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Overlap);
	CollisionBox->SetBoxExtent(FVector(1.5f, 0.5f, 0.5f));

}

void AYuanZuProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	CollisionBox->IgnoreActorWhenMoving(this, true);

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionBox, TEXT(""), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AYuanZuProjectile::OnHit);
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AYuanZuProjectile::OnBeginOverlap);
	}

}

void AYuanZuProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	if (!HasAuthority()) return;

	if (OtherActor == GetOwner())
	{
		return;
	}
	AYuanZuCharacterBase* Character = Cast<AYuanZuCharacterBase>(OtherActor);
	if (Character)
	{
		Character->PlayHitReactMontage(true);
	}
	Destroy();
}

void AYuanZuProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AYuanZuCharacterBase* Character = Cast<AYuanZuCharacterBase>(OtherActor);
	if (Character)
	{
		Destroy();
	}

}

void AYuanZuProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AYuanZuProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}
