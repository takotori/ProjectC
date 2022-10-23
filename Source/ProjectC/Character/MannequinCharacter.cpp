#include "MannequinCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectC/Combat/CombatComponent.h"
#include "ProjectC/Weapon/Weapon.h"

// Sets default values
AMannequinCharacter::AMannequinCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetMesh(), "head");
	FollowCamera->SetRelativeLocation(FVector(10, 20, 0));
	FollowCamera->SetRelativeRotation(FRotator(0, 90, -90));
	FollowCamera->bUsePawnControlRotation = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// Weapon = CreateDefaultSubobject<AWeapon>(TEXT("Weapon"));
	// Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "Head");

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
	Combat->EquipWeapon();

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void AMannequinCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void AMannequinCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMannequinCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMannequinCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMannequinCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMannequinCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMannequinCharacter::LookUp);
}

void AMannequinCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AMannequinCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMannequinCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMannequinCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMannequinCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMannequinCharacter::CrouchButtonPressed()
{
	bIsCrouched ? UnCrouch() : Crouch();
}

void AMannequinCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
