

#include "Enemy.h"
#include <Components/SphereComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/SceneComponent.h>
#include <C:\Program Files\Epic Games\UE_5.3\Engine\Source\Runtime\AIModule\Classes\AIController.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/CapsuleComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/SkinnedMeshComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// 헤드샷용 컴포넌트를 생성
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetupAttachment(GetCapsuleComponent());
	headShot->SetRelativeLocation(FVector(0,0,90));
	headShot->SetSphereRadius(20.0f);

	// 캡슐 컴포넌트 크기 조절
	GetCapsuleComponent()->SetCapsuleHalfHeight(70.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// SkelatalMesh 컴포넌트 생성
	torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	torso->SetupAttachment(GetMesh());
	hand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand"));
	hand->SetupAttachment(GetMesh());
	pants = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pants"));
	pants->SetupAttachment(GetMesh());
	shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	shoes->SetupAttachment(GetMesh());


	//체크포인트 생성
	checkPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint1"));
	checkPoint1->SetRelativeLocation(FVector(700,-700,0));
	checkPoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint2"));

	//캐릭터의 최대 속도, 가속도를 설정.(단위 = cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;

	//캐릭터가 네비게이션 경로로 이동할 때 가속 적용(켜저 있어야 ABP가 작동함)
	GetCharacterMovement()->UseAccelerationForPathFollowing();
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	startLocation = GetActorLocation();

	if (anim1 != nullptr)
	{
		PlayAnimMontage(anim1);
		UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
	}

}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bArrive1 && bGo)
	{
		BasicMoveCycle1();
	}
	else if (!bArrive2 && bGo)
	{
		BasicMoveCycle2();
	}
	else
	{
		delayCheck++;
	}

	if (delayCheck > 1000)
	{
		bGo = true;
		delayCheck = 0;
	}

	//UE_LOG(LogTemp, Warning,TEXT("%d"),delayCheck);

	//UE_LOG(LogTemp, Warning, TEXT("%f"), UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint1->GetRelativeLocation()));
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BasicMoveCycle1()
{
	AAIController* alCon = Cast<AAIController>(GetController());

	alCon->MoveToLocation(checkPoint1->GetRelativeLocation());
	float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint1->GetRelativeLocation());
	if (check < 100)
	{
		bArrive1 = true;
		bArrive2 = false;
		bGo = false;
		if (anim1 != nullptr)
		{
			PlayAnimMontage(anim1);
			UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
		}
	}

}

void AEnemy::BasicMoveCycle2()
{
	AAIController* alCon = Cast<AAIController>(GetController());
	alCon->MoveToLocation(checkPoint2->GetRelativeLocation());
	float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint2->GetRelativeLocation());
	if (check < 100)
	{
		bArrive2 = true;
		bArrive1 = false;
		bGo = false;
		if (anim1 != nullptr)
		{
			PlayAnimMontage(anim1);
			UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
		}
	}
}

void AEnemy::Damaged(float damage)
{
	Life = FMath::Clamp(100.0 - damage, 0, 100);
	
	if (Life <= 0)
	{
		GetCharacterMovement()->DisableMovement();
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0);
		headShot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	UE_LOG(LogTemp, Warning, TEXT("%f"), Life);
}


