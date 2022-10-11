#define Version Trim(FileRead(FileOpen("..\VERSION")))
#define Year GetDateTimeString("yyyy","","")

[Setup]
AppName=BassicManager
OutputBaseFilename=BassicManager-{#Version}-Windows
AppCopyright=Copyright (C) {#Year} Melatonin
AppPublisher=Melatonin
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3"
DisableStartupPrompt=yes

[Files]
Source: "{src}..\Builds\BassicManager_artefacts\Release\VST3\BassicManager.vst3\*.*"; DestDir: "{commoncf64}\VST3\BassicManager.vst3\"; Check: Is64BitInstallMode; Flags: external overwritereadonly ignoreversion; Attribs: hidden system;
