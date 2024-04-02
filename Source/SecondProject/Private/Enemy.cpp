

#include "Enemy.h"
#include <Components/SphereComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/SceneComponent.h>
#include <C:\Program Files\Epic Games\UE_5.3\Engine\Source\Runtime\AIModule\Classes\AIController.h>
<<<<<<< Updated upstream
=======
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/CapsuleComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/SkinnedMeshComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/TextRenderComponent.h>
#include <../../../../../../../Source/Runtime/Core/Public/Internationalization/Text.h>
#include <../../../../../../../Source/Runtime/AIModule/Classes/Perception/AIPerceptionComponent.h>
#include "../SecondProjectCharacter.h"
>>>>>>> Stashed changes

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// ��弦�� ������Ʈ�� ����
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
<<<<<<< Updated upstream
	headShot->SetRelativeLocation(FVector(0,0,85));
	headShot->SetSphereRadius(16.0f);
=======
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

	// TextRender ������Ʈ ����
	warningComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text"));
	warningComp->SetRelativeLocation(FVector(0, 0, 110));
	warningComp->SetHorizontalAlignment(EHTA_Center);
	warningComp->SetText(warningText);
	
	// AI perception ����
	aiperception = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception");
>>>>>>> Stashed changes

	//üũ����Ʈ ����
	checkPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint1"));
	checkPoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint2"));
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	startLocation = GetActorLocation();
<<<<<<< Updated upstream
=======

	if (anim1 != nullptr)
	{
		PlayAnimMontage(anim1);
		UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
	}

	aiperception->OnPerceptionUpdated.AddDynamic(this, &AEnemy::OnPerceptionPlayer)

>>>>>>> Stashed changes
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

<<<<<<< Updated upstream
=======
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

void AEnemy::OnPerceptionPlayer(const TArray<AActor*>& UpdatedActors)
{
	//ASecondProjectCharacter* player = Cast<ASecondProjectCharacter>(UpdatedActors);


}

>>>>>>> Stashed changes
