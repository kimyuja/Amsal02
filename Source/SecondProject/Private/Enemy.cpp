

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

	// ��弦�� ������Ʈ�� ����
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetupAttachment(GetCapsuleComponent());
	headShot->SetRelativeLocation(FVector(0,0,90));
	headShot->SetSphereRadius(20.0f);

	// ĸ�� ������Ʈ ũ�� ����
	GetCapsuleComponent()->SetCapsuleHalfHeight(70.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// SkelatalMesh ������Ʈ ����
	torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	torso->SetupAttachment(GetMesh());
	hand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand"));
	hand->SetupAttachment(GetMesh());
	pants = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pants"));
	pants->SetupAttachment(GetMesh());
	shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	shoes->SetupAttachment(GetMesh());


	//üũ����Ʈ ����
	checkPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint1"));
	checkPoint1->SetRelativeLocation(FVector(700,-700,0));
	checkPoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint2"));

	//ĳ������ �ִ� �ӵ�, ���ӵ��� ����.(���� = cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;

	//ĳ���Ͱ� �׺���̼� ��η� �̵��� �� ���� ����(���� �־�� ABP�� �۵���)
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


