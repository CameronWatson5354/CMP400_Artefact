// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcPlayerCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "ProcInGameGeneratorWidget.h"
#include "ProcSwordData.h"
#include "Components/CapsuleComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AProcPlayerCharacter::AProcPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComp->SetupAttachment(GetRootComponent());

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	SwordRotateComp = CreateDefaultSubobject<USceneComponent>(TEXT("SwordRotateComp"));
	SwordRotateComp->SetupAttachment(CameraComp);
	
	SwordMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	SwordMeshComp->SetupAttachment(SwordRotateComp);

	DynamicSwordMeshComp = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicSwordMeshComp"));
	DynamicSwordMeshComp->SetupAttachment(SwordMeshComp);

	SwordCollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SwordCollisionComp"));
	SwordCollisionComp->SetupAttachment(SwordMeshComp);

	SwordTipCollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SwordTipCollisionComp"));
	SwordTipCollisionComp->SetupAttachment(SwordMeshComp);
	
	
	

}

// Called when the game starts or when spawned
void AProcPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	RestingTransform = SwordMeshComp->GetRelativeTransform();

	DefaultMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;

	AdjustedStartingStabTransform = StartingStabTransform;

	SwordCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SwordTipCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called every frame
void AProcPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (OnAttack.IsBound())
	{
		OnAttack.Execute();
	}
}

// Called to bind functionality to input
void AProcPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	//add IMC
    if (APlayerController* PlayerController = GetController<APlayerController>())
    {
    	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
    	{
    		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    		{
    			InputSystem->AddMappingContext(DefaultInputMapping.LoadSynchronous(), 0);
    		}
    	}
    }
    
    if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
    	//move
    	if (ensure(!MoveAction.IsNull()))
    	{
    		MoveAction.LoadSynchronous();
    		Input->BindAction(MoveAction.Get(), ETriggerEvent::Triggered, this, &AProcPlayerCharacter::MoveFunction, ETriggerEvent::Triggered);
    	}
    	
    	//look
    	if (ensure(!LookAction.IsNull()))
    	{
    		LookAction.LoadSynchronous();
    		Input->BindAction(LookAction.Get(), ETriggerEvent::Triggered, this, &AProcPlayerCharacter::LookFunction, ETriggerEvent::Triggered);
    	}

    	if (ensure(!SlashAction.IsNull()))
    	{
    		SlashAction.LoadSynchronous();
    		Input->BindAction(SlashAction.Get(), ETriggerEvent::Triggered, this, &AProcPlayerCharacter::SlashActionCallback, ETriggerEvent::Triggered);
    		SlashBinding = &Input->BindActionValue(SlashAction.Get());
    	}

    	if (ensure(!StabAction.IsNull()))
    	{
    		StabAction.LoadSynchronous();
    		Input->BindAction(StabAction.Get(), ETriggerEvent::Triggered, this, &AProcPlayerCharacter::StabActionCallback, ETriggerEvent::Triggered);
    		StabBinding = &Input->BindActionValue(StabAction.Get());
    		
    	}
    }

}

void AProcPlayerCharacter::PickupSword_Implementation(UProcSwordData* NewSwordData)
{
	SwordData = NewSwordData;

	SwordMeshComp->SetStaticMesh(nullptr);
	DynamicSwordMeshComp->GetDynamicMesh()->Reset();

	if (IsValid(SwordData))
	{
		SwordAttributes = SwordData->GetSwordAttributes(); 
		 
		SwordMeshComp->SetStaticMesh(SwordData->GetSwordMesh());

		if (IsValid(SwordData->GetDynamicSwordMesh()))
		{
			DynamicSwordMeshComp->SetDynamicMesh(SwordData->GetDynamicSwordMesh()->GetDynamicMesh());
			DynamicSwordMeshComp->SetMaterial(0, SwordData->GetDynamicSwordMesh()->GetMaterial(0));
			DynamicSwordMeshComp->SetMaterial(1, SwordData->GetDynamicSwordMesh()->GetMaterial(1));
			DynamicSwordMeshComp->SetMaterial(2, SwordData->GetDynamicSwordMesh()->GetMaterial(2));
			DynamicSwordMeshComp->SetMaterial(3, SwordData->GetDynamicSwordMesh()->GetMaterial(3));
		}

		float NewWalkSpeed{FMath::Clamp(DefaultMoveSpeed - SwordData->GetSwordAttributes().Weight * 4, 400.0f, DefaultMoveSpeed)};
		GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;

		//apply hitbox offset
		float CapsuleHalfHeight{SwordAttributes.BladeLength / 2 + SwordAttributes.HitboxWidth / 4};
		
		FRotator TempRotation{SwordCollisionComp->GetRelativeRotation()};
		SwordCollisionComp->SetRelativeLocation(FVector{0.0f, 0.0f, SwordAttributes.TotalLength - CapsuleHalfHeight});
		SwordCollisionComp->SetCapsuleHalfHeight(CapsuleHalfHeight);
		SwordCollisionComp->SetCapsuleRadius(SwordAttributes.HitboxWidth / 2); 
		SwordCollisionComp->SetRelativeRotation(TempRotation);

		SwordTipCollisionComp->SetRelativeLocation(FVector{0.0f, 0.0f, SwordAttributes.TotalLength - SwordCollisionComp->GetScaledCapsuleRadius()});
		SwordTipCollisionComp->SetSphereRadius(SwordCollisionComp->GetScaledCapsuleRadius());

		RotationalAcceleration = SwordAttributes.SwingSpeed;
		StabAcceleration = SwordAttributes.StabSpeed;


		AdjustedMaxReach = (MaxReach + SwordAttributes.GripLength);
		float BackReach{FMath::Min(MaxReach, SwordAttributes.BladeLength)};

		AdjustedMaxReach += BackReach;
		
		FVector StartStabLocation{StartingStabTransform.GetLocation()};
		StartStabLocation.Y += BackReach;
		AdjustedStartingStabTransform.SetLocation(StartStabLocation);

		MaxViewSpeed = 1000.0f / SwordAttributes.Weight;

		bPickedUpFirstSword = true;
	}
}

UProcSwordData* AProcPlayerCharacter::GetSwordData_Implementation()
{
	return SwordData;
}

void AProcPlayerCharacter::MovePlayer(FVector2D InputAxis)
{
	AddMovementInput(GetActorForwardVector(), InputAxis.Y);
	AddMovementInput(GetActorRightVector(), InputAxis.X);
}

void AProcPlayerCharacter::LookAround(FVector2D InputAxis)
{
	FVector2D AdjustedInputAxis{InputAxis};

	if (ActiveAttacks > 0)
	{
		float MaxMoveAmount{static_cast<float>(MaxViewSpeed * UGameplayStatics::GetWorldDeltaSeconds(this))};

		FVector2D Direction{};
		float Length{0.0f};
		InputAxis.ToDirectionAndLength(Direction, Length);

		if (Length >= MaxMoveAmount)
		{
			AdjustedInputAxis = Direction * MaxMoveAmount;
		}
	}
	
	AddControllerYawInput(AdjustedInputAxis.X);
	AddControllerPitchInput(AdjustedInputAxis.Y * -1);
}

void AProcPlayerCharacter::MoveFunction(const FInputActionInstance& Value, ETriggerEvent EventType)
{
	FVector2D InputVector = Value.GetValue().Get<FVector2D>();

	switch (EventType)
	{
	case ETriggerEvent::Triggered:
		MovePlayer(InputVector);
		break;
	}
}

void AProcPlayerCharacter::LookFunction(const FInputActionInstance& Value, ETriggerEvent EventType)
{
	FVector2D InputVector = Value.GetValue().Get<FVector2D>();

	switch (EventType)
	{
	case ETriggerEvent::Triggered:
		LookAround(InputVector);
		break;
	}
}

void AProcPlayerCharacter::SlashUpdateCallback()
{
	float DeltaTime{static_cast<float>(UGameplayStatics::GetWorldDeltaSeconds(this))};
	
	RotationSpeed += RotationalAcceleration * DeltaTime;
	float RotationDelta{-RotationSpeed * DeltaTime};

	CurrentRotation += RotationDelta;
	
	if (FMath::Abs(CurrentRotation) >= MaxRotation)
	{
		--ActiveAttacks;
		OnAttack.Unbind();
		CurrentRotation = 0.0f;
		RotationSpeed = 0.0f;
		
		SwordRotateComp->SetRelativeRotation(StartingSlashRotation);
		SwordCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (!StabBinding->GetValue().Get<bool>() && !SlashBinding->GetValue().Get<bool>())
		{
			SwordMeshComp->SetRelativeTransform(RestingTransform);
		}
		HitActors.Empty();
		return;
	}

	SwordRotateComp->AddRelativeRotation(FRotator{0.0f, RotationDelta, 0.0f});
}

void AProcPlayerCharacter::StabUpdateCallback()
{
	float DeltaTime{static_cast<float>(UGameplayStatics::GetWorldDeltaSeconds(this))};

	int Direction = 1;
	if (bReachedStabPeak)
	{
		Direction = -1;
	}

	
	if (!bReachedStabPeak)
	{
		StabSpeed += StabAcceleration * DeltaTime;
	}
	float LocationDelta{StabSpeed * DeltaTime * Direction};

	StabOffset += LocationDelta;

	if (StabOffset >= AdjustedMaxReach && !bReachedStabPeak)
	{
		bReachedStabPeak = true;
		SwordTipCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else if (StabOffset < 0.0f && bReachedStabPeak)
	{
		--ActiveAttacks;
		OnAttack.Unbind();
		StabOffset = 0.0f;
		StabSpeed = 0.0f;
		

		if (!StabBinding->GetValue().Get<bool>() && !SlashBinding->GetValue().Get<bool>())
		{
			SwordMeshComp->SetRelativeTransform(RestingTransform);
		}
		HitActors.Empty();
		bReachedStabPeak = false;
		return;
	}
	
	SwordMeshComp->AddRelativeLocation(FVector{0.0f, -LocationDelta, 0.0f});
}

void AProcPlayerCharacter::StabActionCallback(const FInputActionInstance& Value, ETriggerEvent EventType)
{
	bool BoolValue{Value.GetValue().Get<bool>()};

	switch (EventType)
	{
	case ETriggerEvent::Triggered:

		if (ActiveAttacks <= 0 && bPickedUpFirstSword)
		{
			++ActiveAttacks;

			OnAttack.BindDynamic(this, &AProcPlayerCharacter::StabUpdateCallback);

			StartingStabOffset = SwordMeshComp->GetRelativeLocation();

			SwordTipCollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

			SwordMeshComp->SetRelativeTransform(AdjustedStartingStabTransform);

			AttackType = EAttackType::Stab;
			
		}
		break;
	}
}

void AProcPlayerCharacter::SlashActionCallback(const FInputActionInstance& Value, ETriggerEvent EventType)
{
	bool BoolValue{Value.GetValue().Get<bool>()};

	switch (EventType)
	{
	case ETriggerEvent::Triggered:
		if (ActiveAttacks <= 0 && bPickedUpFirstSword)
		{
			++ActiveAttacks;

			OnAttack.BindDynamic(this, &AProcPlayerCharacter::SlashUpdateCallback);

			StartingSlashRotation = SwordRotateComp->GetRelativeRotation();

			SwordMeshComp->SetRelativeTransform(StartingSlashTransform);

			SwordCollisionComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);

			AttackType = EAttackType::Slash;
		}
		
		break;
	}
}

FVector AProcPlayerCharacter::GetKnockback(AActor* OtherActor, float HorizontalVelocity, float ZVelocity)
{
	if (!IsValid(OtherActor))
	{
		return FVector{};
	}
	
	FVector A{GetActorLocation()};
	FVector B{OtherActor->GetActorLocation()};
	FVector Direction{B - A};
	Direction.Normalize();

	float WeaponKnockback{0.0f};
	
	switch (AttackType)
	{
	case EAttackType::Slash:
		WeaponKnockback = SwordData->GetSwordAttributes().SlashKnockback;
		break;

	case EAttackType::Stab:
		WeaponKnockback = SwordData->GetSwordAttributes().StabKnockback;
		break;
	}
	
	Direction *= WeaponKnockback;
	//Direction.Z = ZVelocity;
	float AverageKnockback{(SwordData->GetSwordAttributes().SlashKnockback + SwordData->GetSwordAttributes().SlashKnockback) / 2.0f};
	
	Direction.Z = FMath::Clamp(AverageKnockback, 0.0f, 300.0f);
	
	return Direction;
}


