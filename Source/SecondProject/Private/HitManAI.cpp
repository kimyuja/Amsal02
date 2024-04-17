

#include "HitManAI.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/SphereComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/SkeletalMeshComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/CapsuleComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/TextRenderComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h>
#include "EnemyCon.h"
#include <../../../../../../../Source/Runtime/Engine/Public/EngineUtils.h>
#include "../SecondProjectCharacter.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include <../../../../../../../Source/Runtime/NavigationSystem/Public/NavigationSystem.h>

// Sets default values
AHitManAI::AHitManAI()
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
	GetMesh()->SetRelativeLocation(FVector(0, 0, -80));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

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
	warningComp->SetText(FText::FromString(TEXT(".")));
	warningComp->SetRelativeLocation(FVector(0, 0, 190));
	warningComp->SetRelativeRotation(FRotator(0, 90, 0));
	warningComp->SetHorizontalAlignment(EHTA_Center);

	//ĳ������ �ִ� �ӵ�, ���ӵ��� ����.(���� = cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;

	//ĳ���Ͱ� �׺���̼� ��η� �̵��� �� ���� ����(���� �־�� ABP�� �۵���)
	GetCharacterMovement()->UseAccelerationForPathFollowing();
}

void AHitManAI::BeginPlay()
{
	Super::BeginPlay();
	
	// �÷��̾�� AI ��Ʈ�ѷ��� �̸� ĳ���� �ؿ´�.
	aiCon = Cast<AEnemyCon>(GetController());
	hitmanPlayer = FindPlayerIterater();

	// ü���� �ִ�ġ�� �����Ѵ�.
	currentHP = maxHP;

	// ���� ��ġ, ������ �����س��´�.
	currentLoc = GetActorLocation();
	currentRot = GetActorRotation();

	// AI Ÿ���� ���� Ÿ���� �ƴ� ��� �Ӹ� ���� �ؽ�Ʈ �ڽ��� �񰡽�ó���Ѵ�.
	if (aiType != 1)
	{
		warningComp->SetVisibility(false);
	}

	// �̵� ��ǥ�� ù��° ��ǥ�� �ʱ�ȭ�Ѵ�.
	if (setMoveLoc.Num() > 0)
	{
		moveLoc = setMoveLoc[currentSetLoc];
	}
	else
	{
		moveLoc = GetActorLocation();
		setMoveLoc[0] = GetActorLocation();
	}
	
}

void AHitManAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDie)
	{
		return;
	}

	delayStack += DeltaTime;

	switch (aiState)
	{
	case EAIState::MOVE:
		MoveArround();
		break;
	case EAIState::MOVEDELAY:
		MoveDelay(DeltaTime);
		break;
	case EAIState::WATCH:
		Watch(DeltaTime);
		break;
	case EAIState::SEARCH:
		SearchArround();
		break;
	case EAIState::ATTACK:
		AttackTarget(attackDamage);
		break;
	case EAIState::ATTACKDELAY:
		AttackDelay();
		break;
	case EAIState::CHASE:
		Chase();
		break;
	case EAIState::RUNAWAY:
		RunAway(DeltaTime);
		break;
	case EAIState::PANIC:
		Panic(DeltaTime);
		break;
	case EAIState::DAMAGED:
		DamagedProcess(DeltaTime);
		break;
	default:
		break;
	}
}

void AHitManAI::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{

}

void AHitManAI::UpdateWarningStack(int32 warnStack)
{
	warningStack += warnStack;
	warningStack = FMath::Clamp(warningStack, 0, 2000);
	//UE_LOG(LogTemp, Warning, TEXT("%d"), warningStack);
}

void AHitManAI::Damaged(int32 damage)
{
	currentHP = FMath::Clamp(currentHP-damage, 0, maxHP);
	if (currentHP <= 0)
	{
		Die();
	}
}

void AHitManAI::DrinkPoison(bool bIsDrink, FVector poisonLocation)
{
	float distance = UKismetMathLibrary::Vector_Distance(GetActorLocation(), poisonLocation);
	UE_LOG(LogTemp, Warning, TEXT("%f"), distance);
	if (distance < 100.0f)
	{
		FTimerHandle deathTimer;
		GetWorld()->GetTimerManager().SetTimer(deathTimer, FTimerDelegate::CreateLambda([&]() {
			Damaged(100.0f);
			}), 4.0f, false);
		SetActorLocation(poisonLocation);
		PlayAnimMontage(drinkPoison);
		UE_LOG(LogTemp, Warning, TEXT("Drink"));
		GetCharacterMovement()->DisableMovement();
	}
	else
	{
		aiCon->MoveToLocation(poisonLocation);
	}
}

void AHitManAI::Die()
{
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0);
	headShot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	warningComp->SetVisibility(false);
	bIsDie = true;
}

void AHitManAI::EquipWeapon()
{
	
}

void AHitManAI::GetRandomLocation()
{
	if (FVector::Distance(GetActorLocation(), ranLoc) > 50.0f)
	{
		UNavigationSystemV1* navSys1 = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
		navSys1->K2_GetRandomLocationInNavigableRadius(GetWorld(), aiCon->targetLoc, ranLoc, 50.0f);
	}
	else
	{
		return;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AHitManAI::MoveArround()
{
	// ��赵�� ���� �̻��� ��� �����ܰ�� ����
	if (warningStack > 1000)
	{
		aiCon->StopMovement();
		UE_LOG(LogTemp, Warning, TEXT("MoveAround->Search"));
		aiState = EAIState::SEARCH;
	}

	// ���⸦ �߰��� �� �ֽôܰ�� ����
	if (bSeeWeapon)
	{
		aiCon->StopMovement();
		aiState = EAIState::WATCH;
	}

	// �̵��� ��ҷ� �̵��� �̵���� ���·� ����
	aiCon->MoveToLocation(moveLoc);
	float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), moveLoc);
	// �̵��� ��ҿ��� �Ÿ��� 70 �̳��� ���
	if (check < 70.0f)
	{
		// �̵��� ��ҿ� �������� ��� �̵�
		SetActorLocation(moveLoc);
		SetActorRotation(setRot);
		// ���� �̵� ���, ���� ���� �� �̵���� ���·� ����
		if (currentSetLoc < setMoveLoc.Num()-1 && currentSetRot < setMoveRot.Num()-1)
		{
			currentSetLoc++;
			currentSetRot++;
		}
		else
		{
			currentSetLoc = 0;
			currentSetRot = 0;
		}
		moveLoc = setMoveLoc[currentSetLoc];
		setRot = setMoveRot[currentSetRot];
		delayStack = 0;
		aiState = EAIState::MOVEDELAY;
	}
	// �̵� ���� �� 30�� �̻��� ������� ���
	else if (delayStack > 30.0f)
	{	
		// ���� �̵� ���, ���� ���� �� �̵���� ���·� ����
		if (currentSetLoc < setMoveLoc.Num() - 1 && currentSetRot < setMoveRot.Num() - 1)
		{
			currentSetLoc++;
			currentSetRot++;
		}
		else
		{
			currentSetLoc = 0;
			currentSetRot = 0;
		}
		moveLoc = setMoveLoc[currentSetLoc];
		setRot = setMoveRot[currentSetRot];
		delayStack = 0;
		aiState = EAIState::MOVEDELAY;
	}
	// ��� ���н� �α� ���
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Move!"));
	}
}

void AHitManAI::MoveDelay(float deltatime)
{
	// ��赵�� ���� �̻��� ��� �����ܰ�� ����
	if (warningStack > 1000)
	{
		aiCon->StopMovement(); 
		UE_LOG(LogTemp, Warning, TEXT("Delay->Search"));
		aiState = EAIState::SEARCH;
		GetRandomLocation();
		delayStack = 0;
	}
	// ���⸦ �߰��� �� �ֽôܰ�� ����
	if (bSeeWeapon)
	{
		aiCon->StopMovement();
		aiState = EAIState::WATCH;
		delayStack = 0;
	}
	// ��Ÿ�ְ� �÷������� �ƴ� �� normal animMontage �� �־���� ��Ÿ���� �������� 1���� ����Ѵ�.
	int32 num = FMath::RandRange(0, normal.Num() - 1);
	idleTime = normal[num];
	bool bIsPlayingMontage = GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();
	if (idleTime != nullptr && !bIsPlayingMontage)
	{
		PlayAnimMontage(idleTime);
	}
	if (delayStack > 10.0f && !bIsPlayingMontage)
	{
		StopAnimMontage(NULL);
		aiState = EAIState::MOVE;
		delayStack = 0;
	}
}

void AHitManAI::Watch(float deltatime)
{
	StopAnimMontage(NULL);
	FRotator watchRot = UKismetMathLibrary::MakeRotFromZX(GetActorUpVector(), hitmanPlayer->GetActorLocation() - GetActorLocation());
	SetActorRotation(watchRot);
	if (warningStack > 1000)
	{
		warningStack += 1;
	}
	if (warningStack > 2000)
	{
		aiState = EAIState::CHASE;
	}
	if (!bSeeWeapon)
	{
		aiCon->StopMovement();
		delayStack = 0;
		aiState = EAIState::MOVE;
	}
}

void AHitManAI::SearchArround()
{
	aiCon->MoveToLocation(ranLoc);
	if (FVector::Distance(GetActorLocation(), ranLoc) < 50.0f)
	{
		aiState = EAIState::WATCH;
		return;
	}
}

void AHitManAI::AttackTarget(int32 hitDamage)
{
	if (FVector::Distance(hitmanPlayer->GetActorLocation(), GetActorLocation()) > 250.0f)
	{
		aiState = EAIState::CHASE;
	}
}

void AHitManAI::AttackDelay()
{
	if (FVector::Distance(hitmanPlayer->GetActorLocation(), GetActorLocation()) > 250.0f)
	{
		aiState = EAIState::CHASE;
	}
}

void AHitManAI::Chase()
{
	if (FVector::Distance(hitmanPlayer->GetActorLocation(), GetActorLocation()) > 250.0f)
	{
		aiCon->MoveToActor(hitmanPlayer, 200.0f);
	}
	else
	{
		aiState = EAIState::ATTACK;
	}
}

void AHitManAI::RunAway(float deltatime)
{
	
}

void AHitManAI::Panic(float deltatime)
{

}

void AHitManAI::DamagedProcess(float deltatime)
{

}

ASecondProjectCharacter* AHitManAI::FindPlayerIterater()
{
	//for(TActorInterator<ã������ Ŭ����> �����̸�(GetWorld()); �����̸�; ++�����̸�)
	TArray<ASecondProjectCharacter*> players;

	for (TActorIterator<ASecondProjectCharacter> player(GetWorld()); player; ++player)
	{
		players.Add(*player);
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