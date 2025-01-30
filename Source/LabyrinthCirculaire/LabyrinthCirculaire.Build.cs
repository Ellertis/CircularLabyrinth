// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LabyrinthCirculaire : ModuleRules
{
	public LabyrinthCirculaire(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
