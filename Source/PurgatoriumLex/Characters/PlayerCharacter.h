
#pragma once

#include "CoreMinimal.h"
#include "PurgatoriumLexCharacterBase.h"
#include "Logging/LogMacros.h"
#include "Input/PurgatoriumLexInputConfig.h"
#include "PurgatoriumLexGameplayTags.h"
#include "GameplayTagContainer.h"
#include "Combat/CombatTypes.h"
#include "Characters/PlayerCharacterTypes.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UPurgatoriumLexInputComponent;
class UFighterCombatComponent;
class ULockOnCameraComponent;
class UAnimMontage;
class UInputMappingContext;
class UEnhancedInputComponent;
struct FInputActionValue;
struct FGameplayTag;

// EPosture → Combat/CombatTypes.h
// EMovementState / ETechType / ability buffer structs → Characters/PlayerCharacterTypes.h

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

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
	 * Fixed-tick combat sim (posture + light attack). Single combat authority for netcode basics.
	 * Tune delays / move set on this component — not duplicated on the character.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Sim")
	TObjectPtr<UFighterCombatComponent> FighterCombat;

	/** If true, play montage from LightAttackMoveSet when sim starts an attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Sim")
	bool bAutoPlaySimAttackMontage = true;

	/**
	 * Lock-on / camera-on-back (presentation). Not combat authority — see LockOnCameraComponent.
	 * Boom + FollowCamera stay on this pawn for attachment; lock rules live on the component.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<ULockOnCameraComponent> LockOnCamera;

	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsCameraLockedOnEnemy() const;

	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsCameraLockedOnCharacterBack() const;

	UFUNCTION(BlueprintPure, Category = "Camera")
	AActor* GetLockedOnActor() const;

	UFUNCTION(BlueprintPure, Category = "Character Info")
	int32 GetTeamId() const { return TeamId; }

	UFUNCTION(BlueprintCallable, Category = "Character Info")
	void SetTeamId(int32 InTeamId) { TeamId = InTeamId; }

	UFUNCTION(BlueprintPure, Category = "Character Info")
	bool IsEnemyPlayer(const APlayerCharacter* Other) const;

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
	 * Presentation mirror of sim posture for AnimBP (read-only in practice).
	 * Authority: FighterCombat sim. Do not write this from Blueprint — it is overwritten each tick.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Sim|Presentation")
	EPosture ActualPosture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	bool bIsPostureActionActive;

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
	// HEALTH & CHARACTER INFO
	// ========================================================================
	
	/** Deprecated: use AttributeSet Health. Kept so old BPs do not hard-fail on missing property. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (DeprecatedProperty, DeprecationMessage = "Use AttributeSet Health / HUD bindings instead of playerHealth."))
	float playerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
	int32 TeamId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
	int32 PlayerNumber = 0;

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
	
	/** Forward posture request to the combat sim (presentation still reads ActualPosture). */
	bool RequestPostureChange(EPosture TargetPosture);

	/** Sync ActualPosture / attack bools from FighterCombat. */
	void SyncPresentationFromCombatSim();

	UFUNCTION()
	void HandleSimLightAttackStarted(EPosture SnapshotPosture, UAnimMontage* Montage, int32 SimFrame);

	UFUNCTION()
	void HandleLockOnEnemyChanged(bool bLockedOnEnemy, AActor* LockedActor);

	UFUNCTION()
	void HandleLockOnBackChanged(bool bLockedOnBack);

	void LockUnlockCameraOnEnemy();

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
	// PRIVATE MEMBER VARIABLES
	// ========================================================================
	
	/** Handles for ability input bindings - used to clean up bindings */
	TArray<uint32> AbilityInputBindHandles;
};
