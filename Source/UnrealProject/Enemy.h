// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyMovementStatus : uint8
{
	EMS_Idle			UMETA(DisplayName = "Idle"),
	EMS_MoveToTarget	UMETA(DisplayName = "MoveToTarget"),
	EMS_Attacking		UMETA(DisplayName = "Attacking"),
	EMS_Dead			UMETA(DisplayName = "Dead"),

	EMS_MAX				UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class UNREALPROJECT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EEnemyMovementStatus EnemyMovementStatus;

	/** When player overlaps the enemy chases */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* AgroSphere;

	/** When player overlaps the enemy attacks */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	USphereComponent* CombatSphere;

	/** Collision used for combat */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollisionLeft;

	/** Collision used for combat */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollisionRight;

	/** AI Controller for the enemy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController;

	/** If target is in range to attack */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bOverlappingCombatSphere;

	/** Target to chase/attack */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AMainCharacter* CombatTarget;

	/** Particles emitted when hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UParticleSystem* HitParticles;

	/** Sound played when hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class USoundCue* HitSound;

	/** Sound played when attacking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* SwingSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* CombatMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking;

	bool bHasValidTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AnimSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 NumOfSections;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 Section;

	FTimerHandle AttackTimer;

	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackMinTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackMaxTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	/**
	/*
	/* Enemy Stats
	/*
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI | Enemy Stats")
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Enemy Stats")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Enemy Stats")
	float Damage;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE EEnemyMovementStatus GetEnemyMovementStatus() { return EnemyMovementStatus; }
	FORCEINLINE void SetEnemyMovementStatus(EEnemyMovementStatus Status) { EnemyMovementStatus = Status; }

	UFUNCTION()
	virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void MoveToTarget(class AMainCharacter* Target);

	UFUNCTION()
	void CombatLeftOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void CombatLeftOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void CombatRightOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void CombatRightOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void ActivateLeftCollision();
	UFUNCTION(BlueprintCallable)
	void DeactivateLeftCollision();

	UFUNCTION(BlueprintCallable)
	void ActivateRightCollision();
	UFUNCTION(BlueprintCallable)
	void DeactivateRightCollision();

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die(AActor* Causer);

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	bool Alive();

	void Disappear();
};