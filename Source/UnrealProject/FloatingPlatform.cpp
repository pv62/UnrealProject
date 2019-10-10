// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingPlatform.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AFloatingPlatform::AFloatingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	StartPoint = FVector(0.f);
	EndPoint = FVector(0.f);

	bInterping = false;

	InterpSpeed = 4.f;
	InterpTime = 1.f;
}

// Called when the game starts or when spawned
void AFloatingPlatform::BeginPlay()
{
	Super::BeginPlay();
	
	StartPoint = GetActorLocation();
	EndPoint += StartPoint;

	bInterping = false;

	GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime);

	Distance = (EndPoint - StartPoint).Size();
}

// Called every frame
void AFloatingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInterping)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector Interp = FMath::VInterpTo(CurrentLocation, EndPoint, DeltaTime, InterpSpeed);
		SetActorLocation(Interp);

		float DistanceTravelled = (GetActorLocation() - StartPoint).Size();
		if ((Distance - DistanceTravelled) <= 1.f)
		{
			ToggleInterping();
			GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime);
			SwapVectors(StartPoint, EndPoint);
		}
	}	
}

void AFloatingPlatform::ToggleInterping()
{
	bInterping = !bInterping;
}

void AFloatingPlatform::SwapVectors(FVector& V1, FVector& V2)
{
	FVector Temp = V1;
	V1 = V2;
	V2 = Temp;
}

