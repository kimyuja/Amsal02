#include "Enemy.h"
#include <Components/SphereComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/SceneComponent.h>
#include <C:\Program Files\Epic Games\UE_5.3\Engine\Source\Runtime\AIModule\Classes\AIController.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/CapsuleComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/SkinnedMeshComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/TextRenderComponent.h>
#include "EnemyCon.h"
#include "../SecondProjectCharacter.h"
#include <../../../../../../../Source/Runtime/Engine/Public/EngineUtils.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Engine/HitResult.h>
#include <../../../../../../../Source/Runtime/Engine/Public/CollisionQueryParams.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <../../../../../../../Source/Runtime/AIModule/Classes/Perception/AIPerceptionComponent.h>
#include <../../../../../../../Source/Runtime/NavigationSystem/Public/NavigationSystem.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Animation/AnimInstance.h>

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// ��弦�� ������Ʈ�� ����
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetupAttachment(GetMesh());
	headShot->SetRelativeLocation(FVector(0, 0, 100));
	headShot->SetSphereRadius(20.0f);

	// ĸ�� ������Ʈ ũ�� ����
	GetCapsuleComponent()->SetCapsuleHalfHeight(80.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// �Ž� ���� ����
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));

	// SkelatalMesh ������Ʈ ����
	torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	torso->SetupAttachment(GetMesh());
	hand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand"));
	hand->SetupAttachment(GetMesh());
	pants = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pants"));
	pants->SetupAttachment(GetMesh());
	shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	shoes->SetupAttachment(GetMesh());

	//��赵 ǥ��
	warningComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Warning"));
	warningComp->SetupAttachment(GetMesh());
	warningComp->SetText(warningText);
	warningComp->SetRelativeLocation(FVector(0, 0, 130));

	//ĳ������ �ִ� �ӵ�, ���ӵ��� ����.(���� = cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;

	//ĳ���Ͱ� �׺���̼� ��η� �̵��� �� ���� ����(���� �־�� ABP�� �۵���)
	GetCharacterMovement()->UseAccelerationForPathFollowing();

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	aiCon = Cast<AEnemyCon>(GetController());
	player = FindPlayerIterater();

	// ������ �� ��ġ�� �����صд�. 
	startLocation = GetActorLocation();

	// normal �� nullptr �� �ƴϸ� �ִϸ��̼��� ����Ѵ�.
	if (normal != nullptr)
	{
		PlayAnimMontage(normal);
		//UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
	}

	
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	currentTime += DeltaTime;
	// ��赵 �Լ��� �����Ѵ�.
	ChangeWarning();

	//UE_LOG(LogTemp, Warning,TEXT("%d"),delayCheck);

	//UE_LOG(LogTemp, Warning, TEXT("%f"), UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint1->GetRelativeLocation()));
	//UE_LOG(LogTemp, Warning, TEXT("%f"), UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint2->GetRelativeLocation()));
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//���� 
void AEnemy::BasicMoveCycle(FVector point, UAnimMontage* anim, bool arrive1, bool arrive2)
{
	if (warningstack > 1000)
	{
		return;
	}
	// AI ��Ʈ�ѷ� �ȿ� �ִ� MoveToLocation �Լ��� ���
	aiCon->MoveToLocation(point);
	// �������� ���ʹ� ������ �Ÿ��� ���Ѵ�.
	float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), point);
	// �Ÿ����� 100 ������ ��
	if (check < 100 || currentTime > 30)
	{
		bArrive1 = arrive1;
		bArrive2 = arrive2;
		bGo = false;
		// �ִϸ��̼� ���
		if (anim != nullptr)
		{
			PlayAnimMontage(anim);
			//UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
		}
	}
}

void AEnemy::Damaged(float damage)
{
	// �������� 0~100�� ��ġ �ȿ��� ���� �������ŭ �����Ѵ�.
	Life = FMath::Clamp(Life - damage, 0, 100);

	// Life�� 0�� �Ǹ� ĳ���� �����Ʈ�� �ݸ����� �����Ѵ�.
	if (Life <= 0)
	{
		GetCharacterMovement()->DisableMovement();
		//GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0);
		headShot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		warningComp->SetVisibility(false);
		bDie = true;
	}

	UE_LOG(LogTemp, Warning, TEXT("%f"), Life);
}

void AEnemy::DrinkPoison(FVector poisonLoc)
{
	aiCon->MoveToLocation(poisonLoc);
	PlayAnimMontage(drink);
	Damaged(100.0f);
}

void AEnemy::ChangeWarning()
{
	if (player == nullptr || gunAttack == nullptr)
	{
		return;
	}
	
	// ��赵(warningstack)�� 0~3000 ���̿��� ���������� ������Ų��.
	warningstack = FMath::Clamp(warningstack, 0, 2000);
	

	if (warningstack < 1000)
	{
		//�⺻ ���� ����Ŭ�� �����Ѵ�.
		if (!bArrive1 && bGo)
		{
			BasicMoveCycle(checkPoint[0], normal, true, false);
		}
		else if (!bArrive2 && bGo)
		{
			BasicMoveCycle(checkPoint[1], normal, false, true);
		}
		else
		{
			delayCheck++;
		}
		// �� ���� ������ �ڿ� 100ƽ ���� ��� �ð��� ������.
		if (delayCheck > delay)
		{
			if (GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
			{
				delayCheck = 0;
			}
			else
			{
				bGo = true;
				currentTime = 0;
				delayCheck = 0;
			}
		}

		warningComp->SetText(FText::FromString(TEXT(".")));
	}
	else if(warningstack < 2000)
	{
		warningComp->SetText(FText::FromString(TEXT("?")));
		GetMesh()->GetAnimInstance()->Montage_Stop(NULL);
		FVector ranLoc;
		if (bLoc == false || currentTime > 30)
		{
			UNavigationSystemV1* navSys1 = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
			navSys1->K2_GetRandomLocationInNavigableRadius(GetWorld(), aiCon->targetLoc, ranLoc, 500.0f);
			aiCon->MoveToLocation(ranLoc);
			bLoc = true;
			currentTime = 0.0f;
			UE_LOG(LogTemp,Warning,TEXT("%f, %f, %f"),ranLoc.X,ranLoc.Y,ranLoc.Z);
		}
		if ((GetActorLocation() - ranLoc).Length() < 100.0f)
		{
			bLoc = false;
		}
		UE_LOG(LogTemp, Warning, TEXT("%f"), (GetActorLocation() - ranLoc).Length());
	}
	else
	{
		warningComp->SetText(FText::FromString(TEXT("!")));

		// bulletCount �� 100�� �Ѿ ������ ���� ����Ѵ�.(��赵 2000 �̻��� ��)
		bulletCount++;
		if (bulletCount > 100)
		{
			bulletCount = 0;
		}
		
		// �÷��̾�� AI�� �Ÿ��� ���Ѵ�.
		float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), player->GetActorLocation());
		// �Ÿ��� 500 �̻��̶��
		if (check > 500)
		{
			// 500�� �ӵ��� �Ÿ��� 400�� �� ������ �÷��̾ �Ѿư���.
			GetCharacterMovement()->MaxWalkSpeed = 500.0f;
			aiCon->MoveToActor(FindPlayerIterater(), 400.0);
		}
		// �ƴ϶��
		else
		{
			//gunAttack�� ����ϰ�, �÷��̾ �����Ѵ�. 
			PlayAnimMontage(gunAttack);
			ShootPlayer();
		}
	}
}

void AEnemy::ShootPlayer()
{
	if (bDie)
	{
		return;
	}
	//�÷��̾ ������� ����Ʈ���̽����Ѵ�.
	FHitResult hitInfo;
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	bool bResult = GetWorld()->LineTraceSingleByObjectType(hitInfo, GetActorLocation(), player->GetActorLocation(), objectQueryParams, queryParams);
	// Ʈ���̽��� �����ϸ�
	if (bResult)
	{
		// �Ҹ� ī��Ʈ�� 0�� �ɶ����� �÷��̾ �ٶ󺸸鼭 �÷��̾�� ������� �ش�.
		if (bulletCount == 0)
		{
			TSubclassOf<UDamageType> damagetype;
			UGameplayStatics::ApplyDamage(player, 10.0, player->GetController(), this, damagetype);
			FRotator newRot = UKismetMathLibrary::MakeRotFromZX(GetActorUpVector(), player->GetActorLocation() - GetActorLocation());
			SetActorRotation(newRot);
		}
	}
}

void AEnemy::UpdateStack(int32 warn)
{
	warningstack += warn;
}

ASecondProjectCharacter* AEnemy::FindPlayerIterater()
{
	//for(TActorInterator<ã������ Ŭ����> �����̸�(GetWorld()); �����̸�; ++�����̸�)
	TArray<ASecondProjectCharacter*> players;

	for (TActorIterator<ASecondProjectCharacter> sPlayer(GetWorld()); sPlayer; ++sPlayer)
	{
		players.Add(*sPlayer);
	}
	// players �迭�� �ϳ��� ���� �� �ִٸ� �� �� �� 0���� ��ȯ�ϰ�
	if (players.Num() > 0)
	{
		return players[0];
	}
	// �ƹ��͵� ������ nullptr �� ��ȯ�Ѵ�.
	else
	{
		return nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Fail player"));
	}
}
