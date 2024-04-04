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

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// 헤드샷용 컴포넌트를 생성
	headShot = CreateDefaultSubobject<USphereComponent>(TEXT("Head Shot"));
	headShot->SetupAttachment(GetCapsuleComponent());
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
	warningComp->SetupAttachment(RootComponent);
	warningComp->SetText(warningText);
	warningComp->SetRelativeLocation(FVector(0, 0, 130));

	//체크포인트 생성
	checkPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("CheckPoint1"));
	checkPoint1->SetRelativeLocation(FVector(700, -700, 0));
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

	ChangeWarning();

	//UE_LOG(LogTemp, Warning,TEXT("%d"),delayCheck);

	//UE_LOG(LogTemp, Warning, TEXT("%f"), UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint1->GetRelativeLocation()));
	//UE_LOG(LogTemp, Warning, TEXT("%f"), UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint2->GetRelativeLocation()));
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BasicMoveCycle1()
{
	// AEnemyCon 캐스팅
	AEnemyCon* aiCon = Cast<AEnemyCon>(GetController());

	// AI 컨트롤러 안에 있는 MoveToLocation 함수를 사용
	aiCon->MoveToLocation(checkPoint1->GetRelativeLocation());
	// 목적지와 에너미 사이의 거리를 구한다.
	float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint1->GetComponentLocation());
	// 거리값이 100 이하일 때
	if (check < 100)
	{
		bArrive1 = true;
		bArrive2 = false;
		bGo = false;
		if (anim1 != nullptr)
		{
			PlayAnimMontage(anim1);
			//UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
		}
	}

}

void AEnemy::BasicMoveCycle2()
{
	AEnemyCon* aiCon = Cast<AEnemyCon>(GetController());
	aiCon->MoveToLocation(checkPoint2->GetRelativeLocation());
	float check = UKismetMathLibrary::Vector_Distance(GetActorLocation(), checkPoint2->GetComponentLocation());
	if (check < 100)
	{
		bArrive2 = true;
		bArrive1 = false;
		bGo = false;
		if (anim1 != nullptr)
		{
			PlayAnimMontage(anim1);
			//UE_LOG(LogTemp, Warning, TEXT("anim"), delayCheck);
		}
	}
}

void AEnemy::Damaged(float damage)
{
	Life = FMath::Clamp(Life - damage, 0, 100);

	if (Life <= 0)
	{
		GetCharacterMovement()->DisableMovement();
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0);
		headShot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		warningComp->SetVisibility(false);
	}

	UE_LOG(LogTemp, Warning, TEXT("%f"), Life);
}

void AEnemy::ChangeWarning()
{
	warningstack = FMath::Clamp(warningstack, 0, 3000);

	if (warningstack < 1000)
	{
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

		if (delayCheck > 100)
		{
			bGo = true;
			delayCheck = 0;
		}

		warningComp->SetText(FText::FromString(TEXT(".")));
	}
	else if(warningstack < 2000)
	{
		warningComp->SetText(FText::FromString(TEXT("?")));
	}
	else
	{
		warningComp->SetText(FText::FromString(TEXT("!")));
		AEnemyCon* aiCon = Cast<AEnemyCon>(GetController());
		FindPlayerIterater();

		if (FindPlayerIterater() != nullptr)
		{
				
			aiCon->MoveToActor(FindPlayerIterater());
			GetCharacterMovement()->MaxWalkSpeed = 500.0f;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Fail"));
		}
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
