// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "ProcSwordInterface.h"
#include "ProcSwordTypes.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "ProcPlayerCharacter.generated.h"

class UDynamicMeshComponent;
class UProcInGameGeneratorWidget;
class USphereComponent;
class UProcSwordData;
class USpringArmComponent;
class UCameraComponent;

class UInputMappingContext;
class UInputAction;

UENUM()
enum class EAttackType
{
	Slash = 0,
	Stab
};

DECLARE_DYNAMIC_DELEGATE(FOnAttackSignature);

UCLASS()
class PROCEDURALSWORDS_API AProcPlayerCharacter : public ACharacter,
	public IProcSwordInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AProcPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	void PickupSword_Implementation(UProcSwordData* NewSwordData);
	UProcSwordData* GetSwordData_Implementation() override;
	
protected:
	
	
	void MovePlayer(FVector2D InputAxis);
	void LookAround(FVector2D InputAxis);
	
	void MoveFunction(const FInputActionInstance& Value, ETriggerEvent EventType);
	void LookFunction(const FInputActionInstance& Value, ETriggerEvent EventType);

	UFUNCTION()
	void SlashUpdateCallback();
	
	UFUNCTION()
	void StabUpdateCallback();
	
	void StabActionCallback(const FInputActionInstance& Value, ETriggerEvent EventType);
	void SlashActionCallback(const FInputActionInstance& Value, ETriggerEvent EventType);

	

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetKnockback(AActor* OtherActor, float HorizontalVelocity, float ZVelocity);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FProcSwordAttributes SwordAttributes;

	FOnAttackSignature OnAttack;

	

protected:
	/* Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComp{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComp{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SwordRotateComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SwordMeshComp{nullptr};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UDynamicMeshComponent> DynamicSwordMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UCapsuleComponent> SwordCollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USphereComponent> SwordTipCollisionComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlashTimeline")
	float SlashLength{0.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StabTimeline")
	float StabLength{0.0f};
	
	/* Inputs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> DefaultInputMapping{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> MoveAction{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> LookAction{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> StabAction{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> SlashAction{nullptr};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UProcSwordData> SwordData{nullptr};

	FOnTimelineEvent SlashUpdateEvent;
	FOnTimelineEvent SlashFinishEvent;

	FOnTimelineEvent StabUpdateEvent;
	FOnTimelineEvent StabFinishEvent;

	

	UPROPERTY(Transient)
	FRotator DefaultMeshRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlashTimeline")
	FRotator MeshSlashRotation;

	UPROPERTY(Transient)
	FRotator SwordRotatorDefaultRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlashTimeline")
	FRotator SwordRotatorRotation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SlashTimeline")
	TObjectPtr<UCurveFloat> SlashCurve{nullptr};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "StabTimeline")
	TObjectPtr<UCurveFloat> StabCurve{nullptr};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "StabTimeline")
	FRotator StabMeshRotation;

	UPROPERTY()
	FRotator StartingStabRotation;

	UPROPERTY()
	FVector StartingRelativePos;

	UPROPERTY()
	FVector StartingStabRelativePos;

	UPROPERTY()
	FRotator StartingSlashRotation;

	UPROPERTY()
	int ActiveAttacks{0};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RotationalAcceleration{0.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RotationSpeed{0.0f};

	UPROPERTY(EditDefaultsOnly, blueprintreadwrite)
	float MaxRotation{0.0f};

	UPROPERTY()
	float CurrentRotation{0.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxReach{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float AdjustedMaxReach{0.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float StabAcceleration{0.0};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float StabSpeed{0.0f};

	UPROPERTY()
	float StabOffset{0.0f};

	UPROPERTY()
	FVector StartingStabOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FTransform RestingTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FTransform StartingStabTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FTransform AdjustedStartingStabTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FTransform StartingSlashTransform;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSet<TSoftObjectPtr<AActor>> HitActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UProcInGameGeneratorWidget> GeneratorWidget{nullptr};

	UPROPERTY()
	bool bReachedStabPeak{false};

	UPROPERTY()
	EAttackType AttackType{0};

	UPROPERTY()
	float DefaultMoveSpeed{0.0f};

	FEnhancedInputActionValueBinding* SlashBinding;
	FEnhancedInputActionValueBinding* StabBinding;

	UPROPERTY()
	float MaxViewSpeed{0.0f};

	UPROPERTY()
	bool bPickedUpFirstSword{false};
};
