

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
#include <../../../../../../../Source/Runtime/Engine/Classes/Kismet/GameplayStatics.h>

// Sets default values
AHitManAI::AHitManAI()
{
	PrimaryActorTick.bCanEverTick = true;

	// ��弦�� ������Ʈ�� ����
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetupAttachment(GetMesh());
	headShot->SetRelativeLocation(FVector(0, 0, 170));
	headShot->SetSphereRadius(20.0f);

	// ĸ�� ������Ʈ ũ�� ����
	GetCapsuleComponent()->SetCapsuleHalfHeight(80.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// �Ž� ���� ����
	GetMesh()->SetRelativeLocation(FVector(0, 0, -80));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	//��赵 ǥ��
	warningComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Warning"));
	warningComp->SetupAttachment(GetMesh());
	warningComp->SetText(FText::FromString(TEXT(".")));
	warningComp->SetRelativeLocation(FVector(0, 0, 190));
	warningComp->SetRelativeRotation(FRotator(0, 90, 0));
	warningComp->SetHorizontalAlignment(EHTA_Center);

	// �����տ� ������ ��⸦ ���� �Ž� �����
	itemMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Equip Item"));
	itemMeshComp->SetupAttachment(GetMesh(), FName("EquipItem"));

	// ����� �� �Ž� �����
	cupMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cup"));
	cupMeshComp->SetupAttachment(GetMesh(), FName("Cup"));

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
	UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
}

void AHitManAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDie)
	{
		return;
	}
	GetVelocity();

	bIsPlayingMontage = GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();

	delayStack += DeltaTime;

	switch (aiState)
	{
	case EAIState::IDLE:
		Idle();
		break;
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

	if (aiType == 1)
	{
		if (warningStack >= 2000)
		{
			bEquipWeapon = true;
		}
		else
		{
			bEquipWeapon = false;
		}
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
		PlayAnimMontage(dying);
		Die();
	}
	else
	{
		bIsDamaged = true;
	}
	hitLoc = GetActorLocation();
	hitDir = GetActorLocation() - hitmanPlayer->GetActorLocation();
	hitDir.Z = 0;
	hitDir = hitDir.GetSafeNormal();
}

void AHitManAI::DrinkPoison(bool bIsDrink, FVector poisonLocation, FRotator poisonRotation)
{
	bIsDrinkPoison = bIsDrink;
	UE_LOG(LogTemp, Warning, TEXT("%f"), poisonDir);
	UE_LOG(LogTemp, Warning, TEXT("Go to Drink"));
	aiCon->MoveToLocation(poisonLocation);
	poisonLoc = poisonLocation;
	poisonRot = poisonRotation;
}

void AHitManAI::CoinReaction(UStaticMeshComponent* coin)
{
	bFindCoin = true;
	StopAnimMontage(NULL);
	aiState = EAIState::IDLE;
	delayStack = 0;
}

void AHitManAI::Die()
{
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	//GetMesh()->SetAllBodiesSimulatePhysics(true);
	//GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0);
	headShot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	warningComp->SetVisibility(false);
	bIsDie = true;
}

void AHitManAI::EquipWeapon()
{
	
}

void AHitManAI::GetRandomLocation(FVector standardLoc, float radius)
{
	navSys1 = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	navSys1->K2_GetRandomLocationInNavigableRadius(GetWorld(), standardLoc, ranLoc, radius);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AHitManAI::Idle()
{
	UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
	if (!bFindCoin || delayStack > 10.0f)
	{
		aiState = EAIState::MOVE;
		delayStack = 0;
	}
	else if (bFindCoin)
	{
		aiCon->MoveToLocation(coinLoc);
		UE_LOG(LogTemp, Warning, TEXT("Coin"));
		if (FVector::Distance(GetActorLocation(), coinLoc) < 120.0f)
		{
			if (delayStack > 5.0f)
			{
				bFindCoin = false;
				delayStack = 0;
			}
			
		}
	}
}

void AHitManAI::MoveArround()
{
	if (bIsDrinkPoison)
	{
		poisonDir = FVector::Distance(GetActorLocation(), poisonLoc);
		if (poisonDir < 100.0f)
		{
			FTimerHandle deathTimer;
			if (!GetWorld()->GetTimerManager().IsTimerActive(deathTimer))
			{
				GetWorld()->GetTimerManager().SetTimer(deathTimer, FTimerDelegate::CreateLambda([&]() {
					Damaged(100.0f);
					}), 4.0f, false);
			}
			SetActorLocation(poisonLoc);
			SetActorRotation(poisonRot);
			if (!bIsPlayingMontage)
			{
				PlayAnimMontage(drinkPoison);
			}
			UE_LOG(LogTemp, Warning, TEXT("Drink"));
			GetCharacterMovement()->DisableMovement();
			if (!bIsPlayingMontage)
			{
				PlayAnimMontage(dying);
			}
		}
		return;
	}
	// ��赵�� ���� �̻��� ��� �����ܰ�� ����
	if (warningStack > 1000)
	{
		aiCon->StopMovement();
		UE_LOG(LogTemp, Warning, TEXT("MoveAround->Search"));
		aiState = EAIState::SEARCH;
		delayStack = 0;
		GetRandomLocation(aiCon->targetLoc, 50.0f);
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}

	// ���⸦ �߰��� �� �ֽôܰ�� ����
	if (bSeeWeapon)
	{
		aiCon->StopMovement();
		aiState = EAIState::WATCH;
		delayStack = 0;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}

	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
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
		delayStack = 0;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
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
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
	}
	// ��� ���н� �α� ���
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Move!"));
	}
}

void AHitManAI::MoveDelay(float deltatime)
{
	if (bIsDrinkPoison)
	{
		poisonDir = FVector::Distance(GetActorLocation(), poisonLoc);
		if (poisonDir < 100.0f)
		{
			FTimerHandle deathTimer;
			if (!GetWorld()->GetTimerManager().IsTimerActive(deathTimer))
			{
				GetWorld()->GetTimerManager().SetTimer(deathTimer, FTimerDelegate::CreateLambda([&]() {
					Damaged(100.0f);
					}), 4.0f, false);
			}
			SetActorLocation(poisonLoc);
			SetActorRotation(poisonRot);
			if (!bIsPlayingMontage)
			{
				PlayAnimMontage(drinkPoison);
			}
			UE_LOG(LogTemp, Warning, TEXT("Drink"));
			GetCharacterMovement()->DisableMovement();
			if (!bIsPlayingMontage)
			{
				PlayAnimMontage(dying);
			}
		}
		return;
	}
	// ��赵�� ���� �̻��� ��� �����ܰ�� ����
	if (warningStack > 1000)
	{
		aiCon->StopMovement(); 
		UE_LOG(LogTemp, Warning, TEXT("Delay->Search"));
		aiState = EAIState::SEARCH;
		GetRandomLocation(aiCon->targetLoc, 50.0f);
		delayStack = 0;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	// ���⸦ �߰��� �� �ֽôܰ�� ����
	if (bSeeWeapon)
	{
		aiCon->StopMovement();
		aiState = EAIState::WATCH;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		delayStack = 0;
		return;
	}
	// �������� �޾��� �� ������ ó�� �ܰ�� ����
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	// ��Ÿ�ְ� �÷������� �ƴ� �� normal animMontage �� �־���� ��Ÿ���� �������� 1���� ����Ѵ�.
	int32 num = FMath::RandRange(0, normal.Num() - 1);
	idleTime = normal[num];
	if (idleTime != nullptr && !bIsPlayingMontage)
	{
		PlayAnimMontage(idleTime);
	}
	if (delayStack > 10.0f && !bIsPlayingMontage)
	{
		StopAnimMontage(NULL);
		aiState = EAIState::MOVE;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		delayStack = 0;
	}
}

void AHitManAI::Watch(float deltatime)
{
	// �������� �޾��� �� ������ ó�� �ܰ�� ����
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	StopAnimMontage(NULL);
	FRotator watchRot = UKismetMathLibrary::MakeRotFromZX(GetActorUpVector(), hitmanPlayer->GetActorLocation() - GetActorLocation());
	SetActorRotation(watchRot);
	if (warningStack > 1000)
	{
		warningStack += 1;
	}
	if (warningStack > 2000)
	{
		if (aiType == 1)
		{
			aiState = EAIState::CHASE;
			UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
			return;
		}
		else
		{
			aiState = EAIState::PANIC;
			UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
			return;
		}
	}
	if (!bSeeWeapon)
	{
		aiCon->StopMovement();
		delayStack = 0;
		aiState = EAIState::MOVE;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
}

void AHitManAI::SearchArround()
{
	// �������� �޾��� �� ������ ó�� �ܰ�� ����
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	aiCon->MoveToLocation(ranLoc);
	if (FVector::Distance(GetActorLocation(), ranLoc) < 50.0f)
	{
		aiState = EAIState::WATCH;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
}

void AHitManAI::AttackTarget(int32 hitDamage)
{
	// �������� �޾��� �� ������ ó�� �ܰ�� ����
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	
	if (FVector::Distance(hitmanPlayer->GetActorLocation(), GetActorLocation()) > 500.0f)
	{
		if (aiType == 1)
		{
			aiState = EAIState::CHASE;
			UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
			return;
		}
		else
		{
			aiState = EAIState::PANIC;
			UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
			return;
		}
	}
	if (!bIsPlayingMontage)
	{
		PlayAnimMontage(attack);
	}
	//�÷��̾ ������� ����Ʈ���̽����Ѵ�.
	FHitResult hitInfo;
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	bool bResult = GetWorld()->LineTraceSingleByObjectType(hitInfo, GetActorLocation(), hitmanPlayer->GetActorLocation(), objectQueryParams, queryParams);
	// Ʈ���̽��� �����ϸ�
	if (bResult)
	{
		ASecondProjectCharacter* hitplayer = Cast<ASecondProjectCharacter>(hitInfo.GetActor());
		if (hitplayer != nullptr)
		{
			TSubclassOf<UDamageType> damagetype;
			UGameplayStatics::ApplyDamage(hitplayer, 10.0, hitplayer->GetController(), this, damagetype);
			FRotator newRot = UKismetMathLibrary::MakeRotFromZX(GetActorUpVector(), hitplayer->GetActorLocation() - GetActorLocation());
			SetActorRotation(newRot);
		}
	}
	aiState = EAIState::ATTACKDELAY;
	UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
}

void AHitManAI::AttackDelay()
{
	// �������� �޾��� �� ������ ó�� �ܰ�� ����
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}

	if (FVector::Distance(hitmanPlayer->GetActorLocation(), GetActorLocation()) > 500.0f)
	{
		if (aiType == 1)
		{
			aiState = EAIState::CHASE;
			UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
			return;
		}
		else
		{
			aiState = EAIState::PANIC;
			UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
			return;
		}
	}
	if (attackdelay != nullptr)
	{
		if (!bIsPlayingMontage)
		{
			PlayAnimMontage(attackdelay);
		}
		/*FTimerHandle attackDelayTimer;
		if (!GetWorld()->GetTimerManager().IsTimerActive(attackDelayTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(attackDelayTimer, FTimerDelegate::CreateLambda([&]() {
				aiState = EAIState::ATTACK;
				}), 3.0f, false);
		}*/
	}
	if (delayStack > 2.5f)
	{
		StopAnimMontage(NULL);
		aiState = EAIState::ATTACK;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		delayStack = 0;
		return;
	}
}

void AHitManAI::Chase()
{
	if (warningStack < 2000)
	{
		aiCon->StopMovement();
		aiState = EAIState::MOVEDELAY;
		delayStack = 0;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	// �������� �޾��� �� ������ ó�� �ܰ�� ����
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}

	if (FVector::Distance(hitmanPlayer->GetActorLocation(), GetActorLocation()) < 400.0f)
	{
		GetCharacterMovement()->MaxWalkSpeed = 100.0f;
		aiState = EAIState::ATTACKDELAY;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.0f;
		aiCon->MoveToActor(hitmanPlayer, 200.0f);
	}
}

void AHitManAI::RunAway(float deltatime)
{
	// �������� �޾��� �� ������ ó�� �ܰ�� ����
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	// 5�� ���� �����ٴ� �� Move ���·� �̵��Ѵ�.
	if (delayStack > 300.0f)
	{
		aiState = EAIState::MOVE;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	// �� �������� ������ ��ġ�� ������ �����ٴѴ�.
	else
	{
		aiCon->MoveToLocation(ranLoc);
		if (FVector::Distance(ranLoc, GetActorLocation()) < 100.0f)
		{
			aiState = EAIState::PANIC;
			UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
			return;
		}
	}
}

void AHitManAI::Panic(float deltatime)
{
	// �������� �޾��� �� ������ ó�� �ܰ�� ����
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	if (panicSound != nullptr)
	{
		UGameplayStatics::PlaySound2D(this, panicSound);
	}
	delayStack = 0;
	GetRandomLocation(GetActorLocation(), 500.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	aiState = EAIState::RUNAWAY;
	UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
}

void AHitManAI::DamagedProcess(float deltatime)
{
	FVector backVec = GetActorForwardVector() * -1.0f;
	FVector targetLoc = hitLoc + hitDir * 50.0f;
	FVector knockBackLoc = FMath::Lerp(GetActorLocation(), targetLoc, deltatime * 7.0f);
	if (FVector::Distance(GetActorLocation(), targetLoc) > 10.0f)
	{
		FTimerHandle damageTimer;
		GetWorldTimerManager().SetTimer(damageTimer, FTimerDelegate::CreateLambda([&]() {
			if (aiType == 1)
			{
				aiState = EAIState::CHASE;
				UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
				return;
			}
			else
			{
				aiState = EAIState::PANIC;
				UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
				return;
			}
			}), 1.0f, false);
		SetActorLocation(knockBackLoc, true);
	}
	bIsDamaged = false;
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