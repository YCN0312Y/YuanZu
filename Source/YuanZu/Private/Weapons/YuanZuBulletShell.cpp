// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/YuanZuBulletShell.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Sets default values
AYuanZuBulletShell::AYuanZuBulletShell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BulletShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("YuanZuBulletShell"));
	SetRootComponent(BulletShellMesh);
	BulletShellMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BulletShellMesh->SetCollisionObjectType(ECC_WorldDynamic);
	BulletShellMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BulletShellMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BulletShellMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	BulletShellMesh->SetUseCCD(true);
	BulletShellMesh->SetSimulatePhysics(true);//开启物理效果
	BulletShellMesh->SetEnableGravity(true);//开启重力效果
	BulletShellMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;

}

// Called when the game starts or when spawned
void AYuanZuBulletShell::BeginPlay()
{
	Super::BeginPlay();

	BulletShellMesh->OnComponentHit.AddDynamic(this, &AYuanZuBulletShell::OnHit);
	BulletShellMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

// Called every frame
void AYuanZuBulletShell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AYuanZuBulletShell::DelayedDestroy()
{
	Destroy();
}

void AYuanZuBulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	if (ShellEjectionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShellEjectionSound, GetActorLocation());

	}
	FTimerHandle MyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(MyTimerHandle, this, &AYuanZuBulletShell::DelayedDestroy, 3.f, false);

}

