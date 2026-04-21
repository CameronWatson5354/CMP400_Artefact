// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcSwordHolder.h"

#include "Components/CapsuleComponent.h"
#include "ProcSwordData.h"
#include "ProcPlayerCharacter.h"

// Sets default values
AProcSwordHolder::AProcSwordHolder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComp"));
	SetRootComponent(DefaultRootComp);

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetupAttachment(GetRootComponent());

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(CapsuleComp);

}

// Called when the game starts or when spawned
void AProcSwordHolder::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProcSwordHolder::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AProcSwordHolder::BeginOverlap);
}

#if WITH_EDITOR
void AProcSwordHolder::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (IsValid(SwordData))
	{
		StaticMeshComp->SetStaticMesh(SwordData->GetSwordMesh());
	}
	else
	{
		StaticMeshComp->SetStaticMesh(nullptr);
	}
}
#endif

// Called every frame
void AProcSwordHolder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProcSwordHolder::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(OtherActor) && OtherActor->ActorHasTag(FName{"Player"}))
	{
		AProcPlayerCharacter* Player{Cast<AProcPlayerCharacter>(OtherActor)};
		if (IsValid(Player))
		{
			IProcSwordInterface::Execute_PickupSword(Player, SwordData);
		}
	}
}

