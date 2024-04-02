

#include "Enemy.h"
#include <Components/SphereComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/SceneComponent.h>
#include <C:\Program Files\Epic Games\UE_5.3\Engine\Source\Runtime\AIModule\Classes\AIController.h>

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// ��弦�� ������Ʈ�� ����
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetRelativeLocation(FVector(0,0,85));
	headShot->SetSphereRadius(16.0f);

	//üũ����Ʈ ����
	checkPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint1"));
	checkPoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint2"));
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	startLocation = GetActorLocation();
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

