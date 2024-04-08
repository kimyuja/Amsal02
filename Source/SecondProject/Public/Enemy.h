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
	class UAnimMontage* normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UAnimMontage* gunAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	class UAnimMontage* drink;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	float Life = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	TArray<FVector> checkPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="My Settings")
	FText warningText = FText::FromString(TEXT("."));

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="My Settings")
	int32 delay = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	int32 warningstack = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="My Settings")
	int32 bulletCount = 0;

	bool bDie = false;

	class AEnemyCon* aiCon;

	class ASecondProjectCharacter* player;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void BasicMoveCycle(FVector point, UAnimMontage* anim, bool arrive1, bool arrive2);

	UFUNCTION(BlueprintCallable)
	void Damaged(float damage);

	UFUNCTION(BlueprintCallable)
	void DrinkPoison(FVector poisonLoc);

	UFUNCTION(BlueprintCallable)
	void ChangeWarning();

	UFUNCTION(BlueprintCallable)
	void ShootPlayer();

	UFUNCTION(BlueprintCallable)
	void UpdateStack(int32 warn);

	UFUNCTION(BlueprintCallable)
	class ASecondProjectCharacter* FindPlayerIterater();

};