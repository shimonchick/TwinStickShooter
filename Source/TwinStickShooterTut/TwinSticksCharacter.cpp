// Fill out your copyright notice in the Description page of Project Settings.


#include "TwinSticksCharacter.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"

#include "Gun.h"
#include "HealthComponent.h"
#include "TwinStickGameMode.h"

// Sets default values
ATwinSticksCharacter::ATwinSticksCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DeathSoundComponent = CreateDefaultSubobject<UAudioComponent>("DeathSoundAudioComponent");
	DeathSoundComponent->bAutoActivate = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComponent");
	HealthComponent->OnDeath.AddDynamic(this, &ATwinSticksCharacter::Die);
}

// Called when the game starts or when spawned
void ATwinSticksCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnStartingGun();
}


void ATwinSticksCharacter::SpawnStartingGun() {
	if (IsValid(StartingGunTemplate) == false) {
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false) {
		UE_LOG(LogTemp, Error, TEXT("ATwinSticksCharacter::SpawnStartingGun() IsValid(World) == false"));
		return;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = Instigator;

	Gun = World->SpawnActor<AGun>(StartingGunTemplate, SpawnInfo);
	AttachGun(Gun);
}

// Called to bind functionality to input
void ATwinSticksCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATwinSticksCharacter::StartFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATwinSticksCharacter::StopFiring);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATwinSticksCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATwinSticksCharacter::MoveRight);
}


void ATwinSticksCharacter::StartFiring() {
	if (IsValid(Gun) == false) {
		UE_LOG(LogTemp, Warning, TEXT("ATwinSticksCharacter::StartFiring() IsValid(Gun) == false"));
		return;
	}

	Gun->PullTrigger();
}

void ATwinSticksCharacter::StopFiring() {
	if (IsValid(Gun) == false) {
		UE_LOG(LogTemp, Warning, TEXT("ATwinSticksCharacter::StopFiring IsValid(Gun) == false"));
		return;
	}

	Gun->ReleaseTrigger();
}


void ATwinSticksCharacter::MoveForward(float Value) {
	AddMovementInput(FVector(1, 0, 0), Value);
}


void ATwinSticksCharacter::MoveRight(float Value) {
	AddMovementInput(FVector(0, 1, 0), Value);
}


void ATwinSticksCharacter::AttachGun(AGun* NewGun) {
	if (IsValid(NewGun) == false) {
		UE_LOG(LogTemp, Error, TEXT(" ATwinSticksCharacter::SpawnGun IsValid(NewGun) == false"));
		return;
	}

	if (IsValid(CharacterMesh) == false) {
		UE_LOG(LogTemp, Error, TEXT(" ATwinSticksCharacter::SpawnGun IsValid(CharacterMesh) == false"));
		return;
	}

	if (CharacterMesh->DoesSocketExist("GunSocket") == false) {
		UE_LOG(LogTemp, Error, TEXT(" ATwinSticksCharacter::SpawnGun CharacterMesh->DoesSocketExist('GunSocket') == false"));
		return;
	}

	Gun = NewGun;
	Gun->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GunSocket");
}


void ATwinSticksCharacter::Die() {
	UWorld* World = GetWorld();
	if (IsValid(World) == false) {
		UE_LOG(LogTemp, Error, TEXT("ATwinSticksCharacter::Die() IsValid(World) == false"));
		return;
	}

	if (IsValid(DeathSoundComponent) == false) {
		UE_LOG(LogTemp, Error, TEXT("ATwinSticksCharacter::Die() IsValid(DeathSoundComponent) == false"));
		return;
	}
	DeathSoundComponent->Play();

	World->GetTimerManager().SetTimer(
		DeathTimerHandle,
		this,
		&ATwinSticksCharacter::OnDeathTimerEnd,
		DeathAnimationTime,
		true
	);

	StopFiring();

	// FIXME: Sloppy workaround because animation requires it.
	bDead = true;
}


void ATwinSticksCharacter::OnDeathTimerEnd() {
	if (IsValid(Gun)) {
		Gun->Destroy();
	}
	Destroy();
}


void ATwinSticksCharacter::LookInDirection(FVector Direction) {
	FRotator Rotation = UKismetMathLibrary::MakeRotFromX(Direction);
	GetController()->SetControlRotation(Rotation);
}


