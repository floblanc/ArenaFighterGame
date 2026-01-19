# Quick Code Comparison: Lyra vs PurgatoriumLex

Side-by-side code examples showing key differences.

## 1. Input Binding Location

### Lyra (Component-Based)
```cpp
// In ULyraHeroComponent::InitializePlayerInput()
void ULyraHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
    // Gets InputConfig from various sources
    const ULyraInputConfig* InputConfig = ...;
    
    // Gets InputComponent through component lookup
    ULyraInputComponent* LyraIC = Pawn->FindComponentByClass<ULyraInputComponent>();
    
    // Binds through component
    LyraIC->BindAbilityActions(InputConfig, this, 
        &ThisClass::Input_AbilityInputTagPressed, 
        &ThisClass::Input_AbilityInputTagReleased, 
        BindHandles);
}
```

### PurgatoriumLex (Direct)
```cpp
// In APlayerCharacter::SetupPlayerInputComponent()
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Direct cast
    UPurgatoriumLexInputComponent* InputComp = 
        Cast<UPurgatoriumLexInputComponent>(PlayerInputComponent);
    
    // Direct binding
    if (InputConfig)
    {
        InputComp->BindAbilityActions(InputConfig, this,
            &ThisClass::Input_AbilityInputTagPressed,
            &ThisClass::Input_AbilityInputTagReleased,
            AbilityInputBindHandles);
    }
}
```

**Difference**: Lyra uses component system, PurgatoriumLex is direct.

---

## 2. Ability Input Tag Pressed Handler

### Lyra (Through Components)
```cpp
// In ULyraHeroComponent
void ULyraHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
    const APawn* Pawn = GetPawn<APawn>();
    
    // Gets ASC through component system
    if (const ULyraPawnExtensionComponent* PawnExtComp = 
        ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
    {
        if (ULyraAbilitySystemComponent* LyraASC = 
            PawnExtComp->GetLyraAbilitySystemComponent())
        {
            LyraASC->AbilityInputTagPressed(InputTag);
        }
    }
}
```

### PurgatoriumLex (Direct Access)
```cpp
// In APlayerCharacter
void APlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
    // Direct cast from base class method
    if (UPurgatoriumLexAbilitySystemComponent* ASC = 
        Cast<UPurgatoriumLexAbilitySystemComponent>(GetAbilitySystemComponent()))
    {
        ASC->AbilityInputTagPressed(InputTag);
    }
}
```

**Difference**: Lyra uses component lookup, PurgatoriumLex uses direct cast.

---

## 3. ProcessAbilityInput - Activation Logic

### Lyra (Custom Activation Policies)
```cpp
// In ULyraAbilitySystemComponent::ProcessAbilityInput()
for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
{
    if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
    {
        AbilitySpec->InputPressed = true;
        
        if (AbilitySpec->IsActive())
        {
            AbilitySpecInputPressed(*AbilitySpec);
        }
        else
        {
            // Uses custom Lyra ability class with activation policies
            const ULyraGameplayAbility* LyraAbilityCDO = 
                Cast<ULyraGameplayAbility>(AbilitySpec->Ability);
            
            // Checks custom activation policy
            if (LyraAbilityCDO && 
                LyraAbilityCDO->GetActivationPolicy() == 
                ELyraAbilityActivationPolicy::OnInputTriggered)
            {
                AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
            }
        }
    }
}
```

### PurgatoriumLex (Standard GAS)
```cpp
// In UPurgatoriumLexAbilitySystemComponent::ProcessAbilityInput()
for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
{
    if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
    {
        AbilitySpec->InputPressed = true;
        
        if (AbilitySpec->IsActive())
        {
            AbilitySpecInputPressed(*AbilitySpec);
        }
        else
        {
            // Uses standard GAS CanActivateAbility check
            const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
            if (AbilitySpec->Ability->CanActivateAbility(
                AbilitySpec->Handle, ActorInfo))
            {
                AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
            }
        }
    }
}
```

**Difference**: 
- Lyra: Custom `ULyraGameplayAbility` with `ELyraAbilityActivationPolicy`
- PurgatoriumLex: Standard `UGameplayAbility` with `CanActivateAbility()`

---

## 4. InputConfig Structure

### Both Are Identical!

```cpp
// Lyra
USTRUCT(BlueprintType)
struct FLyraInputAction
{
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UInputAction> InputAction = nullptr;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};

// PurgatoriumLex
USTRUCT(BlueprintType)
struct FPurgatoriumLexInputAction
{
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UInputAction> InputAction = nullptr;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};
```

**Difference**: Only the name! Structure is identical.

---

## 5. InputComponent Binding Template

### Both Are Identical!

```cpp
// Lyra
template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void ULyraInputComponent::BindAbilityActions(
    const ULyraInputConfig* InputConfig, 
    UserClass* Object, 
    PressedFuncType PressedFunc, 
    ReleasedFuncType ReleasedFunc, 
    TArray<uint32>& BindHandles)
{
    check(InputConfig);
    
    for (const FLyraInputAction& Action : InputConfig->AbilityInputActions)
    {
        if (Action.InputAction && Action.InputTag.IsValid())
        {
            if (PressedFunc)
            {
                BindHandles.Add(BindAction(Action.InputAction, 
                    ETriggerEvent::Triggered, Object, PressedFunc, 
                    Action.InputTag).GetHandle());
            }
            
            if (ReleasedFunc)
            {
                BindHandles.Add(BindAction(Action.InputAction, 
                    ETriggerEvent::Completed, Object, ReleasedFunc, 
                    Action.InputTag).GetHandle());
            }
        }
    }
}

// PurgatoriumLex - EXACT SAME CODE, just different class name!
template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UPurgatoriumLexInputComponent::BindAbilityActions(
    const UPurgatoriumLexInputConfig* InputConfig, 
    UserClass* Object, 
    PressedFuncType PressedFunc, 
    ReleasedFuncType ReleasedFunc, 
    TArray<uint32>& BindHandles)
{
    // Identical implementation
}
```

**Difference**: None! Identical code.

---

## 6. AbilityInputTagPressed Implementation

### Both Are Identical!

```cpp
// Lyra
void ULyraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (InputTag.IsValid())
    {
        for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
        {
            if (AbilitySpec.Ability && 
                (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
            {
                InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
                InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
            }
        }
    }
}

// PurgatoriumLex - EXACT SAME CODE!
void UPurgatoriumLexAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    // Identical implementation
}
```

**Difference**: None! Identical code.

---

## Summary Table

| Component | Lyra | PurgatoriumLex | Difference Level |
|-----------|------|---------------|------------------|
| **InputConfig** | ✅ | ✅ | Identical |
| **InputComponent** | ✅ | ✅ | Identical |
| **BindAbilityActions** | ✅ | ✅ | Identical |
| **AbilityInputTagPressed** | ✅ | ✅ | Identical |
| **ProcessAbilityInput** | ✅ Custom | ✅ Standard | **Different** |
| **Integration Point** | ✅ Component | ✅ Direct | **Different** |
| **ASC Access** | ✅ Component | ✅ Direct | **Different** |
| **Activation Policy** | ✅ Custom | ✅ Standard | **Different** |

---

## Key Takeaway

**The core GAS + Input ID system is IDENTICAL to Lyra!**

The differences are:
1. **Where it's integrated** (component system vs direct)
2. **How abilities activate** (custom policies vs standard GAS)
3. **How you access the ASC** (component lookup vs direct cast)

**The Input ID binding mechanism itself is 100% the same!**

