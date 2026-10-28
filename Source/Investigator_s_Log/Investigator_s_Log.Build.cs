// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Investigator_s_Log : ModuleRules
{
	public Investigator_s_Log(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// 公有依赖：凡是 Public/ 下头文件暴露类型所在的模块，必须放在这里
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Slate", "SlateCore" });

		// 私有依赖：只在 Private/*.cpp 中使用的模块
		PrivateDependencyModuleNames.AddRange(new string[] { "Json", "JsonUtilities", "DeveloperSettings", "ApplicationCore" });

		// CommonUI 暂不启用：项目尚无 CommonUI 资产，加入会显著拖慢编译，待确有资产后再加
	}
}
