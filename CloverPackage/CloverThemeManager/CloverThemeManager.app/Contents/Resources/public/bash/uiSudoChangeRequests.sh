#!/bin/bash

# A script for Clover Theme Manager
# Copyright (C) 2014-2015 Blackosx
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

# ---------------------------------------------------------------------------------------
SendToUI() {
    echo "${1}" >> "$logBashToJs"
}

# ---------------------------------------------------------------------------------------
WriteToLog() {
    printf "‡${1}‡\n" >> "$logFile"
}

# ---------------------------------------------------------------------------------------
MoveThemeToTarget()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}MoveThemeToTarget()"

    local successFlag=1

    # Create theme dir on target.
    chckDir=0
    mkdir "$targetThemeDir" && chckDir=1
    if [ $chckDir -eq 1 ]; then

        # Move unpacked files to target theme path.
        cd "$unPackDir"/themes
        if [ -d "$themeName" ]; then
            shopt -s dotglob
            mv "$themeName"/* "$targetThemeDir" && successFlag=0
            shopt -u dotglob
        fi
    fi

    echo $successFlag
}

# ---------------------------------------------------------------------------------------
UnInstallTheme()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}UnInstallTheme()"

    local successFlag=1

    cd "$targetThemeDir"
    if [ -d "$themeName" ]; then
        rm -rf "$themeName" && WriteToLog "Deletion was successful." && successFlag=0
    fi

    echo $successFlag
}

# ---------------------------------------------------------------------------------------
UpdateTheme()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}UpdateTheme()"

    local successFlag=1

    # Remove existing theme dir on target.
    if [ -d "$targetThemeDir" ]; then
        chckDir=0
        WriteToLog "Removing existing $targetThemeDir files"
        rm -rf "$targetThemeDir"/* && chckDir=1
        if [ $chckDir -eq 1 ]; then
            # Move unpacked files to target theme path.
            cd "$unPackDir"/themes
            if [ -d "$themeName" ]; then
                WriteToLog "Moving updated $themeName theme files to $targetThemeDir"
                shopt -s dotglob
                mv "$themeName"/* "$targetThemeDir" && successFlag=0
                shopt -u dotglob
            fi
        fi
    fi

    echo $successFlag
}

# ---------------------------------------------------------------------------------------
SetNVRAMVariable()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SetNVRAMVariable()"

    local successFlag=1

    WriteToLog "Setting Clover.Theme NVRAM Variable"
    nvram Clover.Theme="$themeName" && successFlag=0

    echo $successFlag
}

# ---------------------------------------------------------------------------------------
DeleteNVRAMVariable()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}DeleteNVRAMVariable()"

    local successFlag=1

    WriteToLog "Deleting Clover.Theme NVRAM Variable"
    nvram -d Clover.Theme && successFlag=0

    echo $successFlag
}

# ---------------------------------------------------------------------------------------
SetNVRAMFile()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SetNVRAMFile()"

    local successFlag=1

    if [ -f "$fileToChange" ]; then

        successFlag=$( DeletePlistEntry )

        if [ $successFlag -eq 0 ]; then
            successFlag=1
            if [[ "$fileToChange" == *"nvram"* ]]; then
                WriteToLog "Adding Clover.Theme Key '$themeName' to $fileToChange"
                /usr/libexec/PlistBuddy -c "Add :Clover.Theme data $themeName" "$fileToChange" && successFlag=0
            elif [[ "$fileToChange" == *"config"* ]]; then
                WriteToLog "Adding theme entry '$themeName' to $fileToChange"
                /usr/libexec/PlistBuddy -c "Add :GUI:Theme string $themeName" "$fileToChange" && successFlag=0
            fi
        fi
    else
        WriteToLog "$fileToChange could not be found."
    fi
    
    echo $successFlag
}

# ---------------------------------------------------------------------------------------
DeletePlistEntry()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}DeletePlistEntry()"

    local successFlag=1

    if [ -f "$fileToChange" ]; then

        # Remove existing entry if already exists
        if [[ "$fileToChange" == *"nvram.plis"* ]]; then
            local readCurrentTheme=$( /usr/libexec/PlistBuddy -c "Print:Clover.Theme" "$fileToChange" 2>/dev/null )
            if [ "$readCurrentTheme" != "" ]; then
                WriteToLog "Removing existing Clover.Theme Key '$readCurrentTheme' from $fileToChange"
                /usr/libexec/PlistBuddy -c "Remove :Clover.Theme" "$fileToChange" && successFlag=0
            else
                WriteToLog "Clover.Theme Key does not already exist in $fileToChange"
                successFlag=0
            fi
        elif [[ "$fileToChange" == *"config.plis"* ]]; then
            local readCurrentTheme=$( /usr/libexec/PlistBuddy -c "Print:GUI:Theme" "$fileToChange" 2>/dev/null )
            if [ "$readCurrentTheme" != "" ]; then
                WriteToLog "Removing Theme Entry '$readCurrentTheme' from $fileToChange"
                /usr/libexec/PlistBuddy -c "Remove :GUI:Theme" "$fileToChange" && successFlag=0
            else
                WriteToLog "Theme entry does not already exist in $fileToChange"
                successFlag=0
            fi
        fi
    fi

    echo $successFlag
}

# ---------------------------------------------------------------------------------------
MountESP()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}MountESP()"

    local passedDevice="$1"
    local passedMountpoint="$2"
    local successFlag=1
    local targetFormat=$( fstyp "$passedDevice" )

    if [ "$passedDevice" != "" ] && [ "$targetFormat" != "" ] && [ "$passedMountpoint" != "" ]; then
        mount -t "$targetFormat" "$passedDevice" "$passedMountpoint" && successFlag=0
    fi

    if [ $successFlag -eq 0 ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Mounted $passedDevice successfully. Checking for /EFI/Clover/Themes"
        # Does this device contain /efi/clover/themes directory?
        local themeDir=$( find "$passedMountpoint"/EFI/Clover -depth 1 -type d -iname "Themes" 2>/dev/null )
        if [ ! "$themeDir" ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}No /EFI/Clover/Themes directory found on $passedDevice"
            umount -f "$passedMountpoint" && successFlag=1
            [[ successFlag -eq 1 ]] && [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}unmounted $passedDevice"
        else
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/EFI/Clover/Themes directory found on $passedDevice"
        fi
    fi

    echo $successFlag
}

# ---------------------------------------------------------------------------------------
ManageESP()
{
    # Read espList.txt file
    # Store identifiers for unmounted ESP's in array
    # espList.txt is created by findThemeDirs.sh script.

    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ManageESP()"

    local mountSuccess=1
    local mountedEspWithThemes=0
    unset unmountedEsp

    if [ -f "$espList" ]; then
        oIFS="$IFS"; IFS=$'\r\n'
        while read -r line
        do
            if [[ "$line" == *@U ]]; then
                unmountedEsp+=( "${line%@*}" )
            fi
        done < "$espList"
        IFS="$oIFS"

        # Loop through partitions
        for (( s=0; s<${#unmountedEsp[@]}; s++ ))
        do
            local mountPoint=`/usr/bin/mktemp -d /Volumes/${gESPMountPrefix}XXXXXXXXX` && mountSuccess=$( MountESP "/dev/${unmountedEsp[$s]}" "$mountPoint" )
            if [ $mountSuccess -eq 0 ]; then
                (( mountedEspWithThemes++ ))
                mountSuccess=1
            fi
        done
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Error. Missing $espList file"
    fi

    echo $mountedEspWithThemes
}

# ---------------------------------------------------------------------------------------
FindMbrDevice()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}FindMbrDevice()"

    local foundDisk=""

    fileToRead="${TEMPDIR}/bootDeviceInfo.txt"
    bootdeviceLine=$( cat "$fileToRead" )
    declare -a bootDeviceInfo
    oIFS="$IFS"; IFS=$'‡'
    bootDeviceInfo=($bootdeviceLine)

    IFS=$'\r\n'
    while read -r line
    do
        if [[ "$line" != "" ]]; then

            [[ DEBUG -eq 1 ]] && WriteToLog "/usr/sbin/fdisk /dev/$line | grep ${bootDeviceInfo[4]}"
            readFdisk=$( /usr/sbin/fdisk /dev/"$line" | grep "${bootDeviceInfo[4]}" )
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}readFdisk=$readFdisk"

            if [ $readFdisk ]; then # We have a match on total sectors
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found block size match: ${bootDeviceInfo[4]}"

                # Check if partition number is the same
                partNum=$( echo "${readFdisk%:*}" | tr -d '* ' )
                if [ "$partNum" == "${bootDeviceInfo[0]}" ]; then # We have a match on partition number
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found partition number match: $partNum"

                    # Check if start block is the same
                    startBlock=$( echo "${readFdisk#*[}" | tr -d ' ' )
                    startBlock=$( echo "${startBlock%%-*}" )
                    if [ "$startBlock" == "${bootDeviceInfo[3]}" ]; then # We have a match on start block
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found start block match: $startBlock"

                        # Check Signature matches by comparing bytes 01B8->01BE
                        signature=$( dd 2>/dev/null if="/dev/$line" bs=4 count=1 skip=110 | perl -ne '@a=split"";for(@a){printf"%02x",ord}' )
                        signatureLE="${bootDeviceInfo[2]:8:2}${bootDeviceInfo[2]:6:2}${bootDeviceInfo[2]:4:2}${bootDeviceInfo[2]:2:2}"
                        signature=$( echo $signature | tr '[:upper:]' '[:lower:]' )
                        signatureLE=$( echo $signatureLE | tr '[:upper:]' '[:lower:]' )
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Comparing signatures: $signature vs $signatureLE"
                        if [ "$signature" == "$signatureLE" ]; then # We have a match on signature
                            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found signature match: $signature"
                            foundDisk="${line}s${partNum}"
                            break
                        fi
                    fi
                fi
            fi
        fi
    done < "$mbrList" # Previously populated list of MBR disks. Done in findThemeDirs.sh
    IFS="$oIFS"

    echo "$foundDisk"
}

# ---------------------------------------------------------------------------------------

# Resolve path
SELF_PATH=$(cd -P -- "$(dirname -- "$0")" && pwd -P) && SELF_PATH=$SELF_PATH/$(basename -- "$0")
source "${SELF_PATH%/*}"/shared.sh

[[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}uiSudoChangeRequests.sh()"

declare -a unmountedEsp

# Passing strings with spaces fails as that's used as a delimiter.
# Instead, I pass each argument delimited by character ‡

# Parse arguments
declare -a "arguments"
passedArguments="$@"
numFields=$( grep -o "‡" <<< "$passedArguments" | wc -l )
(( numFields++ ))

arguments[1]=$( echo "$passedArguments" | awk '{split($0,a,"‡"); print a[1]}' )
arguments[2]=$( echo "$passedArguments" | awk '{split($0,a,"‡"); print a[2]}' )
arguments[3]=$( echo "$passedArguments" | awk '{split($0,a,"‡"); print a[3]}' )
arguments[4]=$( echo "$passedArguments" | awk '{split($0,a,"‡"); print a[4]}' )
arguments[5]=$( echo "$passedArguments" | awk '{split($0,a,"‡"); print a[5]}' )
arguments[6]=$( echo "$passedArguments" | awk '{split($0,a,"‡"); print a[6]}' )

# print results (debug)
for (( u=0; u<${#arguments[@]}; u++ ))
do
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}arguments[$u]=${arguments[$u]}"
done

whichFunction="${arguments[2]}"

case "$whichFunction" in                             
     "Move"                       ) targetThemeDir="${arguments[3]}"
                                    unPackDir="${arguments[4]}"
                                    themeName="${arguments[5]}"
                                    MoveThemeToTarget
                                    ;;
     "UnInstall"                  ) targetThemeDir="${arguments[3]}"
                                    themeName="${arguments[4]}"
                                    UnInstallTheme
                                    ;;
     "Update"                     ) targetThemeDir="${arguments[3]}"
                                    unPackDir="${arguments[4]}"
                                    themeName="${arguments[5]}"
                                    UpdateTheme
                                    ;;
     "SetNVRAMVar"                ) themeName="${arguments[3]}"
                                    SetNVRAMVariable
                                    ;;
     "DeleteNVRAMVar"             ) DeleteNVRAMVariable
                                    ;;
     "SetNVRAMFile"               ) themeName="${arguments[3]}"
                                    fileToChange="${arguments[4]}"
                                    SetNVRAMFile
                                    ;;
     "DeleteThemePlistEntry"      ) fileToChange="${arguments[3]}"
                                    DeletePlistEntry
                                    ;;
     "FindMBrBootDevice"          ) FindMbrDevice
                                    ;;   
     "ManageESP"                  ) ManageESP
                                    ;;   
esac