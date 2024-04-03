// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCon.h"
#include <../../../../../../../Source/Runtime/AIModule/Classes/Perception/AIPerceptionComponent.h>

AEnemyCon::AEnemyCon()
{
	PrimaryActorTick.bCanEverTick = true;

	aiPerception = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception");
}

void AEnemyCon::BeginPlay()
{
	Super::BeginPlay();
}
