
#pragma once

#include "CoreMinimal.h"
#include "PurgatoriumLexCharacterBase.h"
#include "Logging/LogMacros.h"
#include "Input/PurgatoriumLexInputConfig.h"
#include "PurgatoriumLexGameplayTags.h"
#include "GameplayTagContainer.h"
#include "Combat/CombatTypes.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UPurgatoriumLexInputComponent;
class UFighterCombatComponent;
class UAnimMontage;
struct FInputActionValue;
struct FGameplayTag;

// ============================================================================
// ENUMS
// ============================================================================
// EPosture lives in Combat/CombatTypes.h (sim + AnimBP share one definition).

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
enum class ETechType : uint8
{
    E_Standard	UMETA(DisplayName = "STANDARD"),
    E_RollForward	UMETA(DisplayName = "ROLL FORWARD"),
    E_RollBackward	UMETA(DisplayName = "ROLL BACKWARD"),
    E_RollLeft	UMETA(DisplayName = "ROLL LEFT"),
    E_RollRight	UMETA(DisplayName = "ROLL RIGHT"),
    E_Wall		UMETA(DisplayName = "WALL"),
    E_WallJump	UMETA(DisplayName = "WALL JUMP"),
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

// ============================================================================
// STRUCTS
// ============================================================================

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

// ============================================================================
// CLASS DECLARATION
// ============================================================================

UCLASS()
class PURGATORIUMLEX_API APlayerCharacter : public APurgatoriumLexCharacterBase
{
	GENERATED_BODY()

public:
	// ========================================================================
	// CONSTRUCTOR & CORE OVERRIDES
	// ========================================================================
	
	APlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ========================================================================
	// COMPONENTS
	// ========================================================================
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/**
	 * Fixed-tick combat sim bridge (posture + light attack timing).
	 * WHY a component: keeps rules out of this god-class; see Combat/CombatSim_REVIEW.md
	 * for architecture (sim authority vs GAS vs AnimBP).
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Sim")
	TObjectPtr<UFighterCombatComponent> FighterCombat;

	/**
	 * WHY this flag exists (temporary dual path):
	 *   Migration safety — compare old Tick posture vs sim without bricking PIE.
	 *   Delete legacy RequestPostureChange path once you trust the sim.
	 * When true: sim is posture authority; ActualPosture is a presentation mirror.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Sim")
	bool bUseFighterCombatSim = true;

	/**
	 * WHY separate from bUseFighterCombatSim:
	 *   Lets you keep sim posture (AnimBP) while still testing GA_Kick via GAS.
	 * When both true: LightAttack input never calls ASC for that tag.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Sim")
	bool bRouteLightAttackToCombatSim = true;

	/**
	 * WHY optional auto-play: presentation must be swappable (BP VFX, different mesh)
	 * without changing sim rules. Off = listen to OnLightAttackStarted yourself.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Sim")
	bool bAutoPlaySimAttackMontage = true;

	// ========================================================================
	// PUBLIC FUNCTIONS - INPUT HANDLERS
	// ========================================================================
	
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoMoveAroundSomething(float Right, float Forward);
	
	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump released inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	void Roll();

	// ========================================================================
	// PUBLIC FUNCTIONS - POSTURE
	// ========================================================================
	
	/** Called for Posture Action */
	void PostureActionTriggered(const FInputActionValue& Value);
	void ProcessPostureInput(const FInputActionValue& Value);
	void RequestNeutralPosture();
	void PostureActionStopped();
	void TryChangePostureByDefaultMovement(const FInputActionValue& Value);

	// ========================================================================
	// PUBLIC FUNCTIONS - STALING SYSTEMS
	// ========================================================================
	
	/** Get duration multiplier based on current roll staling penalty (1.0 = fresh, increases with penalty). */
	UFUNCTION(BlueprintPure, Category = "Roll Staling")
	float GetRollDurationMultiplier() const { return 1.0f + RollStalePenalty; }

	/** Get intangibility delay in frames for rolls (0 = fresh, 4 = fully stale). */
	UFUNCTION(BlueprintPure, Category = "Roll Staling")
	int32 GetRollIntangibilityDelay() const;

	/** Get duration multiplier based on current posture staling penalty (1.0 = fresh, increases with penalty). */
	UFUNCTION(BlueprintPure, Category = "Posture Staling")
	float GetPostureDurationMultiplier() const;

	/** Get intangibility delay in frames for posture changes (0 = fresh, 4 = fully stale). */
	UFUNCTION(BlueprintPure, Category = "Posture Staling")
	int32 GetPostureIntangibilityDelay() const;

	// ========================================================================
	// PUBLIC FUNCTIONS - BLUEPRINT EVENTS
	// ========================================================================
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_TryInitFloatingHealthBar();

protected:
	// ========================================================================
	// ENHANCED INPUT SYSTEM
	// ========================================================================
	
	/** Input Config - Maps InputActions to GameplayTags for ability binding */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	TObjectPtr<const UPurgatoriumLexInputConfig> InputConfig;

	/** MappingContext Default*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputMappingContext* DefaultMappingContext;

	/** MappingContext Fighting*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputMappingContext* FightingMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* LookAction;

	/** Run Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* RunAction;

	/** Roll Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* RollAction;

	/** Posture Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* PostureAction;

	/** LightAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* LightAttackAction;

	/** Charge Attack Input Action - Pressed = start charging, Released = execute attack or cancel (if held less than MinChargeTime) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* ChargeAttackAction;

	/** SpecialAttack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* SpecialAttackAction;

	/** Guard Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* GuardAction;

	/** BreakGuard Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* BreakGuardAction;

	/** Lock/Unlock Camera Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	class UInputAction* LockUnlockAction;

	// ========================================================================
	// ABILITY SYSTEM
	// ========================================================================
	
	/** Abilities granted with their input tag. One entry per ability (e.g. GA_Kick -> InputTag.LightAttack). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", Meta = (TitleProperty = "AbilityClass"))
	TArray<FAbilityInputMapping> AbilityInputMappings;

	/** GA Kick Ability Class - Legacy, kept for backward compatibility TODO: Remove this once the GAS implementation is complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<class UGameplayAbility> GA_Kick;

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

	/** Grants all abilities from AbilityInputMappings with their input tags. Called from PossessedBy. */
	void GrantAbilitiesWithInputTags();

	// ========================================================================
	// INPUT BUFFERING SYSTEM
	// ========================================================================
	
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

	/** Add an input to the ability buffer (when activation failed or was gated). Only bufferable tags are stored. */
	void BufferAbilityInput(FGameplayTag InputTag);

	/** True if this tag should be buffered when activation fails (e.g. LightAttack, Roll). */
	bool IsInputTagBufferable(FGameplayTag InputTag) const;

	// ========================================================================
	// MOVEMENT & CHARACTER STATE
	// ========================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float WalkingSpeed; // Default walk speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float RunningSpeed; // Default run speed
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsRunning;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void MoveActionStopped();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void StartRunning();
	
	UFUNCTION(BlueprintCallable)
	void StopRunning();

	// ========================================================================
	// POSTURE SYSTEM
	// ========================================================================
	
	/**
	 * Posture seen by AnimBP / Blueprints.
	 * WHY still on the character: zero AnimBP migration — your existing blend reads this.
	 * WHY not authority when sim is on: CombatSim_REVIEW.md ("sim writes, mesh reads").
	 * Writing this from Blueprint while bUseFighterCombatSim=true will be overwritten.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	EPosture ActualPosture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsPostureActionActive;

	/** Base delay in frames before posture change is applied (frame-based for rollback compatibility). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", Meta = (ClampMin = "0"))
	int32 PostureBaseFramesDelay = 3;

	/** Bonus delay in frames added to base delay (can be modified at runtime, e.g. from abilities/status effects). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement", Meta = (ClampMin = "0"))
	int32 PostureBonusFramesDelay = 0;

	/** Pending posture change: target posture and frame when change was requested. */
	EPosture PendingPosture = EPosture::E_Neutral;
	int32 PostureChangeRequestFrame = -1;

	void HandlePostureInputY(float Value);
	void HandlePostureInputX(float Value);

	// ========================================================================
	// ROLL STALING SYSTEM
	// ========================================================================
	
	/** Minimum penalty per roll (forward roll, ForwardVector = 1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll Staling", Meta = (ClampMin = "0.0"))
	float RollMinPenalty = 0.06f;

	/** Maximum penalty per roll (back roll, ForwardVector = -1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll Staling", Meta = (ClampMin = "0.0"))
	float RollMaxPenalty = 0.1f;

	/** Maximum accumulated penalty (caps at this value). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll Staling", Meta = (ClampMin = "0.0"))
	float RollMaxPenaltyValue = 0.5f;

	/** Current accumulated roll staling penalty (0.0 = fresh, increases with each roll). */
	UPROPERTY(BlueprintReadOnly, Category = "Roll Staling")
	float RollStalePenalty = 0.0f;

	/** Frame number when last dodge was performed (for deterministic reset calculation). */
	int32 LastDodgeFrame = -1;

	/** Number of frames without dodging before penalty resets (frame-based for rollback compatibility). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roll Staling", Meta = (ClampMin = "1"))
	int32 RollResetFrames = 60; // ~1 second at 60fps

	// ========================================================================
	// POSTURE STALING SYSTEM
	// ========================================================================
	
	/** Constant penalty per posture change. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture Staling", Meta = (ClampMin = "0.0"))
	float PosturePenalty = 0.08f;

	/** Maximum accumulated penalty (caps at this value). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture Staling", Meta = (ClampMin = "0.0"))
	float PostureMaxPenaltyValue = 0.5f;

	/** Current accumulated posture staling penalty (0.0 = fresh, increases with each posture change). */
	UPROPERTY(BlueprintReadOnly, Category = "Posture Staling")
	float PostureStalePenalty = 0.0f;

	/** Frame number when last posture change was performed (for deterministic reset calculation). */
	int32 LastPostureChangeFrame = -1;

	/** Number of frames without posture changes before penalty resets (frame-based for rollback compatibility). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture Staling", Meta = (ClampMin = "1"))
	int32 PostureResetFrames = 60; // ~1 second at 60fps

	// ========================================================================
	// COMBAT & ACTIONS
	// ========================================================================
	
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

	void LightAttack();
	void StartChargeAttack();
	void ChargeAttack();
	void SpecialAttack();

	void Guard();
	void GuardReleased();
	void BreakGuard();

	// ========================================================================
	// TECH SYSTEM (SSBU-style)
	// ========================================================================
	
	/** Tech window duration in frames (SSBU: 11 frames). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech System", Meta = (ClampMin = "1"))
	int32 TechWindowFrames = 11;

	/** Tech lockout duration in frames after inputting a tech (SSBU: 40 frames). Prevents mashing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech System", Meta = (ClampMin = "1"))
	int32 TechLockoutFrames = 40;

	/** Minimum knockback speed threshold for untechable hits (SSBU: 6.0). Higher = easier to tech. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech System", Meta = (ClampMin = "0.0"))
	float TechKnockbackThreshold = 6.0f;

	/** Frame when tech window started (frame-based for rollback compatibility). -1 = no active window.
	 *  NOTE: This is client-side prediction state. During rollback, this will be recalculated deterministically
	 *  based on contact events (Landed/NotifyHit) and input state. */
	int32 TechWindowStartFrame = -1;

	/** Frame when last tech input was pressed (for lockout calculation). -1 = no lockout.
	 *  NOTE: Frame-based for rollback compatibility. Lockout is recalculated deterministically during rollback. */
	int32 LastTechInputFrame = -1;

	/** Whether shield/guard button is currently held (for ground tech hold input).
	 *  NOTE: This is prediction state. During rollback, input state is replayed deterministically. */
	bool bIsTechInputHeld = false;

	/** Whether jump input is currently held (for wall tech jump detection).
	 *  NOTE: This is prediction state. During rollback, input state is replayed deterministically. */
	bool bIsJumpInputHeld = false;

	/** Whether character is in a techable state (tumbling/reeling with hitstun).
	 *  NOTE: Derived from GameplayTags state (State.Hitstun, State.Tech) which are part of rollback state. */
	bool bIsTechable = false;

	/** Last detected wall hit normal (for wall tech direction).
	 *  NOTE: Set deterministically from NotifyHit callback during contact. */
	FVector LastWallHitNormal = FVector::ZeroVector;

	/** Check if character can tech (in techable state and not in lockout). */
	UFUNCTION(BlueprintPure, Category = "Tech System")
	bool CanTech() const;

	/** Check if currently in tech window. */
	UFUNCTION(BlueprintPure, Category = "Tech System")
	bool IsInTechWindow() const;

	/** Handle tech input (shield/guard button pressed). */
	void OnTechInputPressed();

	/** Handle tech input released (for hold input detection). */
	void OnTechInputReleased();

	/** Perform tech based on current situation (ground/wall) and input direction. */
	void PerformTech(ETechType TechType);

	/** Check if character should enter techable state (tumbling/reeling). */
	void UpdateTechableState();

	/** Detect ground contact and attempt tech if input was buffered. */
	virtual void Landed(const FHitResult& Hit) override;

	/** Detect wall contact and attempt tech if input was buffered. Called from NotifyHit. */
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	// ========================================================================
	// CAMERA & LOCK-ON SYSTEM
	// ========================================================================
	
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

	void LockUnlockCameraOnEnemy();

	/** Update camera lock-on logic - called deterministically for rollback compatibility */
	void UpdateCameraLockOn();

	/** Fills lockOnCandidates with enemy players in range. Call before picking a lock-on target. */
	void RefreshLockOnCandidates();

	// ========================================================================
	// HEALTH & CHARACTER INFO
	// ========================================================================
	
	/** The amount of health the player currently has */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float playerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
	int TeamId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
	int PlayerNumber;

	// ========================================================================
	// PROTECTED HELPER FUNCTIONS
	// ========================================================================
	
	/** Setup legacy input bindings (used when InputConfig is not set) TODO: Remove this once the GAS implementation is complete */
	void SetupLegacyInputBindings(UEnhancedInputComponent* EnhancedInputComponent);

private:
	// ========================================================================
	// PRIVATE INITIALIZATION
	// ========================================================================
	
	void InitAbilitySystemComponent();
	void InitHUD() const;

	// ========================================================================
	// PRIVATE POSTURE HELPERS
	// ========================================================================
	
	/** Request a posture change with frame delay. Returns true if change was queued, false if one is already pending. */
	bool RequestPostureChange(EPosture TargetPosture);
	
	/** Process pending posture change in Tick - applies change when delay elapses. */
	void ProcessPendingPostureChange();

	/** Sync ActualPosture / attack bools from FighterCombat when sim is enabled. */
	void SyncPresentationFromCombatSim();

	UFUNCTION()
	void HandleSimLightAttackStarted(EPosture SnapshotPosture, UAnimMontage* Montage, int32 SimFrame);

	// ========================================================================
	// PRIVATE CAMERA HELPERS
	// ========================================================================
	
	void LockCameraOnCharacterBack();
	
	UFUNCTION(BlueprintCallable)
	void UnlockCharacterBackFromCamera();

	// ========================================================================
	// PRIVATE COMBAT HELPERS
	// ========================================================================
	
	/** Calculate penalty increment based on roll direction (ForwardVector: 1 = forward, -1 = backward). */
	float CalculateRollPenaltyIncrement(float ForwardVectorDot) const;

	// ========================================================================
	// PRIVATE TECH HELPERS
	// ========================================================================
	
	/** Start tech window (called when entering techable state or detecting contact). */
	void StartTechWindow();

	/** End tech window (called when tech is performed or window expires). */
	void EndTechWindow();

	/** Check if character is about to hit ground (for predictive tech window). */
	bool WillHitGroundSoon() const;

	/** Check if character is about to hit wall (for predictive tech window). */
	bool WillHitWallSoon() const;

	/** Determine tech type based on input direction and contact type. */
	ETechType DetermineTechType(bool bIsWallContact, float InputForward, float InputRight) const;

	// ========================================================================
	// PRIVATE UTILITY FUNCTIONS
	// ========================================================================
	
	bool IsEnemy(int id);
	bool IsEnemy(APlayerCharacter *fighter);

	int  GetTeamId();
	void SetTeamId(int teamId);

	// ========================================================================
	// PRIVATE MEMBER VARIABLES
	// ========================================================================
	
	/** Handles for ability input bindings - used to clean up bindings */
	TArray<uint32> AbilityInputBindHandles;
};
