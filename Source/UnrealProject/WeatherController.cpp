// Fill out your copyright notice in the Description page of Project Settings.


#include "WeatherController.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"

// Sets default values
AWeatherController::AWeatherController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	RainParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RainParticles"));
	RainParticles->SetupAttachment(GetRootComponent());
	RainParticles->bAutoActivate = false;

	RainSound = CreateDefaultSubobject<UAudioComponent>(TEXT("RainSound"));
	RainSound->SetupAttachment(GetRootComponent());
	RainSound->bAutoActivate = false;

	TargetRainLevel = 0.f;
	TargetCloudOpacity = 0.f;

	ChangeRate = 0.1f;
}

// Called when the game starts or when spawned
void AWeatherController::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWeatherController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSky();
	UpdateRain();
}

void AWeatherController::UpdateSky()
{
	UWorld* World = GetWorld();
	if (World && Collection && ParameterName != "")
	{
		float RainLevel = UKismetMaterialLibrary::GetScalarParameterValue(World, Collection, ParameterName);
		if (FMath::Abs(TargetCloudOpacity - RainLevel) > 0.1f)
		{
			if (TargetCloudOpacity > RainLevel)
			{
				ChangeRate *= 1.f;
			}
			else if (TargetCloudOpacity > RainLevel)
			{
				ChangeRate *= -1.f;
			}
			RainLevel += ChangeRate;
			UKismetMaterialLibrary::SetScalarParameterValue(World, Collection, ParameterName, RainLevel);
		}
	}
}

void AWeatherController::UpdateRain()
{
	UWorld* World = GetWorld();
	if (World && Collection && ParameterName != "")
	{
		float RainLevel = UKismetMaterialLibrary::GetScalarParameterValue(World, Collection, ParameterName);
		if (FMath::Abs(TargetRainLevel - RainLevel) > 0.1f)
		{
			if (TargetRainLevel > RainLevel)
			{
				ChangeRate *= 1.f;
			}
			else if (TargetRainLevel > RainLevel)
			{
				ChangeRate *= -1.f;
			}
			RainLevel += ChangeRate;
			UKismetMaterialLibrary::SetScalarParameterValue(World, Collection, ParameterName, RainLevel);
		}
	}
}

void AWeatherController::ToggleRain(bool Enable)
{
	if (Enable)
	{
		RainParticles->ActivateSystem();
		RainSound->Play();
		TargetRainLevel = 0.9f;
	}
	else
	{
		RainParticles->DeactivateSystem();
		RainSound->Stop();
		TargetRainLevel = 0.f;
	}
}

