// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HitManAI.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
	MOVE		UMETA(DisplayName = "Move State"),
	MOVEDELAY	UMETA(DisplayName = "Move Delay State"),
	WATCH		UMETA(DisplayName = "watch State"),
	SEARCH		UMETA(DisplayName = "search State"),
	ATTACK		UMETA(DisplayName = "Attack State"),
	ATTACKDELAY	UMETA(DisplayName = "Attack Delay State"),
	CHASE		UMETA(DisplayName = "Chase State"),
	RUNAWAY		UMETA(DisplayName = "RunAway State"),
	PANIC		UMETA(DisplayName = "Panic State"),
	DAMAGED		UMETA(DisplayName = "Damaged State"),
};
UCLASS()
class SECONDPROJECT_API AHitManAI : public ACharacter
{
	GENERATED_BODY()

public:
	AHitManAI();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	EAIState aiState = EAIState::MOVEDELAY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USphereComponent* headShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* torso;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* hand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* pants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* shoes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTextRenderComponent* warningComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UStaticMeshComponent* gunMeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	int32 aiType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	TArray<class UAnimMontage*> normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UAnimMontage* idleTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UAnimMontage* attack;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UAnimMontage* attackdelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UAnimMontage* drinkPoison;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	TArray<FVector> setMoveLoc;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="My Settings")
	int32 currentSetLoc = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="My Settings")
	int32 currentSetRot = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	TArray<FRotator> setMoveRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	FVector moveLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	FRotator setRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	FVector currentLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	FRotator currentRot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="My Settings")
	int32 warningStack = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	float actionDelay = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	int32 attackDamage = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="My Settings")
	bool bIsDrinkPoison = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="My Settings")
	bool bIsDie = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="My Settings")
	bool bEquipWeapon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	bool bSeeWeapon = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="My Settings")
	FVector ranLoc;

	UFUNCTION(BlueprintCallable)
	void UpdateWarningStack(int32 warnStack);

	UFUNCTION(BlueprintCallable)
	void Damaged(int32 damage);

	UFUNCTION(BlueprintCallable)
	void DrinkPoison(bool bIsDrink, FVector poisonLocation);
	
	// AI 세이브용 파일 출력 함수(보류)
	//UFUNCTION(BlueprintCallable)
private:
	
	int32 maxHP = 100;
	int32 currentHP;
	float delayStack = 0;
	

	class AEnemyCon* aiCon;
	class ASecondProjectCharacter* hitmanPlayer;

	UFUNCTION()
	void Die();

	UFUNCTION()
	void EquipWeapon();

	UFUNCTION()
	void GetRandomLocation();

	void MoveArround();
	void MoveDelay(float deltatime);
	void Watch(float deltatime);
	void SearchArround();
	void AttackTarget(int32 hitDamage);
	void AttackDelay();
	void Chase();
	void RunAway(float deltatime);
	void Panic(float deltatime);
	void DamagedProcess(float deltatime);

	UFUNCTION(BlueprintCallable)
	class ASecondProjectCharacter* FindPlayerIterater();
};
