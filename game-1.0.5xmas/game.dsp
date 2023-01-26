# Microsoft Developer Studio Project File - Name="game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=game - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "game.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "game - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "game - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GAME_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GAME_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GAME_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\\" /D "SC_GAME" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GAME_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../data/game.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "game - Win32 Release"
# Name "game - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\client.c
# End Source File
# Begin Source File

SOURCE=.\e_alien_shared.c
# End Source File
# Begin Source File

SOURCE=.\e_aliengreen.c
# End Source File
# Begin Source File

SOURCE=.\e_alienred.c
# End Source File
# Begin Source File

SOURCE=.\e_astronaut.c
# End Source File
# Begin Source File

SOURCE=.\e_balloon.c
# End Source File
# Begin Source File

SOURCE=.\e_bgm.c
# End Source File
# Begin Source File

SOURCE=.\e_bombchicken.c
# End Source File
# Begin Source File

SOURCE=.\e_chicken.c
# End Source File
# Begin Source File

SOURCE=.\e_chicken_shared.c
# End Source File
# Begin Source File

SOURCE=.\e_comet.c
# End Source File
# Begin Source File

SOURCE=.\e_debris.c
# End Source File
# Begin Source File

SOURCE=.\e_egg.c
# End Source File
# Begin Source File

SOURCE=.\e_enemyship.c
# End Source File
# Begin Source File

SOURCE=.\e_feather.c
# End Source File
# Begin Source File

SOURCE=.\e_missile.c
# End Source File
# Begin Source File

SOURCE=.\e_motherchicken.c
# End Source File
# Begin Source File

SOURCE=.\e_particle.c
# End Source File
# Begin Source File

SOURCE=.\e_planet.c
# End Source File
# Begin Source File

SOURCE=.\e_player.c
# End Source File
# Begin Source File

SOURCE=.\e_racer01.c
# End Source File
# Begin Source File

SOURCE=.\e_racer02.c
# End Source File
# Begin Source File

SOURCE=.\e_racer_shared.c
# End Source File
# Begin Source File

SOURCE=.\e_rocket.c
# End Source File
# Begin Source File

SOURCE=.\e_satellite.c
# End Source File
# Begin Source File

SOURCE=.\e_score.c
# End Source File
# Begin Source File

SOURCE=.\e_sonde.c
# End Source File
# Begin Source File

SOURCE=.\e_sound.c
# End Source File
# Begin Source File

SOURCE=.\e_superchicken.c
# End Source File
# Begin Source File

SOURCE=.\e_weaponmodel.c
# End Source File
# Begin Source File

SOURCE=.\g_damage.c
# End Source File
# Begin Source File

SOURCE=.\g_entchar.c
# End Source File
# Begin Source File

SOURCE=.\g_entmanage.c
# End Source File
# Begin Source File

SOURCE=.\g_hack_entitycache.c
# End Source File
# Begin Source File

SOURCE=.\g_import.c
# End Source File
# Begin Source File

SOURCE=.\g_level.c
# End Source File
# Begin Source File

SOURCE=.\g_misc.c
# End Source File
# Begin Source File

SOURCE=.\g_script.c
# End Source File
# Begin Source File

SOURCE=.\g_shared.c
# End Source File
# Begin Source File

SOURCE=.\g_weapon.c
# End Source File
# Begin Source File

SOURCE=.\gs_action.c
# End Source File
# Begin Source File

SOURCE=.\gs_fade.c
# End Source File
# Begin Source File

SOURCE=.\gs_gameover.c
# End Source File
# Begin Source File

SOURCE=.\gs_intermission.c
# End Source File
# Begin Source File

SOURCE=.\gs_main.c
# End Source File
# Begin Source File

SOURCE=.\gs_pause.c
# End Source File
# Begin Source File

SOURCE=.\gs_titlescreen.c
# End Source File
# Begin Source File

SOURCE=..\math3d.c
# End Source File
# Begin Source File

SOURCE=.\math3d_support.c
# End Source File
# Begin Source File

SOURCE=.\menu.c
# End Source File
# Begin Source File

SOURCE=.\sc_io.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\g_import.h
# End Source File
# Begin Source File

SOURCE=.\g_local.h
# End Source File
# Begin Source File

SOURCE=.\g_shared.h
# End Source File
# Begin Source File

SOURCE=.\gs_main.h
# End Source File
# Begin Source File

SOURCE=..\math3d.h
# End Source File
# Begin Source File

SOURCE=.\math3d_support.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\script.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
