

#pragma once

#include "CoreMinimal.h"
#include "PurgatoriumLexCharacterBase.h"
#include "Logging/LogMacros.h"
#include "Input/PurgatoriumLexInputConfig.h"
#include "PurgatoriumLexGameplayTags.h"
#include "GameplayTagContainer.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UPurgatoriumLexInputComponent;
struct FInputActionValue;
struct FGameplayTag;

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    E_Idle		UMETA(DisplayName = "IDLE"),
    E_Walking	UMETA(DisplayName = "WALKING"),
    E_Running	UMETA(DisplayName = "RUNNING"),
    E_Rolling	UMETA(DisplayName = "ROLLING"),
    E_Hitting	UMETA(DisplayName = "HITTING"),
    E_Other		UMETA(DisplayName = "OTHER"),
};

UENUM(BlueprintType)
enum class EPosture : uint8
{
    E_Neutral	UMETA(DisplayName = "NEUTRAL"),
    E_Up		UMETA(DisplayName = "UP"),
    E_Down		UMETA(DisplayName = "DOWN"),
    E_Left		UMETA(DisplayName = "LEFT"),
    E_Right		UMETA(DisplayName = "RIGHT"),
    // E_UpLeft	UMETA(DisplayName = "UPLEFT"),
    // E_UpRight	UMETA(DisplayName = "UPRIGHT"),
    E_DownLeft	UMETA(DisplayName = "DOWNLEFT"),
    E_DownRight	UMETA(DisplayName = "DOWNRIGHT"),
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

struct FGameplayTag;

/** Maps a Gameplay Ability to an Input Tag. Used to grant abilities and bind them to input in one place. */
USTRUCT(BlueprintType)
struct FAbilityInputMapping
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<class UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/** Single entry in the ability input buffer. Frame-based for client-side feel improvement.
 *  NOTE: Client-side only - not replicated. Server only sees successful activations (handled by GAS). */
USTRUCT(BlueprintType)
struct FBufferedAbilityInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Buffer")
	FGameplayTag InputTag;

	/** Frame number when this input was buffered (for deterministic expiry calculation). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Buffer")
	int32 BufferedFrame = 0;

	FBufferedAbilityInput() = default;
	FBufferedAbilityInput(const FGameplayTag& Tag, int32 Frame) : InputTag(Tag), BufferedFrame(Frame) {}

	/** Calculate frames remaining based on current frame and buffer duration. */
	int32 GetFramesRemaining(int32 CurrentFrame, int32 BufferFrames) const
	{
		return FMath::Max(0, BufferFrames - (CurrentFrame - BufferedFrame));
	}
};

UCLASS()
class PURGATORIUMLEX_API APlayerCharacter : public APurgatoriumLexCharacterBase
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	protected:

	//////////////////////////////////////////////////////////

	/** Input Config - Maps InputActions to GameplayTags for ability binding */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<const UPurgatoriumLexInputConfig> InputConfig;

	/** MappingContext Default*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputMappingContext* DefaultMappingContext;

	/** MappingContext Fighting*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputMappingContext* FightingMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* LookAction;

	/** Run Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* RunAction;

	/** Roll Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* RollAction;

	/** Posture Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* PostureAction;

	/** LightAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* LightAttackAction;

	/** Abilities granted with their input tag. One entry per ability (e.g. GA_Kick -> InputTag.LightAttack). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", Meta = (TitleProperty = "AbilityClass"))
	TArray<FAbilityInputMapping> AbilityInputMappings;

	/** GA Kick Ability Class - Legacy, kept for backward compatibility TODO: Remove this once the GAS implementation is complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<class UGameplayAbility> GA_Kick;

	/** Charge Attack Input Action - Pressed = start charging, Released = execute attack or cancel (if held less than MinChargeTime) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* ChargeAttackAction;

	/** SpecialAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* SpecialAttackAction;

	/** Guard Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* GuardAction;

	/** BreakGuard Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* BreakGuardAction;

	/** Lock/Unlock Camera Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	class UInputAction* LockUnlockAction;

	///** Heal Camera Input Action */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	//class UInputAction* HealAction;

	///** TakeDamages Input Action */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input" )
	//class UInputAction* TakeDamagesAction;
	
public:
	// Sets default values for this character's properties
	APlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_TryInitFloatingHealthBar();

private:
	void InitAbilitySystemComponent();
	void InitHUD() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void MoveActionStopped();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void HandlePostureInputY(float Value);
	void HandlePostureInputX(float Value);

	void LightAttack();
	void StartChargeAttack();
	void ChargeAttack();
	void SpecialAttack();

	void Guard();
	void BreakGuard();

	void StartRunning();
	
	UFUNCTION(BlueprintCallable)
	void StopRunning();

	//void StartGuarding();
	//void StopGuarding();

	void LockUnlockCameraOnEnemy();

	/** Update camera lock-on logic - called deterministically for rollback compatibility */
	void UpdateCameraLockOn();

	/** Fills lockOnCandidates with enemy players in range. Call before picking a lock-on target. */
	void RefreshLockOnCandidates();

	/** Grants all abilities from AbilityInputMappings with their input tags. Called from PossessedBy. */
	void GrantAbilitiesWithInputTags();

	/** Single gate for all ability input: return false to block forwarding this tag to the ASC. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Abilities")
	bool CanActivateAbilityForInputTag(FGameplayTag InputTag) const;
	virtual bool CanActivateAbilityForInputTag_Implementation(FGameplayTag InputTag) const;

	/** Conditions for LightAttack (grounded, not attacking, not charging). Override in Blueprint to extend. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Abilities")
	bool CanPerformLightAttack() const;
	virtual bool CanPerformLightAttack_Implementation() const;

	/** Handle ability input tag pressed - called by InputComponent when ability input is triggered */
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);

	/** Handle ability input tag released - called by InputComponent when ability input is released */
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	/** Add an input to the ability buffer (when activation failed or was gated). Only bufferable tags are stored. */
	void BufferAbilityInput(FGameplayTag InputTag);

	/** True if this tag should be buffered when activation fails (e.g. LightAttack, Roll). */
	bool IsInputTagBufferable(FGameplayTag InputTag) const;

	/** Setup legacy input bindings (used when InputConfig is not set) TODO: Remove this once the GAS implementation is complete */
	void SetupLegacyInputBindings(UEnhancedInputComponent* EnhancedInputComponent);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoMoveAroundSomething(float Right, float Forward);

	/** Progressive movement around locked enemy - blends circling and forward/back movement proportionally */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoMoveAroundSomethingProgressive(float Right, float Forward);

	/** UE5 Assistant approach - direct tangent/radial movement with smooth rotation toward enemy */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoMoveAroundSomethingUE5Assistant(float Right, float Forward);
	
	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	void Roll();

	// Called for Posture Action
	void PostureActionTriggered(const FInputActionValue& Value);
	void ChangePosture(const FInputActionValue& Value);
	void SetPostureToNeutral();
	void PostureActionStopped();
	void TryChangePostureByDefaultMovement(const FInputActionValue& Value);

	void LockCameraOnCharacterBack();
	
	UFUNCTION(BlueprintCallable)
	void UnlockCharacterBackFromCamera();

	bool IsEnemy(int id);
	bool IsEnemy(APlayerCharacter *fighter);

	int  GetTeamId();
	void SetTeamId(int teamId);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
	int TeamId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
	int PlayerNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float WalkingSpeed; // Default walk speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float RunningSpeed; // Default run speed
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsMoving; //

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsRunning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Actions")
	bool bAttackHasBeenUsed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Actions")
	bool bIsInAttackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Actions")
	bool bIsGuarding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Actions")
	bool bIsCharging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Hitboxs")
	bool bIsRigthPunchHitboxActive;

	/** Timer: auto-release charged attack after max hold time */
	FTimerHandle inputHeldTimer;

	/** Max time the charge input can be held before auto-releasing the attack */
	float maxInputHoldTime;

	/** Time when charge started (for cancel vs commit on release) */
	float ChargeAttackStartTime;

	/** Minimum hold time to commit attack; release before this = cancel charge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Actions", Meta = (ClampMin = "0.0"))
	float MinChargeTime;

	/** Input buffer: how many frames a failed ability input is kept before expiring (frame-based for rollback). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Buffer", Meta = (ClampMin = "1"))
	int32 AbilityInputBufferFrames = 6;

	/** Input tags that are buffered when activation fails (e.g. LightAttack, Roll). Add tags here to enable buffering. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Buffer", Meta = (Categories = "InputTag"))
	TArray<FGameplayTag> BufferableInputTags;

	/** Pending ability inputs to try activating each Tick until they expire or succeed.
	 *  CLIENT-SIDE ONLY: This is purely for feel improvement (responsive input when gates block activation).
	 *  Not replicated - server only sees successful activations (handled by GAS replication).
	 *  Rollback compatibility: When inputs are replayed, buffer is recreated deterministically. */
	UPROPERTY(BlueprintReadOnly, Category = "Input Buffer")
	TArray<FBufferedAbilityInput> AbilityInputBuffer;

	/** Current simulation frame counter (increments each Tick). Used for buffer expiry calculation. */
	int32 SimulationFrame = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	EPosture ActualPosture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsPostureActionActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	bool bIsCameraLockedOnEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	bool bIsCameraLockedOnCharacterBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	TArray<AActor*> lockOnCandidates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	AActor* lockedOnActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	float targetingHeighOffset;

	/** Lock-on: max distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", Meta = (ClampMin = "100", ClampMax = "5000"))
	float LockOnMaxDistance = 2000.f;

	/** Lock-on: half-angle in degrees from camera view (unused when using screen projection). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", Meta = (ClampMin = "5", ClampMax = "90"))
	float LockOnFOVDegrees = 45.f;

	//the amount of health the player currently has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float playerHealth;

	/** Handles for ability input bindings - used to clean up bindings */
	TArray<uint32> AbilityInputBindHandles;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
