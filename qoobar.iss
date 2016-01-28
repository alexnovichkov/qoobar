﻿; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#pragma option -v+
#pragma verboselevel 9

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)

#define VERSION "1.7.0"

AppId={{59A39891-F88A-49E6-A738-AC5580273280}
AppName=Qoobar
AppCopyright=Copyright (C) 2009-2016 Alex Novichkov
AppVersion={#VERSION}
AppVerName=Qoobar {#VERSION}
AppPublisher=Alex Novichkov
AppPublisherURL=http://qoobar.sourceforge.net
AppSupportURL=http://qoobar.sourceforge.net
AppUpdatesURL=http://qoobar.sourceforge.net
AppMutex=QoobarMutex
DefaultDirName={pf}\Qoobar\
DefaultGroupName=Qoobar
AllowNoIcons=yes
LicenseFile=LICENSE.rtf
InfoBeforeFile=README
OutputBaseFilename=qoobar-{#VERSION}
Compression=lzma2/ultra
SolidCompression=yes
AlwaysShowComponentsList=no

UninstallDisplayIcon={app}\qoobar.exe

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"; InfoBeforeFile: "README"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"; InfoBeforeFile: "README.ru"
Name: "nl"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"

[CustomMessages]
en.Main=Main Program Files
en.Help=Help Files
en.RG=Additional utilities (recommended)
en.Plugins=Additional tags processing plugins

ru.Main=Основные файлы программы
ru.Help=Файлы справки
ru.RG=Дополнительные утилиты
ru.Plugins=Дополнительные плагины обработки тегов

nl.Main=Belangrijkste bestanden
nl.Help=Helpbestanden
nl.RG=Aanvullende programma's (aanbevolen)
nl.Plugins=Extra labels verwerken plugins

pl.Main=Główne pliki
pl.Help=Pliki pomocy
pl.RG=Dodatkowe narzędzia (zalecane)
pl.Plugins=Dodatkowe wtyczki przetwarzania tagów

it.Main=File prinzipali di programma
it.Help=File di aiuto
it.RG=Programmi di utilità aggiuntivi (consigliati)
it.Plugins=Plugin aggiuntivi

de.Main=Hauptseite Dateien
de.Help=Hilfedateien
de.RG=Zusätzliche Dienstprogramme (empfohlen)
de.Plugins=Zusätzliche plugins

fr.Main=Fichiers principaux de programme
fr.Help=Fichiers d'aide
fr.RG=Services supplémentaires (recommandé)
fr.Plugins=Plugins supplémentaires

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Components]
Name: "main"; Description: "{cm:Main}"; Types: full compact custom; Flags: fixed
Name: "help"; Description: "{cm:Help}"; Types: full custom
Name: "utilities"; Description: "{cm:RG}"; Types: custom full
Name: "plugins"; Description: "{cm:Plugins}"; Types: custom full

[Dirs]
Name: "{app}\html\en"; Flags: uninsalwaysuninstall; Components: help
Name: "{app}\html\en\images"; Flags: uninsalwaysuninstall; Components: help
Name: "{app}\imageformats"; Flags: uninsalwaysuninstall; Components: main
Name: "{app}\schemes"; Flags: uninsalwaysuninstall; Components: main
Name: "{app}\completions"; Flags: uninsalwaysuninstall; Components: main
Name: "{app}\platforms"; Flags: uninsalwaysuninstall; Components: main
Name: "{app}\plugins"; Flags: uninsalwaysuninstall; Components: plugins
Name: "{app}\icons\default"; Flags: uninsalwaysuninstall; Components: main
Name: "{app}\icons\coloured"; Flags: uninsalwaysuninstall; Components: main

[Files]
#define QT_VERSION 5

#if QT_VERSION==5
#define PATH_TO_QT "K:/Qt/Qt5.5.0/5.5/mingw492_32"
#else
#define PATH_TO_QT "K:/Qt/4.8.4"
#endif

; main
Source: "release\qoobar.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: "release\plugins\*.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion  createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: plugins
; libraries dependent on current Qt version
#if QT_VERSION==5
Source: {#PATH_TO_QT+"/plugins/imageformats/qjpeg.dll"}; DestDir: "{app}\imageformats"; Flags: ignoreversion  createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/plugins/imageformats/qgif.dll"}; DestDir: "{app}\imageformats"; Flags: ignoreversion  createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/plugins/imageformats/qico.dll"}; DestDir: "{app}\imageformats"; Flags: ignoreversion  createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/plugins/platforms/qminimal.dll"}; DestDir: "{app}\platforms"; Flags: ignoreversion createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/plugins/platforms/qwindows.dll"}; DestDir: "{app}\platforms"; Flags: ignoreversion createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/Qt5Core.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/Qt5Concurrent.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/Qt5Gui.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/Qt5Network.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/Qt5Widgets.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/Qt5WinExtras.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/libwinpthread-1.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/libstdc++-6.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
;Source: {#PATH_TO_QT+"/bin/icu*.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
#else
Source: {#PATH_TO_QT+"/plugins/imageformats/qjpeg4.dll"}; DestDir: "{app}\imageformats"; Flags: ignoreversion  createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/plugins/imageformats/qgif4.dll"}; DestDir: "{app}\imageformats"; Flags: ignoreversion  createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/plugins/imageformats/qico4.dll"}; DestDir: "{app}\imageformats"; Flags: ignoreversion  createallsubdirs recursesubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/QtCore4.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/QtGui4.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/QtNetwork4.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: {#PATH_TO_QT+"/bin/mingwm10.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
#endif
Source: {#PATH_TO_QT+"/bin/libgcc_s_dw2-1.dll"}; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
; libraries that do not change
Source: "..\windows\libdiscid.dll"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly replacesameversion; Attribs: readonly; Components: main
Source: "..\windows\libz-1.dll"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly replacesameversion; Attribs: readonly; Components: main
Source: "..\windows\WinSparkle.dll"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly replacesameversion; Attribs: readonly; Components: main
Source: "..\windows\flac.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\mac.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\shntool.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\enca.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\aacgain.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\metaflac.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\mpcgain.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\vorbisgain.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\wvgain.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\replaygain.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
Source: "..\windows\mppdec.exe"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: utilities
; other files
Source: "src\qoobar_app\args.json"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\splitandconvert.bat"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\*.qm"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: "src\qoobar_app\schemes\*.xml"; DestDir: "{app}\schemes"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\completions\*.txt"; DestDir: "{app}\completions"; Flags: ignoreversion overwritereadonly; Components: main
Source: "COPYING"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: "LICENSE.rtf"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: main
Source: "src\qoobar_app\icons\*.ico"; DestDir: "{app}\icons\default"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\icons\*.png"; DestDir: "{app}\icons\default"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\icons\*.gif"; DestDir: "{app}\icons\default"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\icons\coloured\*.ico"; DestDir: "{app}\icons\coloured"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\icons\coloured\*.png"; DestDir: "{app}\icons\coloured"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\icons\coloured\*.gif"; DestDir: "{app}\icons\coloured"; Flags: ignoreversion overwritereadonly; Components: main
Source: "src\qoobar_app\icons\coloured\properties.json"; DestDir: "{app}\icons\coloured"; Flags: ignoreversion overwritereadonly; Components: main

; help
Source: "html\*"; DestDir: "{app}\html"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: help
Source: "html\en\*.htm"; DestDir: "{app}\html\en"; Flags: ignoreversion recursesubdirs createallsubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: help
Source: "html\en\images\*.png"; DestDir: "{app}\html\en\images"; Flags: ignoreversion recursesubdirs createallsubdirs overwritereadonly uninsremovereadonly; Attribs: readonly; Components: help
Source: "README"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly isreadme uninsremovereadonly; Attribs: readonly; Components: help
Source: "README.ru"; DestDir: "{app}"; Flags: ignoreversion overwritereadonly uninsremovereadonly; Attribs: readonly; Components: help

[Icons]
Name: "{group}\Qoobar"; Filename: "{app}\qoobar.exe"; WorkingDir: "{app}"
Name: "{group}\Qoobar Help"; Filename: "{app}\html\en\index.htm"; Components: help; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,Qoobar}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Qoobar"; Filename: "{app}\qoobar.exe"; Tasks: desktopicon; WorkingDir: "{app}"
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Qoobar"; Filename: "{app}\qoobar.exe"; Tasks: quicklaunchicon; WorkingDir: "{app}"

[Run]
Filename: "{app}\qoobar.exe"; Description: "{cm:LaunchProgram,Qoobar}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: "HKCU"; Subkey: "Software\qoobar\gui"; ValueType: string; ValueName: "lang"; ValueData: "{language}"; Flags: createvalueifdoesntexist

[UninstallDelete]
Type: dirifempty; Name: "{app}\html"
Type: dirifempty; Name: "{app}\schemes"
Type: dirifempty; Name: "{app}\plugins"
Type: dirifempty; Name: "{app}\platforms"
Type: dirifempty; Name: "{app}\imageformats"
Type: dirifempty; Name: "{app}\completions"
Type: dirifempty; Name: "{app}\icons"
Type: dirifempty; Name: "{app}"
