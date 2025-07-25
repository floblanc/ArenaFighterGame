// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PurgatoriumLex : ModuleRules
{
	public PurgatoriumLex(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"PurgatoriumLex",
			"PurgatoriumLex/Variant_Platforming",
			"PurgatoriumLex/Variant_Combat",
			"PurgatoriumLex/Variant_Combat/AI",
			"PurgatoriumLex/Variant_SideScrolling",
			"PurgatoriumLex/Variant_SideScrolling/Gameplay",
			"PurgatoriumLex/Variant_SideScrolling/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
