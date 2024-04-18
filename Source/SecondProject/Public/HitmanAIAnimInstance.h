// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HitManAI.h"
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HitmanAIAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SECONDPROJECT_API UHitmanAIAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:

	virtual void NativeInitializeAnimation()override;

	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MySettings")
	class AHitManAI* ai;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MySettings")
	EAIState currentState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MySettings")
	bool bIsArmed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MySettings")
	FVector speed;
};
