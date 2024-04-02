
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
	class USkeletalMeshComponent* torso;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class USkeletalMeshComponent* hand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class USkeletalMeshComponent* pants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class USkeletalMeshComponent* shoes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class USceneComponent* checkPoint1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class USceneComponent* checkPoint2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UAnimMontage* anim1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	float Life = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	float damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	FText warningText;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	FVector startLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	bool bArrive1 = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	bool bArrive2 = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	bool bGo = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	int32 play = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	int32 delayCheck = 0;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void BasicMoveCycle1();

	UFUNCTION()
	void BasicMoveCycle2();
};
