#
# This is the nis-installer-script for M.A.X. Reloaded
#

#--- Headers ---
!include "MUI2.nsh"
!include "FileFunc.nsh"

# --- Main defines ---
!define VERSION                    "0.2.4"
!define FILESFOLDER                "D:\Spiele\Max-Reloaded - 2\"
!define RESINSTALLER_EXE           "${FILESFOLDER}resinstaller.exe"
!define RESINSTALLER_TESTFILE      "${FILESFOLDER}init.pcx"
!define RES_KEEP_SPACE             138444
!define RES_OVER_SPACE             132300

Name "M.A.X. Reloaded"
OutFile "maxr-${VERSION}.exe"
InstallDir "$PROGRAMFILES\M.A.X. Reloaded"

# --- Page Settings ---
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "top_small.bmp"

!define MUI_WELCOMEFINISHPAGE_BITMAP "left.bmp"

!define MUI_LICENSEPAGE_CHECKBOX

# --- Pages ---

# Welcome Page
!insertmacro MUI_PAGE_WELCOME
# License Page
!insertmacro MUI_PAGE_LICENSE "license.txt"
# Components Page
!insertmacro MUI_PAGE_COMPONENTS
# Directoy Page
Page custom DirectoryDialog DirectoryDialogLeave
# Install files Page
!insertmacro MUI_PAGE_INSTFILES
# Finished Page
!insertmacro MUI_PAGE_FINISH

# Set Language
!insertmacro MUI_LANGUAGE "English"
;!insertmacro MUI_LANGUAGE "German"
/*!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Hungarian"*/


Var SELECTION_STATUS
Var ORIGINAL_DIR

# --- Main files ---
Section "Main Files (requierd)" Section_Main
        SetOutPath $INSTDIR
        
        File "${RESINSTALLER_EXE}"
        File "${RESINSTALLER_TESTFILE}"
        File /r "${FILESFOLDER}*.xml"
        
        ${If} $SELECTION_STATUS == "Keep"
        CreateDirectory "$INSTDIR\gfx"
        CreateDirectory "$INSTDIR\fx"
        CreateDirectory "$INSTDIR\maps"
        CreateDirectory "$INSTDIR\music"
        CreateDirectory "$INSTDIR\sounds"
        CreateDirectory "$INSTDIR\voices"
        CreateDirectory "$INSTDIR\mve"
        File /r "${FILESFOLDER}*effect_org.pcx"
        File /r "${FILESFOLDER}*effect.pcx"
        File /r "${FILESFOLDER}hud_stuff.pcx"
        ExecWait "${RESINSTALLER_EXE} $\"$ORIGINAL_DIR$\" $\"$INSTDIR$\""
        ${EndIF}
        
        File /r /x "*.xml" /x "resinstaller.exe" /x "init.pcx" "${FILESFOLDER}"
        
        ${If} $SELECTION_STATUS == "Over"
        ExecWait "${RESINSTALLER_EXE} $\"$ORIGINAL_DIR$\" $\"$INSTDIR$\""
        ${EndIF}
SectionEnd

# --- Resinstaller options ---
SectionGroup "Run resinstaller" Section_Resinstaller
Section "Keep free graphics" Section_Keep
SectionEnd
Section /o "Overwrite free graphics" Section_Over
SectionEnd
SectionGroupEnd

# --- OnInit function ---
Function .onInit
         !insertmacro MUI_LANGDLL_DISPLAY

         #Set section statuses
         IntOp $0 ${SF_SELECTED} | ${SF_RO}
         SectionSetFlags ${Section_Main} $0
         SectionSetFlags ${Section_Keep} ${SF_SELECTED}
         SectionSetFlags ${Section_Over} 0
         
         SectionSetSize ${Section_Keep} ${RES_KEEP_SPACE}
         SectionSetSize ${Section_Over} ${RES_OVER_SPACE}

         StrCpy $SELECTION_STATUS "Keep"
FunctionEnd

# --- On Section Selection Change function ---
Function .onSelChange
         Push $R0
         Push $R1
         
         SectionGetFlags ${Section_Keep} $R0
         SectionGetFlags ${Section_Over} $R1
         
         ${If} $R0 == ${SF_SELECTED}
         StrCmp $SELECTION_STATUS "Over" SelectKeep
         ${EndIF}
         
         ${If} $R1 == ${SF_SELECTED}
         StrCmp $SELECTION_STATUS "Keep" SelectOver
         ${EndIF}
         
         SelectKeep:
         SectionSetFlags ${Section_Over} 0
         StrCpy $SELECTION_STATUS "Keep"
         GoTo End
         
         SelectOver:
         SectionSetFlags ${Section_Keep} 0
         StrCpy $SELECTION_STATUS "Over"
         
         End:
         Pop $R0
         Pop $R1
FunctionEnd

# --- Directory Page Variables ---
Var DIRECTORY_FIELD
Var DIRECTORY_BROWSE
Var ORI_DIRECTORY_FIELD
Var ORI_DIRECTORY_BROWSE
Var REQ_SPACE
Var AVA_SPACE

# --- Directory Page Function ---
Function DirectoryDialog
         Push $R0
         Push $R1

         # Create the Dialog
         GetDlgItem $0 $HWNDPARENT 1
         EnableWindow $0 0

         nsDialogs::Create 1018
         !insertmacro MUI_HEADER_TEXT "Choose Install Location" "Choose the folder in which to install $(^NameDA)."
         
         # Calculate Values
         SectionGetFlags ${Section_Keep} $R0
         SectionGetFlags ${Section_Over} $R1
         
         SectionGetSize Section_Main $3
         IntOp $0 $3 + 0
         ${If} $R0 == ${SF_SELECTED}
               SectionGetSize ${Section_Keep} $1
               IntOp $0 $3 + $1
         ${EndIF}
         ${If} $R1 == ${SF_SELECTED}
               SectionGetSize ${Section_Over} $1
               IntOp $0 $3 + $1
         ${EndIF}
         IntOp $1 $0 / 1024
         IntOp $2 $0 % 1024
         StrCpy $REQ_SPACE "Space required: $1.$2MB"
	
         # Create Dialog-Items
         ${NSD_CreateLabel} 0 0 100% 36u "Setup will install M.A.X. Reloaded in the following folder. To install in a different folder, click Browse and select another folder. Click Install to start the installation"

         # Original Dir Elements
         ${If} $R0 == ${SF_SELECTED}
         ${OrIF} $R1 == ${SF_SELECTED}
         ${NSD_CreateGroupBox} 0 54 100% 57 "Original M.A.X.-CD/Installation Folder"
         ${NSD_CreateDirRequest} 15 78 315 12u ""
         Pop $ORI_DIRECTORY_FIELD
         System::Call shlwapi::SHAutoComplete($ORI_DIRECTORY_FIELD,i1)
         ${NSD_CreateBrowseButton} 342 75 90 24 "Browse..."
         Pop $ORI_DIRECTORY_BROWSE
         ${NSD_OnClick} $ORI_DIRECTORY_BROWSE OriDirBrowse
         ${EndIF}

         # Directoy Filed
         ${NSD_CreateGroupBox} 0 114 100% 57 "Destination Folder"
         ${NSD_CreateDirRequest} 15 138 315 12u $INSTDIR
         Pop $DIRECTORY_FIELD
         ${NSD_OnChange} $DIRECTORY_FIELD DirChange
         System::Call shlwapi::SHAutoComplete($DIRECTORY_FIELD,i1)
         
         Call DirChange
	
	 # Browse Button
         ${NSD_CreateBrowseButton} 342 135 90 24 "Browse..."
         Pop $DIRECTORY_BROWSE
         ${NSD_OnClick} $DIRECTORY_BROWSE DirBrowse
         
         # Space Labels
         ${NSD_CreateLabel} 0 187 100% 12u $REQ_SPACE
         ${NSD_CreateLabel} 0 203 100% 12u ""
         Pop $AVA_SPACE

         Call UpdateAvaSpace
         
         nsDialogs::Show

         Pop $R0
         Pop $R1
FunctionEnd

Function DirectoryDialogLeave
         ${NSD_GetText} $ORI_DIRECTORY_FIELD $ORIGINAL_DIR
FunctionEnd

# --- Function to update available Space ---
Function UpdateAvaSpace

	${GetRoot} $INSTDIR $0
	StrCpy $1 " bytes"

	System::Call kernel32::GetDiskFreeSpaceEx(tr0,*l,*l,*l.r0)

	${If} $0 > 1024
	${OrIf} $0 < 0
		System::Int64Op $0 / 1024
		Pop $0
		StrCpy $1 "KB"
		${If} $0 > 1024
		${OrIf} $0 < 0
			System::Int64Op $0 / 1024
			Pop $0
			StrCpy $1 "MB"
			${If} $0 > 1024
			${OrIf} $0 < 0
				System::Int64Op $0 / 1024
				Pop $0
				StrCpy $1 "GB"
			${EndIf}
		${EndIf}
	${EndIf}
	${NSD_SetText} $AVA_SPACE "Space available: $0$1"
FunctionEnd

# --- Callback function when directoy has chanded ---
Function DirChange
	Pop $0 # dir hwnd
	GetDlgItem $0 $HWNDPARENT 1
	System::Call user32::GetWindowText(i$DIRECTORY_FIELD,t.d,i${NSIS_MAX_STRLEN})
	${If} ${FileExists} $INSTDIR
		EnableWindow $0 1
	${Else}
		EnableWindow $0 0
	${EndIf}
	Call UpdateAvaSpace

FunctionEnd

# --- Browse button functions ---
Function DirBrowse
        ${NSD_GetText} $DIRECTORY_FIELD $R0
        nsDialogs::SelectFolderDialog "" $R0
        Pop $R0
        ${If} $R0 != error
              ${NSD_SetText} $DIRECTORY_FIELD "$R0"
        ${EndIf}
FunctionEnd

Function OriDirBrowse
        ${NSD_GetText} $ORI_DIRECTORY_FIELD $R0
        nsDialogs::SelectFolderDialog "" $R0
        Pop $R0
        ${If} $R0 != error
              ${NSD_SetText} $ORI_DIRECTORY_FIELD "$R0"
        ${EndIf}
FunctionEnd
