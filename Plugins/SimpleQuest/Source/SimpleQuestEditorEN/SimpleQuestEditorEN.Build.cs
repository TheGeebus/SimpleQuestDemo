using UnrealBuildTool;
using System.IO;

public class SimpleQuestEditorEN : ModuleRules
{
    public SimpleQuestEditorEN(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Slate", "SlateCore",
            "UnrealEd", "GraphEditor", "BlueprintGraph",
            "SimpleQuestEditor"
        });

        string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

        if (FindElectronicNodes(EngineDir, ModuleDirectory, out string ENSourceDir))
        {
            PrivateDependencyModuleNames.Add("ElectronicNodes");
            PrivateIncludePaths.Add(Path.Combine(ENSourceDir, "Private"));
            PrivateDefinitions.Add("WITH_ELECTRONIC_NODES=1");
        }
    }

    static bool FindElectronicNodes(string EngineDir, string ModuleDir, out string ENSourceDir)
    {
        ENSourceDir = "";

        // ModuleDir = {ProjectRoot}/Plugins/SimpleQuest/Source/SimpleQuestEditorEN
        // Three levels up = {ProjectRoot}/Plugins
        string ProjectPluginsDir = Path.GetFullPath(Path.Combine(ModuleDir, "../../.."));

        string[] Roots =
        {
            Path.Combine(EngineDir, "Plugins", "Marketplace"),
            Path.Combine(EngineDir, "Plugins"),
            ProjectPluginsDir
        };

        foreach (string Root in Roots)
        {
            if (!Directory.Exists(Root)) continue;

            foreach (string Dir in Directory.GetDirectories(Root))
            {
                string Candidate = Path.Combine(Dir, "Source", "ElectronicNodes", "ElectronicNodes.Build.cs");
                if (File.Exists(Candidate))
                {
                    ENSourceDir = Path.Combine(Dir, "Source", "ElectronicNodes");
                    return true;
                }
            }
        }

        return false;
    }
}
