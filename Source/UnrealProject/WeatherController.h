// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeatherController.generated.h"

UCLASS()
class UNREALPROJECT_API AWeatherController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeatherController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
	class UParticleSystemComponent* RainParticles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
	class UAudioComponent* RainSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
	class UMaterialParameterCollection* Collection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
	FName ParameterName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rain")
	float TargetRainLevel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rain")
	float TargetCloudOpacity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rain")
	float ChangeRate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void UpdateSky();
	UFUNCTION(BlueprintCallable)
	void UpdateRain();

	UFUNCTION(BlueprintCallable)
	void ToggleRain(bool Enable);
};
