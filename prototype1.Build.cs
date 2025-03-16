// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class prototype1 : ModuleRules
{
	public prototype1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput","InputCore", "NavigationSystem", "AIModule" });
	}
}
