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
!define VERSION                    "0.2.9"
!define NAME                       "M.A.X. Reloaded"
!define FILESFOLDER                "..\..\..\data\" ;the folder to the files that should be installed with this installer
!define RESINSTALLER_EXE           "resinstaller.exe"
!define MAXR_EXE                   "maxr.exe"
# estimated size of components (in kb):
!define SIZE_RES_SOUND             145801
!define SIZE_RES_GRAPHIC           1 ; original graphics doen't need more space, than ours

Name "${NAME}"
OutFile "maxr-${VERSION}.exe"
InstallDir "$PROGRAMFILES\M.A.X. Reloaded"

VIProductVersion "${VERSION}.0"
VIAddVersionKey ProductName "${NAME}"
VIAddVersionKey Comments "${NAME} ${VERSION}"
VIAddVersionKey CompanyName maxr.org
VIAddVersionKey LegalCopyright maxr.org
VIAddVersionKey FileDescription "${NAME} installer"
VIAddVersionKey ProductVersion ${VERSION}
VIAddVersionKey OriginalFilename "maxr.exe"

# --- Page Settings ---
!define MUI_ICON "${FILESFOLDER}maxr.ico"
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

Var ORIGINAL_DIR
Var SELECTED_LANGUAGE

# --- Main files ---
Section "Main Files (requierd)" Section_Main
    SetOutPath $INSTDIR

    # now install all files
    File /r "${FILESFOLDER}"

	# create initial config file and set language
    CreateDirectory "$DOCUMENTS\maxr"
    SetOutPath "$DOCUMENTS\maxr"
	SetOverwrite off ; do not overwrite an existing user config
    File "maxr.xml"
	SetOverwrite on
    SetOutPath $INSTDIR
    Call ChangeLanguage
SectionEnd


Section "Start menu shortcut" Section_Start_Menu_Link
	CreateDirectory "$SMPROGRAMS\M.A.X. Reloaded"
	CreateShortCut  "$SMPROGRAMS\M.A.X. Reloaded\M.A.X. Reloaded.lnk" "$INSTDIR\${MAXR_EXE}"
	CreateShortCut  "$SMPROGRAMS\M.A.X. Reloaded\resinstaller.lnk" "$INSTDIR\${RESINSTALLER_EXE}"
SectionEnd

Section "Desktop shortcut" Section_Desktop_Link
	CreateShortCut "$DESKTOP\M.A.X. Reloaded.lnk" "$INSTDIR\${MAXR_EXE}"
SectionEnd

SectionGroup "Import ressources from original M.A.X." Section_Group_Res
    Section "Unit graphics" Section_Res_Graphics
        AddSize ${SIZE_RES_GRAPHIC}
    SectionEnd

    Section "Sounds/Maps" Section_Res_Sound
        AddSize ${SIZE_RES_SOUND}
    SectionEnd
SectionGroupEnd

# hidden section for executing resinstaller
VAR RES_OPTION
Section "" Section_Resinstaller
    Push $R0
    Push $R1

    SectionGetFlags ${Section_Res_Graphics} $R0
    SectionGetFlags ${Section_Res_Sound} $R1

    #build ressource selection
    ${If} $R0 == ${SF_SELECTED}
        StrCpy $RES_OPTION "6789a"
    ${EndIf}
    ${If} $R1 == ${SF_SELECTED}
        StrCpy $RES_OPTION "012345b$RES_OPTION"
    ${EndIf}

    ${If} $RES_OPTION != ""
        ExecWait "${RESINSTALLER_EXE} $\"$ORIGINAL_DIR$\" $\"$INSTDIR$\" $LANGUAGE_CODE $RES_OPTION"
    ${EndIf}

    Pop $R1
    Pop $R0
SectionEnd

# --- Component descriptions
LangString DESC_Section_Main            ${LANG_ENGLISH} "M.A.X. Reloaded game files."
LangString DESC_Section_Start_Menu_Link ${LANG_ENGLISH} "Creates a link in the start menu."
LangString DESC_Section_Desktop_Link    ${LANG_ENGLISH} "Creates a link on the desktop."
LangString DESC_Section_Group_Res       ${LANG_ENGLISH} "Import graphics, sounds and maps from a CD or installation of the original M.A.X. You can import the ressources anytime later by starting the resinstaller tool."
LangString DESC_Section_Res_Graphics    ${LANG_ENGLISH} "Import unit graphics from original M.A.X. This will replace the M.A.X. Reloaded unit graphics."
LangString DESC_Section_Res_Sound       ${LANG_ENGLISH} "Import sounds and maps from original M.A.X. This does not overwrite any of the M.A.X. Reloaded files."


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_Main}            $(DESC_Section_Main)
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_Start_Menu_Link} $(DESC_Section_Start_Menu_Link)
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_Desktop_Link}    $(DESC_Section_Desktop_Link)
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_Group_Res}       $(DESC_Section_Group_Res)
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_Res_Graphics}    $(DESC_Section_Res_Graphics)
  !insertmacro MUI_DESCRIPTION_TEXT ${Section_Res_Sound}       $(DESC_Section_Res_Sound)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# --- OnInit function ---
Function .onInit
    !insertmacro MUI_LANGDLL_DISPLAY

    #Set section flags
    IntOp $0 ${SF_SELECTED} | ${SF_RO}
    SectionSetFlags ${Section_Main} $0
	
FunctionEnd

# --- Language Page Variables ---
Var LANGUAGE_DROPLIST

# --- Language Page Function ---

VAR MAX_XML_FILE
VAR TMP_STRING
VAR FILE_LENGTH
VAR LANGUAGE_CODE

Function ChangeLanguage
    # open the maxr.xml
    FileOpen $MAX_XML_FILE $DOCUMENTS\maxr\maxr.xml a
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
    ;${NSD_CB_AddString} $LANGUAGE_DROPLIST "Russian (RUS)"
    ;${NSD_CB_AddString} $LANGUAGE_DROPLIST "Hungarian (HUN)"
    ${NSD_CB_AddString} $LANGUAGE_DROPLIST "Dutch (DUT)"
    ;${NSD_CB_AddString} $LANGUAGE_DROPLIST "Slovenian (SLV)"
    ;${NSD_CB_AddString} $LANGUAGE_DROPLIST "French (FRE)"
    ${NSD_CB_AddString} $LANGUAGE_DROPLIST "Spanish (SPA)"

    # by default english is selected
    ${NSD_CB_SelectString} $LANGUAGE_DROPLIST "English (ENG)"

    nsDialogs::Show

    Pop $R1
    Pop $R0
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
Var ORI_NOT_FOUND_LABEL
Var REQ_SPACE
Var AVA_SPACE
Var ORI_PATH_FOUND

# --- Directory Page Function ---
Function DirectoryDialog
    Push $R0
    Push $R1

    # Create the Dialog
    nsDialogs::Create 1018
    !insertmacro MUI_HEADER_TEXT "Choose Install Location" "Choose the folder in which to install $(^NameDA)."

    # Calculate Values
    SectionGetFlags ${Section_Res_Sound} $R0
    SectionGetFlags ${Section_Res_Graphics} $R1

    SectionGetSize Section_Main $3
    IntOp $0 $3 + 0
    ${If} $R0 == ${SF_SELECTED}
        SectionGetSize ${Section_Res_Sound} $1
        IntOp $0 $3 + $1
    ${EndIF}
    ${If} $R1 == ${SF_SELECTED}
        SectionGetSize ${Section_Res_Graphics} $1
        IntOp $0 $3 + $1
    ${EndIF}
    IntOp $1 $0 / 1024
    IntOp $2 $0 % 1024
    StrCpy $REQ_SPACE "Space required: $1.$2MB"

    # Create Dialog-Items
    ${NSD_CreateLabel} 0 0 100% 26u "Setup will install M.A.X. Reloaded in the following folder. To install in a different folder, click Browse and select another folder. Click Install to start the installation"

    # Original Dir Elements
    ${If} $R0 == ${SF_SELECTED}
    ${OrIF} $R1 == ${SF_SELECTED}
		${NSD_CreateGroupBox} 0 95 100% 100 "Original M.A.X.-CD or Installation Folder"
		${NSD_CreateDirRequest} 15 119 315 12u ""
		Pop $ORI_DIRECTORY_FIELD
		${NSD_OnChange} $ORI_DIRECTORY_FIELD OriDirChange
		System::Call shlwapi::SHAutoComplete($ORI_DIRECTORY_FIELD,i1)

		${NSD_CreateBrowseButton} 342 116 90 24 "Browse..."
		Pop $ORI_DIRECTORY_BROWSE
		${NSD_OnClick} $ORI_DIRECTORY_BROWSE OriDirBrowse
		
		${NSD_CreateLabel} 15 144 95% 26u ""
		Pop $ORI_NOT_FOUND_LABEL
    ${EndIF}

    # Directoy Filed
    ${NSD_CreateGroupBox} 0 30 100% 57 "Destination Folder"
    ${NSD_CreateDirRequest} 15 54 315 12u $INSTDIR
    Pop $DIRECTORY_FIELD
    ${NSD_OnChange} $DIRECTORY_FIELD DirChange
    System::Call shlwapi::SHAutoComplete($DIRECTORY_FIELD,i1)

    Call DirChange

    # Browse Button
    ${NSD_CreateBrowseButton} 342 51 90 24 "Browse..."
    Pop $DIRECTORY_BROWSE
    ${NSD_OnClick} $DIRECTORY_BROWSE DirBrowse

    # Space Labels
    ${NSD_CreateLabel} 0 200 100% 12u $REQ_SPACE
    ${NSD_CreateLabel} 0 214 100% 12u ""
    Pop $AVA_SPACE

    Call UpdateAvaSpace

    nsDialogs::Show

    Pop $R1
    Pop $R0
FunctionEnd

# --- Function to get the directory of the original installation or CD from the textfield ---
Function DirectoryDialogLeave
    Push $R0
    Push $R1
	SectionGetFlags ${Section_Res_Sound} $R0
    SectionGetFlags ${Section_Res_Graphics} $R1
	
	${If} $R0 == ${SF_SELECTED}
    ${OrIF} $R1 == ${SF_SELECTED}
		Call CheckOriPath
		${if} $ORI_PATH_FOUND == "0"
			Abort
		${EndIf}
	${EndIf}
	
    ${NSD_GetText} $ORI_DIRECTORY_FIELD $ORIGINAL_DIR
	
	Pop $R1
    Pop $R0
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

Function CheckOriPath
	Push $R0
	${NSD_GetText} $ORI_DIRECTORY_FIELD $R0
	
	${NSD_SetText} $ORI_NOT_FOUND_LABEL "Original M.A.X. found."
	StrCpy $ORI_PATH_FOUND "1"
	IfFileExists "$R0\MAX.RES" FOUND 0
	IfFileExists "$R0\MAX\MAX.RES" FOUND 0
	${NSD_SetText} $ORI_NOT_FOUND_LABEL "No original M.A.X found. Please choose the correct path, or disable the Import option in the installation settings. You can import the original ressources anytime later by using the resinstaller tool."
	StrCpy $ORI_PATH_FOUND "0"
	FOUND:
	
	Pop $R0
FunctionEnd

# --- Callback function when directoy has chanded ---
Function DirChange
    Pop $0 # dir hwnd
    GetDlgItem $0 $HWNDPARENT 1
    System::Call user32::GetWindowText(i$DIRECTORY_FIELD,t.d,i${NSIS_MAX_STRLEN})

    Call UpdateAvaSpace
FunctionEnd

Function OriDirChange
	Call CheckOriPath 
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
