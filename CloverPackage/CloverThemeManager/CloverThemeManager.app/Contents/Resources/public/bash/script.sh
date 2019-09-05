#!/bin/bash

# A script for Clover Theme Manager
# Copyright (C) 2014-2019 Blackosx
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Credits:
# Thanks to SoThOr for helping with svn communications
# Thanks to apianti for setting up the Clover git theme repository.
# Thanks to apianti, dmazar & JrCs for their git know-how. 
# Thanks to alexq, asusfreak, chris1111, droplets, eMatoS, kyndder & oswaldini for testing.

VERS="0.78.3"

# =======================================================================================
# Helper Functions/Routines
# =======================================================================================



# ---------------------------------------------------------------------------------------
CreateSymbolicLinks()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CreateSymbolicLinks()"

    local checkCount=0

    # Create symbolic link to local images
    if [ ! -L "${WORKING_PATH}/${APP_DIR_NAME}/themes" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating symbolic link to ${WORKING_PATH}/${APP_DIR_NAME}/themes"
        ln -s "${WORKING_PATH}/${APP_DIR_NAME}"/themes "$TEMPDIR"
        ((checkCount++))
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Symbolic link to ${WORKING_PATH}/${APP_DIR_NAME}/themes exists"
        ((checkCount++))
    fi

    # Create symbolic link to local help page
    if [ ! -L "${WORKING_PATH}/${APP_DIR_NAME}/CloverThemeManagerApp/help/add_theme.html" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating symbolic link to ${WORKING_PATH}/${APP_DIR_NAME}/CloverThemeManagerApp/help/add_theme.html"
        ln -s "${WORKING_PATH}/${APP_DIR_NAME}"/CloverThemeManagerApp/help/add_theme.html "$TEMPDIR"
        ((checkCount++))
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Symbolic link to ${WORKING_PATH}/${APP_DIR_NAME}/CloverThemeManagerApp/help/add_theme.html exists"
        ((checkCount++))
    fi

    # Create symbolic links for scripts dir files, except cloverthememanager.js which is copied to tmp earlier on    
    local filecount=0
    for f in "$JSSCRIPTS_DIR"/*
    do
        if [ "${f##*/}" != "cloverthememanager.js" ]; then
            if [ ! -L "$f" ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating symbolic link to $f"
                ln -s "$f" "$TEMPDIR"/scripts
                ((checkCount++))
            else
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Symbolic link to $f exists"
            fi
            ((filecount++))
        fi
    done

    # Create symbolic links for styles dir
    if [ ! -L "${PUBLIC_DIR}/styles" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating symbolic link to ${PUBLIC_DIR}/styles"
        ln -s "${PUBLIC_DIR}/styles" "$TEMPDIR"
        ((checkCount++))
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Symbolic link to ${PUBLIC_DIR}/styles exists"
        ((checkCount++))
    fi

    # Create symbolic links for assets dir
    if [ ! -L "${PUBLIC_DIR}/assets" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating symbolic link to ${PUBLIC_DIR}/assets"
        ln -s "${PUBLIC_DIR}/assets" "$TEMPDIR"
        ((checkCount++))
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Symbolic link to ${PUBLIC_DIR}/assets exists"
        ((checkCount++))
    fi

    # Add messages in to log for initialise.js to detect.
    local countToMatch=$(( 4 + filecount ))
    if [ $checkCount -eq $countToMatch ]; then
        WriteToLog "CTM_SymbolicLinksOK"
    else
        WriteToLog "CTM_SymbolicLinksFail"
    fi
}

# ---------------------------------------------------------------------------------------
SendToUIUVersionedDir() {
    echo "${1}" >> "$logBashToJsVersionedDir"
}

# ---------------------------------------------------------------------------------------
FindStringInPlist() {
    # Check if file contains carriage returns (CR) as opposed to Line Feed (LF)
    checkForCR=$( tr -cd '\r' < "$2" | wc -c )
    if [ $checkForCR -gt 0 ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}${2##*/} contains carriage returns (CR)"
        local a=$( cat -v "$2" )
        local b="${a##*${1}</key>}"
        local c="${b%%</string>*}"
        local string="${c##*<string>}"
    else
        local string=$( grep -A 1 "<key>${1}</key>" "${2}" | head -n 2 | tail -2 | sed 1d | sed -e 's/<\/string>//g' )
    fi
    string=${string#*<string>}
    echo "$string"
}

# ---------------------------------------------------------------------------------------
FindStringInPlistVariable() {
    local string=$( echo "${2}" | grep -A 1 "<key>${1}</key>" | head -n 2 | tail -2 | sed 1d | sed -e 's/<\/string>//g' )
    string=${string#*<string>}
    echo "$string"
}

# ---------------------------------------------------------------------------------------
RemoveFile()
{
    if [ -f "$1" ]; then
        rm "$1"
    fi
}

# ---------------------------------------------------------------------------------------
CalculateMd5() {
	local hash=$( md5 "$1" )
    echo "${hash##*= }"
}

# ---------------------------------------------------------------------------------------
ResetNewlyInstalledThemeVars()
{
    # Reset vars for newly installed theme
    gNewInstalledThemeName=""
    gNewInstalledThemePath=""
    gNewInstalledThemePathDevice=""
    gNewinstalledThemePartitionGUID=""
}

# ---------------------------------------------------------------------------------------
ResetUnInstalledThemeVars()
{
    # Reset vars for newly installed theme
    gUnInstalledThemeName=""
    gUnInstalledThemePath=""
    gUnInstalledThemePathDevice=""
    gUninstalledThemePartitionGUID=""
}

# ---------------------------------------------------------------------------------------
ResetInternalThemeArrays()
{
    # Reset arrays for newly installed theme
    unset installedThemeName
    unset installedThemePath
    unset installedThemePathDevice
    unset installedThemePartitionGUID
}

# ---------------------------------------------------------------------------------------
ResetInternalDiskArrays()
{
    # Reset arrays for newly installed theme
    unset duIdentifier
    unset duVolumeName
    unset duVolumeMountPoint
    unset duContent
    unset duPartitionGuid
    unset themeDirPaths
}

# ---------------------------------------------------------------------------------------
RenameInternalESPMountPointToEFI()
{
    #[[ DEBUG -eq 1 ]] && WriteLinesToLog
    #[[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}RenameInternalESPMountPointToEFI()"

    # Rename internal ESP internal mountpoint in supplied path to EFI
    if [[ "$1" == *"$gESPMountPrefix"* ]]; then
        local tmpVolume="${1%/EFI*}"
        local tmpPath="${1##*$tmpVolume}"
        local finalPath="/Volumes/EFI${tmpPath}"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Renaming for UI $1 to $finalPath"
    else
        finalPath="$1"
    fi
    echo "$finalPath"
}

# ---------------------------------------------------------------------------------------
MaintainInstalledThemeListInPrefs()
{    
    # This routine creates the InstalledThemes array which is
    # then written to the user's preferences file.

    # The InstalledThemes array keeps track of the current state
    # of all theme installations done by this application.
    # It also records the update state of each installed theme.

    # When themes are UnInstalled/deleted by the user, the pref
    # entry is also removed.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}MaintainInstalledThemeListInPrefs()"

    chmod 755 "$gUserPrefsFile".plist 

    openArray="<array>"
    closeArray="</array>"
    openDict="<dict>"
    closeDict="</dict>"
    local themeToAppend=0

    InsertDictionaryIntoArray()
    {
        local passedPath="$1"
        local passedDevice="$2"
        local passedUuid="$3"
        local passedUpdate="$4"

        # Rename any ESP internal mountpoint to EFI
        passedPath=$( RenameInternalESPMountPointToEFI "$passedPath" )

        # open dictionary
        arrayString="${arrayString}$openDict"

        # Add theme entries
        arrayString="${arrayString}<key>ThemePath</key>"
        arrayString="${arrayString}<string>$passedPath</string>"
        arrayString="${arrayString}<key>ThemePathDevice</key>"
        arrayString="${arrayString}<string>$passedDevice</string>"
        arrayString="${arrayString}<key>VolumeUUID</key>"
        arrayString="${arrayString}<string>$passedUuid</string>"
        #arrayString="${arrayString}<key>UpdateAvailable</key>"
        #arrayString="${arrayString}<string>$passedUpdate</string>"

        # close dictionary
        arrayString="${arrayString}$closeDict"
    }

    # Is there a newly installed theme to add?
    # And on a partition with a unique partition GUID?
    if [ "$gNewInstalledThemeName" != "" ] && [ "$gNewinstalledThemePartitionGUID" != "$zeroUUID" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Newly installed theme to be added to prefs: $gNewInstalledThemeName"
        # Is this new theme already installed elsewhere?
        for ((n=0; n<${#installedThemeName[@]}; n++ ));
        do
            if [ "$gNewInstalledThemeName" == "${installedThemeName[$n]}" ]; then
                themeToAppend=1
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$gNewInstalledThemeName is already in prefs - will append to entry"
                break
            fi
        done
    fi

    # Is there an UnInstalled theme to remove?
    local dontReAddThemeId=9999
    if [ "$gUnInstalledThemeName" != "" ] && [ "$gNewinstalledThemePartitionGUID" != "$zeroUUID" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}UnInstalled theme to be removed: $gUnInstalledThemeName"

        # Check for ESP mountpoint
        local pathToCheck
        if [[ "$gUnInstalledThemePath" == *$gESPMountPrefix* ]]; then
            local tmpStrip="${themeDirPaths[$entry]#*/}"
            tmpStrip="${tmpStrip#*/}"
            tmpStrip="${tmpStrip#*/}"
            pathToCheck="/Volumes/EFI/${tmpStrip}"
        else
            pathToCheck="$gUnInstalledThemePath"
        fi

        # Loop though array of installed themes to find ID of theme to remove.
        for ((n=0; n<${#installedThemeName[@]}; n++ ));
        do 
            if [ "${installedThemeName[$n]}" == "$gUnInstalledThemeName" ] && [ "${installedThemePath[$n]}" == "$pathToCheck" ] && \
               [ "${installedThemePartitionGUID[$n]}" == "$gUninstalledThemePartitionGUID" ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Will remove ${installedThemeName[$n]},${installedThemePath[$n]},${installedThemePartitionGUID[$n]}"
                dontReAddThemeId=$n
                ResetUnInstalledThemeVars
                break
            fi
        done
    fi

    # Construct InstalledThemes array
    arrayString=""
    lastAddedThemeName=""
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Updating InstalledThemes prefs"
    for ((n=0; n<${#installedThemeName[@]}; n++ ));
    do
         # Don't write back a theme if marked to be removed
         if [ $n -ne $dontReAddThemeId ]; then

            # Housekeeping can change a theme name to a dash.
            # This indicates the theme entry in no longer required.
            if [ "${installedThemeName[$n]}" != "-" ]; then

                # Add theme key
                if [ "${installedThemeName[$n]}" != "$lastAddedThemeName" ]; then

                    # Check if there's a newly installed theme to append to this current array
                    if [ $themeToAppend -eq 1 ] && [ "$lastAddedThemeName" == "$gNewInstalledThemeName" ]; then
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Appending $gNewInstalledThemeName dictionary to existing array."
                        InsertDictionaryIntoArray "$gNewInstalledThemePath" "$gNewInstalledThemePathDevice" "$gNewinstalledThemePartitionGUID" ""
                        themeToAppend=0
                        ResetNewlyInstalledThemeVars
                    fi

                    # close any previous arrays
                    if [ "$lastAddedThemeName" != "" ]; then
                        arrayString="${arrayString}$closeArray"
                    fi

                    # Write new theme key
                    arrayString="${arrayString}<key>${installedThemeName[$n]}</key>"

                    # open array
                    arrayString="${arrayString}$openArray"
                    lastAddedThemeName="${installedThemeName[$n]}"
                fi
                InsertDictionaryIntoArray "${installedThemePath[$n]}" "${installedThemePathDevice[$n]}" "${installedThemePartitionGUID[$n]}"
            fi
        fi
    done

    # Did the loop finish before appending a newly installed theme to an existing them entry?
    # Check if there's a newly installed theme to append to this current array
    if [ $themeToAppend -eq 1 ] && [ "$lastAddedThemeName" == "$gNewInstalledThemeName" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Append didn't happen. Attempting to appending $gNewInstalledThemeName now."
        InsertDictionaryIntoArray "$gNewInstalledThemePath" "$gNewInstalledThemePathDevice" "$gNewinstalledThemePartitionGUID"
        themeToAppend=0
        ResetNewlyInstalledThemeVars
    fi

    # Was the above loop run?
    if [ "$lastAddedThemeName" != "" ]; then
        # close array
        arrayString="${arrayString}$closeArray"
    fi

    # Did the newly installed theme get appended? If not then it needs adding at end.
    if [ "$gNewInstalledThemeName" != "" ]; then
        # Write new theme key
        arrayString="${arrayString}<key>${gNewInstalledThemeName}</key>"
        # open array
        arrayString="${arrayString}$openArray"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Append still hasn't completed. Appending $gNewInstalledThemeName now."
        InsertDictionaryIntoArray "$gNewInstalledThemePath" "$gNewInstalledThemePathDevice" "$gNewinstalledThemePartitionGUID"
        # close array
        arrayString="${arrayString}$closeArray"
        lastAddedThemeName="$gNewInstalledThemeName"
        ResetNewlyInstalledThemeVars
    fi

    # Delete existing and write new InstalledThemes prefs key
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Removing previous InstalledThemes array from prefs file"
    defaults delete "$gUserPrefsFile" "InstalledThemes"

    # Only add back if there's something to write.
    if [ "$lastAddedThemeName" != "" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting InstalledThemes array in to prefs file"
        defaults write "$gUserPrefsFile" InstalledThemes -array "$openDict$arrayString$closeDict"
    fi
    chmod 755 "$gUserPrefsFile".plist 
    ReadPrefsFile
}

# ---------------------------------------------------------------------------------------
UpdatePrefsKey()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}UpdatePrefsKey()"

    local passedKey="$1"
    local passedValue="$2"

    # Rename any ESP internal mountpoint to EFI
    if [ "$passedKey" == "LastSelectedPath" ]; then
        passedValue=$( RenameInternalESPMountPointToEFI "$passedValue" )
    fi

    if [ -f "$gUserPrefsFile".plist ]; then
        defaults delete "$gUserPrefsFile" "$passedKey"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Writing prefs key $passedKey = $passedValue"
        defaults write "$gUserPrefsFile" "$passedKey" "$passedValue"
    else
        WriteToLog "Error! ${gUserPrefsFile}.plist not found."
    fi
}

# ---------------------------------------------------------------------------------------
ClearTopOfMessageLog()
{
    # removes the first line of the log file.
    local log=$(tail -n +2 "$1"); > "$1" && if [ "$log" != "" ]; then echo "$log" > "$1"; fi
}

# ---------------------------------------------------------------------------------------
RunThemeAction()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}RunThemeAction()"

    local passedAction="$1" # Will be either Install, UnInstall or Update
    local themeTitleToActOn="$2"
    local successFlag=1

    CheckPathIsWriteable "${TARGET_THEME_DIR}"
    local isPathWriteable=$? # 1 = not writeable / 0 = writeable

    case "$passedAction" in
                "Install")  WriteToLog "Installing theme $themeTitleToActOn to ${TARGET_THEME_DIR}"
                            local successFlag=1

                            # Only clone the theme from the Clover repo if not already installed
                            # in which case the bare repo will already be in the local support dir.
                            if [ ! -d "${WORKING_PATH}/${APP_DIR_NAME}"/"$themeTitleToActOn".git ]; then
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating a bare git clone of $themeTitleToActOn"
                                local themeNameWithSpacesFixed=$( echo "$themeTitleToActOn" | sed 's/ /%20/g' )

                                cd "${WORKING_PATH}/${APP_DIR_NAME}"
                                feedbackCheck=$("$gitCmd" clone --progress --depth=1 --bare "$remoteRepositoryUrl"/themes.git/themes/"${themeNameWithSpacesFixed}"/theme.git "$themeTitleToActOn".git 2>&1 )
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Install git clone: $feedbackCheck"

                            else
                                WriteToLog "Bare git clone of $themeTitleToActOn already exists. Will checkout from that."
                            fi

                            if [ -d "${WORKING_PATH}/${APP_DIR_NAME}"/"$themeTitleToActOn".git ]; then
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Checking out bare git clone of ${themeTitleToActOn}."

                                # Theme currently gets checked out as /path/to/EFI/Clover/Themes/<theme>/themes/<theme>/
                                # Desired path is                     /path/to/EFI/Clover/Themes/<theme>
                                # So checkout to a directory for unpacking first.
                                if [ -d "$UNPACKDIR" ]; then
                                    cd "${WORKING_PATH}/${APP_DIR_NAME}"
                                    feedbackCheck=$("$gitCmd" --git-dir="$themeTitleToActOn".git --work-tree="$UNPACKDIR" checkout . 2>&1 )
                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}checkout .: $feedbackCheck"
                                    feedbackCheck=$("$gitCmd" --git-dir="$themeTitleToActOn".git --work-tree="$UNPACKDIR" checkout HEAD -- 2>&1 ) && successFlag=0
                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}checkout HEAD --: $feedbackCheck"
                                else
                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Error. UnPack dir does not exist."
                                fi

                                # Read current hash from packed-refs file.
                                local currentThemeHash=$( cat "${WORKING_PATH}/${APP_DIR_NAME}"/"$themeTitleToActOn".git/packed-refs | grep refs/heads/master )
                                currentThemeHash="${currentThemeHash% refs*}"
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}hash=$currentThemeHash"
                            fi

                            if [ ${successFlag} -eq 0 ]; then 

                                # Write hash to file in to unpacked theme dir.
                                local addFile=0
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Writing hash to ${UNPACKDIR}/themes/${themeTitleToActOn}/.hash"
                                echo $currentThemeHash > "$UNPACKDIR"/themes/"$themeTitleToActOn"/.hash && addFile=1
                                if [ $addFile -eq 1 ]; then
                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Added hash successfully"
                                    chmod 755 "$UNPACKDIR"/themes/"$themeTitleToActOn"/.hash && [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Set hash file permissions"
                                    # Enable glob to match dot files.
                                    shopt -s dotglob
                                fi

                                # Delete theme.git dir from theme
                                if [ -d "$UNPACKDIR"/themes/"$themeTitleToActOn"/theme.git ]; then
                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Removing theme.git directory from ${UNPACKDIR}/themes/${themeTitleToActOn}"
                                    rm -rf "$UNPACKDIR"/themes/"$themeTitleToActOn"/theme.git
                                fi

                                # Create theme dir on target and move unpacked theme files to the target dir.
                                targetThemeDir="${TARGET_THEME_DIR}"/"$themeTitleToActOn"

                                if [ $isPathWriteable -eq 1 ]; then # Not Writeable
                                    if [ $(CheckOsVersion) -ge 13 ]; then
                                        successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                                                       /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡Move\" & \"‡$targetThemeDir\" & \"‡$UNPACKDIR\" & \"‡$themeTitleToActOn\" with administrator privileges" )
                                    else
                                        successFlag=$( /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡Move\" & \"‡$targetThemeDir\" & \"‡$UNPACKDIR\" & \"‡$themeTitleToActOn\" with administrator privileges" )
                                    fi  
                                else
                                    chckDir=0
                                    mkdir "$targetThemeDir" && chckDir=1
                                    if [ $chckDir -eq 1 ]; then
                                        # Move unpacked files to target theme path.
                                        cd "$UNPACKDIR"/themes
                                        if [ -d "$themeTitleToActOn" ]; then
                                            mv "$themeTitleToActOn"/* "$targetThemeDir" && WriteToLog "Installation was successful." && successFlag=0
                                        fi
                                    fi
                                fi

                                # Remove the unpacked files.
                                if [ -d "$UNPACKDIR"/themes ]; then
                                    rm -rf "$UNPACKDIR"/themes 
                                fi

                                # Disable glob to match dot files.
                                shopt -u dotglob
                            fi
                            ;;

               "UnInstall") WriteToLog "Deleting ${TARGET_THEME_DIR}/$themeTitleToActOn"

                            # Check if theme needs elevated privileges to remove
                            CheckPathIsWriteable "${TARGET_THEME_DIR}/$themeTitleToActOn"
                            local isPathWriteable=$? # 1 = not writeable / 0 = writeable

                            if [ $isPathWriteable -eq 1 ]; then # Not Writeable
                                if [ $(CheckOsVersion) -ge 13 ]; then
                                    successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                                                   /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡UnInstall\" & \"‡${TARGET_THEME_DIR}\" & \"‡$themeTitleToActOn\" with administrator privileges" )
                                else
                                    successFlag=$( /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡UnInstall\" & \"‡${TARGET_THEME_DIR}\" & \"‡$themeTitleToActOn\" with administrator privileges" )
                                fi 
                            else
                                cd "${TARGET_THEME_DIR}"
                                if [ -d "$themeTitleToActOn" ]; then
                                    rm -rf "$themeTitleToActOn" && WriteToLog "Deletion was successful." && successFlag=0
                                fi
                            fi
                            ;;

                "Update")   WriteToLog "Updating ${TARGET_THEME_DIR}/$themeTitleToActOn"

                            # Save current path
                            local currentPath=$( pwd )

                            # Check if bare git repo for this theme exists and delete if yes.
                            if [ -d "${WORKING_PATH}/${APP_DIR_NAME}"/"$themeTitleToActOn".git ]; then
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}bare repo for $themeTitleToActOn exists. Deleting"
                                cd "${WORKING_PATH}/${APP_DIR_NAME}"
                                rm -rf "$themeTitleToActOn".git
                            fi

                            # Clone theme from repo.
                            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating a bare git clone of $themeTitleToActOn"
                            local themeNameWithSpacesFixed=$( echo "$themeTitleToActOn" | sed 's/ /%20/g' )
                            cd "${WORKING_PATH}/${APP_DIR_NAME}"
                            feedbackCheck=$("$gitCmd" clone --progress --depth=1 --bare "$remoteRepositoryUrl"/themes.git/themes/"${themeNameWithSpacesFixed}"/theme.git "$themeTitleToActOn".git 2>&1 )
                            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Install git clone: $feedbackCheck"

                            # Checkout the bare repo to the unpack dir then replace on target dir.
                            if [ -d "${TARGET_THEME_DIR}"/"$themeTitleToActOn" ] && [ -d "${WORKING_PATH}/${APP_DIR_NAME}"/"$themeTitleToActOn".git ]; then

                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Force checking out bare git clone of ${themeTitleToActOn}."
                                cd "${WORKING_PATH}/${APP_DIR_NAME}"
                                feedbackCheck=$("$gitCmd" --git-dir="$themeTitleToActOn".git --work-tree="$UNPACKDIR" checkout --force 2>&1) && successFlag=0
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}checkout git clone: $feedbackCheck"

                                if [ $successFlag -eq 0 ]; then

                                    # Read current hash from packed-refs file.
                                    local currentThemeHash=$( cat "${WORKING_PATH}/${APP_DIR_NAME}"/"$themeTitleToActOn".git/packed-refs | grep refs/heads/master )
                                    currentThemeHash="${currentThemeHash:0:40}"
                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}hash=$currentThemeHash"

                                    # Write hash to file in to unpacked theme dir.
                                    local addFile=0
                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Writing hash to ${UNPACKDIR}/themes/${themeTitleToActOn}/.hash"
                                    echo $currentThemeHash > "$UNPACKDIR"/themes/"$themeTitleToActOn"/.hash && addFile=1
                                    if [ $addFile -eq 1 ]; then
                                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Added hash successfully"
                                        chmod 755 "$UNPACKDIR"/themes/"$themeTitleToActOn"/.hash && [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Set hash file permissions"
                                        # Enable glob to match dot files.
                                        shopt -s dotglob
                                    fi

                                    # Delete theme.git dir from theme
                                    if [ -d "$UNPACKDIR"/themes/"$themeTitleToActOn"/theme.git ]; then
                                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Removing theme.git directory from ${UNPACKDIR}/themes/${themeTitleToActOn}"
                                        rm -rf "$UNPACKDIR"/themes/"$themeTitleToActOn"/theme.git
                                    fi

                                    targetThemeDir="${TARGET_THEME_DIR}"/"$themeTitleToActOn"

                                    if [ $isPathWriteable -eq 1 ]; then # Not Writeable
                                       if [ $(CheckOsVersion) -ge 13 ]; then
                                            successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                                                           /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡Update\" & \"‡$targetThemeDir\" & \"‡$UNPACKDIR\" & \"‡$themeTitleToActOn\" with administrator privileges" )
                                        else
                                            successFlag=$( /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡Update\" & \"‡$targetThemeDir\" & \"‡$UNPACKDIR\" & \"‡$themeTitleToActOn\" with administrator privileges" )
                                        fi
                                    else
                                        if [ -d "$targetThemeDir" ]; then
                                            chckDir=0
                                            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Removing existing $targetThemeDir files"
                                            rm -rf "$targetThemeDir"/* && chckDir=1
                                            if [ $chckDir -eq 1 ]; then
                                                # Move unpacked files to target theme path.
                                                cd "$UNPACKDIR"/themes
                                                if [ -d "$themeTitleToActOn" ]; then
                                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Moving updated $themeTitleToActOn theme files to $targetThemeDir"
                                                    mv "$themeTitleToActOn"/* "$targetThemeDir" && WriteToLog "Updating was successful." && successFlag=0
                                                fi
                                            fi
                                        fi
                                    fi
                                    # Remove the unpacked files.
                                    if [ -d "$UNPACKDIR"/themes ]; then
                                        rm -rf "$UNPACKDIR"/themes 
                                    fi

                                    # Disable glob to match dot files.
                                    shopt -u dotglob
                                fi
                            fi

                            # change back to previous directory
                            cd "$currentPath"
                            ;;
    esac

    # Was operation a success?
    if [ $successFlag -eq 0 ]; then
        if [ $COMMANDLINE -eq 0 ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$themeTitleToActOn : ${passedAction} : Success"
            #[[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Success@${passedAction}@$themeTitleToActOn"
            #SendToUI "Success@${passedAction}@$themeTitleToActOn"

            if [ "$passedAction" == "Install" ] && [ "$TARGET_THEME_PARTITIONGUID" != "$zeroUUID" ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Saving settings for newly installed theme."
                SendToUI "Success‡${passedAction}‡$themeTitleToActOn"
                # Save new theme details for adding to prefs file
                gNewInstalledThemeName="$themeTitleToActOn"
                gNewInstalledThemePath="$TARGET_THEME_DIR"
                gNewInstalledThemePathDevice="$TARGET_THEME_DIR_DEVICE"
                gNewinstalledThemePartitionGUID="$TARGET_THEME_PARTITIONGUID"
            fi

            if [ "$passedAction" == "UnInstall" ] && [ "$TARGET_THEME_PARTITIONGUID" != "$zeroUUID" ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Saving settings for UnInstalled theme."
                SendToUI "Success‡${passedAction}‡$themeTitleToActOn"
                # Save new theme details for adding to prefs file
                gUnInstalledThemeName="$themeTitleToActOn"
                gUnInstalledThemePath="$TARGET_THEME_DIR"
                gUnInstalledThemePathDevice="$TARGET_THEME_DIR_DEVICE"
                gUninstalledThemePartitionGUID="$TARGET_THEME_PARTITIONGUID"
            fi     

            # Record what theme was installed where.
            MaintainInstalledThemeListInPrefs

            if [ "$passedAction" == "UnInstall" ]; then
                # Delete <theme name>.git from local support directory if no longer needed
                CheckIfThemeNoLongerInstalledThenDeleteLocalTheme "$themeTitleToActOn"
            fi
        fi
        return 0
    else
        if [ $COMMANDLINE -eq 0 ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$themeTitleToActOn : ${passedAction} : Fail"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Fail@${passedAction}@$themeTitleToActOn"
            SendToUI "Fail‡${passedAction}‡$themeTitleToActOn"
        fi
        return 1
    fi
}

# ---------------------------------------------------------------------------------------
CreateThemeListHtml()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CreateThemeListHtml()"

    # Build html for each theme.    
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating html theme list."
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Number of theme titles=${#themeTitle[@]}"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Number of theme description=${#themeDescription[@]}"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Number of theme author=${#themeAuthor[@]}"

    local imageFormat="png"

    if [ ${#themeTitle[@]} -eq ${#themeDescription[@]} ] && [ ${#themeTitle[@]} -eq ${#themeAuthor[@]} ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found ${#themeTitle[@]} Titles, Descriptions and Authors"
        for ((n=0; n<${#themeTitle[@]}; n++ ));
        do
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating html for ${themeTitle[$n]} theme"
            themeHtml="$themeHtml"$(printf "    <div id=\"ThemeBand\" class=\"accordion\">\r")
            themeHtml="$themeHtml"$(printf "        <div id=\"ThemeItems\">\r")

            if [ -f "${TEMPDIR}/themes/${themeTitle[$n]}/Theme.svg" ]; then
              themeHtml="$themeHtml"$(printf "            <div class=\"thumbnail\"><img src=\"${TEMPDIR}/themes/${themeTitle[$n]}/Theme.svg\" onerror=\"imgErrorThumb(this);\"></div>\r")
            elif [ -f "${TEMPDIR}/themes/${themeTitle[$n]}/screenshot.$imageFormat" ]; then
              themeHtml="$themeHtml"$(printf "            <div class=\"thumbnail\"><img src=\"${TEMPDIR}/themes/${themeTitle[$n]}/screenshot.$imageFormat\" onerror=\"imgErrorThumb(this);\"></div>\r")
            fi

            themeHtml="$themeHtml"$(printf "            <div id=\"ThemeText\"><p class=\"themeTitle\">${themeTitle[$n]}<br><span class=\"themeDescription\">${themeDescription[$n]}</span><br><span class=\"themeAuthor\">${themeAuthor[$n]}</span></p></div>\r")
            themeHtml="$themeHtml"$(printf "            <div class=\"versionControl\" id=\"indicator_${themeTitle[$n]}\"></div>\r")
            themeHtml="$themeHtml"$(printf "            <div class=\"buttonInstall\" id=\"button_${themeTitle[$n]}\"></div>\r")
            themeHtml="$themeHtml"$(printf "        </div> <!-- End ThemeItems -->\r")
            themeHtml="$themeHtml"$(printf "    </div> <!-- End ThemeBand -->\r")

            if [ -f "${TEMPDIR}/themes/${themeTitle[$n]}/Theme.svg" ]; then
              themeHtml="$themeHtml"$(printf "    <div class=\"accordionContent\"><img src=\"${TEMPDIR}/themes/${themeTitle[$n]}/Theme.svg\" onerror=\"imgErrorPreview(this);\" width=\"100%%\"></div>\r")
            elif [ -f "${TEMPDIR}/themes/${themeTitle[$n]}/screenshot.$imageFormat" ]; then
              themeHtml="$themeHtml"$(printf "    <div class=\"accordionContent\"><img src=\"${TEMPDIR}/themes/${themeTitle[$n]}/screenshot.$imageFormat\" onerror=\"imgErrorPreview(this);\" width=\"100%%\"></div>\r")
            fi

            themeHtml="$themeHtml"$(printf "\r")
        done
        WriteToLog "CTM_ThemeListOK"
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Error: Title(${#themeTitle[@]}), Author(${#themeAuthor[@]}), Description(${#themeDescription[@]}) mismatch."
        for ((n=0; n<${#themeTitle[@]}; n++ ));
        do
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$n : ${themeTitle[$n]} | ${themeDescription[$n]} | ${themeAuthor[$n]}"
        done
        WriteToLog "CTM_ThemeListFail"
    fi
}

# ---------------------------------------------------------------------------------------
InsertThemeListHtmlInToManageThemes()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}InsertThemeListHtmlInToManageThemes()"

    local passedOptionalCommand="$1"
    local check=1

    if [ "$passedOptionalCommand" == "file" ]; then
        # Read previously saved file
        themeHtml=$( cat "${WORKING_PATH}/${APP_DIR_NAME}"/theme.html )
        # Escape all ampersands
        themeHtml=$( echo "$themeHtml" | sed 's/&/\\\&/g' );
    else
        # Use internal string var
        # Escape forward slashes
        themeHtml=$( echo "$themeHtml" | sed 's/\//\\\//g' )
        # Save html to file
        echo "$themeHtml" > "${WORKING_PATH}/${APP_DIR_NAME}"/theme.html
    fi

    # Insert Html in to placeholder
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting HTML in to managethemes.html"
    LANG=C sed -ie "s/<!--INSERT_THEMES_HERE-->/${themeHtml}/g" "${TEMPDIR}"/managethemes.html && check=0

    # Clean up
    if [ -f "${TEMPDIR}"/managethemes.htmle ]; then
        rm "${TEMPDIR}"/managethemes.htmle
    fi

    # Add messages in to log for initialise.js to detect.
    if [ $check -eq 0 ]; then
        WriteToLog "CTM_InsertHtmlOK"
    else
        WriteToLog "CTM_InsertHtmlFail"
    fi
}

# ---------------------------------------------------------------------------------------
SetControlOptionHtmlSections()
{
    local disabledConfigPlist=""
    local nvramText=""
    local configPlistText=""

    if [ "$gNvramPlistFullPath" == "Native NVRAM" ]; then
        nvramText="Native"
    else
        if [ ! "$gNvramPlistFullPath" == "" ]; then
            # nvram.plist found in bootlog
            nvramText="using file nvram.plist"
        fi
    fi

    if [ -f "$gConfigPlistFullPath" ]; then
        configPlistText=$( RenameInternalESPMountPointToEFI "$gConfigPlistFullPath" )
    else
        configPlistText="$gConfigPlistFullPath"
        disabledConfigPlist="disabled"
    fi

    # Escape paths
    nvramText=$( echo "$nvramText" | sed 's/\//\\\//g' )
    configPlistText=$( echo "$configPlistText" | sed 's/\//\\\//g' )

    # Set html sections
    ctOuterOpen=$(printf "    <div id=\"changeThemeContainer\">\r")
    ctOpen=$(printf "        <div id=\"changeThemeBand\" class=\"fillDarkerGrey\">\r")
    ctOpenTitle=$(printf "        <div id=\"changeThemeBandTitle\" class=\"fillVeryDarkGrey\">\r")

    ctBandTitle=$(printf "            <div class=\"ctOptionDescription\" id=\"ctTitleDescription\">Set which theme to use next boot. (Note: Any NVRAM entry will take precedence over config.plist)<\/div>\r")

    # Build NVRAM band
    ctBandNvram=$(printf "            <div class=\"ctOptionTitleHeader\">NVRAM:<\/div>\r")
    ctBandNvram="$ctBandNvram"$(printf "            <div class=\"ctOptionTitleResult\" id=\"ctTitleNvram\">${nvramText}<\/div>\r")

    # UEFI boot with Native NVRAM   or   Legacy boot AND Launch Daemon & rc scripts working
    if [[ "$gNvramPlistFullPath" == "Native NVRAM" ]] || [[ "$gBootType" == "Legacy" && $gNvramSave -eq 0 ]]; then
        ctBandNvram="$ctBandNvram"$(printf "            <div class=\"ctOptionSetHeader\">Selected Theme:<\/div>\r")
        ctBandNvram="$ctBandNvram"$(printf "            <select id=\"installedThemeDropDownNvram\" class=\"changeThemeDropdown\" tabindex=\"14\">\r")
        ctBandNvram="$ctBandNvram"$(printf "                <!--Menu entries will be appended here by cloverthememanager.js -->\r")
        ctBandNvram="$ctBandNvram"$(printf "            <\/select>\r")
    else
        ctBandNvram="$ctBandNvram"$(printf "            <div class=\"ctOptionSetHeader\">Action: Install Clover rc scripts to change<\/div>\r")
    fi

    # Build Config.plist band
    ctBandConfig=$(printf "            <div class=\"ctOptionTitleHeader\">CONFIG:<\/div>\r")
    if [ -z "$disabledConfigPlist" ]; then
        ctBandConfig="$ctBandConfig"$(printf "            <div class=\"ctOptionTitleResult\" id=\"ctTitleConfig\">${configPlistText}<\/div>\r")
    else
        ctBandConfig="$ctBandConfig"$(printf "            <div class=\"ctOptionTitleResultNotAvailable\" id=\"ctTitleConfig\">${configPlistText}<\/div>\r")
    fi
    ctBandConfig="$ctBandConfig"$(printf "            <div class=\"ctOptionSetHeader\">Selected Theme:<\/div>\r")
    ctBandConfig="$ctBandConfig"$(printf "            <select id=\"installedThemeDropDownConfigP\" class=\"changeThemeDropdown\" tabindex=\"15\" ${disabledConfigPlist}>\r")
    ctBandConfig="$ctBandConfig"$(printf "                <!--Menu entries will be appended here by cloverthememanager.js -->\r")
    ctBandConfig="$ctBandConfig"$(printf "            <\/select>\r")

    # Build next boot band
    ctNextBootTheme=$(printf "            <div class=\"ctPredictionTitleHeader\">PREDICTION:<\/div>\r")
    ctNextBootTheme="$ctNextBootTheme"$(printf "            <div class=\"ctPredictionTitleResult\" id=\"ctTitleNvram\">Theme to be used for next boot from device $TARGET_THEME_DIR_DEVICE<\/div>\r")
    ctNextBootTheme="$ctNextBootTheme"$(printf "            <div class=\"ctOptionEntryHeader\">Entry:<\/div>\r")
    ctNextBootTheme="$ctNextBootTheme"$(printf "            <div id=\"themePredictionTheme\">\r")
    ctNextBootTheme="$ctNextBootTheme"$(printf "                    <span class=\"predictionText\" id=\"predictionTheme\"><\/span>\r")
    ctNextBootTheme="$ctNextBootTheme"$(printf "            <\/div> <!-- End themePredictionTheme -->\r")

    ctClose=$(printf "        <\/div> <!-- End changeThemeBand -->\r")
    ctOuterClose=$(printf "    <\/div> <!-- End changeThemeContainer -->")     
}

# ---------------------------------------------------------------------------------------
CreateControlOptionsHtmlAndInsert()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CreateControlOptionsHtmlAndInsert()"

    local check=1
    SetControlOptionHtmlSections

    local controlOptionsHtml="${ctOuterOpen}"

    # Add section title band
    controlOptionsHtml="${controlOptionsHtml}${ctOpenTitle}${ctBandTitle}${ctClose}"

    # Add nvram control band if UEFI boot with Native NVRAM   or   Legacy boot AND Launch Daemon & rc scripts working
    #if [[ "$gNvramPlistFullPath" == "Native NVRAM" ]] || [[ "$gBootType" == "Legacy" && $gNvramSave -eq 0 ]]; then
    #    controlOptionsHtml="${controlOptionsHtml}${ctOpen}${ctBandNvram}${ctClose}"
    #fi

    # Add nvram.plist control band if not using Native NVRAM AND Launch Daemon & rc scripts are not working
    #if [ "$gNvramPlistFullPath" != "" ] && [ "$gNvramPlistFullPath" != "Native NVRAM" ] && [ $gNvramSave -eq 1 ]; then
    #    controlOptionsHtml="${controlOptionsHtml}${ctOpen}${ctBandNvramP}${ctClose}"
    #fi

    controlOptionsHtml="${controlOptionsHtml}${ctOpen}${ctBandNvram}${ctClose}"

    # Add config.plist control band
    if [ "$gConfigPlistFullPath" != "" ]; then
        controlOptionsHtml="${controlOptionsHtml}${ctOpen}${ctBandConfig}${ctClose}"
    fi

    # Add next boot theme prediction band
    if [ "$gConfigPlistFullPath" != "" ]; then
        controlOptionsHtml="${controlOptionsHtml}${ctOpen}${ctNextBootTheme}${ctClose}"
    fi

    controlOptionsHtml="${controlOptionsHtml}${ctOuterClose}"

    # Insert control options band Html in to placeholder
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting control options band HTML in to managethemes.html"
    LANG=C sed -ie "s/<!--INSERT_CONTROL_OPTIONS_BAND_HERE-->/${controlOptionsHtml}/g" "${TEMPDIR}"/managethemes.html && check=0

    # Clean up
    if [ -f "${TEMPDIR}"/managethemes.htmle ]; then
        rm "${TEMPDIR}"/managethemes.htmle
    fi

    # Add messages in to log for initialise.js to detect.
    if [ $check -eq 0 ]; then
        WriteToLog "CTM_ControlOptionsOK"
    else
        WriteToLog "CTM_ControlOptionsFail"
    fi
}

# ---------------------------------------------------------------------------------------
InsertNotificationCodeInToJS()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}InsertNotificationCodeInToJS()"

    local check=1

    codeToInsert="macgap.notice.notify({ title: 'Clover Theme Manager', content: messageBody, sound: true});"

    # Insert Html in to placeholder
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting JS notification code in to cloverthememanager.js"
    LANG=C sed -ie "s/\/\/ INSERT_NOTIFICATION_CODE_HERE/${codeToInsert}/g" "${TEMPDIR}"/scripts/cloverthememanager.js && check=0

    # Clean Up
    if [ -f "${TEMPDIR}"/scripts/cloverthememanager.jse ]; then
        rm "${TEMPDIR}"/scripts/cloverthememanager.jse
    fi

    # Add messages in to log for initialise.js to detect.
    if [ $check -eq 0 ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting notification code was successful."
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting notification code failed."
    fi
}

# ---------------------------------------------------------------------------------------
CheckOsVersion()
{
    local osVer=$( uname -r )
    echo ${osVer%%.*}
}


# =======================================================================================
# Routines for checking ownership and elevating privileges, if necessary
# =======================================================================================


# ---------------------------------------------------------------------------------------
ResolveVolumePathFromGUID()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ResolveVolumePathFromGUID()"

    # Resolve volume path from GUID
    local volumePath=""

    # MBR partition scheme does not use GUID's so check
    if [ "$1" != "$zeroUUID" ]; then
        for (( u=0; u<${#themeDirPaths[@]}; u++ ))
        do
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Matching ${duPartitionGuid[$u]} : $1"
            if [[ "${duPartitionGuid[$u]}" == "$1" ]]; then
                volumePath="${themeDirPaths[$u]}"
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Match: Volume Path=$volumePath"
                break
            fi
        done
    else
        volumePath=""
    fi
    echo "$volumePath"
}
    
# ---------------------------------------------------------------------------------------
CheckPathIsWriteable()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckPathIsWriteable()"

    local passedMountPoint="$1"     
    local isWriteable=1

    if [ "$passedMountPoint" != "" ]; then
        touch "$passedMountPoint"/.test 2>/dev/null && rm -f "$passedMountPoint"/.test || isWriteable=0
    fi

    if [ $isWriteable -eq 0 ]; then
        return 1
    else
        return 0
    fi
}

# ---------------------------------------------------------------------------------------
FindArrayIdFromTarget()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}FindArrayIdFromTarget()"

    local success=0
    for ((a=0; a<${#duIdentifier[@]}; a++))
    do
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Does ${duPartitionGuid[$a]}=${TARGET_THEME_PARTITIONGUID} && ${themeDirPaths[$a]}=${TARGET_THEME_DIR}"
        if [ "${duPartitionGuid[$a]}" == "${TARGET_THEME_PARTITIONGUID}" ] && [ "${themeDirPaths[$a]}" == "${TARGET_THEME_DIR}" ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Match found. Returning $a"
            echo $a
            success=1
            break
        fi 
    done

    [[ $success -eq 0 ]] && echo -
}


# =======================================================================================
# Initialisation Routines
# =======================================================================================



# ---------------------------------------------------------------------------------------
ReadRepoUrlList()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadRepoUrlList()"

    WriteToLog "Looking for URL list"
    if [ -f "$gThemeRepoUrlFile" ]; then
        WriteToLog "Reading URL list"
        oIFS="$IFS"; IFS=$'\n'
        while read -r line
        do
            if [ ! "${line:0:1}" == "#" ]; then
                WriteToLog "Found URL $line"
                repositoryUrls+=( "${line##*#}" )
            fi
        done < "$gThemeRepoUrlFile"
        IFS="$oIFS"
        WriteToLog "Number of repositories found: ${#repositoryUrls[@]}"
    else
        WriteToLog "$gThemeRepoUrlFile not found."
    fi
}

# ---------------------------------------------------------------------------------------
RefreshHtmlTemplates()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}RefreshHtmlTemplates()"

    passedTemplate="$1"
    local check=1

    # For now remove previous managethemes.html and copy template
    if [ -f "${PUBLIC_DIR}"/$passedTemplate ]; then
        if [ -f "${PUBLIC_DIR}"/$passedTemplate.template ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Setting $passedTemplate to default."
            rm "${PUBLIC_DIR}"/$passedTemplate
            #cp "${PUBLIC_DIR}"/$passedTemplate.template "${PUBLIC_DIR}"/$passedTemplate && check=0
            cp "${PUBLIC_DIR}"/$passedTemplate.template "${TEMPDIR}"/$passedTemplate && check=0
        else
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Error: missing ${PUBLIC_DIR}/$passedTemplate.template"
        fi
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating: $passedTemplate"
        #cp "${PUBLIC_DIR}"/$passedTemplate.template "${PUBLIC_DIR}"/$passedTemplate && check=0
        cp "${PUBLIC_DIR}"/$passedTemplate.template "${TEMPDIR}"/$passedTemplate && check=0
    fi

    # Add message in to log for initialise.js to detect.
    if [ $check -eq 0 ]; then
        WriteToLog "CTM_HTMLTemplateOK"
    else
        WriteToLog "CTM_HTMLTemplateFail"
    fi
}

# ---------------------------------------------------------------------------------------
IsRepositoryLive()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}IsRepositoryLive()"

    local noConnection=1
    #local gitRepositoryUrl=$( echo ${remoteRepositoryUrl}/ | sed 's/http:/git:/' )
    local gitRepositoryUrl=$remoteRepositoryUrl

    WriteToLog "CTM_RepositoryCheckNetwork"
    WriteToLog "CTM_RepositoryChecking"

    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Connecting to ${gitRepositoryUrl}/themes"

    local testConnection=$( "$gitCmd" ls-remote ${gitRepositoryUrl}/themes )
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$testConnection"

    if [ "$testConnection" ]; then
        WriteToLog "CTM_RepositorySuccess"
    else
        WriteToLog "${debugIndentTwo}git ls-remote failed: $testConnection"
        noConnection=0
    fi

    if [ $noConnection -eq 0 ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Cannot contact the Repository ${gitRepositoryUrl}/themes"
        WriteToLog "CTM_RepositoryError" # initialise.js should pick this up, notify the user, then quit.
        exit 1
    fi
}

# ---------------------------------------------------------------------------------------
EnsureLocalSupportDir()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}EnsureLocalSupportDir()"

    # Check for local support directory
    local pathToCreate="${WORKING_PATH}/${APP_DIR_NAME}"
    if [ ! -d "$pathToCreate" ]; then
        WriteToLog "Creating $pathToCreate"
        mkdir -p "$pathToCreate"
    fi

    # Create unpacking directory for checking out cloned bare theme repo's
    # from clover repo. This is because the themes checkout as:
    # /path/to/EFI/Clover/Themes/<theme>/themes/<theme>/
    if [ ! -d "$UNPACKDIR" ]; then
        mkdir "$UNPACKDIR"
    fi

    # Add message in to log for initialise.js to detect.
    if [ -d "$pathToCreate" ] && [ -d "$UNPACKDIR" ]; then
        WriteToLog "CTM_SupportDirOK"
    else
        WriteToLog "CTM_SupportDirFail"
    fi
}

# ---------------------------------------------------------------------------------------
EnsureSymlinks()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}EnsureSymlinks()"

    # Rather than check if a valid one exists, it's quicker to simply re-create it.
    if [ -h "$ASSETS_DIR"/themes ] || [ -L "$ASSETS_DIR"/themes ]; then
        rm "$ASSETS_DIR"/themes
    fi

    if [ -h "$PUBLIC_DIR"/add_theme.html ] || [ -L "$PUBLIC_DIR"/add_theme.html ]; then
        rm "$PUBLIC_DIR"/add_theme.html
    fi

    CreateSymbolicLinks
}

# ---------------------------------------------------------------------------------------
DownloadPublicDirFromServer()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}DownloadPublicDirFromServer()"

    # Remove app files from a previous run
    if [ -d "${WORKING_PATH}/${APP_DIR_NAME}"/CloverThemeManagerApp ]; then
        WriteToLog "Removing previous CloverThemeManagerApp directory"
        rm -rf "${WORKING_PATH}/${APP_DIR_NAME}"/CloverThemeManagerApp
    fi

    # Download public dir
    local success=0
    local filePath="CloverThemeManagerApp/CloverThemeManager/public"
    local pathToWorkingPublicDir="${WORKING_PATH}/${APP_DIR_NAME}"/CloverThemeManagerApp/CloverThemeManager/public
    #local gitRepositoryUrl=$( echo ${remoteRepositoryUrl}/ | sed 's/http:/git:/' )
    local gitRepositoryUrl=$remoteRepositoryUrl
    cd "${WORKING_PATH}/${APP_DIR_NAME}"
    "$gitCmd" archive --remote="${gitRepositoryUrl}/themes" HEAD "$filePath" | tar -x && success=1
    if [ $success -eq 1 ]; then
        WriteToLog "Downloading app files from the repo was successful."
        return 0
    else
        WriteToLog "Error. Downloading app files from the repo failed."
        return 1
    fi
}

# ---------------------------------------------------------------------------------------
GetLatestIndexAndEnsureThemeHtml()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}GetLatestIndexAndEnsureThemeHtml()"

    BuildThemeTextInformation()
    {
        # Read local theme.plists and parse author and description info.
        # Create array of directory list alphabetically
        oIFS="$IFS"; IFS=$'\r\n'
        themeList=( $( ls -d "${WORKING_PATH}/${APP_DIR_NAME}"/themes/* | sort -f ))

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Reading theme plists."

        # Read each themes' Author & Description.
        for ((n=0; n<${#themeList[@]}; n++ ));
        do
            tmpTitle="${themeList[$n]##*/}"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Reading theme plists for $tmpTitle" 
            themeTitle+=("$tmpTitle")

            if [ -f "${WORKING_PATH}/${APP_DIR_NAME}/themes/${tmpTitle}/theme.plist" ]; then

              # Read theme.plist to extract Author & Description.

              themeAuthor+=( $(FindStringInPlist "Author" "${WORKING_PATH}/${APP_DIR_NAME}/themes/${tmpTitle}/theme.plist"))
              themeDescription+=( $(FindStringInPlist "Description" "${WORKING_PATH}/${APP_DIR_NAME}/themes/${tmpTitle}/theme.plist"))

            elif [ -f "${WORKING_PATH}/${APP_DIR_NAME}/themes/${tmpTitle}/theme.svg" ]; then
            
              # convert to unix line endings and extract Author & Description.

              tmpAuthor=$( tr '\r' '\n' < "${WORKING_PATH}/${APP_DIR_NAME}/themes/${tmpTitle}/theme.svg" | grep "Author=" )
              tmpAuthor="${tmpAuthor#*=\"}"
              tmpAuthor="${tmpAuthor%*\"}"
              themeAuthor+=("$tmpAuthor")

              tmpDescription=$( tr '\r' '\n' < "${WORKING_PATH}/${APP_DIR_NAME}/themes/${tmpTitle}/theme.svg" | grep "Description=" )
              tmpDescription="${tmpDescription#*=\"}"
              tmpDescription="${tmpDescription%*\"}"
              themeDescription+=("$tmpDescription")

            fi
        done
        IFS="$oIFS"
    }

    CloneAndCheckoutIndex()
    {
        local check=1

        # Remove index.git from a previous run
        if [ -d "${WORKING_PATH}/${APP_DIR_NAME}"/index.git ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Removing previous index.git"
            rm -rf "${WORKING_PATH}/${APP_DIR_NAME}"/index.git
        fi

        # Remove any images from a previous run
        if [ -d "${WORKING_PATH}/${APP_DIR_NAME}"/images ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Removing previous index images directory"
            rm -rf "${WORKING_PATH}/${APP_DIR_NAME}"/images
        fi

        # Remove any theme.plists from a previous run
        if [ -d "${WORKING_PATH}/${APP_DIR_NAME}"/themes ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Removing previous index themes directory"
            rm -rf "${WORKING_PATH}/${APP_DIR_NAME}"/themes
        fi

        # Get new index.git from CloverRepo
        cd "${WORKING_PATH}/${APP_DIR_NAME}"
        WriteToLog "CTM_IndexCloneAndCheckout"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Cloning bare repo index.git"
        "$gitCmd" clone --depth=1 --bare "$remoteRepositoryUrl"/themes.git/index.git
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Checking out index.git"
        "$gitCmd" --git-dir="${WORKING_PATH}/${APP_DIR_NAME}"/index.git --work-tree="${WORKING_PATH}/${APP_DIR_NAME}" checkout --force && check=0

        # Add message in to log for initialise.js to detect.
        if [ $check -eq 0 ]; then
            WriteToLog "CTM_IndexOK"
        else
            WriteToLog "CTM_IndexFail"
        fi
    }

    GetIndexAndProcessThemeList()
    {
        CloneAndCheckoutIndex
        BuildThemeTextInformation
        CreateThemeListHtml
        InsertThemeListHtmlInToManageThemes
    }

    if [ ! -d "${WORKING_PATH}/${APP_DIR_NAME}"/index.git ]; then
        GetIndexAndProcessThemeList
    else
        # Check existing index.git is not older than when repo was rebuilt
        # Clover Theme Repo was rebuilt on 14th December 2014. Any index.git
        # from before then will not fetch and needs to be deleted.
        # To be safe I am using 15th December 2104 as date to check.
        # epoch for that is calculated with: date -j -f "%d-%B-%y" 15-DEC-14 +%s
        # Giving epoch of: 1418667240
        repoRebuildEpoch=1500708860

        # Get epoch of existing index.git
        indexFileEpoch=$( stat -f "%m" "${WORKING_PATH}/${APP_DIR_NAME}"/index.git )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}indexFileEpoch=$indexFileEpoch"
        if [ $indexFileEpoch -lt $repoRebuildEpoch ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}index.git is from before repo was rebuilt"
            GetIndexAndProcessThemeList
        else
            # Check for updates to index.git
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Checking for update to index.git"
            cd "${WORKING_PATH}/${APP_DIR_NAME}"/index.git
            local updateCheck=$( "$gitCmd" fetch --progress origin master:master 2>&1 )
            if [[ "$updateCheck" == *done.*  ]]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}index.git has been updated. Re-downloading"
                GetIndexAndProcessThemeList
            else
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}No updates to index.git"
                WriteToLog "CTM_IndexOK"

                # Commented out the below condition for now to force rebuild of file.
                # This is because we now check themeList array in CheckForThemeUpdates()
                # to see if a theme still exists on the Repo.

                # Use previously saved theme.html
                #if [ -f "${WORKING_PATH}/${APP_DIR_NAME}"/theme.html ]; then
                #    WriteToLog "CTM_ThemeListOK"
                #    InsertThemeListHtmlInToManageThemes "file"
                #else
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Error!. ${WORKING_PATH}/${APP_DIR_NAME}/theme.html not found"
                    BuildThemeTextInformation
                    CreateThemeListHtml
                    InsertThemeListHtmlInToManageThemes
                #fi 
            fi
        fi

        # Check for help directory and add_theme.html file.
        addThemeHelpFile=$( find "${WORKING_PATH}/${APP_DIR_NAME}"/ -type f -name "add_theme.html" 2>/dev/null )
        if [ ! "$addThemeHelpFile" ]; then
            "$gitCmd" --git-dir="${WORKING_PATH}/${APP_DIR_NAME}"/index.git --work-tree="${WORKING_PATH}/${APP_DIR_NAME}" checkout --force
        fi
    fi
}

# ---------------------------------------------------------------------------------------
GetFreeSpaceOfTargetDeviceAndSendToUI()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}GetFreeSpaceOfTargetDeviceAndSendToUI()"

    # Read available space on volume and send to the UI.
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Getting free space on target device $TARGET_THEME_DIR_DEVICE"

    oIFS="$IFS"; IFS=$'\r\n'
    deviceResult=( $( df -laH | grep "$TARGET_THEME_DIR_DEVICE" | awk '{print $1}' ))
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}deviceResult=$deviceResult"
    IFS="$oIFS"

    local found=99
    for (( d=0; d<${#deviceResult[@]}; d++ ))
    do
        if [ "${deviceResult[$d]##*/}" == "$TARGET_THEME_DIR_DEVICE" ]; then
           found=$d
           [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}found=$found"
        fi
    done

    if [ $found -lt 99 ]; then
        local freeSpace=$(df -laH | grep "$TARGET_THEME_DIR_DEVICE" | awk '{print $4}' | head -n$(( found + 1 )) | tail -n1)
    else
        local freeSpace="0M"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}*Couldn't get free space."
    fi

    WriteToLog "Freespace on target: $freeSpace"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: FreeSpace:$freeSpace"
    SendToUI "FreeSpace‡${freeSpace}‡"
}

# ---------------------------------------------------------------------------------------
ReadThemeDirList()
{
    # Read /tmp/CloverThemeManager/themeDirInfo.txt file and populate
    # arrays for theme directory information.
    # themeDirInfo.txt is created by findThemeDirs.sh script.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadThemeDirList()"

    ResetInternalDiskArrays

    if [ -f "$themeDirInfo" ]; then
        oIFS="$IFS"; IFS=$'\r\n'
        while read -r line
        do
            duIdentifier+=( $( cut -d@ -f1 <<<"${line}" ))
            duVolumeName+=( $( cut -d@ -f2 <<<"${line}" ))
            duVolumeMountPoint+=( $( cut -d@ -f3 <<<"${line}" ))
            duContent+=( $( cut -d@ -f4 <<<"${line}" ))
            duPartitionGuid+=( $( cut -d@ -f5 <<<"${line}" ))
            themeDirPaths+=( $( cut -d@ -f6 <<<"${line}" ))
        done < "$themeDirInfo"
        IFS="$oIFS"

        # Check array contents match and send message to UI via log
        local total=${#duIdentifier[@]}
        if [ ${#duVolumeName[@]} -ne $total ] || [ ${#duVolumeMountPoint[@]} -ne $total ] || \
           [ ${#duContent[@]} -ne $total ] || [ ${#duPartitionGuid[@]} -ne $total ] || [ ${#themeDirPaths[@]} -ne $total ]; then
            [[ $gInitialising -eq 0 ]] && WriteToLog "CTM_ThemeDirsFail" 

            # Print results
            for (( s=0; s<${#duIdentifier[@]}; s++ ))
            do
                WriteToLog "${duIdentifier[$s]} | ${duVolumeName[$s]} | ${duVolumeMountPoint[$s]} | ${duContent[$s]} | ${duPartitionGuid[$s]} | ${themeDirPaths[$s]}"
            done   
            exit 1 
        else
            [[ $gInitialising -eq 0 ]] && WriteToLog "CTM_ThemeDirsOK" 
        fi
    else
        WriteToLog "Missing $themeDirInfo file. No mounted volumes contain /EFI/Clover/Themes directory."
        WriteToLog "CTM_ThemeDirsOKFail"
    fi
}

# ---------------------------------------------------------------------------------------
SetTargetAndMountpoint()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SetTargetAndMountpoint()"

    if [ "$gBootDeviceIdentifier" != "" ] && [ "$gBootDeviceIdentifier" != "Failed" ]; then
        for (( t=0; t<${#duIdentifier[@]}; t++ ))
        do
            if [ "${duIdentifier[$t]}" == "$gBootDeviceIdentifier" ]; then
                TARGET_THEME_DIR="${themeDirPaths[$t]}"
                TARGET_THEME_DIR_DEVICE="${duIdentifier[$t]}"
                TARGET_THEME_PARTITIONGUID="${duPartitionGuid[$t]}"
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Set target volume to boot device: $gBootDeviceIdentifier"
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_DIR=$TARGET_THEME_DIR"
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_DIR_DEVICE=$TARGET_THEME_DIR_DEVICE"
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_PARTITIONGUID=$TARGET_THEME_PARTITIONGUID"
                gBootDeviceMountPoint="${TARGET_THEME_DIR%/EFI*}"
                # Send result to UI
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI message: BootDevice@Mounted@${gBootDeviceIdentifier}@${gBootDeviceMountPoint}"
                SendToUI "BootDevice‡Mounted‡${gBootDeviceIdentifier}‡${gBootDeviceMountPoint}"
                break
            fi
        done
    else
        # Send message to UI if identifier is set to 'Failed'. Otherwise a blank identifier could mean bootlog did not exist.
        if [ "$gBootDeviceIdentifier" == "Failed" ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI message: BootDevice@Failed@@"
            SendToUI "BootDevice‡Failed‡‡"
        fi
    fi
}

# ---------------------------------------------------------------------------------------
GetBootlog()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}GetBootlog()"

    local bootLog=$( ioreg -lw0 -pIODeviceTree | grep boot-log )
    #bootLog=${bootLog#*'<'}
    #bootLog=${bootLog%%'>'*}
    # Use cut instead of above as a large bootlog a couple of minutes removing > from end.
    bootLog=$(cut -d '<' -f2 <<< "$bootLog")
    bootLog=$(cut -d '>' -f1 <<< "$bootLog")

    if [ "$bootLog" != "" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found a bootlog in ioreg. Writing bootlog to disk"
        echo "$bootLog" > "${TEMPDIR}"/bootLogtmp
        xxd -r -p "${TEMPDIR}"/bootLogtmp > "${bootLogFile}_tmp" && rm "${TEMPDIR}"/bootLogtmp
        # Convert to Unix Line Feed endings
        tr -d '\r' < "${bootLogFile}_tmp" > "${bootLogFile}" && rm "${bootLogFile}_tmp"

        # Remove bootlog if not from Clover
        local checkLog=$( grep "Starting Clover" $bootLogFile )
        if [ "$checkLog" == "" ]; then
             [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}bootlog is not from Clover. Deleting"
             rm $bootLogFile
        fi
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}bootlog from ioreg is blank"
    fi
}

# ---------------------------------------------------------------------------------------
ReadBootLogAndSetPaths()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadBootLogAndSetPaths()"

    local runInfo="$1" # Will be either 'Init' or 'Rescan'

    # Run bootlog script to read bootlog for theme info.
    # This script builds the bootlog html and injects it into the managethemes template.
    "$bootlogScript" "$runInfo" "$gBootDeviceIdentifier" "$gBootDeviceMountPoint"

    # The bootlog script writes some paths (or text 'Native NVRAM') to file. Read them in.
    if [ -f "$bootlogScriptOutfile" ]; then
        gNvramPlistFullPath=$( grep "nvram‡" "$bootlogScriptOutfile" ) && gNvramPlistFullPath="${gNvramPlistFullPath##*‡}"
        gNvramPlistThemeEntry=$( grep "nvramThemeEntry‡" "$bootlogScriptOutfile" ) && gNvramPlistThemeEntry="${gNvramPlistThemeEntry##*‡}"
        gConfigPlistFullPath=$( grep "config‡" "$bootlogScriptOutfile" ) && gConfigPlistFullPath="${gConfigPlistFullPath##*‡}"
        gBootType=$( grep "bootType‡" "$bootlogScriptOutfile" ) && gBootType="${gBootType##*‡}"
        gNvramSave=$( grep "nvramSave‡" "$bootlogScriptOutfile" ) && gNvramSave="${gNvramSave##*‡}"

        # Honour the users choice to have the bootlog closed or expanded
        if [ "$gBootlogState" == "Close" ]; then
            gBootlogState="ShowClosed"
        else
            gBootlogState="Show"
        fi
    fi
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Returned back from bootlog script: gNvramPlistFullPath=$gNvramPlistFullPath | gConfigPlistFullPath=$gConfigPlistFullPath"
}

# ---------------------------------------------------------------------------------------
GetSelfDevicePath()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}GetSelfDevicePath()"

    if [ -f "$bootLogFile" ]; then

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Reading bootlog"

        local selfDevicePath=$( grep -a SelfDevicePath "$bootLogFile" )
        if [[ "$selfDevicePath" ]]; then

            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found selfDevicePath"

            # Will look something like these examples
            #0:999  0:000  SelfDevicePath=PcieRoot(0x0)\Pci(0x1F,0x2)\USB(0x0,0x0)\HD(1,MBR,0x00000000,0x2,0x3BB7BE) @1DBDFE98
            #0:100  0:000  SelfDevicePath=PciRoot(0x0)\Pci(0x1F,0x2)\Sata(0x0,0xFFFF,0x0)\HD(1,GPT,BC1B343C-2D6B-4C0C-8B88-71C2AFCF6E65,0x28,0x64000) @C7AA598
            #0:100 0:000 SelfDevicePath=PciRoot(0x0)\Pci(0x1F,0x2)\VenHw(CF31FAC5-C24E-11D2-85F3-00A0C93EC93B,02)\HD(1,GPT,AA434287-E363-4254-AC5A-27189F7BDCC0,0x28,0x64000) @97548018

            # Get device path and split in to parts
            devicePath="${selfDevicePath#*=}"
            declare -a devicePathArr
            IFS=$'\\'
            devicePathArr=($devicePath)
            IFS="$oIFS"

            #local deviceType="-"
            #deviceType="${devicePathArr[2]%(*}"

            if [ $DEBUG -eq 1 ]; then
                for ((i=0; i<${#devicePathArr[@]}; i++))
                do
                    WriteToLog "${debugIndentTwo}devicePathArr[$i]=${devicePathArr[$i]}"
                done
            fi

            idx=${#devicePathArr[@]}
            while [[ "${devicePathArr[$idx]}" != "HD("* ]]
            do
                ((idx--))
            done

            # Split HD in to parts
            devicePathHD="${devicePathArr[$idx]%)*}"
            devicePathHD="${devicePathHD#*(}"
            # Should be something like these examples:
            #1,MBR,0x2A482A48,0x2,0x4EFC1B80
            #2,GPT,F55D9AC4-08A8-4269-9A8E-396DBE7C7943,0x64028,0x1C0000
            declare -a hdArr
            IFS=$','
            hdArr=($devicePathHD)
            IFS="$oIFS"
            local bootDevicePartition="${hdArr[0]}"
            local bootDevicePartType="${hdArr[1]}"
            local bootDevicePartSignature="${hdArr[2]}"
            local bootDevicePartStart="${hdArr[3]}"
            local bootDevicePartSize="${hdArr[4]}"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}PartType=$bootDevicePartType"
            if [ "$bootDevicePartType" == "GPT" ]; then
                local identifier=$( "$partutil" --search-uuid $bootDevicePartSignature )
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}identifier=$identifier"
                # is this an unmounted ESP?
                if [ -f "$espList" ] && [ "$identifier" != "" ]; then
                    checkESP=$( grep "$identifier" "$espList" )
                    if [ "$checkESP" != "" ] && [[ "$checkESP" == *@U ]]; then
                        # Instruct UI to tell user that it needs password to mount ESP
                        WriteToLog "CTM_BootDeviceGPT"
                        identifier=$(MountESPAndSearchThemesPath "$identifier")
                    fi
                fi
                [[ "$identifier" == "" ]] && identifier="Failed"
            elif [ "$bootDevicePartType" == "MBR" ]; then
                # Convert device hex values to human readable
                local bootDevicePartStartDec=$(echo "ibase=16; ${bootDevicePartStart#*x}" | bc)
                local bootDevicePartSizeDec=$(echo "ibase=16; ${bootDevicePartSize#*x}" | bc)

                # Save boot device details to file
                echo "${bootDevicePartition}‡${bootDevicePartType}‡${bootDevicePartSignature}‡${bootDevicePartStartDec}‡${bootDevicePartSizeDec}" > "$bootDeviceInfo"

                # Instruct UI to tell user that it needs password for identifying MBR device
                WriteToLog "CTM_BootDeviceMBR"

                local identifier=$(DetectMBRDevice)
            fi

            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Boot device part type=${bootDevicePartType} | identifier=${identifier}"
            echo "$identifier"

        else
            echo ""
        fi
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$bootLog not found"
    fi
    WriteToLog "CTM_BootDeviceCloseWindow"
}

# ---------------------------------------------------------------------------------------
CreateAndSendVolumeDropDownMenu()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CreateAndSendVolumeDropDownMenu()"

    # Send new dropdown list for UI
    for (( p=0; p<${#themeDirPaths[@]}; p++ ))
    do
        # Check if mountpoint is temporary for ESP partition.
        # If yes then make it human readable by changing to EFI.
        if [[ "${themeDirPaths[$p]}" == *$gESPMountPrefix* ]]; then
            local tmpStrip="${themeDirPaths[$p]#*/}"
            tmpStrip="${tmpStrip#*/}"
            tmpStrip="${tmpStrip#*/}"
            pathToPrint="/Volumes/EFI/${tmpStrip}"
            espID=$p
        else
            pathToPrint="${themeDirPaths[$p]}"
        fi

        if [ "${duIdentifier[$p]}" == "$gBootDeviceIdentifier" ]; then
            pathToPrint="BOOT DEVICE | $pathToPrint"
        fi

        local newPathList="${newPathList}","${p};${pathToPrint} [${duIdentifier[$p]}] [${duPartitionGuid[$p]}]"
    done

    if [ "$newPathList" != "" ]; then
        # Remove leading comma from string
        newPathList="${newPathList#?}"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI message: NewVolumeDropDown@${newPathList}@"
        SendToUI "NewVolumeDropDown‡${newPathList}‡"
        return 0
    else
        # Still send UI DropDown message, even though there are no entries.
        SendToUI "NewVolumeDropDown‡‡"
        return 1
    fi
}

# ---------------------------------------------------------------------------------------
MountESPAndSearchThemesPath()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}MountESPAndSearchThemesPath()"

    local espMountedCount=0
    local identifier="$1"

    if [ $(CheckOsVersion) -ge 13 ]; then
        espMountedCount=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                           /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡ManageESP\" with administrator privileges" )
    else
        espMountedCount=$( /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡ManageESP\" with administrator privileges" )
    fi

    if [ $espMountedCount -gt 0 ]; then
        WriteToLog "Checked and found $espMountedCount ESP with /EFI/Clover/Themes dir."
        "$findThemeDirs"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI message: MessageESP@Mounted@${espMountedCount}"
        SendToUI "MessageESP‡Mounted‡${espMountedCount}"
        echo "$identifier"
    elif [ -z $espMountedCount ]; then
        WriteToLog "User cancelled password dialog"
        # Send UI message so the message box shows close box button.
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI message: MessageESP@Cancelled@0"
        SendToUI "MessageESP‡Cancelled‡0"
        echo "Failed"
    else
        WriteToLog "Checked and found no unmounted ESP(s) with /EFI/Clover/Themes dir."
        # Send UI message that no ESP's were mounted. This changes message box content and shows close box button.
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI message: MessageESP@Mounted@0"
        SendToUI "MessageESP‡Mounted‡0"
        echo "Failed"
    fi
}

# ---------------------------------------------------------------------------------------
DetectMBRDevice()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}DetectMBRDevice()"

    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}User selected to detect MBR device."

    local device=""
    if [ $(CheckOsVersion) -ge 13 ]; then
        device=$( /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡FindMBrBootDevice\" with administrator privileges" )
    else
        device=$( /usr/bin/osascript -e "do shell script \"$uiSudoChanges \" & \"‡FindMBrBootDevice\" with administrator privileges" )
    fi

    if [ "$device" != "" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found boot device. $deviceFound"
        echo "$device"
    elif [ -z $device ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}User cancelled password dialog"
        echo "Failed"
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Failed to match a mounted boot device."
        echo "Failed"
    fi
}

# ---------------------------------------------------------------------------------------
ReadPrefsFile()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadPrefsFile()"

    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Read user preferences file"
    # Check for preferences file
    if [ -f "$gUserPrefsFile".plist ]; then

        gLastSelectedPath=$( defaults read "$gUserPrefsFile" LastSelectedPath )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}gLastSelectedPath=$gLastSelectedPath"

        gLastSelectedPathDevice=$( defaults read "$gUserPrefsFile" LastSelectedPathDevice )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}gLastSelectedPathDevice=$gLastSelectedPathDevice"

        gLastSelectedPartitionGUID=$( defaults read "$gUserPrefsFile" LastSelectedPartitionGUID )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}gLastSelectedPartitionGUID=$gLastSelectedPartitionGUID"

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Resetting internal theme arrays"
        ResetInternalThemeArrays

        gBootlogState=$( defaults read "$gUserPrefsFile" ShowHideBootlog )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}gBootlogState=${gBootlogState}"

        local tmp=$( defaults read "$gUserPrefsFile" Thumbnail )
        if [ "$tmp" != "" ]; then
            gThumbSizeX="${tmp% *}"
            gThumbSizeY="${tmp#* }"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}ThumbnailSize=${gThumbSizeX}x${gThumbSizeY}"
        fi

        gUISettingViewUnInstalled=$( defaults read "$gUserPrefsFile" UnInstalledButton )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}gUISettingViewUnInstalled=${gUISettingViewUnInstalled}"

        gUISettingViewThumbnails=$( defaults read "$gUserPrefsFile" ViewThumbnails )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}gUISettingViewThumbnails=${gUISettingViewThumbnails}"

        gUISettingViewPreviews=$( defaults read "$gUserPrefsFile" ShowPreviewsButton )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}gUISettingViewPreviews=${gUISettingViewPreviews}"

        # Find installed themes
        oIFS="$IFS"; IFS=$'\n'
        local readVar=( $( defaults read "$gUserPrefsFile" InstalledThemes | grep = ) )
        IFS="$oIFS"

        # get total count of lines, less one for zero based index.
        local count=(${#readVar[@]}-1)
        foundThemeName=0
        for (( x=0; x<=$count; x++ ))
        do
            if [ $foundThemeName -eq 1 ] || [[ "${readVar[$x]}" == *ThemePath* ]]; then
                local tmpOption="${readVar[$x]%=*}"
                tmpOption="${tmpOption//[[:space:]]}"           # Remove whitespace
                local tmpValue="${readVar[$x]#*=}"
                tmpValue=$( echo "$tmpValue" | sed 's/^ *//')   # Remove leading whitespace  
                tmpValue=$( echo "$tmpValue" | tr -d '";' )     # Remove quotes and semicolon from the string
                case "$tmpOption" in
                           "ThemePath"       )   installedThemeName+=( "$themeName" )
                                                 installedThemePath+=("$tmpValue") ;;
                           "ThemePathDevice" )   installedThemePathDevice+=("$tmpValue") ;;
                           "VolumeUUID"      )   installedThemePartitionGUID+=("$tmpValue")
                                                 ;;
                esac
            fi

            # Look for an open parenthesis to indicate start of array entry
            if [[ "${readVar[$x]}" == *\(* ]]; then
                themeName="${readVar[$x]% =*}"                      # Remove all after ' ='    
                themeName=$( echo "$themeName" | sed 's/^ *//')     # Remove leading whitespace  
                themeName=$( echo "$themeName" | sed 's/\"//g' )    # Remove any quotes
                foundThemeName=1
            fi
        done

        # Add message in to log for initialise.js to detect.
        #[[ $gInitialising -eq 0 ]] && WriteToLog "CTM_ReadPrefsOK"
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Preferences file not found."
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Creating initial prefs file: $gUserPrefsFile"
        defaults write "$gUserPrefsFile" "LastSelectedPath" "-"
        defaults write "$gUserPrefsFile" "LastSelectedPathDevice" "-"
        defaults write "$gUserPrefsFile" "LastSelectedPartitionGUID" "-"
        TARGET_THEME_DIR="-"
        TARGET_THEME_DIR_DEVICE="-"
        TARGET_THEME_PARTITIONGUID="-"

        # Add message in to log for initialise.js to detect.
        WriteToLog "CTM_ReadPrefsCreate"
    fi

    if [ "$gBootlogState" == "" ]; then
        gBootlogState="Open"
    fi

    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_DIR=$TARGET_THEME_DIR"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_DIR_DEVICE=$TARGET_THEME_DIR_DEVICE"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_PARTITIONGUID=$TARGET_THEME_PARTITIONGUID"
    [[ DEBUG -eq 1 ]] && SendInternalThemeArraysToLogFile
}

# ---------------------------------------------------------------------------------------
MapLastSelectedPathToGUID()
{
    # Map $gLastSelectedPath against $gLastSelectedPartitionGUID to catch differences
    # which will occur when using ESP. Internal random mountpoint gets written to 
    # prefs as /Volumes/EFI because random mountpoint will not happen again.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}MapLastSelectedPathToGUID()"

    if [ "$gLastSelectedPartitionGUID" != "-" ] && [ "$gLastSelectedPath" != "-" ] && [ "$gLastSelectedPathDevice" != "-" ]; then

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Mapping last selected partition GUID against mounted partition GUIDs"
        local checkPath=""
        for (( u=0; u<${#themeDirPaths[@]}; u++ ))
        do
            # Note: Two MBR partitioned, FAT32 formatted USB sticks will both have zero UUID.
            if [ "${gLastSelectedPartitionGUID}" == "${zeroUUID}" ]; then
                if [ "${duPartitionGuid[$u]}" == "$zeroUUID" ]; then
                    # Attempt to match theme path
                    if [ "${themeDirPaths[$u]}" == "$TARGET_THEME_DIR" ]; then
                        checkPath="${themeDirPaths[$u]}"
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Match found:checkPath=$checkPath"
                        break
                    fi
                fi
            else
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Checking ${duPartitionGuid[$u]} = $gLastSelectedPartitionGUID"
                if [ "${duPartitionGuid[$u]}" == "$gLastSelectedPartitionGUID" ]; then
                    checkPath="${themeDirPaths[$u]}"
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Match found:checkPath=$checkPath"
                fi
            fi
        done

        if [ "$checkPath" != "" ]; then
            TARGET_THEME_DIR="$checkPath"
        else
            TARGET_THEME_DIR="$gLastSelectedPath"
        fi
        TARGET_THEME_DIR_DEVICE="$gLastSelectedPathDevice"
        TARGET_THEME_PARTITIONGUID="$gLastSelectedPartitionGUID"
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}No last selected partition is set."
        TARGET_THEME_DIR="-"
        TARGET_THEME_DIR_DEVICE="-"
        TARGET_THEME_PARTITIONGUID="-"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_DIR=$TARGET_THEME_DIR"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_DIR_DEVICE=$TARGET_THEME_DIR_DEVICE"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_PARTITIONGUID=$TARGET_THEME_PARTITIONGUID"
    fi
}

# ---------------------------------------------------------------------------------------
SendInternalThemeArraysToLogFile()
{
    # This is only called if DEBUG is set to 1
    # It will loop through the internal arrays for installed themes and
    # print them to the log file.
    # They arrays are saved to prefs in MaintainInstalledThemeListInPrefs()

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SendInternalThemeArraysToLogFile()"

    WriteLinesToLog
    local totalPath="${#installedThemePath[@]}"
    local totalPathDevice="${#installedThemePathDevice[@]}"
    local totalVolUuid="${#installedThemePartitionGUID[@]}"
    if [ $totalPath -ne $totalPathDevice ] && [ $totalPath -ne $totalVolUuid ]; then
        WriteToLog "${debugIndentTwo}Error. Preferences are corrupt"
        exit 1
    else
        WriteToLog "${debugIndentTwo}Prefs shows total number of installed themes=${#installedThemeName[@]}"
        for ((n=0; n<${#installedThemeName[@]}; n++ ));
        do
            WriteToLog "${debugIndentTwo}$n: ${installedThemeName[$n]}, ${installedThemePath[$n]}, ${installedThemePathDevice[$n]}, ${installedThemePartitionGUID[$n]}"
        done
    fi  
}

# ---------------------------------------------------------------------------------------
SendTargetToUiRunChecks()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SendTargetToUiRunChecks()"

    if [ ! "$TARGET_THEME_DIR" == "" ] && [ ! "$TARGET_THEME_DIR" == "-" ] ; then
        local entry=$( FindArrayIdFromTarget )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}entry=$entry"
        CheckThemePathIsStillValid
        retVal=$? # returns 1 if invalid / 0 if valid
        if [ $retVal -eq 0 ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Target@$entry"
            SendToUI "Target‡$entry"
            GetListOfInstalledThemesAndSendToUI
            GetFreeSpaceOfTargetDeviceAndSendToUI 
            
            # Run this regardless of path chosen as JS is waiting to hear it.
            CheckAndRecordUnManagedThemesAndSendToUI 
        fi
    fi
}

# ---------------------------------------------------------------------------------------
SendUIInitData()
{
    # This is called once after much of the initialisation routines have run.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SendUIInitData()"

    # Send UI setting for BootlogView and adjust footer height
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: BootlogView@${gBootlogState}@"
    SendToUI "BootlogView‡${gBootlogState}‡"

    SendTargetToUiRunChecks

    if [ "$TARGET_THEME_DIR" == "" ] || [ "$TARGET_THEME_DIR" == "-" ] ; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: NoPathSelected@@"
        SendToUI "NoPathSelected‡‡"
        
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Target@-@"
        SendToUI "Target‡-‡"
        
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: InstalledThemes@-@"
        SendToUI "InstalledThemes‡-‡"
    fi

    # Send thumbnail size
    if [ $gThumbSizeX -gt 0 ] && [ $gThumbSizeY -gt 0 ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: ThumbnailSize@${gThumbSizeX}@${gThumbSizeY}"
        SendToUI "ThumbnailSize‡${gThumbSizeX}‡${gThumbSizeY}"
    fi

    # Send UI view choice for UnInstalled themes
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: UnInstalledView@${gUISettingViewUnInstalled}@"
    SendToUI "UnInstalledView‡${gUISettingViewUnInstalled}‡"

    # Send UI view choice for Thumbnails
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: ThumbnailView@${gUISettingViewThumbnails}@"
    SendToUI "ThumbnailView‡${gUISettingViewThumbnails}‡"

    # Send UI view choice for Previews
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: PreviewView@${gUISettingViewPreviews}@"
    SendToUI "PreviewView‡${gUISettingViewPreviews}‡"

    # Add message in to log for initialise.js to detect.
    WriteToLog "CTM_InitInterface"
}

# ---------------------------------------------------------------------------------------
ReadThemeEntriesAndSendToUI()
{
    if [ "$TARGET_THEME_DIR" != "-" ] && [ "$TARGET_THEME_DIR_DEVICE" != "-" ] && [ "$TARGET_THEME_PARTITIONGUID" != "-" ]; then

        # Check if selected device is boot device
        if [ "$TARGET_THEME_DIR_DEVICE" == "$gBootDeviceIdentifier" ]; then

            # Read current Clover.Theme Nvram variable and send to UI.
            ReadAndSendCurrentNvramTheme

            # Read current nvram.plist theme var and send to UI
            ReadAndSendCurrentNvramPlistTheme

            # Read current config.plist theme entry and send to UI
            ReadAndSendCurrentConfigPlistTheme
        fi
    fi
}


# =======================================================================================
# After Initialisation Routines
# =======================================================================================


# ---------------------------------------------------------------------------------------
RespondToUserDeviceSelection()
{
    # Called from the Main Message Loop when a user has changed the
    # themes file path from the drop down menu in the UI.
    #
    # This routine takes the message, and splits it to find the device
    # and volume name. Then providing the user has not chosen 'Please Choose'
    # from the menu (indicated by a - for each device and volumeName), the 
    # path is double checked before writing the choice to the user prefs file.
    #
    # Routines are then called to perform the following:
    # 1 - Get a list of theme directories at selected file path.
    # 2 - Get available free space on target volume.
    # 3 - Housekeep locally installed bare git clones.
    # 4 - Check for any theme updates on target volume.
    # 5 - Decide to show or hide control options.
    # 6 - Send list of installed themes to the UI.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}RespondToUserDeviceSelection()"

    local messageFromUi="$1"

    # parse message
    # remove everything up until, and including, the first ‡
    local messageFromUi="${messageFromUi#*‡}"
    local pathOption="${messageFromUi##*‡}"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}RespondToUserDeviceSelection() messageFromUi=$messageFromUi | pathOption=$pathOption"
    # Check user did actually change from default
    if [ ! "$pathOption" == "-" ]; then

        WriteLinesToLog
        WriteToLog "Target path changed: ${themeDirPaths[$pathOption]} on device ${duIdentifier[$pathOption]} with GUID ${duPartitionGuid[$pathOption]}" 

        local volumePath=$( ResolveVolumePathFromGUID "${duPartitionGuid[$pathOption]}" )
        if [ "$volumePath" != "" ]; then
            TARGET_THEME_DIR="$volumePath"
        else
            TARGET_THEME_DIR="${themeDirPaths[$pathOption]}"
        fi
        TARGET_THEME_DIR_DEVICE="${duIdentifier[$pathOption]}"
        TARGET_THEME_PARTITIONGUID="${duPartitionGuid[$pathOption]}"

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_DIR=$TARGET_THEME_DIR"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_DIR_DEVICE=$TARGET_THEME_DIR_DEVICE"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}TARGET_THEME_PARTITIONGUID=$TARGET_THEME_PARTITIONGUID"

        CheckThemePathIsStillValid
        retVal=$? # returns 1 if invalid / 0 if valid
        if [ $retVal -eq 0 ]; then

            UpdatePrefsKey "LastSelectedPath" "$TARGET_THEME_DIR"  
            UpdatePrefsKey "LastSelectedPathDevice" "$TARGET_THEME_DIR_DEVICE"
            UpdatePrefsKey "LastSelectedPartitionGUID" "$TARGET_THEME_PARTITIONGUID"

            GetListOfInstalledThemesAndSendToUI
            GetFreeSpaceOfTargetDeviceAndSendToUI
            CheckAndRecordUnManagedThemesAndSendToUI
            CheckForAndRemoveThemeGitDirs
            CheckAndRemoveBareClonesNoLongerNeeded
            CheckForThemeUpdates &
            ShowHideUIControlOptions
            ReadThemeEntriesAndSendToUI
        else
            # Run these regardless of path chosen as JS is waiting to hear it. 
            CheckAndRecordUnManagedThemesAndSendToUI
        fi
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "User de-selected Volume path and chose menu title. Do Nothing."
        TARGET_THEME_DIR="-"
        TARGET_THEME_DIR_DEVICE="-"
        TARGET_THEME_PARTITIONGUID="-"

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Setting prefs for default dir,path & GUID to -"
        UpdatePrefsKey "LastSelectedPath" "$TARGET_THEME_DIR"  
        UpdatePrefsKey "LastSelectedPathDevice" "$TARGET_THEME_DIR_DEVICE"
        UpdatePrefsKey "LastSelectedPartitionGUID" "$TARGET_THEME_PARTITIONGUID"

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: InstalledThemes@-@"
        SendToUI "InstalledThemes‡-‡"
    fi

    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: EnableInterface@@"
    SendToUI "EnableInterface‡‡"
}

# ---------------------------------------------------------------------------------------
ShowHideUIControlOptions()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ShowHideUIControlOptions()"

    # Check if this selected device is boot device or not
    if [ "$TARGET_THEME_DIR_DEVICE" != "$gBootDeviceIdentifier" ]; then
        # Hide theme control options
        WriteToLog "$TARGET_THEME_DIR_DEVICE is not boot device. Hiding control options."
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: ShowHideControlOptions@Hide@"
        SendToUI "ShowHideControlOptions‡Hide‡"
    else
        # Show theme control options
        WriteToLog "$TARGET_THEME_DIR_DEVICE is boot device. Show control options."
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: ShowHideControlOptions@Show@"
        SendToUI "ShowHideControlOptions‡Show‡"
    fi
}

# ---------------------------------------------------------------------------------------
RespondToUserThemeAction()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}RespondToUserThemeAction()"

    local messageFromUi="$1"

    # remove everything up until, and including, the first ‡
    messageFromUi="${messageFromUi#*‡}"
    chosenTheme="${messageFromUi%%‡*}"
    desiredAction="${messageFromUi##*‡}"

    # further strip theme name and action
    chosenTheme="${chosenTheme##*button_}"
    desiredAction="${desiredAction##*button}"

    # Note - desiredAction will be either: Install, UnInstall or Update

    if [ ! "$chosenTheme" == "" ] && [ ! "$desiredAction" == "" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}User chose to $desiredAction theme $chosenTheme"

        CheckThemePathIsStillValid
        retVal=$? # returns 1 if invalid / 0 if valid
        if [ $retVal -eq 0 ]; then
            RunThemeAction "$desiredAction" "$chosenTheme"
            # Add 'Update success' notification to UI here so to not conflict with the 'Update All' functionality
            local checkSuccess=$?
            if [ "$desiredAction" == "Update" ] && [ $checkSuccess -eq 0 ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Success@${desiredAction}@$chosenTheme"
                SendToUI "Success‡${desiredAction}‡$chosenTheme"
            fi
            return $checkSuccess
        else
            return 1
        fi
    fi
}

# ---------------------------------------------------------------------------------------
CheckThemePathIsStillValid()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckThemePathIsStillValid()"

    local findDevice=""
    local stillMounted=0

    # Find device by previously used UUID.
    if [ "$TARGET_THEME_PARTITIONGUID" != "$zeroUUID" ] && [ "$TARGET_THEME_PARTITIONGUID" != "-" ]; then
        findDevice=$( "$partutil" --search-uuid $TARGET_THEME_PARTITIONGUID )
    else
        findDevice="$TARGET_THEME_DIR_DEVICE"
    fi

    # Match device to current list of mounted partitions with valid theme paths.
    if [ "$findDevice" != "" ]; then
        for ((i=0; i<${#duIdentifier[@]}; i++))
        do
            if [ $findDevice == ${duIdentifier[$i]} ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Device $TARGET_THEME_PARTITIONGUID is ${duIdentifier[$i]}"

                # Check path actually exists
                if [ -d "$TARGET_THEME_DIR" ]; then 
                    stillMounted=1
                    # Ensure current TARGET_THEME_DIR_DEVICE matches device
                    TARGET_THEME_DIR_DEVICE=${duIdentifier[$i]}
                fi
            fi
        done
    fi

    if [ $stillMounted -eq 0 ] && [ "$TARGET_THEME_DIR" != "-" ]; then
        WriteToLog "Theme directory $TARGET_THEME_DIR on $TARGET_THEME_PARTITIONGUID does not exist! Setting to -"

        local entry=$( FindArrayIdFromTarget )
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}entry=$entry"
        local pathToPrint="$TARGET_THEME_DIR"
        if [ "$entry" != "-" ]; then
            if [[ "${themeDirPaths[$entry]}" == *$gESPMountPrefix* ]]; then
                local tmpStrip="${themeDirPaths[$entry]#*/}"
                tmpStrip="${tmpStrip#*/}"
                tmpStrip="${tmpStrip#*/}"
                pathToPrint="/Volumes/EFI/${tmpStrip}"
            fi
        fi
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: NotExist@${TARGET_THEME_PARTITIONGUID}@${pathToPrint}@$entry"
        SendToUI "NotExist‡${TARGET_THEME_PARTITIONGUID}‡${pathToPrint}‡$entry"

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}NoPathSelected"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: NoPathSelected@@"
        SendToUI "NoPathSelected‡‡"

        # Re-build theme directory list
        "$findThemeDirs"
        ReadThemeDirList
        CreateAndSendVolumeDropDownMenu
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Target@-"
        SendToUI "Target‡-‡"
        RespondToUserDeviceSelection "‡-"

        return 1
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Theme directory $TARGET_THEME_DIR exists."
        return 0
    fi
}

# ---------------------------------------------------------------------------------------
GetListOfInstalledThemesAndSendToUI()
{
    # Scan the selected EFI/Clover/Themes directory for a list of installed themes.
    # The user could add themes without using the app so we need to keep to track of
    # what's there.
    # Send the list of installed themes to the UI.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}GetListOfInstalledThemesAndSendToUI()"

    installedThemeStr=""
    unset installedThemesFoundAfterSearch
    unset installedThemesOnCurrentVolume
    if [ "$TARGET_THEME_DIR" != "" ] && [ "$TARGET_THEME_DIR" != "-" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Looking for installed themes at $TARGET_THEME_DIR on $TARGET_THEME_DIR_DEVICE"
        oIFS="$IFS"; IFS=$'\r\n'
        installedThemesFoundAfterSearch=( $( find "$TARGET_THEME_DIR"/* -type d -depth 0 ))
        for ((i=0; i<${#installedThemesFoundAfterSearch[@]}; i++))
        do
            installedThemesOnCurrentVolume[$i]="${installedThemesFoundAfterSearch[$i]##*/}"
            # Create comma separated string for sending to the UI
            installedThemeStr="${installedThemeStr},${installedThemesOnCurrentVolume[$i]}"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found installed theme: ${installedThemesOnCurrentVolume[$i]}"
        done
        IFS="$oIFS"
        # Remove leading comma from string
        installedThemeStr="${installedThemeStr#?}"
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Can't check for installed themes at $TARGET_THEME_DIR"
    fi

    # Sort comma separated list. 
    # This is necessary only for applying different fills (shadows/without shadows) in JS ChangeButtonAndBandToUnInstall()
    installedThemeStr=$( LC_ALL=C; echo "$installedThemeStr" | tr , "\n" | sort -f | tr "\n" , )

    # Remove trailing comma from string
    if [ "${installedThemeStr: -1}" == "," ]; then
        installedThemeStr="${installedThemeStr%?}"
    fi

    WriteToLog "Installed Themes:${installedThemeStr}"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: InstalledThemes@${installedThemeStr}@"
    SendToUI "InstalledThemes‡${installedThemeStr}‡"
}

# ---------------------------------------------------------------------------------------
CheckThemeIsInPrefs()
{
    # Check for any inconsistency where a theme entry in user prefs may be missing when
    # it's clearly installed in the users EFI/Clover/Themes directory AND has a parent
    # bare clone in the support directory.
    # If found - Add this theme in to prefs.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckThemeIsInPrefs()"

    local themeToFind="$1"
    local inPrefs=0

    for ((n=0; n<${#installedThemeName[@]}; n++ ))
    do
        if [ "${installedThemeName[$n]}" == "$themeToFind" ] && [ "${installedThemePartitionGUID[$n]}" == "$TARGET_THEME_PARTITIONGUID" ]; then
            inPrefs=1
        fi
    done

    if [ $inPrefs -eq 0 ]; then
        # Should add in to prefs
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}* $themeToFind is in ${TARGET_THEME_DIR} and bare clone exists but not in prefs! Adding now."

        # Add the details for this theme for adding to prefs file
        gNewInstalledThemeName="$themeToFind"
        gNewInstalledThemePath="$TARGET_THEME_DIR"
        gNewInstalledThemePathDevice="$TARGET_THEME_DIR_DEVICE"
        gNewinstalledThemePartitionGUID="$TARGET_THEME_PARTITIONGUID"

        # Run routine to update prefs file.
        MaintainInstalledThemeListInPrefs  
    fi
}

# ---------------------------------------------------------------------------------------
CheckAndRecordUnManagedThemesAndSendToUI()
{
    # Note: installedThemesOnCurrentVolume[] contains list of themes installed on the current theme path.
    # Plan: loop through this array and check for existence of .hash file
    #       Create list of any installed themes missing a .hash file in $unversionedThemeStr
    # Send the list to the UI so a cross is drawn to the right of the 'UnInstall' button.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckAndRecordUnManagedThemesAndSendToUI()"

    if [ ! "$TARGET_THEME_DIR" == "-" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Checking $TARGET_THEME_DIR for any unmanaged themes (without a .hash)."
        unversionedThemeStr=""
        local prefsNeedUpdating=0
        for ((t=0; t<${#installedThemesOnCurrentVolume[@]}; t++))
        do

            #if [ ! -d "${WORKING_PATH}/${APP_DIR_NAME}"/"${installedThemesOnCurrentVolume[$t]}.git" ]; then
                #WriteToLog "${TARGET_THEME_DIR}/${installedThemesOnCurrentVolume[$t]} is missing parent bare clone from support dir!"

            # Check for .hash inside installed theme dir
            if [ ! -f "$TARGET_THEME_DIR"/"${installedThemesOnCurrentVolume[$t]}"/.hash ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}${TARGET_THEME_DIR}/${installedThemesOnCurrentVolume[$t]} has no hash"

                # Append to list of themes that cannot be checked for updates
                unversionedThemeStr="${unversionedThemeStr},${installedThemesOnCurrentVolume[$t]}"

                # Remove any pref entry for this theme
                for ((d=0; d<${#installedThemeName[@]}; d++))
                do
                    if [ "${installedThemeName[$d]}" == "${installedThemesOnCurrentVolume[$t]}" ] && \
                       [ "${installedThemePartitionGUID[$d]}" == "${TARGET_THEME_PARTITIONGUID}" ] && \
                       [ "$TARGET_THEME_PARTITIONGUID" != "$zeroUUID" ]; then
                        # Doing this will effectively delete the theme from prefs as it 
                        # will be skipped in the loop in MaintainInstalledThemeListInPrefs()
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Housekeeping: Will remove prefs entry for ${installedThemeName[$d]} on $TARGET_THEME_PARTITIONGUID"
                        prefsNeedUpdating=1
                        installedThemeName[$d]="-"
                    fi
                done
            else
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}${TARGET_THEME_DIR}/${installedThemesOnCurrentVolume[$t]} has parent bare clone in support dir"
                # Match - theme dir in users theme path that also has a parent bare clone in app support dir.
                # Double check this is also in user prefs file.
                if [ "$TARGET_THEME_PARTITIONGUID" != "$zeroUUID" ]; then
                    CheckThemeIsInPrefs "${installedThemesOnCurrentVolume[$t]}"
                fi
            fi
        done

        # Run routine to update prefs file.
        if [ $prefsNeedUpdating -eq 1 ]; then
            MaintainInstalledThemeListInPrefs  
        fi

        # Remove leading comma from string
        unversionedThemeStr="${unversionedThemeStr#?}"

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI list of themes not installed by this app: UnversionedThemes@${unversionedThemeStr}@"
        SendToUI "UnversionedThemes‡${unversionedThemeStr}‡"
    fi
}

# ---------------------------------------------------------------------------------------
CheckForAndRemoveThemeGitDirs()
{
    # Note: installedThemesOnCurrentVolume[] contains list of themes installed on the current theme path.
    # Plan: loop through this array and check for existence of theme.git directories.
    #       Remove any, if found

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckForAndRemoveThemeGitDirs()"

    if [ ! "$TARGET_THEME_DIR" == "-" ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Checking $TARGET_THEME_DIR for any theme.git dirs."

        local dirToCheck=""
        for ((t=0; t<${#installedThemesOnCurrentVolume[@]}; t++))
        do
            # Check for theme.git dir inside installed theme dir
            dirToCheck="$TARGET_THEME_DIR"/"${installedThemesOnCurrentVolume[$t]}"/theme.git        
            [[ -d "$dirToCheck" ]] && rm -rf "$dirToCheck" && [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found and deleted $dirToCheck."               
        done
    fi
}

# ---------------------------------------------------------------------------------------
IsThemeInstalled()
{
    local passedTheme="$1"
    local found=1
    for ((t=0; t<${#installedThemesOnCurrentVolume[@]}; t++ ));
    do
        if [ "$passedTheme" == "${installedThemesOnCurrentVolume[$t]}" ]; then
            found=0
            break
        fi
    done
    echo $found
}

# ---------------------------------------------------------------------------------------
ReadConfigPList()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadConfigPList()"

    # Set default vars
    gConfigTextOnly=1 # Set to 0 if config plist file has TextOnly set to true
    gConfigFastBoot=1 # Set to 0 if config plist file has Fast Boot set to true

    # check for TextOnly and Fast boot options set by user
    if [ "$gConfigPlistFullPath" != "" ] && [ -f "$gConfigPlistFullPath" ]; then
        local checkTextOnly=$( grep -A1 "<key>TextOnly</key>" "$gConfigPlistFullPath" )
        if [[ "$checkTextOnly" == *true* ]]; then
            gConfigTextOnly=0
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}Text Only boot option is enabled"
        fi
        local checkFast=$( grep -A1 "<key>Fast</key>" "$gConfigPlistFullPath" )
        if [[ "$checkFast" == *true* ]]; then
            gConfigFastBoot=0
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}Fast boot option is enabled"
        fi
    fi
}

# ---------------------------------------------------------------------------------------
PredictNextTheme()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}PredictNextTheme()"

    local themeToSend=""

    if [ "$gBootType" == "UEFI" ]; then
        if [ "$CURRENT_THEME_ENTRY_NVRAM" != "" ]; then
            local checkTheme=$( IsThemeInstalled "$CURRENT_THEME_ENTRY_NVRAM" )
            if [ $checkTheme -eq 0 ] || [ "$CURRENT_THEME_ENTRY_NVRAM" == "embedded" ]; then
                themeToSend="$CURRENT_THEME_ENTRY_NVRAM"
            fi
        elif [ "$CURRENT_THEME_ENTRY_CONFIG_PLIST" != "" ]; then
            local checkTheme=$( IsThemeInstalled "$CURRENT_THEME_ENTRY_CONFIG_PLIST" )
            if [ $checkTheme -eq 0 ] || [ "$CURRENT_THEME_ENTRY_CONFIG_PLIST" == "embedded" ]; then
                themeToSend="$CURRENT_THEME_ENTRY_CONFIG_PLIST"
            fi
        fi
    elif [ "$gBootType" == "Legacy" ]; then
        if [ "$CURRENT_THEME_ENTRY_NVRAM" != "" ] && [ $gNvramSave -eq 0 ]; then
            local checkTheme=$( IsThemeInstalled "$CURRENT_THEME_ENTRY_NVRAM" )
            if [ $checkTheme -eq 0 ] || [ "$CURRENT_THEME_ENTRY_NVRAM" == "embedded" ]; then
                themeToSend="$CURRENT_THEME_ENTRY_NVRAM"
            fi
        elif [ "$CURRENT_THEME_ENTRY_NVRAM_PLIST" != "" ] && [ $gNvramSave -eq 1 ]; then
            local checkTheme=$( IsThemeInstalled "$CURRENT_THEME_ENTRY_NVRAM_PLIST" )
            if [ $checkTheme -eq 0 ] || [ "$CURRENT_THEME_ENTRY_NVRAM_PLIST" == "embedded" ]; then
                themeToSend="$CURRENT_THEME_ENTRY_NVRAM_PLIST"
            fi
        elif [ "$CURRENT_THEME_ENTRY_CONFIG_PLIST" != "" ]; then
            local checkTheme=$( IsThemeInstalled "$CURRENT_THEME_ENTRY_CONFIG_PLIST" )
            if [ $checkTheme -eq 0 ] || [ "$CURRENT_THEME_ENTRY_CONFIG_PLIST" == "embedded" ]; then
                themeToSend="$CURRENT_THEME_ENTRY_CONFIG_PLIST"
            fi
        fi
    fi

    if [ "$themeToSend" == "" ]; then
        if [ ${#installedThemesOnCurrentVolume[@]} -gt 0 ]; then
            themeToSend="random"
        else
            themeToSend="embedded"
        fi
    fi

    # Check for special themes (using /rEFIt_UEFI/Platform/Settings.c for ref)
    local month=$( date +"%m" )
    local day=$( date +"%d" )
    if [[ $month -eq 12 ]] && [[ $day -ge 25 && $day -le 31 ]]; then 
        local checkTheme=$( IsThemeInstalled "christmas" )
        [[ checkTheme -eq 0 ]] && themeToSend="christmas"
    elif [[ $month -eq 1 ]] && [[ $day -ge 1 && $day -le 7 ]]; then 
        local checkTheme=$( IsThemeInstalled "newyear" )
        [[ checkTheme -eq 0 ]] && themeToSend="newyear"
    fi

    # Check for Fast boot or Text Only boot options enabled in config.plist
    ReadConfigPList
    if [ $gConfigTextOnly -eq 0 ]; then
        themeToSend="None (Text Only is enabled)"
    fi

    if [ $gConfigFastBoot -eq 0 ]; then
        themeToSend="None (Fast boot is enabled)"
    fi

    WriteToLog "Prediction: Next boot from this device will load theme: $themeToSend"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: SetPrediction@${themeToSend}@"
    SendToUI "SetPrediction‡${themeToSend}‡"
}

# ---------------------------------------------------------------------------------------
RespondToDropDownMenuChange()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}RespondToDropDownMenuChange()"

    local messageFromUi="$1"
    local wantToChange="${messageFromUi%‡*}"
    local themeToSet="${messageFromUi#*‡}"
    local lastChar=$( echo -n "$wantToChange" | tail -c -1 )

    if [ "$lastChar" == "N" ]; then
        if [ "$themeToSet" != "!Remove!" ]; then
            WriteToLog "User chose to set nvram var to $themeToSet"
            SetNvramTheme "$themeToSet"
        else
            WriteToLog "User chose to delete clover.Theme nvram var"
            DeleteNvramThemeVar
        fi
    elif [ "$lastChar" == "P" ]; then
        if [ "$themeToSet" != "!Remove!" ]; then
            WriteToLog "User chose to set nvram plist to $themeToSet"
            SetNvramFile "$themeToSet"
        else
            WriteToLog "User chose to remove nvram.plist theme entry"
            DeleteNvramPlistThemeEntry
        fi
    elif [ "$lastChar" == "C" ]; then
        if [ "$themeToSet" != "!Remove!" ]; then
            WriteToLog "User chose to set config.plist to $themeToSet"
            SetConfigFile "$themeToSet"
        else
            WriteToLog "User chose to remove config.plist theme entry"
            DeleteConfigPlistThemeEntry
        fi
    fi
}

# ---------------------------------------------------------------------------------------
RespondToUpdateAll()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}RespondToUpdateAll()"

    local messageFromUi="$1"
    local updateThemeList="${messageFromUi#*‡}"
    local updateFail=0

    WriteToLog "User chose to update all themes"

    oIFS="$IFS"; IFS=$','
    read -a arr <<< "$updateThemeList"
    IFS="$oIFS"

    for (( u=0; u<${#arr[@]}; u++ ));
    do
        CheckThemePathIsStillValid
        retVal=$? # returns 1 if invalid / 0 if valid
        if [ $retVal -eq 0 ]; then
            WriteToLog "Calling update for theme: ${arr[$u]}"
            RunThemeAction "Update" "${arr[$u]}"
            updateFail=$? # returns 1 if process failed / 0 if success
            if [ $updateFail -eq 0 ]; then
                SendToUI "UpdateAll‡Theme‡${arr[$u]}"
            fi
        else
            updateFail=1
        fi
    done

    if [ $updateFail -eq 0 ]; then
        # Operation was successful
        SendToUI "UpdateAll‡Complete‡-"
        GetListOfInstalledThemesAndSendToUI
        GetFreeSpaceOfTargetDeviceAndSendToUI
        CheckAndRecordUnManagedThemesAndSendToUI
        ReadThemeEntriesAndSendToUI
    fi 
}

# ---------------------------------------------------------------------------------------
ReadAndSendCurrentNvramTheme()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadAndSendCurrentNvramTheme()"

    local readNvramVar=$( nvram -p | grep Clover.Theme | tr -d '\011' )

    # Extract theme name
    local themeName="${readNvramVar##*Clover.Theme}"

    if [ ! -z "$readNvramVar" ]; then
        WriteToLog "Clover.Theme NVRAM variable is set to $themeName"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Nvram@${themeName}@"
        SendToUI "Nvram‡${themeName}‡"
        # Add message in to log for initialise.js to detect.
        [[ $gInitialising -eq 0 ]] && WriteToLog "CTM_NvramFound"
        CURRENT_THEME_ENTRY_NVRAM="$themeName"
        PredictNextTheme
    else
        # Sending '-' to the UI here causes the drop down menu to be populated to the value '-'
        # which currently defaults to menu option ' '
        WriteToLog "Clover.Theme NVRAM variable is not set"
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Nvram@-@"
        SendToUI "Nvram‡-‡"
        # Add message in to log for initialise.js to detect.
        [[ $gInitialising -eq 0 ]] && WriteToLog "CTM_NvramNotFound"
        PredictNextTheme
    fi
}

# ---------------------------------------------------------------------------------------
ReadAndSendCurrentNvramPlistTheme()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadAndSendCurrentNvramPlistTheme()"

    if [ -f "$gNvramPlistFullPath" ]; then

        # Extract theme name
        local themeName=$( /usr/libexec/PlistBuddy -c "Print:Clover.Theme" "$gNvramPlistFullPath" 2>/dev/null )

        if [ "$themeName" != "" ]; then
            WriteToLog "$gNvramPlistFullPath contains theme entry $themeName"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: NvramP@${themeName}@"
            #SendToUI "NvramP@${themeName}@"
            SendToUI "Nvram‡${themeName}‡"
            CURRENT_THEME_ENTRY_NVRAM_PLIST="$themeName"
            PredictNextTheme
        else
            # Sending '-' to the UI here causes the drop down menu to be populated to the value '-'
            # which currently defaults to menu option ' '
            WriteToLog "$gNvramPlistFullPath does not contain a theme entry"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: NvramP@-@"
            #SendToUI "NvramP@-@"
            SendToUI "Nvram‡-‡"
            PredictNextTheme
        fi
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$gNvramPlistFullPath does not exist."

        # Is there a theme entry from nvram.plist from bootlog?
        if [ "$gNvramPlistThemeEntry" != "" ]; then
            WriteToLog "$gNvramPlistFullPath contained the following theme at boot time: $gNvramPlistThemeEntry"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: NvramP@${gNvramPlistThemeEntry}@"
            #SendToUI "NvramP@${gNvramPlistThemeEntry}@"
            SendToUI "Nvram‡${gNvramPlistThemeEntry}‡"
            CURRENT_THEME_ENTRY_NVRAM_PLIST="$gNvramPlistThemeEntry"
            PredictNextTheme
        fi
    fi
}

# ---------------------------------------------------------------------------------------
ReadAndSendCurrentConfigPlistTheme()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadAndSendCurrentConfigPlistTheme()"

    if [ -f "$gConfigPlistFullPath" ]; then

        # Extract theme name
        local themeName=$( /usr/libexec/PlistBuddy -c "Print:GUI:Theme" "$gConfigPlistFullPath" 2>/dev/null )

        if [ "$themeName" != "" ]; then
            WriteToLog "$gConfigPlistFullPath contains theme entry $themeName"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: ConfigP@${themeName}@"
            SendToUI "ConfigP‡${themeName}‡"
            CURRENT_THEME_ENTRY_CONFIG_PLIST="$themeName"
            PredictNextTheme
        else
            # Sending '-' to the UI here causes the drop down menu to be populated to the value '-'
            # which currently defaults to menu option ' '
            WriteToLog "$gConfigPlistFullPath does not contain a theme entry"
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: ConfigP@-@"
            SendToUI "ConfigP‡-‡"
            PredictNextTheme
        fi
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$gConfigPlistFullPath does not exist."
    fi
}

# ---------------------------------------------------------------------------------------
SetNvramTheme()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SetNvramTheme()"

    local chosenTheme="$1"
    local successFlag=1

    if [ $(CheckOsVersion) -ge 13 ]; then
        # com.apple.security.agentStub on Mavericks?
        successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                       /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡SetNVRAMVar\" & \"‡${chosenTheme}\" with administrator privileges" )
    else
        successFlag=$( /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡SetNVRAMVar\" & \"‡${chosenTheme}\" with administrator privileges" )
    fi

    # Was operation a success?
    if [ $successFlag -eq 0 ]; then
        WriteToLog "Setting NVRAM Variable was successful."
    else
        WriteToLog "Setting NVRAM Variable failed."
    fi

    # Read current Clover.Theme Nvram variable and send to UI.
    ReadAndSendCurrentNvramTheme
}

# ---------------------------------------------------------------------------------------
DeleteNvramThemeVar()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}DeleteNvramThemeVar()"

    local successFlag=1

    if [ $(CheckOsVersion) -ge 13 ]; then
        # com.apple.security.agentStub on Mavericks?
        successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                       /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡DeleteNVRAMVar\" with administrator privileges" )
    else
        successFlag=$( /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡DeleteNVRAMVar\" with administrator privileges" )
    fi

    # Was operation a success?
    if [ $successFlag -eq 0 ]; then
        WriteToLog "Deleting NVRAM Variable was successful."
        CURRENT_THEME_ENTRY_NVRAM=""
    else
        WriteToLog "Deleting NVRAM Variable failed."
    fi

    # Read current Clover.Theme Nvram variable and send to UI.
    ReadAndSendCurrentNvramTheme
}

# ---------------------------------------------------------------------------------------
SetNvramFile()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SetNvramFile()"

    local chosenTheme="$1"
    local successFlag=1

    if [ -f "$gNvramPlistFullPath" ]; then
        if [ $(CheckOsVersion) -ge 13 ]; then
            # com.apple.security.agentStub on Mavericks?
            successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                           /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡SetNVRAMFile\" & \"‡${chosenTheme}\" & \"‡${gNvramPlistFullPath}\" with administrator privileges" )
        else
            successFlag=$( /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡SetNVRAMFile\" & \"‡${chosenTheme}\" & \"‡${gNvramPlistFullPath}\" with administrator privileges" )
        fi
    fi

    # Was operation a success?
    if [ $successFlag -eq 0 ]; then
        WriteToLog "Setting NVRAM Variable in $gNvramPlistFullPath was successful."
    else
        WriteToLog "Setting NVRAM Variable in $gNvramPlistFullPath failed."
    fi

    # Read current nvram.plist theme var and send to UI
    ReadAndSendCurrentNvramPlistTheme
}

# ---------------------------------------------------------------------------------------
DeleteNvramPlistThemeEntry()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}DeleteNvramPlistThemeEntry()"

    local successFlag=1

    if [ -f "$gNvramPlistFullPath" ]; then
        if [ $(CheckOsVersion) -ge 13 ]; then
            # com.apple.security.agentStub on Mavericks?
            successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                           /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡DeleteThemePlistEntry\" & \"‡${gNvramPlistFullPath}\" with administrator privileges" )
        else
            successFlag=$( /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡DeleteThemePlistEntry\" & \"‡${gNvramPlistFullPath}\" with administrator privileges" )
        fi
    fi

    # Was operation a success?
    if [ $successFlag -eq 0 ]; then
        WriteToLog "Deleting theme entry from $gNvramPlistFullPath was successful."
        CURRENT_THEME_ENTRY_NVRAM_PLIST=""
    else
        WriteToLog "Deleting theme entry from $gNvramPlistFullPath failed."
    fi

    # Read current nvram.plist theme var and send to UI
    ReadAndSendCurrentNvramPlistTheme
}

# ---------------------------------------------------------------------------------------
SetConfigFile()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SetConfigFile()"

    local chosenTheme="$1"
    local successFlag=1

    if [ -f "$gConfigPlistFullPath" ]; then
        if [ $(CheckOsVersion) -ge 13 ]; then
            # com.apple.security.agentStub on Mavericks?
            successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                           /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡SetNVRAMFile\" & \"‡${chosenTheme}\" & \"‡${gConfigPlistFullPath}\" with administrator privileges" )
        else
            successFlag=$( /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡SetNVRAMFile\" & \"‡${chosenTheme}\" & \"‡${gConfigPlistFullPath}\" with administrator privileges" )
        fi
    fi

    # Was operation a success?
    if [ $successFlag -eq 0 ]; then
        WriteToLog "Setting theme in $gConfigPlistFullPath was successful."
    else
        [WriteToLog "Setting theme in $gConfigPlistFullPath failed."
    fi

    # Read current config.plist theme entry and send to UI
    ReadAndSendCurrentConfigPlistTheme
}

# ---------------------------------------------------------------------------------------
DeleteConfigPlistThemeEntry()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}DeleteConfigPlistThemeEntry()"

    local successFlag=1

    if [ -f "$gConfigPlistFullPath" ]; then
        if [ $(CheckOsVersion) -ge 13 ]; then
            # com.apple.security.agentStub on Mavericks?
            successFlag=$( /usr/bin/osascript -e 'tell application "SecurityAgent" to activate'; \
                           /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡DeleteThemePlistEntry\" & \"‡${gConfigPlistFullPath}\" with administrator privileges" )
        else
            successFlag=$( /usr/bin/osascript -e  "do shell script \"$uiSudoChanges \" & \"‡DeleteThemePlistEntry\" & \"‡${gConfigPlistFullPath}\" with administrator privileges" )
        fi
    fi

    # Was operation a success?
    if [ $successFlag -eq 0 ]; then
        WriteToLog "Deleting theme entry from $gConfigPlistFullPath was successful."
        CURRENT_THEME_ENTRY_CONFIG_PLIST=""
    else
        WriteToLog "Deleting theme entry from $gConfigPlistFullPath failed."
    fi

    # Read current nvram.plist theme var and send to UI
    ReadAndSendCurrentConfigPlistTheme
}


# ---------------------------------------------------------------------------------------
CheckIfThemeNoLongerInstalledThenDeleteLocalTheme()
{
    # If all instances of a local bare repo theme.git have been uninstalled
    # then delete the local bare repo.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckIfThemeNoLongerInstalledThenDeleteLocalTheme()"

    local passedThemeName="$1"
    local foundTheme=0
    for ((n=0; n<${#installedThemeName[@]}; n++ ));
    do
        if [ "${installedThemeName[$n]}" == "$passedThemeName" ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Keeping ${passedThemeName}.git local bare repo as it's still in use."
            foundTheme=1
            break
        fi
    done
    if [ $foundTheme -eq 0 ]; then
        if [ -d "${WORKING_PATH}/${APP_DIR_NAME}/${passedThemeName}".git ]; then
            #WriteToLog "Local bare repo ${passedThemeName}.git is no longer in use. Deleting."
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Local bare repo ${passedThemeName}.git is now being deleted."
            rm -rf "${WORKING_PATH}/${APP_DIR_NAME}/${passedThemeName}".git
        fi
    fi
}

# ---------------------------------------------------------------------------------------
CheckForThemeUpdates()
{
    # Note: installedThemesOnCurrentVolume[] contains list of themes installed on the current theme path.
    # Plan: loop through this array and check for commit hash saved in .hash file at root of local theme dir.
    #       If found, get current commit hash from server and compare. Any change indicates an update.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckForThemeUpdates()"
    [[ DEBUG -eq 1 ]] && WriteToLog "Num Themes in themeList=${#themeList[@]}"

    local updateAvailThemeStr=""
    local themeHashLocal=""
    local themeHashRepo=""
    local themeStillExistsInRepo=0

    if [ "$TARGET_THEME_DIR" != "-" ]; then

        WriteToLog "Checking $TARGET_THEME_DIR for any theme updates."

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: CheckingThemeUpdates@@"
        SendToUI "CheckingThemeUpdates‡‡"

        for ((t=0; t<${#installedThemesOnCurrentVolume[@]}; t++))
        do

            themeStillExistsInRepo=0

            # Check theme still exists in the repo
            for ((e=0; e<${#themeList[@]}; e++))
            do

                if [ "${installedThemesOnCurrentVolume[$t]}" == "${themeList[$e]##*/}" ]; then

                    themeStillExistsInRepo=1
                    break

                fi

            done

            # read hash from currently installed theme
            if [ -f "$TARGET_THEME_DIR"/"${installedThemesOnCurrentVolume[$t]}"/.hash ] && [ $themeStillExistsInRepo -eq 1 ]; then
                themeHashLocal=$( cat "$TARGET_THEME_DIR"/"${installedThemesOnCurrentVolume[$t]}"/.hash )
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}themeHashLocal=$themeHashLocal"

                # get hash of theme in the repo
                themeHashRepo=$( "$gitCmd" ls-remote ${remoteRepositoryUrl}/themes.git/themes/"${installedThemesOnCurrentVolume[$t]}"/theme | grep refs/heads/master)
                WriteToLog "$themeHashRepo"
                themeHashRepo="${themeHashRepo:0:40}"
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}themeHashRepo=$themeHashRepo"

                if [ "$themeHashRepo" != "" ]; then
                    if [ "$themeHashLocal" != "$themeHashRepo" ]; then
                        # Theme has been updated.
                        WriteToLog "${installedThemesOnCurrentVolume[$t]} has an update available."
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}hash diff: $themeHashLocal | $themeHashRepo"
                        updateAvailThemeStr="${updateAvailThemeStr},${installedThemesOnCurrentVolume[$t]}" 
                    else
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}hash matches. No update for ${installedThemesOnCurrentVolume[$t]}"
                    fi
                else
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Failed to read hash for ${installedThemesOnCurrentVolume[$t]} from repository."
                fi

            else
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Theme ${installedThemesOnCurrentVolume[$t]} either has no hash, or theme no longer exists in the repository."
            fi
        done

        if [ "$updateAvailThemeStr" != "" ] && [ "${updateAvailThemeStr:0:1}" == "," ]; then
            # Sort comma separated list. 
            # This is necessary only for applying different fills (shadows/without shadows) in JS ChangeButtonAndBandToUpdate()
            updateAvailThemeStr=$( LC_ALL=C; echo "$updateAvailThemeStr" | tr , "\n" | sort -f | tr "\n" , )

            # Remove leading comma from string
            updateAvailThemeStr="${updateAvailThemeStr#?}"
        else
            WriteToLog "No theme updates found."
        fi

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: UpdateAvailThemes‡${updateAvailThemeStr}‡"
        SendToUI "UpdateAvailThemes‡${updateAvailThemeStr}‡"
    fi
}

# ---------------------------------------------------------------------------------------
CheckAndRemoveBareClonesNoLongerNeeded()
{   
    # Check each installed theme entry in prefs against themes installed in current
    # /EFI/Clover/Themes dir selected by user.
    # If prefs says a theme should be on selected volume but it's not (maybe user
    # manually removed it?), then remove entry from prefs.
    # Also check to see if the bare clone in support dir can be deleted.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckAndRemoveBareClonesNoLongerNeeded()"

    if [ ! "$TARGET_THEME_DIR" == "-" ]; then

        foundCloneToDelete=0
        prefsNeedUpdating=0
        # Loop through themes installed in prefs file
        for ((n=0; n<${#installedThemeName[@]}; n++ ));
        do
            # Check current partition GUID in prefs matches current partition GUID
            if [ "${installedThemePartitionGUID[$n]}" == "$TARGET_THEME_PARTITIONGUID" ]; then
                # Is theme installed in current theme dir?
                local themeIsInDir=0
                for ((t=0; t<${#installedThemesOnCurrentVolume[@]}; t++))
                do
                    if [ "${installedThemeName[$n]}" == "${installedThemesOnCurrentVolume[$t]}" ]; then
                        themeIsInDir=1
                        break
                    fi
                done
                if [ $themeIsInDir -eq 0 ]; then
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Housekeeping: ${installedThemeName[$n]} exists in prefs for $TARGET_THEME_DIR but it's not installed!"
                    foundCloneToDelete=1

                    # if bare clone exists in support dir then there's a chance it could be deleted.
                    if [ -d "${WORKING_PATH}/${APP_DIR_NAME}/${installedThemeName[$n]}".git ]; then

                        # Need to check the bare clone is not needed for a different volume though..
                        for ((x=0; x<${#installedThemeName[@]}; x++ ));
                        do
                            if [ "${installedThemeName[$n]}" == "${installedThemeName[$x]}" ]; then
                                if [ "${installedThemePath[$n]}" != "${installedThemePath[$x]}" ]; then
                                   foundCloneToDelete=0
                                fi
                            fi
                        done

                        if [ $foundCloneToDelete -eq 1 ]; then
                            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Housekeeping: Deleting bare clone ${installedThemeName[$n]}.git"
                            cd "${WORKING_PATH}/${APP_DIR_NAME}"
                            rm -rf "${installedThemeName[$n]}".git
                        else
                            WriteToLog "Housekeeping: Keeping bare clone ${installedThemeName[$n]}.git as it's used on another volume."
                        fi
                    fi

                    # Set theme name to -
                    # Doing this will effectively delete the theme from prefs as it 
                    # will be skipped in the loop in MaintainInstalledThemeListInPrefs()
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Housekeeping: Will remove prefs entry for ${installedThemeName[$n]} in $TARGET_THEME_DIR"
                    prefsNeedUpdating=1
                    installedThemeName[$n]="-"
                fi
            fi
        done

        # Run routine to update prefs file.
        if [ $foundCloneToDelete -eq 1 ] || [ $prefsNeedUpdating -eq 1 ]; then
            MaintainInstalledThemeListInPrefs  
        fi
    fi
}

# ---------------------------------------------------------------------------------------
CleanInstalledThemesPrefEntries()
{
    # Check for and remove any duplicate installed theme entries from prefs.
    # This should not happen in the first place but I have found some examples
    # during my local testing here. Could be a bug that needs finding!

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CleanInstalledThemesPrefEntries()"

    foundEntryToDelete=0
    for ((n=0; n<${#installedThemeName[@]}; n++ ));
    do
        for ((m=0; m<${#installedThemeName[@]}; m++ ));
        do
            if [ $m -ne $n ] && [ "${installedThemeName[$n]}" == "${installedThemeName[$m]}" ]; then
                # Found another theme entry by same name
                # Is this installed elsewhere or a duplicate entry?
                if [ "${installedThemePath[$n]}" == "${installedThemePath[$m]}" ] && \
                   [ "${installedThemePathDevice[$n]}" == "${installedThemePathDevice[$m]}" ] && \
                   [ "${installedThemePartitionGUID[$n]}" == "${installedThemePartitionGUID[$m]}" ]; then
                    # Duplicate entry. Remove
                    foundEntryToDelete=1
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Housekeeping: Removing duplicate prefs entry for ${installedThemeName[$n]} at ${installedThemePath[$n]}."
                    installedThemeName[$n]="-"
                fi
            fi
        done
    done

    # Run routine to update prefs file.
    if [ $foundEntryToDelete -eq 1 ]; then
        MaintainInstalledThemeListInPrefs  
    fi
}

# ---------------------------------------------------------------------------------------
IsGitInstalled()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}IsGitInstalled()"

    local tmp=$( which -a git )
    local num=$( which -a git | wc -l )
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Number of git installations: ${num##* }"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}git installations: $tmp"

    if [ $( which -s git) ]; then 
        # Alert user in UI
        WriteToLog "CTM_GitFail"
    else
        # a git file exists which is a start.
        gitCmd=$( which git )

        # However....
        # File /usr/bin/git exists by default from virgin OS X install.
        # But this is not the actual git executable that's installed
        # with Xcode command line tools.
        # /usr/bin/xcrun can find and return true location.

        # $ /usr/bin/xcrun --find git
        # Applications/Xcode.app/Contents/Developer/usr/bin/git

        # But....
        # If user has not installed the Xcode command line developer tools
        # then trying to run /usr/bin/git or /usr/bin/xcrun will result in a
        # dialog in the Finder and also the following command line message:
        #     xcode-select: note: no developer tools were found at
        #     '/Applications/Xcode.app', requesting  install. Choose an option
        #     in the dialog to download the command line developer tools.
        # Thing is, we don't want to see a dialog box pop up in the Finder,
        # well not yet anyway.

        # Also, user may not want to install full Xcode command line tools.
        # They have the option to just install git from http://git-scm.com
        # If they install only git then we can use that.
        # Note: git installer creates:
        #       directory /usr/local/git contain git files.
        #       file /etc/paths.d/git containing /usr/local/git/bin
        #       file /etc/manpaths.d/git containing /usr/local/git/bin/share/man
        # The paths.d entry appends /usr/local/git/bin to the end of $PATH
        # So we can't call git using just 'git' or /usr/bin/git gets called.

        # Also.... 
        # We can't prepend /usr/local/git/bin to $PATH because
        # the users local command line returns different results to
        # what's returned from the command line when launched from a GUI app.
        # This could be a Yosemite? bug but any adjusted $PATH from say
        # a users ~/.bash_profile does not get presented to script from GUI. 

        # For example from the users local command line:
        # $ which -a git
        # /usr/local/git/bin/git    <-- $PATH entry added in ~/.bash_profile
        # /usr/bin/git

        # But from script launched from app
        # $ which -a git
        # /usr/bin/git              <-- $PATH entry (above) is missing

        # So let's check for the full path and use that (if present).
        # Check for installed git from http://git-scm.com
        if [ -f /usr/local/git/bin/git ]; then
            gitCmd="/usr/local/git/bin/git"
        else
            # Nope..
            # Time to actually run /usr/bin/git and see if Xcode developer
            # tools have been installed. If not a dialog will show in Finder.
            if [ "$gitCmd" == "/usr/bin/git" ]; then
                local catchReturn=$( /usr/bin/git 2>&1)
                if [[ "$catchReturn" == *"no developer tools were found"* ]]; then
                    WriteToLog "CTM_GitFail"
                    gitCmd=""
                else
                    gitCmd="/usr/bin/git"
                fi
            fi
        fi

        if [ "$gitCmd" != "" ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}using git at :$gitCmd"

            # Check version
            local gitVersion=$( $gitCmd --version )
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}$( $gitCmd --version )"
            if [ "$gitVersion" != "" ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}git version detection passed."
                WriteToLog "CTM_GitOK"
            else
                # Basic check for Xcode licence agreement
                local gitCheck=$( $gitCmd 2>&1 )
                if [[ "$gitCheck" == *Xcode* ]]; then
                    WriteToLog "Xcode licence has not yet been agreed to."
                    WriteToLog "CTM_GitLicence"
                else
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}git version detection failed."
                    WriteToLog "CTM_GitVersionFail"
                fi
                exit 1
            fi

        fi
    fi
}

# ---------------------------------------------------------------------------------------
CleanUp()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CleanUp()"

    RemoveFile "$logJsToBash"
    RemoveFile "$logFile"
    RemoveFile "$themeDirInfo"
    RemoveFile "$logBashToJs"
    RemoveFile "$updateScript"
    RemoveFile "$espList"
    RemoveFile "$mbrList"
    RemoveFile "$bootLogFile"
    RemoveFile "$bootlogScriptOutfile"
    RemoveFile "$bootDeviceInfo"
    RemoveFile "${TEMPDIR}/add_theme.html"
    RemoveFile "${TEMPDIR}/managethemes.html"

    # Remove symbolic link for the themes dir
    if [ -h "${TEMPDIR}"/themes ]; then
        rm "${TEMPDIR}"/themes
    fi

    # Remove symbolic link for the scripts files
    if [ -h "${TEMPDIR}"/scripts/initialise.js ]; then
        rm "${TEMPDIR}"/scripts/initialise.js
    fi
    if [ -h "${TEMPDIR}"/scripts/jquery-2.1.3.min.js ]; then
        rm "${TEMPDIR}"/scripts/jquery-2.1.3.min.js
    fi

    # Remove symbolic link for the styles dir
    if [ -h "${TEMPDIR}"/styles ]; then
        rm "${TEMPDIR}"/styles
    fi

    # Remove symbolic link for the assets dir
    if [ -h "${TEMPDIR}"/assets ]; then
        rm "${TEMPDIR}"/assets
    fi

    #if [ -d "$tmp_dir" ]; then
    #    rm -rf "$tmp_dir"
    #fi

    # Remove the copy of cloverthememanager.js
    if [ -f "${TEMPDIR}"/scripts/cloverthememanager.js ]; then
        rm "${TEMPDIR}"/scripts/cloverthememanager.js
    fi

    # Remove the temporary scripts dir
    if [ -d "${TEMPDIR}"/scripts ]; then
        rmdir "${TEMPDIR}"/scripts
    fi

    # Remove the temporary dir
    if [ -d "${TEMPDIR}" ]; then
        rmdir "${TEMPDIR}"
    fi

    # Check again and force if not removed
    if [ -d "${TEMPDIR}" ]; then
        rm -rf "${TEMPDIR}"
    fi
}

#===============================================================
# Main
#===============================================================


# Make sure this script exits when parent app is closed.
# Get process ID of this script
scriptPid=$( echo "$$" )
# Get process ID of parent
appPid=$( ps -p ${pid:-$$} -o ppid= )

# Resolve path
SELF_PATH=$(cd -P -- "$(dirname -- "$0")" && pwd -P) && SELF_PATH=$SELF_PATH/$(basename -- "$0")
source "${SELF_PATH%/*}"/shared.sh

# Globals
#gThemeRepoUrlFile="$PUBLIC_DIR"/theme_repo_url_list.txt
gUserPrefsFileName="org.black.CloverThemeManager"
gUserPrefsFile="$HOME/Library/Preferences/$gUserPrefsFileName"
gThumbSizeX=0
gThumbSizeY=0
gUISettingViewUnInstalled="Show"
gUISettingViewThumbnails="Show"
gUISettingViewPreviews="Hide"
gInitialising=0                                                    # Send init messages to log file for initialise.js to read.
gitCmd=""                                                          # Will be set to path to installed git binary to use
gBootlogState="Open"                                               # Default state for bootlog in UI. This is overridden by user prefs
gNvramPlistFullPath=""                                             # Will become full path if theme entry exists in nvram.plist
gNvramPlistThemeEntry=""                                           # Will become theme entry in nvram.plist
gConfigPlistFullPath=""                                            # Will become full path if theme entry exists in config.plist
gBootType=""                                                       # Will become either UEFI or Legacy
gNvramSave=1                                                       # Set to 0 if writing to NVRAM is saved for next boot
gBootDeviceIdentifier=""                                           # Will become identifier of boot device
gBootDeviceMountPoint=""                                           # Will become mountpoint of boot device
gConfigTextOnly=1                                                  # Set to 0 if config plist contains textOnly
gConfigFastBoot=1                                                  # Set to 0 if config plist contains textOnly

CURRENT_THEME_ENTRY_NVRAM=""
CURRENT_THEME_ENTRY_NVRAM_PLIST=""
CURRENT_THEME_ENTRY_CONFIG_PLIST=""

TARGET_THEME_DIR=""
TARGET_THEME_DIR_DEVICE=""
TARGET_THEME_PARTITIONGUID=""

# Find version of main app.
mainAppInfoFilePath="${SELF_PATH%Resources*}"
mainAppVersion=$( /usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "$mainAppInfoFilePath"/Info.plist  )

# Begin log file
RemoveFile "$logFile"
WriteToLog "CTM_Version ${mainAppVersion}"
WriteToLog "Started Clover Theme Manager script"
getDate=$(date); WriteToLog "$getDate"
WriteLinesToLog
WriteToLog "scriptPid=$scriptPid | appPid=$appPid"
WriteLinesToLog
[[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}PATH=$PATH"

# Ensure permissions of findThemeDirs script
if [ -f "$findThemeDirs" ]; then
    chmod 755 "$findThemeDirs" && [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}Set permissions of findThemeDirs script"
fi

# Ensure permissions of findThemeDirs script
if [ -f "$bootlogScript" ]; then
    chmod 755 "$bootlogScript" && [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}Set permissions of bootlog script"
fi

WriteToLog "==== Initialisation Start ===="

IsGitInstalled

# Only continue if git is installed
if [ "$gitCmd" != "" ]; then

    # Was this script called from a script or the command line
    identityCallerCheck=`ps -o stat= -p $$`
    if [ "${identityCallerCheck:1:1}" == "+" ]; then
        # Called from command line so interpret arguments.

        # Will expect 2 arguments
        # 1 - The install path
        # 2 - The theme name

        if [ "$#" -eq 2 ]; then
	        TARGET_THEME_DIR="$1"
	        themeToInstall="$2"
        else
	        echo "Error - wrong number of arguments passed."
	        echo "Expects 1st as full target path. 2nd Theme name"
	        exit 1
        fi

        # Redirect all log file output to stdout
        COMMANDLINE=1

        # Should we be checking the theme exists on the repo?
        # Currently this does not happen.

        # Does theme path exist?
        if [ -d "$TARGET_THEME_DIR" ]; then
            RunThemeAction "Install" "$themeToInstall"
            returnValue=$?
            if [ ${returnValue} -eq 0 ]; then
                # Operation was successful
                echo "Theme $themeToInstall was successfully installed to $TARGET_THEME_DIR"
                exit 0
            else
                echo "Error - Theme $themeToInstall failed to be installed to $TARGET_THEME_DIR"
                exit 1
            fi
        else
            echo "Error - Target path $TARGET_THEME_DIR does not exist."
            exit 1
        fi

    else
        # Called from Clover Theme Manager.app

        # Arrays for theme.plists
        declare -a themeList                       # holds the name of each theme
        declare -a themeTitle                      # holds the title of each theme
        declare -a themeAuthor                     # holds the author of each theme
        declare -a themeDescription                # holds the description of each theme

        # Arrays for saving volume info
        declare -a duIdentifier                    # holds the Disk Util identifier of available volume paths
        declare -a duVolumeName                    # holds the Disk Util volume name of available volume paths
        declare -a duVolumeMountPoint              # holds the Disk Util mountpoint of available volume paths
        declare -a duContent                       # holds the Disk Util content of available volume paths
        declare -a duPartitionGuid                 # holds the Disk Util UUID of available volume paths

        # Arrays for theme
        declare -a themeDirPaths                   # holds full path of theme directory on each volume
        declare -a installedThemesOnCurrentVolume  # holds list of themes on currently selected volume
        declare -a installedThemesFoundAfterSearch # holds list of themes found on the selected volume

        # Arrays for list of what themes are installed where.
        declare -a installedThemeName              # holds installed theme names read from prefs
        declare -a installedThemePath              # holds theme paths for installed themes read from prefs
        declare -a installedThemePathDevice        # holds theme device identifiers for installed themes read from prefs
        declare -a installedThemePartitionGUID     # holds theme device UUID's for installed themes read from prefs

        # Globals for newly installed theme before adding to prefs
        ResetNewlyInstalledThemeVars
        ResetUnInstalledThemeVars

        # For using additional theme repositories.
        # Not working in this version
        #declare -a repositoryUrls
        #declare -a repositoryThemes
        #tmp_dir=$(mktemp -d -t theme_manager)
        #ReadRepoUrlList

        # Run child script to gather information of volumes with /EFI/Clover/Themes directories 
        "$findThemeDirs" &

        RefreshHtmlTemplates "managethemes.html"
        IsRepositoryLive
        EnsureLocalSupportDir

        # Clean any old files from support dir from previous app versions
        if [ -f "${WORKING_PATH}/${APP_DIR_NAME}"/*.plist ]; then
            rm "${WORKING_PATH}/${APP_DIR_NAME}"/*.plist
        fi
        if [ -f "${WORKING_PATH}/${APP_DIR_NAME}"/dropdown_html ]; then
            rm "${WORKING_PATH}/${APP_DIR_NAME}"/dropdown_html
        fi
        if [ -f "${WORKING_PATH}/${APP_DIR_NAME}"/theme_html ]; then
            rm "${WORKING_PATH}/${APP_DIR_NAME}"/theme_html
        fi

        # Copy cloverthememanager.js to temp dir
        mkdir "$TEMPDIR"/scripts
        cp "$JSSCRIPTS_DIR"/cloverthememanager.js "$TEMPDIR"/scripts

        EnsureSymlinks
        GetLatestIndexAndEnsureThemeHtml
        WriteToLog "CTM_ThemeDirsScan"
        wait

        # Check for preferences file and add message in to log for initialise.js to detect.
        if [ -f "$gUserPrefsFile".plist ]; then
            [[ $gInitialising -eq 0 ]] && WriteToLog "CTM_ReadPrefsOK"
        fi

        ReadPrefsFile
        CleanInstalledThemesPrefEntries

        # Search ioreg for bootlog and write to file.
        GetBootlog

        # Identify boot device from bootlog and try to have it available.
        # If MBR, try to find it among currently mounted devices.
        # If GPT, match GUID from devicepath. If unmounted ESP, then mount.
        # Returns identifier. For example, disk0s1
        gBootDeviceIdentifier=$( GetSelfDevicePath ) # Calls MountESPAndSearchThemesPath (if GPT and ESP is not mounted)

        # Build internal theme list
        ReadThemeDirList

        # Set internal target and mountpoint then notify UI
        SetTargetAndMountpoint

        # Send list of volumes with /EFI/Clover/Themes directories to UI
        CreateAndSendVolumeDropDownMenu
        if [ $? -eq 0 ]; then
            WriteToLog "CTM_DropDownListOK"
        else
            WriteToLog "CTM_DropDownListNone"
        fi

        # Run bootlog script to read bootlog for theme info and populate theme control paths
        ReadBootLogAndSetPaths "Init"

        # Set last used volumes (as read from prefs), if boot device was not found.
        if [ "$TARGET_THEME_DIR" == "" ] || [ "$TARGET_THEME_DIR" == "-" ]; then
            MapLastSelectedPathToGUID
        fi

        # Inject this as it will be hidden if boot device has not been identified.
        # This way the html will exist so if the user decides to rescan boot device, the
        # paths can be updated in cloverthememanager.js
        CreateControlOptionsHtmlAndInsert

        WriteToLog "Using target $TARGET_THEME_DIR on device $TARGET_THEME_DIR_DEVICE with GUID $TARGET_THEME_PARTITIONGUID" 

        # Send UI target theme entry to display, and other data to set default / restore state of main UI page
        SendUIInitData

        # Read each available place where theme entry could be
        ReadThemeEntriesAndSendToUI

        # Show theme setting control options depending on currently selected device
        ShowHideUIControlOptions

        # If OS is newer than Lion then enable notifications
        if [ $(CheckOsVersion) -gt 11 ]; then
            InsertNotificationCodeInToJS
        fi

        # Write string to mark the end of init.
        # initialise.js looks for this to signify initialisation is complete.
        # At which point it then redirects to the main UI page.
        WriteToLog "Complete!"

        WriteToLog "==== Initialisation End ===="

        # Feedback for command line
        echo "Initialisation complete. Entering loop."

        # Set initialising var to 1 to disable some init messages no longer needed in the log file.
        gInitialising=1

        # Remember parent process id
        parentId=$appPid

        CheckAndRemoveBareClonesNoLongerNeeded
        CheckForAndRemoveThemeGitDirs

        CheckForThemeUpdates &

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: EnableInterface@@"
        SendToUI "EnableInterface‡‡"

        # The messaging system is event driven and quite simple.
        # Run a loop for as long as the parent process ID still exists
        while [ "$appPid" == "$parentId" ];
        do
            sleep 0.25  # Check every 1/4 second.

            #===============================================================
            # Main Message Loop for responding to UI feedback
            #===============================================================

            # Read first line of log file
            logLine=$(head -n 1 "$logJsToBash")

            # Has user selected partition for an /EFI/Clover/themes directory?
            if [[ "$logLine" == *CTM_selectedPartition* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                # js sends "CTM_selectedPartition@" + selectedPartition
                # where selectedPartition is the array element id of 
                RespondToUserDeviceSelection "$logLine"

            # Has the user clicked the Rescan Boot Device button?
            elif [[ "$logLine" == *RescanBootDevice* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                WriteToLog "User selected to Rescan boot device"
                gBootDeviceIdentifier=$( GetSelfDevicePath )
                ReadThemeDirList
                SetTargetAndMountpoint
                CreateAndSendVolumeDropDownMenu
                ReadBootLogAndSetPaths "Rescan"
                SendTargetToUiRunChecks
                CheckForThemeUpdates &
                ReadThemeEntriesAndSendToUI
                nvramPath=""
                configPath=""
                mountpoint=""
                if [ -f "$gNvramPlistFullPath" ]; then
                    nvramPath="$gNvramPlistFullPath"
                fi
                if [ -f "$gConfigPlistFullPath" ]; then
                    configPath="$gConfigPlistFullPath"
                    configPath=$( RenameInternalESPMountPointToEFI "$configPath" )
                fi
                if [ "$gBootDeviceMountPoint" != "" ]; then
                    mountpoint="$gBootDeviceMountPoint"
                fi
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: UpdateThemePaths@${nvramPath}@${configPath}@${mountpoint}"
                SendToUI "UpdateThemePaths‡${nvramPath}‡${configPath}‡${mountpoint}"
                ShowHideUIControlOptions

            # Has the user clicked the OpenPath button?
            elif [[ "$logLine" == *OpenPath* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                CheckThemePathIsStillValid
                retVal=$? # returns 1 if invalid / 0 if valid
                if [ $retVal -eq 0 ]; then
                    [[ ! "$TARGET_THEME_DIR" == "-" ]] && Open "$TARGET_THEME_DIR"
                fi
                WriteToLog "User selected to open $TARGET_THEME_DIR"

            # Has the user clicked the MountESP button?
            elif [[ "$logLine" == *MountESP* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                WriteToLog "User selected to Mount ESP. Looking for EFI/Clover/Themes directory."
                checkAction=$(MountESPAndSearchThemesPath "$gBootDeviceIdentifier")
                if [ "$checkAction" != "Failed" ]; then
                    ReadThemeDirList
                    CreateAndSendVolumeDropDownMenu
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Sending UI: Target@$espID"
                    SendToUI "Target‡$espID"
                    RespondToUserDeviceSelection "‡$espID"
                fi

            # Has the user pressed a theme button to install, uninstall or update?
            elif [[ "$logLine" == *CTM_ThemeAction* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                RespondToUserThemeAction "$logLine"
                returnValue=$?
                if [ ${returnValue} -eq 0 ]; then
                    # Operation was successful
                    GetListOfInstalledThemesAndSendToUI
                    GetFreeSpaceOfTargetDeviceAndSendToUI
                    CheckAndRecordUnManagedThemesAndSendToUI
                    ReadThemeEntriesAndSendToUI
                fi 

            # Has the user pressed the Update All button?
            elif [[ "$logLine" == *CTM_UpdateAll* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                RespondToUpdateAll "$logLine"

            # Has user selected a theme control option?
            elif [[ "$logLine" == *CTM_changeTheme* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                RespondToDropDownMenuChange "$logLine"

            # Has user changed the thumbnail size?
            elif [[ "$logLine" == *CTM_thumbSize* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                # parse message
                # remove everything up until, and including, the first ‡
                thumbSize="${logLine#*‡}"
                UpdatePrefsKey "Thumbnail" "$thumbSize"
                WriteToLog "User changed thumbnail size to $thumbSize"

            # Has user chosen to show uninstalled themes?
            elif [[ "$logLine" == *CTM_showUninstalled* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                UpdatePrefsKey "UnInstalledButton" "Show"
                WriteToLog "User chose to show all themes"

            # Has user chosen to hide uninstalled themes?
            elif [[ "$logLine" == *CTM_hideUninstalled* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                UpdatePrefsKey "UnInstalledButton" "Hide"
                WriteToLog "User chose to hide uninstalled themes"

            # Has user chosen to show thumbnails?
            elif [[ "$logLine" == *CTM_hideThumbails* ]]; then
               ClearTopOfMessageLog "$logJsToBash"
                UpdatePrefsKey "ViewThumbnails" "Show"
                WriteToLog "User chose to show thumbnails"

            # Has user chosen to hide thumbnails?
            elif [[ "$logLine" == *CTM_showThumbails* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                UpdatePrefsKey "ViewThumbnails" "Hide"
                WriteToLog "User chose to hide thumbnails"

            # Has user chosen to hide previews?
            elif [[ "$logLine" == *CTM_hidePreviews* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                UpdatePrefsKey "ShowPreviewsButton" "Hide"
                WriteToLog "User chose to hide previews"

            # Has user chosen to show preview?
            elif [[ "$logLine" == *CTM_showPreviews* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                UpdatePrefsKey "ShowPreviewsButton" "Show"
                WriteToLog "User chose to show previews"

            elif [[ "$logLine" == *started* ]]; then
                ClearTopOfMessageLog "$logJsToBash" 

            # Has user chosen to show/hide bootlog info?
            elif [[ "$logLine" == *CTM_bootlog* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
                # remove everything up until, and including, the first ‡
                gBootlogState="${logLine#*‡}"
                UpdatePrefsKey "ShowHideBootlog" "$gBootlogState"

            # Clear and Relaunch messages?
            elif [[ "$logLine" == *Relaunch* ]]; then
                ClearTopOfMessageLog "$logJsToBash"

            # Look out for App Transport Security message and clear
            elif [[ "$logLine" == *Transport* ]]; then
                ClearTopOfMessageLog "$logJsToBash"

            # App temporary work around for *** WARNING: Method convertPointToBase:
            elif [[ "$logLine" == *convertPointToBase* ]]; then
                ClearTopOfMessageLog "$logJsToBash"
            fi

            # Get process ID of parent
            appPid=$( ps -p ${pid:-$$} -o ppid= )
        done
        # CleanUp  # /tmp/CloverThemeManager Dir is now deleted by main app on exit.
        exit 0
    fi
else
    WriteToLog "CTM_GitFail"
    exit 1
fi