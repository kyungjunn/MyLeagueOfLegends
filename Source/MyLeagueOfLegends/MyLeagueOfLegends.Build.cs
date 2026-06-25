// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MyLeagueOfLegends : ModuleRules
{
	public MyLeagueOfLegends(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara", "UMG"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// 내 모듈의 모든 하위 폴더 경로를 인클루드 경로에 포함
		//PublicIncludePaths.Add(ModuleDirectory);

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
