// Fill out your copyright notice in the Description page of Project Settings.


#include "AIAnimInstance.h"
#include "Enemy.h"
#include <../../../../../../../Source/Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h>

void UAIAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	enemy = Cast<AEnemy>(GetOwningActor());

	if (enemy != nullptr)
	{
		enemyMovement = enemy->GetCharacterMovement();
	}
}

void UAIAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//aiSpeed = enemyMovement->Velocity;

}
