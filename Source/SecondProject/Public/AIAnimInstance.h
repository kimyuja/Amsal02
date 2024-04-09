// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AIAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SECONDPROJECT_API UAIAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation()override;

	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "My Settings")
	bool bRangedAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "My Settings")
	bool bMeleeAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "My Settings")
	int32 enemyType = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "My Settings")
	FVector aiSpeed; 
private:
	
	UPROPERTY()
	class AEnemy* enemy;

	UPROPERTY()
	class UCharacterMovementComponent* enemyMovement;

};
