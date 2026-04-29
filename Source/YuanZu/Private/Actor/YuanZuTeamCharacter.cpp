#include "Actor/YuanZuTeamCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"

// Sets default values
AYuanZuTeamCharacter::AYuanZuTeamCharacter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TeamCharacter = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TeamCharacter"));
	TeamCharacter->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AYuanZuTeamCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AYuanZuTeamCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AYuanZuTeamCharacter::SetTeamCharacter(ETeamType PlayerTeam)
{
	if (MI_TeamCharacter.IsEmpty() || !TeamCharacter) return;

	if (PlayerTeam == ETeamType::ETT_Red|| PlayerTeam == ETeamType::ETT_Blue)
	{
		TeamCharacter->SetVisibility(true);
	}
	else
	{
		TeamCharacter->SetVisibility(false);
	}

	switch (PlayerTeam)
	{
	case ETeamType::ETT_Red:
		TeamCharacter->SetMaterial(1, MI_TeamCharacter[0]);
		TeamCharacter->SetMaterial(2, MI_TeamCharacter[1]);
		TeamCharacter->SetMaterial(3, MI_TeamCharacter[2]);
		TeamCharacter->SetMaterial(7, MI_TeamCharacter[3]);
		break;
	case ETeamType::ETT_Blue:
		TeamCharacter->SetMaterial(1, MI_TeamCharacter[4]);
		TeamCharacter->SetMaterial(2, MI_TeamCharacter[5]);
		TeamCharacter->SetMaterial(3, MI_TeamCharacter[6]);
		TeamCharacter->SetMaterial(7, MI_TeamCharacter[7]);
		break;
	case ETeamType::ETT_None:
	default:
		break;
	}

}
