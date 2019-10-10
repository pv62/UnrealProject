// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	/** Reference to the UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	/** Variable to hold the widget after creating it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	/** Enemy Health Bar UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	/** Enemy Health Bar Widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;

	/** Pause Menu UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WPauseMenu;

	/** Pause Menu Widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;

	bool bEnemyHealthBarVisible;
	bool bPauseMenuVisible;

	FVector EnemyLocation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayPauseMenu();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemovePauseMenu();

	void TogglePauseMenu();

	void GameModeOnly();
};
