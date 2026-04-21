// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProceduralSwordGenerator : ModuleRules
{
	public ProceduralSwordGenerator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"GeometryCore",
				"DynamicMesh",
				"GeometryScriptingCore",
				"GeometryFramework", 
				//"Blutility",
				"UMG",
				//"UMGEditor",
				"Slate",
				"SlateCore", 
				//"GeometryScriptingEditor",
				//"UnrealEd",
				//"EditorFramework",
				//"ToolMenus",
				//"EditorScriptingUtilities",
				//"PropertyEditor",
				//"EditorWidgets",
				//"ScriptableEditorWidgets",
				//"AssetTools",
				//"AdvancedPreviewScene"
				"Json",
				"JsonUtilities"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				
				
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
