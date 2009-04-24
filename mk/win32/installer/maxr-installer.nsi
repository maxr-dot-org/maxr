#
# This is the nis-installer-script for M.A.X. Reloaded
#

#--- Headers ---
!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "StrFunc.nsh"
!include "StrContains.nsh"

${StrStr}

# --- Main defines ---
!define VERSION                    "0.2.4"
!define FILESFOLDER                "D:\Spiele\Max-Reloaded - 2\" ;the folder to the files that should be installed with this installer (svn game directory)
!define RESINSTALLER_EXE           "${FILESFOLDER}resinstaller.exe"
!define RESINSTALLER_TESTFILE      "${FILESFOLDER}init.pcx"
!define RES_KEEP_SPACE             138444 ;these are just about values
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
# Language Page
Page custom LanguageDialog LanguageDialogLeave
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
VAr SELECTED_LANGUAGE

# --- Main files ---
Section "Main Files (requierd)" Section_Main
        SetOutPath $INSTDIR

        # first install files which will be needed by the resinstaller
        File "${RESINSTALLER_EXE}"
        File "${RESINSTALLER_TESTFILE}"
        File /r "${FILESFOLDER}*.xml" ; xml files are just installed to get the directory structure of buildings and vehicles
        
        # the resinstaller will run _before_ copying the free graphics so that they will be keeped
        ${If} $SELECTION_STATUS == "Keep"
        # create rest of directorys for the resinstaller
        CreateDirectory "$INSTDIR\gfx"
        CreateDirectory "$INSTDIR\fx"
        CreateDirectory "$INSTDIR\maps"
        CreateDirectory "$INSTDIR\music"
        CreateDirectory "$INSTDIR\sounds"
        CreateDirectory "$INSTDIR\voices"
        CreateDirectory "$INSTDIR\mve"
        # copy graphics which will be needed by the resinstaller
        # these files will be replaced after the resinstaller has pased by the free graphics in this mode
        File /r "${FILESFOLDER}*effect_org.pcx"
        File /r "${FILESFOLDER}*effect.pcx"
        File /r "${FILESFOLDER}hud_stuff.pcx"
        # run the resinstaller
        ExecWait "${RESINSTALLER_EXE} $\"$ORIGINAL_DIR$\" $\"$INSTDIR$\""
        ${EndIF}
        
        # now install the rest of the files
        File /r /x "*.xml" /x "resinstaller.exe" /x "init.pcx" "${FILESFOLDER}"
        
        # run the resinstaller after copying the files when the free graphics should be overwritten
        ${If} $SELECTION_STATUS == "Over"
        ExecWait "${RESINSTALLER_EXE} $\"$ORIGINAL_DIR$\" $\"$INSTDIR$\""
        ${EndIF}
        
        Call ChangeLanguage
SectionEnd

VAR MAX_XML_FILE
VAR TMP_STRING
VAR FILE_LENGTH
VAR LANGUAGE_CODE

Function ChangeLanguage
         # open the max.xml
         FileOpen $MAX_XML_FILE $INSTDIR\max.xml a
         StrCpy $FILE_LENGTH "0"
         
         # get the code of the selected language
         StrCpy $LANGUAGE_CODE $SELECTED_LANGUAGE 3 -4
         
         # find the language code postion in the xml file
         # the xml must contain this line: "<Language Text="XYZ" />" !!!
         ${While} 0 < 1
                  # read the next line
                  FileRead $MAX_XML_FILE $TMP_STRING 1024

                  # look whether in this line is the language code
                  ${StrContains} $0 "Language Text=$\"" $TMP_STRING
         
                  StrCmp $0 "" notfound
                         # calulate the length before the language entry
                         ${StrStr} $R1 $TMP_STRING "Language Text=$\""
                         StrLen $0 $TMP_STRING
                         StrLen $1 $R1
                         IntOp $0 $0 - $1
                         
                         # add the length to the postion counter
                         IntOp $FILE_LENGTH $FILE_LENGTH + $0
                         IntOp $FILE_LENGTH $FILE_LENGTH + 15
                         
                         # replace the language code
                         FileSeek $MAX_XML_FILE $FILE_LENGTH SET
                         FileWrite $MAX_XML_FILE $LANGUAGE_CODE
                         
                         GoTo done
                   notfound:
                         # if the code is not in this line, get the length of the line and add it to the position counter
                         StrLen $0 $TMP_STRING
                         IntOp $FILE_LENGTH $FILE_LENGTH + $0
         ${EndWhile}
         done:
         
         FileClose $MAX_XML_FILE
         
FunctionEnd

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
         
         #Set section sizes.
         SectionSetSize ${Section_Keep} ${RES_KEEP_SPACE}
         SectionSetSize ${Section_Over} ${RES_OVER_SPACE}

         #Set start selection to the selection-variable
         StrCpy $SELECTION_STATUS "Keep"
FunctionEnd

# --- On Section Selection Change function ---
# handles that only one (keep _OR_ overwrite free graphics) can be selcted
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
         
         StrCpy $SELECTION_STATUS "None"
         GoTo End
         
         SelectKeep:
         SectionSetFlags ${Section_Over} 0
         StrCpy $SELECTION_STATUS "Keep"
         GoTo End
         
         SelectOver:
         SectionSetFlags ${Section_Keep} 0
         StrCpy $SELECTION_STATUS "Over"
         GoTo End
         
         
         End:
         Pop $R0
         Pop $R1
FunctionEnd

# --- Language Page Variables ---
Var LANGUAGE_DROPLIST

# --- Language Page Function ---
Function LanguageDialog
         Push $R0
         Push $R1

         # Create the Dialog
         GetDlgItem $0 $HWNDPARENT 1
         EnableWindow $0 1

         nsDialogs::Create 1018
         !insertmacro MUI_HEADER_TEXT "Choose language" "Choose the language you would prefer to play $(^NameDA) in."

         # Create Dialog-Items
         ${NSD_CreateLabel} 0 0 100% 36u "Setup will change the default language of $(^NameDA). Please select your prefered language from the following list. If your language is not listed there, the language is not supported yet. Feel free to help us translating the game and write the translation yourselve. More information at www.maxr.org."
         ${NSD_CreateDropList} 0 78 180 12 ""
         Pop $LANGUAGE_DROPLIST
         
         # Add the by maxr supported languages
         ${NSD_CB_AddString} $LANGUAGE_DROPLIST "English (ENG)"
         ${NSD_CB_AddString} $LANGUAGE_DROPLIST "German (GER)"
         ${NSD_CB_AddString} $LANGUAGE_DROPLIST "Russian (RUS)"
         ${NSD_CB_AddString} $LANGUAGE_DROPLIST "Hungarian (HUN)"
         ${NSD_CB_AddString} $LANGUAGE_DROPLIST "Dutch (DUT)"
         ${NSD_CB_AddString} $LANGUAGE_DROPLIST "Slovenian (SLV)"
         
         # by default english is selected
         ${NSD_CB_SelectString} $LANGUAGE_DROPLIST "English (ENG)"
         
         nsDialogs::Show

         Pop $R0
         Pop $R1
FunctionEnd

# --- Function to get the selected language ---
Function LanguageDialogLeave
         ${NSD_GetText} $LANGUAGE_DROPLIST $SELECTED_LANGUAGE
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

# --- Function to get the directory of the original installation or CD from the textfield ---
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
