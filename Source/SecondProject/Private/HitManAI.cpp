

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

	// 헤드샷용 컴포넌트를 생성
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetupAttachment(GetMesh());
	headShot->SetRelativeLocation(FVector(0, 0, 100));
	headShot->SetSphereRadius(20.0f);

	// 캡슐 컴포넌트 크기 조절
	GetCapsuleComponent()->SetCapsuleHalfHeight(80.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// 매쉬 높이 조절
	GetMesh()->SetRelativeLocation(FVector(0, 0, -80));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	// SkelatalMesh 컴포넌트 생성
	torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	torso->SetupAttachment(GetMesh());
	hand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand"));
	hand->SetupAttachment(GetMesh());
	pants = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pants"));
	pants->SetupAttachment(GetMesh());
	shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Shoes"));
	shoes->SetupAttachment(GetMesh());

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
	// 경계도가 일정 이상일 경우 수색단계로 진입
	if (warningStack > 1000)
	{
		aiCon->StopMovement();
		UE_LOG(LogTemp, Warning, TEXT("MoveAround->Search"));
		aiState = EAIState::SEARCH;
	}

	// 무기를 발견할 시 주시단계로 진입
	if (bSeeWeapon)
	{
		aiCon->StopMovement();
		aiState = EAIState::WATCH;
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
	}
	// 모두 실패시 로그 출력
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Move!"));
	}
}

void AHitManAI::MoveDelay(float deltatime)
{
	// 경계도가 일정 이상일 경우 수색단계로 진입
	if (warningStack > 1000)
	{
		aiCon->StopMovement(); 
		UE_LOG(LogTemp, Warning, TEXT("Delay->Search"));
		aiState = EAIState::SEARCH;
		GetRandomLocation();
		delayStack = 0;
	}
	// 무기를 발견할 시 주시단계로 진입
	if (bSeeWeapon)
	{
		aiCon->StopMovement();
		aiState = EAIState::WATCH;
		delayStack = 0;
	}
	// 몽타주가 플레이중이 아닐 때 normal animMontage 에 넣어놓은 몽타주중 랜덤으로 1개를 재생한다.
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