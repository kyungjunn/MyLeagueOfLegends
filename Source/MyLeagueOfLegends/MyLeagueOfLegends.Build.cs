// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MyLeagueOfLegends : ModuleRules
{
	public MyLeagueOfLegends(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara", "UMG"
		, "HTTP", "Json", "JsonUtilities", "OnlineSubsystem"	, "OnlineSubsystemUtils"
		, "Sockets", "Networking"});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// �� ����� ��� ���� ���� ��θ� ��Ŭ��� ��ο� ����
		PublicIncludePaths.Add(ModuleDirectory);

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
