// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyCon.generated.h"

/**
 * 
 */
UCLASS()
class SECONDPROJECT_API AEnemyCon : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyCon();

	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UAIPerceptionComponent* aiPerception;
};
