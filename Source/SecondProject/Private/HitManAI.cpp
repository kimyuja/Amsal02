

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

	// 헤드샷용 컴포넌트를 생성
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetupAttachment(GetMesh());
	headShot->SetRelativeLocation(FVector(0, 0, 10));
	headShot->SetSphereRadius(20.0f);

	// 캡슐 컴포넌트 크기 조절
	GetCapsuleComponent()->SetCapsuleHalfHeight(80.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// 매쉬 높이 조절
	GetMesh()->SetRelativeLocation(FVector(0, 0, -80));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	//경계도 표시
	warningComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Warning"));
	warningComp->SetupAttachment(GetMesh());
	warningComp->SetText(FText::FromString(TEXT(".")));
	warningComp->SetRelativeLocation(FVector(0, 0, 190));
	warningComp->SetRelativeRotation(FRotator(0, 90, 0));
	warningComp->SetHorizontalAlignment(EHTA_Center);

	//캐릭터의 최대 속도, 가속도를 설정.(단위 = cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;

	//캐릭터가 네비게이션 경로로 이동할 때 가속 적용(켜저 있어야 ABP가 작동함)
	GetCharacterMovement()->UseAccelerationForPathFollowing();
}

void AHitManAI::BeginPlay()
{
	Super::BeginPlay();
	
	// 플레이어와 AI 컨트롤러를 미리 캐스팅 해온다.
	aiCon = Cast<AEnemyCon>(GetController());
	hitmanPlayer = FindPlayerIterater();

	// 체력을 최대치로 설정한다.
	currentHP = maxHP;

	// 시작 위치, 방향을 저장해놓는다.
	currentLoc = GetActorLocation();
	currentRot = GetActorRotation();

	// AI 타입이 경비원 타입이 아닌 경우 머리 위의 텍스트 박스를 비가시처리한다.
	if (aiType != 1)
	{
		warningComp->SetVisibility(false);
	}

	// 이동 좌표를 첫번째 좌표로 초기화한다.
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
	///*if (ranLoc == FVector::ZeroVector)
	//{
	//	navSys1 = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	//	navSys1->K2_GetRandomLocationInNavigableRadius(GetWorld(), standardLoc, ranLoc, radius);
	//}
	//if (FVector::Distance(GetActorLocation(), ranLoc) < 50.0f)
	//{
	//	navSys1 = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	//	navSys1->K2_GetRandomLocationInNavigableRadius(GetWorld(), standardLoc, ranLoc, radius);
	//}
	//else
	//{
	//	return;
	//}*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////

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
	// 경계도가 일정 이상일 경우 수색단계로 진입
	if (warningStack > 1000)
	{
		aiCon->StopMovement();
		UE_LOG(LogTemp, Warning, TEXT("MoveAround->Search"));
		aiState = EAIState::SEARCH;
		GetRandomLocation(aiCon->targetLoc, 50.0f);
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}

	// 무기를 발견할 시 주시단계로 진입
	if (bSeeWeapon)
	{
		aiCon->StopMovement();
		aiState = EAIState::WATCH;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}

	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}

	// 이동할 장소로 이동후 이동대기 상태로 진입
	aiCon->MoveToLocation(moveLoc);
	float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), moveLoc);
	// 이동할 장소와의 거리가 70 이내일 경우
	if (check < 70.0f)
	{
		// 이동할 장소와 방향으로 즉시 이동
		SetActorLocation(moveLoc);
		SetActorRotation(setRot);
		// 다음 이동 장소, 방향 설정 후 이동대기 상태로 진입
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
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
	}
	// 이동 시작 후 30초 이상이 경과했을 경우
	else if (delayStack > 30.0f)
	{	
		// 다음 이동 장소, 방향 설정 후 이동대기 상태로 진입
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
	// 모두 실패시 로그 출력
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
	// 경계도가 일정 이상일 경우 수색단계로 진입
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
	// 무기를 발견할 시 주시단계로 진입
	if (bSeeWeapon)
	{
		aiCon->StopMovement();
		aiState = EAIState::WATCH;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		delayStack = 0;
		return;
	}
	// 데미지를 받았을 때 데미지 처리 단계로 진입
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	// 몽타주가 플레이중이 아닐 때 normal animMontage 에 넣어놓은 몽타주중 랜덤으로 1개를 재생한다.
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
	// 데미지를 받았을 때 데미지 처리 단계로 진입
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
	// 데미지를 받았을 때 데미지 처리 단계로 진입
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
	// 데미지를 받았을 때 데미지 처리 단계로 진입
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
	//플레이어를 대상으로 라인트레이스를한다.
	FHitResult hitInfo;
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	bool bResult = GetWorld()->LineTraceSingleByObjectType(hitInfo, GetActorLocation(), hitmanPlayer->GetActorLocation(), objectQueryParams, queryParams);
	// 트레이스에 성공하면
	if (bResult)
	{
		ASecondProjectCharacter* hitplayer = Cast<ASecondProjectCharacter>(hitInfo.GetActor());
		TSubclassOf<UDamageType> damagetype;
		UGameplayStatics::ApplyDamage(hitplayer, 10.0, hitplayer->GetController(), this, damagetype);
		FRotator newRot = UKismetMathLibrary::MakeRotFromZX(GetActorUpVector(), hitplayer->GetActorLocation() - GetActorLocation());
		SetActorRotation(newRot);
	}
	aiState = EAIState::ATTACKDELAY;
	UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
}

void AHitManAI::AttackDelay()
{
	// 데미지를 받았을 때 데미지 처리 단계로 진입
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
			PlayAnimMontage(attackdelay, 0.25f);
		}
		/*FTimerHandle attackDelayTimer;
		if (!GetWorld()->GetTimerManager().IsTimerActive(attackDelayTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(attackDelayTimer, FTimerDelegate::CreateLambda([&]() {
				aiState = EAIState::ATTACK;
				}), 3.0f, false);
		}*/
	}
	if (delayStack > 4.0f)
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
	// 데미지를 받았을 때 데미지 처리 단계로 진입
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
	// 데미지를 받았을 때 데미지 처리 단계로 진입
	if (bIsDamaged)
	{
		aiState = EAIState::DAMAGED;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	// 5분 동안 도망다닌 후 Move 상태로 이동한다.
	if (delayStack > 300.0f)
	{
		aiState = EAIState::MOVE;
		UE_LOG(LogTemp, Warning, TEXT("State Transition: %s"), *StaticEnum<EAIState>()->GetValueAsString(aiState));
		return;
	}
	// 그 전까지는 랜덤한 위치를 지정해 도망다닌다.
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
	// 데미지를 받았을 때 데미지 처리 단계로 진입
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
	//for(TActorInterator<찾으려는 클래스> 변수이름(GetWorld()); 변수이름; ++변수이름)
	TArray<ASecondProjectCharacter*> players;

	for (TActorIterator<ASecondProjectCharacter> player(GetWorld()); player; ++player)
	{
		players.Add(*player);
	}
	// players 배열에 하나라도 값이 들어가 있다면 그 값 중 0번을 반환하고
	if (players.Num() > 0)
	{
		return players[0];
	}
	// 아무것도 없으면 nullptr 를 반환한다.
	else
	{
		return nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Fail player"));
	}
}