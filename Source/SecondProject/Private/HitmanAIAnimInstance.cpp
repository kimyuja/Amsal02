// Fill out your copyright notice in the Description page of Project Settings.


#include "HitmanAIAnimInstance.h"

void UHitmanAIAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	ai = Cast<AHitManAI>(GetOwningActor());
}

void UHitmanAIAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (ai != nullptr)
	{
		currentState = ai->aiState;
		bIsArmed = ai->bEquipWeapon;
		speed = ai->GetVelocity();
	}
}
