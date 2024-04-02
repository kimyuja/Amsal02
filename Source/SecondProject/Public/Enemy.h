
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class SECONDPROJECT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class USphereComponent* headShot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class USceneComponent* checkPoint1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class USceneComponent* checkPoint2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	float Life = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	float damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	FText warningText;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	FVector startLocation;


protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
