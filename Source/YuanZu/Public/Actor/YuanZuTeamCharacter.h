#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UI/Lobby/Rests/YuanZuTeamSlotInfo.h"
#include "YuanZuTeamCharacter.generated.h"

class USkeletalMeshComponent;
class UMaterialInterface;
class UStaticMesh;

UCLASS()
class YUANZU_API AYuanZuTeamCharacter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AYuanZuTeamCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//查看的角色
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* TeamCharacter;
	//角色材质
	UPROPERTY(EditAnywhere, Category = YuanZu)
	TArray<UMaterialInterface*> MI_TeamCharacter;

	//设置显示次数
	int32 SettingTime = 0;
public:
	void SetTeamCharacter(ETeamType PlayerTeam);
	
	FORCEINLINE USkeletalMeshComponent* GetTeamCharacter() const { return TeamCharacter; }
};
