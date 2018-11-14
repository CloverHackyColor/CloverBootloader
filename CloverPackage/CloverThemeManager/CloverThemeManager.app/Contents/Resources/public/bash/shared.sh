#!/bin/bash

# Resolve path
#SELF_PATH=$(cd -P -- "$(dirname -- "$0")" && pwd -P) && SELF_PATH=$SELF_PATH/$(basename -- "$0")

# Set out other directory paths based on SELF_PATH
PUBLIC_DIR="${SELF_PATH%/*}"
PUBLIC_DIR="${PUBLIC_DIR%/*}"
ASSETS_DIR="$PUBLIC_DIR"/assets
SCRIPTS_DIR="$PUBLIC_DIR"/bash
JSSCRIPTS_DIR="$PUBLIC_DIR"/scripts
TOOLS_DIR="$PUBLIC_DIR"/tools
WORKING_PATH="${HOME}/Library/Application Support"
APP_DIR_NAME="CloverThemeManager"
TEMPDIR="/tmp/${APP_DIR_NAME}"
UNPACKDIR="${WORKING_PATH}/${APP_DIR_NAME}/UnPack"

# Scripts
uiSudoChanges="${SCRIPTS_DIR}/uiSudoChangeRequests.sh"
findThemeDirs="${SCRIPTS_DIR}/findThemeDirs.sh"
bootlogScript="${SCRIPTS_DIR}/bootlog.sh"
updateScript="${TEMPDIR}/updateScript.sh"

# Double escape spaces for osascript
uiSudoChanges=$( echo "$uiSudoChanges" | sed  's/ /\\\\ /g' )

# Set out file paths
logFile="${TEMPDIR}/CloverThemeManagerLog.txt"
themeDirInfo="${TEMPDIR}/themeDirInfo.txt"
espList="${TEMPDIR}/espList.txt"
mbrList="${TEMPDIR}/mbrList.txt"
bootlogScriptOutfile="${TEMPDIR}/bootlogOut.txt"
bootDeviceInfo="${TEMPDIR}/bootDeviceInfo.txt"
bootLogFile="${TEMPDIR}/boot.log"
partutil="${TOOLS_DIR}"/partutil
logJsToBash="${TEMPDIR}/jsToBash" # Note - this is created in AppDelegate.m
logBashToJs="${TEMPDIR}/bashToJs" # Note - this is created in AppDelegate.m

# Globals
remoteRepositoryUrl="https://git.code.sf.net/p/cloverefiboot"
zeroUUID="00000000-0000-0000-0000-000000000000"
gESPMountPrefix="ctmTempMp"
debugIndent="    "
debugIndentTwo="${debugIndent}${debugIndent}"
COMMANDLINE=0
DEBUG=1

# Common Functions
# ---------------------------------------------------------------------------------------
WriteToLog() {
    if [ $COMMANDLINE -eq 0 ]; then
        # printf "${1}\n" >> "$logFile"
        printf "%s\n" "${1}" >> "$logFile"
    else
        # printf "${1}\n"
        printf "%s\n" "${1}"
    fi
}

# ---------------------------------------------------------------------------------------
WriteLinesToLog() {
    if [ $COMMANDLINE -eq 0 ]; then
        if [ $DEBUG -eq 1 ]; then
            printf "${debugIndent}===================================\n" >> "$logFile"
        else
            printf "===================================\n" >> "$logFile"
        fi
    else
        printf "===================================\n"
    fi
}

# ---------------------------------------------------------------------------------------
SendToUI() {
    echo "${1}" >> "$logBashToJs"
}
