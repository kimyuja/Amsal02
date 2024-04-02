
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
	class UTextRenderComponent* warningComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	float Life = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
<<<<<<< Updated upstream
	float damage;
=======
	class UAIPerceptionComponent* aiperception;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	float Life = 100.0f;
>>>>>>> Stashed changes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	FText warningText = FText::FromString(".");
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	FVector startLocation;


protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

<<<<<<< Updated upstream
=======
	UFUNCTION()
	void BasicMoveCycle1();

	UFUNCTION()
	void BasicMoveCycle2();

	UFUNCTION(BlueprintCallable)
	void Damaged(float damage);

	UFUNCTION()
	void OnPerceptionPlayer(const TArray<AActor*>& UpdatedActors);
>>>>>>> Stashed changes
};
