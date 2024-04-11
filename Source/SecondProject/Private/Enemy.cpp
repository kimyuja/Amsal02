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

	// 헤드샷용 컴포넌트를 생성
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetupAttachment(GetMesh());
	headShot->SetRelativeLocation(FVector(0, 0, 100));
	headShot->SetSphereRadius(20.0f);

	// 캡슐 컴포넌트 크기 조절
	GetCapsuleComponent()->SetCapsuleHalfHeight(80.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// 매쉬 높이 조절
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));

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
	warningComp->SetText(warningText);
	warningComp->SetRelativeLocation(FVector(0, 0, 130));

	//캐릭터의 최대 속도, 가속도를 설정.(단위 = cm/s)
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;

	//캐릭터가 네비게이션 경로로 이동할 때 가속 적용(켜저 있어야 ABP가 작동함)
	GetCharacterMovement()->UseAccelerationForPathFollowing();

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	aiCon = Cast<AEnemyCon>(GetController());
	player = FindPlayerIterater();

	// 시작할 때 위치를 저장해둔다. 
	startLocation = GetActorLocation();

	// normal 이 nullptr 가 아니면 애니매이션을 재생한다.
	if (normal != nullptr)
	{
		PlayAnimMontage(normal);
		//UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
	}

	if (enemyType == 0)
	{
		warningComp->SetVisibility(false);
	}
	
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	currentTime += DeltaTime;
	// 경계도 함수를 실행한다.
	ChangeWarning();

	if (FVector::Distance(player->GetActorLocation(), GetActorLocation()) < 200.0f)
	{
		FVector backVec = GetActorForwardVector() * -1.0f;
		FVector targetLoc = GetActorLocation() + backVec * 50.0f;
		FVector knockBackLoc = FMath::Lerp(GetActorLocation(), targetLoc, DeltaTime * 7.0f);
	}
	//UE_LOG(LogTemp, Warning,TEXT("%d"),delayCheck);
	
	//UE_LOG(LogTemp, Warning, TEXT("%f"), UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint1->GetRelativeLocation()));
	//UE_LOG(LogTemp, Warning, TEXT("%f"), UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint2->GetRelativeLocation()));
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//순찰 
void AEnemy::BasicMoveCycle(FVector point, UAnimMontage* anim, bool arrive1, bool arrive2)
{
	if (warningstack > 1000)
	{
		return;
	}
	// AI 컨트롤러 안에 있는 MoveToLocation 함수를 사용
	aiCon->MoveToLocation(point);
	// 목적지와 에너미 사이의 거리를 구한다.
	float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), point);
	// 거리값이 100 이하일 때
	if (check < 100 || currentTime > 30)
	{
		SetActorLocation(point);
		bArrive1 = arrive1;
		bArrive2 = arrive2;
		bGo = false;
		currentTime = 0;
		// 애니매이션 재생
		if (anim != nullptr)
		{
			PlayAnimMontage(anim);
			//UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
		}
	}
}

void AEnemy::Damaged(float damage)
{
	// 라이프를 0~100의 수치 안에서 들어온 대미지만큼 차감한다.
	Life = FMath::Clamp(Life - damage, 0, 100);

	// Life가 0이 되면 캐릭터 무브먼트와 콜리전을 정리한다.
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
	
	// 경계도(warningstack)을 0~3000 사이에서 점진적으로 증가시킨다.
	warningstack = FMath::Clamp(warningstack, 0, 2000);
	

	if (warningstack < 1000)
	{
		//기본 순찰 사이클을 실행한다.
		if (!bArrive1 && bGo)
		{
			BasicMoveCycle(checkPoint[0], normal, true, false);
		}
		else if (!bArrive2 && bGo)
		{
			BasicMoveCycle(checkPoint[1], normal, false, true);
		}
		// 한 곳에 도착한 뒤에 100틱 동안 대기 시간을 가진다.
		if (currentTime > delay)
		{
			if (GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
			{
				currentTime = 0;
			}
			else
			{
				bGo = true;
				currentTime = 0;
			}
		}

		warningComp->SetText(FText::FromString(TEXT(".")));
	}
	else if(warningstack < 2000)
	{
		warningComp->SetText(FText::FromString(TEXT("?")));
		GetMesh()->GetAnimInstance()->Montage_Stop(NULL);
		MoveRandom(30.0f, 500.0f, aiCon->targetLoc);
	}
	else
	{
		warningComp->SetText(FText::FromString(TEXT("!")));

		if (enemyType == 0)
		{
			GetCharacterMovement()->MaxWalkSpeed = 1000.0f;
			MoveRandom(1.0f, 1000.0f, GetActorLocation());
		}
		else
		{
			// bulletCount 가 100이 넘어갈 때마다 총을 쏘게한다.(경계도 2000 이상일 때)
			bulletCount++;
			if (bulletCount > 100)
			{
				bulletCount = 0;
			}

			// 플레이어와 AI의 거리를 구한다.
			float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), player->GetActorLocation());
			// 거리가 500 이상이라면
			if (check > 500)
			{
				// 500의 속도로 거리가 400이 될 때까지 플레이어를 쫓아간다.
				GetCharacterMovement()->MaxWalkSpeed = 500.0f;
				aiCon->MoveToActor(FindPlayerIterater(), 400.0);
			}
			// 아니라면
			else
			{
				//gunAttack를 재생하고, 플레이어를 공격한다. 
				PlayAnimMontage(gunAttack);
				ShootPlayer();
			}
		}
	}
}

void AEnemy::ShootPlayer()
{
	if (bDie)
	{
		return;
	}
	//플레이어를 대상으로 라인트레이스를한다.
	FHitResult hitInfo;
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	bool bResult = GetWorld()->LineTraceSingleByObjectType(hitInfo, GetActorLocation(), player->GetActorLocation(), objectQueryParams, queryParams);
	// 트레이스에 성공하면
	if (bResult)
	{
		// 불릿 카운트가 0이 될때마다 플레이어를 바라보면서 플레이어에게 대미지를 준다.
		if (bulletCount == 0)
		{
			TSubclassOf<UDamageType> damagetype;
			UGameplayStatics::ApplyDamage(player, 10.0, player->GetController(), this, damagetype);
			FRotator newRot = UKismetMathLibrary::MakeRotFromZX(GetActorUpVector(), player->GetActorLocation() - GetActorLocation());
			SetActorRotation(newRot);
		}
	}
}

void AEnemy::MoveRandom(float delayT, float moveRadius, FVector moveLoc)
{
	FVector ranLoc;
	if (bLoc == false || currentTime > delayT)
	{
		UNavigationSystemV1* navSys1 = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
		navSys1->K2_GetRandomLocationInNavigableRadius(GetWorld(), moveLoc, ranLoc, moveRadius);
		aiCon->MoveToLocation(ranLoc);
		bLoc = true;
		currentTime = 0.0f;
		//UE_LOG(LogTemp,Warning,TEXT("%f, %f, %f"),ranLoc.X,ranLoc.Y,ranLoc.Z);
	}
	if ((GetActorLocation() - ranLoc).Length() < 100.0f)
	{
		bLoc = false;
	}
}

void AEnemy::UpdateStack(int32 warn)
{
	warningstack += warn;
}

ASecondProjectCharacter* AEnemy::FindPlayerIterater()
{
	//for(TActorInterator<찾으려는 클래스> 변수이름(GetWorld()); 변수이름; ++변수이름)
	TArray<ASecondProjectCharacter*> players;

	for (TActorIterator<ASecondProjectCharacter> sPlayer(GetWorld()); sPlayer; ++sPlayer)
	{
		players.Add(*sPlayer);
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
