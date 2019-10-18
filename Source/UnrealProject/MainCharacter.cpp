// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Weapon.h"
#include "Enemy.h"
#include "FirstSaveGame.h"
#include "ItemStorage.h"
#include "MainPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Camera Boom (pulls towards player if there's a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; // Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller

	// Set size for collision for capsule
	GetCapsuleComponent()->InitCapsuleSize(50.f, 105.f);

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	// Set out turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Don't rotate when the controller rotates. Let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure Character Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;			// Character moves in the direction of input....
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);	// ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.2f;

	bHasCombatTarget = false;

	MaxHealth = 100.f;
	Health = 100.f;

	MaxStamina = 150.f;
	Stamina = 150.f;

	Coins = 0.f;

	WalkingSpeed = 250.f;
	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bSprintKeyDown = false;
	bAttackDown = false;
	bEquipDown = false;
	bPauseDown = false;
	bCrouchDown = false;
	bBlockDown = false;

	Section = 0;
	NumOfSections = 2;

	bMovingForward = false;
	bMovingRight = false;

	// Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	//LoadGameNoSwitch();
	if (MainPlayerController)
	{
		MainPlayerController->GameModeOnly();
	}
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }

	float DeltaStamnia = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
		case EStaminaStatus::ESS_Normal:
			if (bSprintKeyDown && (bMovingForward == true || bMovingRight == true))
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
				Stamina -= DeltaStamnia;
				if (Stamina <= MinSprintStamina)
				{
					SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				}
			}
			else // Sprint Key Up
			{
				if (Stamina + DeltaStamnia >= MaxStamina)
				{
					Stamina = MaxStamina;
				}
				else
				{
					Stamina += DeltaStamnia;
				}
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;
		case EStaminaStatus::ESS_BelowMinimum:
			if (bSprintKeyDown && (bMovingForward == true || bMovingRight == true))
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
				if (Stamina - DeltaStamnia <= 0.f)
				{
					SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
					Stamina = 0.f;
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
				else
				{
					Stamina -= DeltaStamnia;
				}
			}
			else // Sprint Key Up
			{
				Stamina += DeltaStamnia;
				if (Stamina + DeltaStamnia >= MinSprintStamina)
				{
					SetStaminaStatus(EStaminaStatus::ESS_Normal);
					
				}
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;
		case EStaminaStatus::ESS_Exhausted:
			if (bSprintKeyDown && (bMovingForward == true || bMovingRight == true))
			{
				Stamina = 0.f;
			}
			else // Sprint Key Up
			{
				SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
				Stamina += DeltaStamnia;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		case EStaminaStatus::ESS_ExhaustedRecovering:
			Stamina += DeltaStamnia;
			if (Stamina + DeltaStamnia >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		default:
			break;
	}

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AMainCharacter::SprintKeyDown);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AMainCharacter::SprintKeyUp);

	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &AMainCharacter::AttackDown);
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Released, this, &AMainCharacter::AttackUp);

	PlayerInputComponent->BindAction(TEXT("Pause"), IE_Pressed, this, &AMainCharacter::PauseDown);
	PlayerInputComponent->BindAction(TEXT("Pause"), IE_Released, this, &AMainCharacter::PauseUp);

	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &AMainCharacter::EquipDown);
	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Released, this, &AMainCharacter::EquipUp);

	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AMainCharacter::CrouchDown);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &AMainCharacter::CrouchUp);

	PlayerInputComponent->BindAction(TEXT("Block"), IE_Pressed, this, &AMainCharacter::BlockDown);
	PlayerInputComponent->BindAction(TEXT("Block"), IE_Released, this, &AMainCharacter::BlockUp);
	
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMainCharacter::MoveRight);

	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AMainCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AMainCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("TurnAtRate"), this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUpAtRate"), this, &AMainCharacter::LookUpAtRate);
}

void AMainCharacter::Jump()
{
	if (MainPlayerController) { if (MainPlayerController->bPauseMenuVisible) { return; } }
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump();
	}
}

bool AMainCharacter::CanMove(float Value)
{
	if (MainPlayerController)
	{
		return (Value != 0.f) && (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Dead) && (!MainPlayerController->bPauseMenuVisible);
	}
	return false;
}

void AMainCharacter::MoveForward(float Value)
{
	bMovingForward = false;

	if (CanMove(Value))
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}
}

void AMainCharacter::MoveRight(float Value)
{
	bMovingRight = false;

	if (CanMove(Value))
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true;
	}
}

void AMainCharacter::Turn(float Value)
{
	if (CanMove(Value))
	{
		AddControllerYawInput(Value);
	}
}

void AMainCharacter::LookUp(float Value)
{
	if (CanMove(Value))
	{
		AddControllerPitchInput(Value);
	}
}

void AMainCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

FRotator AMainCharacter::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

void AMainCharacter::FootStep(bool IsLeft)
{
	if (IsLeft && LeftFootSound)
	{
		UGameplayStatics::PlaySound2D(this, LeftFootSound);
	}
	if (!IsLeft && RightFootSound)
	{
		UGameplayStatics::PlaySound2D(this, RightFootSound);
	}
}

void AMainCharacter::PlayAttackAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		int32 CurrentSection = (Section % NumOfSections) + 1;
		FString SectionName(FString::Printf(TEXT("Attack_%d"), CurrentSection));
		AnimInstance->Montage_Play(CombatMontage, 2.f);
		AnimInstance->Montage_JumpToSection(FName(*SectionName), CombatMontage);
	}
}

void AMainCharacter::Attack()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (!bAttacking)
		{
			bAttacking = true;
			SetInterpToEnemy(true);

			PlayAttackAnimation();
		}
		else
		{
			bInCombo = true;
			++Section;
		}
	}	
}

void AMainCharacter::Combo()
{
	if (bInCombo)
	{
		bInCombo = false;
		PlayAttackAnimation();
	}
}

void AMainCharacter::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bAttackDown && bInCombo)
	{
		++Section;
		Attack();
	}
}

void AMainCharacter::ComboEnd()
{
	Section = 0;
	bInCombo = false;
	bAttacking = false;
	SetInterpToEnemy(false);
}

void AMainCharacter::GainCoins(int32 Amount)
{
	Coins += Amount;
}

void AMainCharacter::GainHealth(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += Amount;
	}
}

float AMainCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	if (Health <= 0.f)
	{
		Die();
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
				Enemy->bHasValidTarget = false;
			}
		}
	}
	return DamageAmount;
}

void AMainCharacter::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMainCharacter::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMainCharacter::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		if (bCrouchDown)
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
		}
	}
}

void AMainCharacter::SprintKeyDown()
{
	bSprintKeyDown = true;
}

void AMainCharacter::SprintKeyUp()
{
	bSprintKeyDown = false;
}

void AMainCharacter::AttackDown()
{
	bAttackDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }
	if (MainPlayerController) { if (MainPlayerController->bPauseMenuVisible) { return; } }

	if (EquippedWeapon)
	{
		Attack();
	}
	else
	{
		if (UnequippedWeapon)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance && UpperBodyMontage)
			{
				AnimInstance->Montage_Play(UpperBodyMontage, 1.f);
				AnimInstance->Montage_JumpToSection(FName("Equip"), UpperBodyMontage);
			}
		}
	}
}

void AMainCharacter::AttackUp()
{
	bAttackDown = false;
}

void AMainCharacter::BlockDown()
{
	if (EquippedWeapon)
	{
		bBlockDown = true;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && UpperBodyMontage)
		{
			AnimInstance->Montage_Play(UpperBodyMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Block"), UpperBodyMontage);
		}
	}	
}

void AMainCharacter::BlockUp()
{
	bBlockDown = false;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && UpperBodyMontage)
	{
		AnimInstance->Montage_Stop(0.25f, UpperBodyMontage);
	}
}

void AMainCharacter::EquipDown()
{
	bEquipDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }
	if (MainPlayerController) { if (MainPlayerController->bPauseMenuVisible) { return; } }

	if (ActiveOverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}
	else
	{
		PlayEquipMontage();		
	}
}

void AMainCharacter::EquipUp()
{
	bEquipDown = false;
}

void AMainCharacter::EquipWeapon()
{
	UnequippedWeapon->Equip(this);
}

void AMainCharacter::UnequipWeapon()
{
	EquippedWeapon->Unequip(this);
}

void AMainCharacter::PlayEquipMontage()
{
	if (EquippedWeapon)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && UpperBodyMontage)
		{
			AnimInstance->Montage_Play(UpperBodyMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Unequip"), UpperBodyMontage);
		}
	}
	else if (UnequippedWeapon)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && UpperBodyMontage)
		{
			AnimInstance->Montage_Play(UpperBodyMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Equip"), UpperBodyMontage);
		}
	}
}

void AMainCharacter::PauseDown()
{
	bPauseDown = true;

	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}
}

void AMainCharacter::PauseUp()
{
	bPauseDown = false;
}

void AMainCharacter::CrouchDown()
{
	bCrouchDown = !bCrouchDown;
}

void AMainCharacter::CrouchUp()
{
	//bCrouchDown = false;
}

void AMainCharacter::ShowPickupLocations()
{
	for (int32 i = 0; i < PickupLocations.Num(); i++)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 25.f, 8, FLinearColor::Green, 10.f, 1.f);

	}
}

void AMainCharacter::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon)
	{
		UnequippedWeapon = EquippedWeapon;
	}
	EquippedWeapon = WeaponToSet;
}

void AMainCharacter::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

void AMainCharacter::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0) 
	{ 
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}
	
	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if (ClosestEnemy)
	{
		FVector Location = GetActorLocation();
		float MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size();
		for (auto Actor : OverlappingActors)
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
			if (DistanceToActor < MinDistance)
			{
				MinDistance = DistanceToActor;
				ClosestEnemy = Enemy;
			}
		}

		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

void AMainCharacter::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();

		if (FName(*CurrentLevel) != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMainCharacter::SaveGame()
{
	UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	if (EquippedWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}
	if (UnequippedWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = UnequippedWeapon->Name;
	}

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	SaveGameInstance->CharacterStats.LevelName = MapName;

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMainCharacter::LoadGame(bool SetPosition)
{
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;
			if (WeaponName != TEXT(""))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}			
		}
	}

	if (SetPosition)
	{
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;


	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);
		SwitchLevel(LevelName);
	}
}

void AMainCharacter::LoadGameNoSwitch()
{
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;
			if (WeaponName != TEXT(""))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}