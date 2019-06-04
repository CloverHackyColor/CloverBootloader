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
# Extracts bootlog from ioreg and then parses it for theme info.
# Html is then constructed and injected in to the main template.

# v0.76.9
    
# ---------------------------------------------------------------------------------------
SetHtmlBootlogSectionTemplates()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SetHtmlBootlogSectionTemplates()"

    blcOpen=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">Boot Device Info<\/span><\/div>\r")

    blcLineDeviceInfoMbr=$(printf "        <div id=\"bandDescription\">\r")
    blcLineDeviceInfoMbr="$blcLineDeviceInfoMbr"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Type:<\/span><span class=\"infoBody\">${blBootDeviceType} (${blBootDevicePartType})<\/span><\/div>\r")
    blcLineDeviceInfoMbr="$blcLineDeviceInfoMbr"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Signature:<\/span><span class=\"infoBody\">${blBootDevicePartSignature}<\/span><\/div>\r")
    blcLineDeviceInfoMbr="$blcLineDeviceInfoMbr"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Partition #:<\/span><span class=\"infoBody\">${blBootDevicePartition}<\/span><\/div>\r")
    blcLineDeviceInfoMbr="$blcLineDeviceInfoMbr"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Start:<\/span><span class=\"infoBody\">${blBootDevicePartStartDec}<\/span><\/div>\r")
    blcLineDeviceInfoMbr="$blcLineDeviceInfoMbr"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Size:<\/span><span class=\"infoBody\">${blBootDevicePartSizeDec}<\/span><\/div>\r")
    blcLineDeviceInfoMbr="$blcLineDeviceInfoMbr"$(printf "        <\/div>\r")
    blcLineDeviceInfoMbr="$blcLineDeviceInfoMbr"$(printf "\r")

    blcLineDeviceInfoGpt=$(printf "        <div id=\"bandDescription\">\r")
    blcLineDeviceInfoGpt="$blcLineDeviceInfoGpt"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Type:<\/span><span class=\"infoBody\">${blBootDeviceType} (${blBootDevicePartType})<\/span><\/div>\r")
    blcLineDeviceInfoGpt="$blcLineDeviceInfoGpt"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Signature:<\/span><span class=\"infoBody\">${blBootDevicePartSignature}<\/span><\/div>\r")
    blcLineDeviceInfoGpt="$blcLineDeviceInfoGpt"$(printf "        <\/div>\r")
    blcLineDeviceInfoGpt="$blcLineDeviceInfoGpt"$(printf "\r")

    blcLineDevice=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">Boot Device<\/span><\/div>\r")
    blcLineDevice="$blcLineDevice"$(printf "        <div id=\"bandDescription\">\r")
    blcLineDevice="$blcLineDevice"$(printf "            <div id=\"bandIdentifer\"><span class=\"infoTitle\">Identifier:<\/span><span class=\"infoBody\">${gBootDeviceIdentifierPrint}<\/span><\/div>\r")
    blcLineDevice="$blcLineDevice"$(printf "            <div id=\"bandMountpoint\"><span class=\"infoTitle\">mountpoint:<\/span><span class=\"infoBody\">${mountpointPrint}<\/span><\/div>\r")
    blcLineDevice="$blcLineDevice"$(printf "        <\/div>\r")
    blcLineDevice="$blcLineDevice"$(printf "\r")

    blcLineDeviceRescan=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">Boot Device<\/span><\/div>\r")
    blcLineDeviceRescan="$blcLineDeviceRescan"$(printf "        <div id=\"bandDescription\">\r")
    blcLineDeviceRescan="$blcLineDeviceRescan"$(printf "            <div id=\"bandIdentifer\"><span class=\"infoTitle\">Identifier:<\/span><span class=\"infoBody\">${gBootDeviceIdentifierPrint}<\/span><\/div>\r")
    blcLineDeviceRescan="$blcLineDeviceRescan"$(printf "            <div id=\"bandMountpoint\"><span class=\"infoTitle\">mountpoint:<\/span><span class=\"infoBody\">${mountpointPrint}<\/span><\/div>\r")
    blcLineDeviceRescan="$blcLineDeviceRescan"$(printf "            <div id=\"RescanButton\">\r")
    blcLineDeviceRescan="$blcLineDeviceRescan"$(printf "                <button type=\"button\" id=\"RescanBootDeviceButton\" class=\"rescanButton\">Rescan Boot Device<\/button>\r")
    blcLineDeviceRescan="$blcLineDeviceRescan"$(printf "            <\/div> <!-- End RescanButton -->\r")
    blcLineDeviceRescan="$blcLineDeviceRescan"$(printf "        <\/div>\r")
    blcLineDeviceRescan="$blcLineDeviceRescan"$(printf "\r")

    if [ $blFastOption -eq 0 ]; then

        # No theme was used so don't we don't know if theme existed or not.
        blcLineNvram=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">NVRAM<\/span><\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "        <div id=\"bandDescription\">\r")
        blcLineNvram="$blcLineNvram"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">read from:<\/span><span class=\"infoBody\">${blNvramReadFromPrint}<\/span><\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Clover.Theme:<\/span><span class=\"infoBodyTheme\">${blNvramThemeEntry}<\/span><\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "        <\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "\r")

    else

        blcLineNvram=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">NVRAM<\/span><\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "        <div id=\"bandDescription\">\r")
        blcLineNvram="$blcLineNvram"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">read from:<\/span><span class=\"infoBody\">${blNvramReadFromPrint}<\/span><\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Clover.Theme:<\/span><span class=\"infoBodyTheme\">${blNvramThemeEntry}<\/span><\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">theme existed?:<\/span><span class=\"${nvramThemeExistsCssClass}\">${nvramExistText}<\/span><\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "        <\/div>\r")
        blcLineNvram="$blcLineNvram"$(printf "\r")
    
        blcLineNvramNoTheme=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">NVRAM<\/span><\/div>\r")
        blcLineNvramNoTheme="$blcLineNvramNoTheme"$(printf "        <div id=\"bandDescription\">\r")
        blcLineNvramNoTheme="$blcLineNvramNoTheme"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">read from:<\/span><span class=\"infoBody\">${blNvramReadFromPrint}<\/span><\/div>\r")
        blcLineNvramNoTheme="$blcLineNvramNoTheme"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">Clover.Theme:<\/span><span class=\"infoBodyTheme\"><\/span><\/div>\r")
        blcLineNvramNoTheme="$blcLineNvramNoTheme"$(printf "        <\/div>\r")
        blcLineNvramNoTheme="$blcLineNvramNoTheme"$(printf "\r")

    fi

    blcLineConfig=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">config.plist<\/span><\/div>\r")
    blcLineConfig="$blcLineConfig"$(printf "        <div id=\"bandDescription\">\r")
    blcLineConfig="$blcLineConfig"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">path:<\/span><span class=\"infoBodyReplaceable\">${blConfigPlistFilePathPrint}<\/span><\/div>\r")
    blcLineConfig="$blcLineConfig"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">theme entry:<\/span><span class=\"infoBodyTheme\">${blConfigPlistThemeEntry}<\/span><\/div>\r")
    blcLineConfig="$blcLineConfig"$(printf "        <\/div>\r")
    blcLineConfig="$blcLineConfig"$(printf "\r")

    blcLineThemeAsked=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">Theme asked for<\/span><\/div>\r")
    blcLineThemeAsked="$blcLineThemeAsked"$(printf "        <div id=\"bandDescription\">\r")
    blcLineThemeAsked="$blcLineThemeAsked"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">path:<\/span><span class=\"infoBodyReplaceable\">${blThemeAskedForPathPrint}<\/span><\/div>\r")
    blcLineThemeAsked="$blcLineThemeAsked"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">did theme exist?<\/span><span class=\"${themeExistCssClass}\">${blThemeAskedForExisted}<\/span><\/div>\r")
    blcLineThemeAsked="$blcLineThemeAsked"$(printf "        <\/div>\r")
    blcLineThemeAsked="$blcLineThemeAsked"$(printf "\r")

    blcLineThemeUsed=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">Theme used<\/span><\/div>\r")
    blcLineThemeUsed="$blcLineThemeUsed"$(printf "        <div id=\"bandDescription\">\r")
    blcLineThemeUsed="$blcLineThemeUsed"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">path:<\/span><span class=\"infoBodyReplaceable\">${blThemeUsedPathPrint}<\/span><\/div>\r")
    blcLineThemeUsed="$blcLineThemeUsed"$(printf "            <div id=\"bandCbandColumnLeftolumnRight\"><span class=\"infoTitle\">Chosen:<\/span><span class=\"infoBodyTheme\">${blThemeNameChosen}<\/span><\/div>\r")
    blcLineThemeUsed="$blcLineThemeUsed"$(printf "        <\/div>\r")
    blcLineThemeUsed="$blcLineThemeUsed"$(printf "\r")

    blcLineOverrideUi=$(printf "        <div id=\"bandHeader\"><span class=\"infoTitle\">Override in GUI<\/span><\/div>\r")
    blcLineOverrideUi="$blcLineOverrideUi"$(printf "        <div id=\"bandDescription\">\r")
    blcLineOverrideUi="$blcLineOverrideUi"$(printf "            <div id=\"bandColumnLeft\"><span class=\"infoTitle\">theme:<\/span><span class=\"infoBody\">${blGuiOverrideTheme}<\/span><\/div>\r")
    blcLineOverrideUi="$blcLineOverrideUi"$(printf "        <\/div>\r")
    blcLineOverrideUi="$blcLineOverrideUi"$(printf "\r")
}

# ---------------------------------------------------------------------------------------
SetBootlogTextColourClasses()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}SetBootlogTextColourClasses()"

    # Set class and text of NVRAM theme exists
    if [ $blNvramThemeExists -eq 0 ]; then
        nvramThemeExistsCssClass="infoBodyGreen"
        nvramExistText="Yes"
    elif [ $blNvramThemeExists -eq 1 ]; then
        nvramThemeExistsCssClass="infoBodyRed"
        nvramExistText="No"
    fi

    # Set class of 'theme exists' text
    if [ "$blThemeAskedForExisted" == "No" ]; then
        themeExistCssClass="infoBodyRed"
    else
        themeExistCssClass="infoBodyGreen"
    fi
}
  
# ---------------------------------------------------------------------------------------
ReadBootLog()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}ReadBootLog()"

    # Set default vars
    blCloverRevision=""               # Clover revision
    blBootType=""                     # Either legacy or UEFI
    blBootDeviceType="-"              # Example: USB, SATA, VenHW, NVMe
    blBootDevicePartition=""          # Example: 1
    blBootDevicePartType=""           # Example: MBR, GPT
    blBootDevicePartSignature=""      # Example: GUID (for GPT), 0x00000000 (for MBR)
    blBootDevicePartStart=""          # Example: 0x28
    blBootDevicePartSize=""           # Example: 0x64000
    blConfigOem=""                    # Example: OEM
    blEmuVariable=1                   # Set to 0 if EmuVariable driver is loaded
    blNvramPlistVolume=""             # OS X volume name: Example: Macintosh HD
    blNvramPlistExists=1              # Set to 0 if existence of nvram.plist is detected
    blNvramThemeEntry=""              # Theme name from nvram
    blNvramBootArgs=1                 # Set to 0 if boot-args found in NVRAM. (Shows NVRAM is working if Clover.Theme var not used).
    blNvramReadFrom=""                # Either full path to nvram.plist or 'Native NVRAM'
    blNvramThemeExists=1              # Set to 0 if theme exists
    blNvramThemeAbsent=1              # Set to 0 if theme in nvram is absent
    blConfigPlistFilePath=""          # Path for config.plist
    blConfigPlistThemeEntry=""        # Theme name from config.plist
    blConfigThemePlistNotFound=1      # Set to 0 if theme plist not found
    blGuiOverrideTheme=""             # Theme name if set from GUI
    blGuiOverrideThemeChanged=1       # Set to 0 if theme set in GUI was used
    blThemeAskedForPath=""            # Set to path
    blThemeAskedForTitle=""           # Set to theme name as it's detected.
    blThemeAskedForExisted="Yes"      # Set to 'No' if not exists, or 'Always' if embedded.
    blUsedRandomTheme=1               # Set to 0 if no default theme set and random theme used
    blThemeUsedPath=""                # Path of the theme used
    blUsingTheme=""                   # Theme used
    blThemeNameChosen=""              # Name of theme finally chosen
    blUsingEmbedded=1                 # Set to 0 if embedded theme used
    blTextOnlyOption=1                # Set to 0 if boot log has TextOnly set to true
    blFastOption=1                    # Set to 0 if boot log has Fast Boot set to true

    # gBootDeviceIdentifier is passed from script.sh # Example disk0s1 or 'Failed'
    # gBootDeviceMountpoint is passed from script.sh
    mountpointPrint=""

    while read -r lineRead
    do

        if [[ "$lineRead" == *"Starting Clover"* ]]; then
            blCloverRevision="${lineRead##*Starting Clover revision: }"
            blCloverRevision="${blCloverRevision% on*}"
            blBootType="${lineRead#*on }"
            if [[ "$blBootType" == *"CLOVER EFI"* ]]; then
                blBootType="Legacy"
            else
                blBootType="UEFI"
            fi
        fi

        #0:100  0:000  SelfDevicePath=PciRoot(0x0)\Pci(0x1F,0x2)\Sata(0x0,0xFFFF,0x0)\HD(1,GPT,BC1B343C-2D6B-4C0C-8B88-71C2AFCF6E65,0x28,0x64000) @C7AA598
        if [[ "$lineRead" == *"SelfDevicePath"* ]]; then

            oIFS="$IFS"
            # Get device path and split in to parts
            devicePath="${lineRead#*=}"
            declare -a devicePathArr
            IFS=$'\\'
            devicePathArr=($devicePath)
            IFS="$oIFS"

            # Identify sections
            idx=${#devicePathArr[@]}
            while [[ "${devicePathArr[$idx]}" != "HD("* ]]
            do
                ((idx--))
            done

            blBootDeviceType="${devicePathArr[$((idx-1))]%(*}"
            devicePathHD="${devicePathArr[$idx]%)*}"

            # Split HD in to parts   
            devicePathHD="${devicePathHD#*(}"
            # Should be something like these examples:
            #1,MBR,0x2A482A48,0x2,0x4EFC1B80
            #2,GPT,F55D9AC4-08A8-4269-9A8E-396DBE7C7943,0x64028,0x1C0000
            declare -a hdArr
            IFS=$','
            hdArr=($devicePathHD)
            IFS="$oIFS"
            blBootDevicePartition="${hdArr[0]}"
            blBootDevicePartType="${hdArr[1]}"
            blBootDevicePartSignature="${hdArr[2]}"
            blBootDevicePartStart="${hdArr[3]}"
            blBootDevicePartSize="${hdArr[4]}"

            if [[ "$blBootDevicePartType" == *GPT* ]]; then
                # Translate Device UUID to mountpoint
                if [ "$gBootDeviceIdentifier" != "" ]; then
                    #mountpoint=$( "$partutil" --show-mountpoint "$gBootDeviceIdentifier" )
                    if [[ "$gBootDeviceMountpoint" == *$gESPMountPrefix* ]]; then
                        mountpointPrint="$gBootDeviceMountpoint (aka /Volumes/EFI)"
                    else
                        mountpointPrint="$gBootDeviceMountpoint"
                    fi
                fi
            elif [[ "$blBootDevicePartType" == *MBR* ]]; then
                if [ "$gBootDeviceIdentifier" != "" ] && [ "$gBootDeviceIdentifier" != "Failed" ]; then
                    #mountpoint="/"$( df -laH | grep /dev/"$gBootDeviceIdentifier" | cut -d'/' -f 4- )
                    # If only a forward slash then get current volume name
                    if [ "$gBootDeviceMountpoint" == "/" ]; then
                        gBootDeviceMountpoint="/Volumes/"$(ls -1F /Volumes | sed -n 's:@$::p')
                    fi
                    mountpointPrint="$gBootDeviceMountpoint"
                fi
            fi
        fi

        # 3:539  0:023  Using OEM config.plist at path: EFI\CLOVER\config.plist
        if [[ "$lineRead" == *"config.plist at path:"* ]]; then
            blConfigOem="${lineRead##*Using }"
            blConfigOem="${blConfigOem% config.plist*}"
        fi

        # 3:539  0:000  EFI\CLOVER\config.plist loaded: Success
        if [[ "$lineRead" == *"config.plist loaded: Success"* ]]; then
            blConfigPlistFilePath=$( echo "$lineRead" | awk '{print $3}' )
            blConfigPlistFilePath="/"$( echo "$blConfigPlistFilePath" | sed 's/\\/\//g' )
        fi

        # 1:862  0:000  Fast option enabled
        if [[ "$lineRead" == *"Fast option enabled" ]]; then
            blFastOption=0
        fi

        # 1:862  0:000  TextOnly option enabled
        if [[ "$lineRead" == *"TextOnly option enabled" ]]; then
            blTextOnlyOption=0
        fi

        # 0:110  0:000  Default theme: red
        if [[ "$lineRead" == *"Default theme"* ]]; then
            blConfigPlistThemeEntry="${lineRead##*: }"
            blThemeAskedForTitle="$blConfigPlistThemeEntry"
        fi

        # 1:104  0:000  EmuVariable InstallEmulation: orig vars copied, emu.va.....
        if [[ "$lineRead" == *"EmuVariable InstallEmulation"* ]] && [[ "$lineRead" == *"orig vars copied"* ]] ; then
            blEmuVariable=0
        fi

        # 6:149  0:172  Loading nvram.plist from Vol 'OSX' - loaded, size=2251
        if [[ "$lineRead" == *"Loading nvram.plist"* ]]; then
            blNvramPlistVolume="${lineRead#*\'}"
            blNvramPlistVolume="${blNvramPlistVolume%\'*}"
        fi

        # 6:167  0:018  PutNvramPlistToRtVars ...
        if [[ "$lineRead" == *"PutNvramPlistToRtVars"* ]]; then
            blNvramPlistExists=0
        fi

        # 6:167  0:000   Adding Key: Clover.Theme: Size = 11, Data: 62 6C 61 63 6B 5F 67 72 65 65 6E 
        if [[ "$lineRead" == *"Adding Key: Clover.Theme:"* ]]; then
            # Remove any trailing spaces
            blNvramThemeEntry=$( echo "${lineRead##*Data:}" | sed 's/ *$//g' )
            # Check for new style boot log
            if [ "${blNvramThemeEntry:0:5}" != " Size" ]; then
                blNvramThemeEntry="${blNvramThemeEntry// /\\x}"
                blNvramThemeEntry="$blNvramThemeEntry\\n"
                blNvramThemeEntry=$( printf "$blNvramThemeEntry" )
                blNvramReadFrom="/Volumes/${blNvramPlistVolume}/nvram.plist"
                blThemeAskedForTitle="$blNvramThemeEntry"
            else # older style boot log
                blNvramThemeEntry="not shown in bootlog"
                blNvramReadFrom="/Volumes/${blNvramPlistVolume}/nvram.plist"
            fi
        fi

        # 0:718  0:000  theme ios7 chosen from nvram is absent, using theme defined in config: red
        if [[ "$lineRead" == *"chosen from nvram is absent"* ]]; then
            blNvramThemeEntry="${lineRead#*theme }"
            blNvramThemeEntry=$( echo "${blNvramThemeEntry%chosen from*}" | sed 's/ *$//g' )
            if [ "$blNvramReadFrom" == "" ]; then
                blNvramReadFrom="Native NVRAM"
            fi
            blNvramThemeAbsent=0
            blThemeAskedForTitle="$blConfigPlistThemeEntry"
        fi

        # 0:732  0:000  found boot-args in NVRAM:-v kext-dev-mode=1, size=18
        if [[ "$lineRead" == *"found boot-args in NVRAM"* ]]; then
            blNvramBootArgs=0
        fi

        if [[ "$lineRead" == *"EDITED:"* ]]; then
            blGuiOverrideTheme="${lineRead##*: }"
        fi

        if [[ "$lineRead" == *"change theme"* ]]; then
            if [ "$blGuiOverrideTheme" != "" ]; then
                blGuiOverrideThemeChanged=0
            fi
        fi

        if [[ "$lineRead" == *"no default theme"* ]]; then
            if [[ "$lineRead" == *"get random"* ]]; then
                blUsedRandomTheme=0
            fi
        fi

        # 1:805  0:000  GlobalConfig: theme.plist not found, get random theme BGM
        if [[ "$lineRead" == *"theme.plist not found, get random theme"* ]]; then
            blConfigThemePlistNotFound=0
            blUsedRandomTheme=0
        fi

        # 0:718  0:000  Using theme 'red' (EFI\CLOVER\themes\red)
        if [[ "$lineRead" == *"Using theme"* ]]; then
            blUsingTheme="${lineRead#*\'}"
            blUsingTheme="${blUsingTheme%\'*}"
            blThemeUsedPath="${lineRead#*(}"
            blThemeUsedPath="${blThemeUsedPath%)*}"
            blThemeUsedPath=$( echo "$blThemeUsedPath" | sed 's/\\/\//g' )
            blThemeUsedPath="/${blThemeUsedPath%/*}"
        fi

        # 5:740  0:002  Using vector theme 'Clovy' (EFI\CLOVER\themes\Clovy)
        if [[ "$lineRead" == *"Using vector theme"* ]]; then
            blUsingTheme="${lineRead#*\'}"
            blUsingTheme="${blUsingTheme%\'*}"
            blThemeUsedPath="${lineRead#*(}"
            blThemeUsedPath="${blThemeUsedPath%)*}"
            blThemeUsedPath=$( echo "$blThemeUsedPath" | sed 's/\\/\//g' )
            blThemeUsedPath="/${blThemeUsedPath%/*}"
        fi

        # 6:208  0:000  theme black_green defined in NVRAM found and theme.plist parsed
        if [[ "$lineRead" == *"defined in NVRAM found"* ]]; then
            blNvramThemeEntry="${lineRead#*theme }"
            blNvramThemeEntry="${blNvramThemeEntry% defined*}"
            if [ "$blNvramReadFrom" == "" ]; then
                blNvramReadFrom="Native NVRAM"
            fi
            blNvramThemeExists=0
        fi

        # 6:227  0:000  Choosing theme black_green
        if [[ "$lineRead" == *"Choosing theme"* ]]; then
            blThemeNameChosen=$( echo "${lineRead##*Choosing theme }" | sed 's/ *$//' )
        fi

        # 4:769  0:002  Chosen theme black_green
        if [[ "$lineRead" == *"Chosen theme"* ]]; then
            blThemeNameChosen=$( echo "${lineRead##*Chosen theme }" | sed 's/ *$//' )
        fi

        # 1:848  0:000  no themes available, using embedded
        if [[ "$lineRead" == *"no themes available, using embedded"* ]]; then
            blUsingEmbedded=0
        fi

        # 2:963  0:000   using embedded theme
        if [[ "$lineRead" == *"using embedded theme"* ]]; then
            blUsingEmbedded=0
        fi

    done < "$bootLogFile"
}

# ---------------------------------------------------------------------------------------
PostProcess()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}PostProcess()"

    # check for random set by user
    if [ "$blConfigPlistThemeEntry" == "random" ] || [ "$blNvramThemeEntry" == "random" ]; then
        if [ "$blThemeNameChosen" == "random" ] && [ "$blUsingTheme" != "" ]; then
            blThemeNameChosen="$blUsingTheme"
        fi
    fi

    # Was embedded theme used?
    if [ $blUsingEmbedded -eq 0 ]; then
        blThemeUsedPath="internal"
        blThemeNameChosen="embedded"
    fi

    # Set theme asked for. Nvram setting takes precedence over config.plist
    if [ "$blNvramThemeEntry" != "" ] && [ $blNvramThemeAbsent -eq 1 ]; then
        blThemeAskedForTitle="$blNvramThemeEntry"
    elif [ "$blConfigPlistThemeEntry" != "" ]; then
        blThemeAskedForTitle="$blConfigPlistThemeEntry"
    else
        blThemeAskedForTitle=""
    fi

    # No theme asked for so no theme could have existed
    if [ "$blThemeAskedForTitle" == "" ]; then
        blThemeAskedForExisted=""

    # Was theme name asked for, used?
    elif [ "$blThemeAskedForTitle" == "$blUsingTheme" ]; then
        blThemeAskedForPath="${blThemeUsedPath}"/"${blThemeAskedForTitle}"

    # Was embedded theme asked for?
    elif [ "$blThemeAskedForTitle" == "embedded" ] && [ "$blUsingTheme" == "" ] ; then
        blThemeAskedForPath="internal"
        blThemeAskedForExisted="Always"
    fi

    # NVRAM entry points to non-existent theme OR no default theme found
    if [ $blNvramThemeAbsent -eq 0 ] && [ $blUsedRandomTheme -eq 0 ]; then
        blThemeAskedForExisted="No"
    fi

    # Is Native NVRAM working?
    if [ "$blNvramReadFrom" == "Native NVRAM" ]; then
        gNvramWorkingType="Native"
    fi

    # Is nvram.plist being used?
    if [ $blNvramPlistExists -eq 0 ]; then
        gNvramWorkingType="Fake"
    fi

    if [ "$blNvramReadFrom" == "" ] && [ $blNvramPlistExists -eq 0 ] && [ "$blNvramThemeEntry" == "" ] && [ "$blNvramPlistVolume" != "" ]; then
        blNvramReadFrom="/Volumes/${blNvramPlistVolume}/nvram.plist"
    fi

    if [ "$gBootDeviceMountpoint" != "" ] && [ "$blThemeUsedPath" != "" ]; then
        if [[ "$gBootDeviceMountpoint" == *$gESPMountPrefix* ]]; then
            blThemeUsedPath="/Volumes/EFI${blThemeUsedPath}"
        else
            blThemeUsedPath="${gBootDeviceMountpoint}${blThemeUsedPath}"
        fi
    fi

    if [ "$gBootDeviceMountpoint" != "" ]; then
        if [[ "$gBootDeviceMountpoint" == *$gESPMountPrefix* ]]; then
            blConfigPlistFilePathPrint="/Volumes/EFI${blConfigPlistFilePath}"
            if [ "$blThemeAskedForPath" != "" ]; then
               blThemeAskedForPathPrint="/Volumes/EFI${blThemeAskedForPath}"
            fi
        else
            blConfigPlistFilePathPrint="${gBootDeviceMountpoint}${blConfigPlistFilePath}"
            if [ "$blThemeAskedForPath" == "internal" ]; then
                blThemeAskedForPathPrint="${blThemeAskedForPath}/embedded"
            else
                blThemeAskedForPathPrint="${gBootDeviceMountpoint}${blThemeAskedForPath}"
            fi
        fi
        blConfigPlistFilePath="${gBootDeviceMountpoint}${blConfigPlistFilePath}"
    else
        gBootDeviceMountpoint="not found" && mountpointPrint="not found"
        blConfigPlistFilePathPrint="${blConfigPlistFilePath}"
        if [ "$blThemeAskedForPath" != "" ]; then
            blThemeAskedForPathPrint="${blThemeAskedForPath}"
        fi
    fi

    # If Fast Boot was used then there will be no NVRAM messages in bootlog linked to themes
    # So if all other checks remain blank and UEFI boot was used without the emulation driver...
    if [ "$blNvramReadFrom" == "" ] && [ "$gNvramWorkingType" == "" ] && [ "$blBootType" == "UEFI" ] && [ $blEmuVariable -eq 1 ] && [ $blFastOption -eq 0 ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}NVRAM checks are blank but UEFI boot without EmuVariable and FastBoot detected. Setting as working"
        blNvramReadFrom="Native NVRAM"
        gNvramWorkingType="Native"

    # Added in response to SavageAUS' example where NVRAM was working with UEFI boot but just not being used for themes.
    # So as above but without fast boot.
    elif [ "$blNvramReadFrom" == "" ] && [ "$gNvramWorkingType" == "" ] && [ "$blBootType" == "UEFI" ] && [ $blEmuVariable -eq 1 ]; then
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}NVRAM checks are blank but UEFI boot without EmuVariable. Setting as working"
        blNvramReadFrom="Native NVRAM"
        gNvramWorkingType="Native"
    fi

    # Convert device hex info to human readable
    blBootDevicePartStartDec=$(echo "ibase=16; ${blBootDevicePartStart#*x}" | bc)
    blBootDevicePartSizeDec=$(echo "ibase=16; ${blBootDevicePartSize#*x}" | bc)

    if [ "$gBootDeviceIdentifier" == "" ]; then
        gBootDeviceIdentifier="not found"
    else
        gBootDeviceIdentifierPrint="$gBootDeviceIdentifier"
    fi
    [[ "$gBootDeviceIdentifier" == "Failed" ]] && gBootDeviceIdentifierPrint="Failed to detect"
}

# ---------------------------------------------------------------------------------------
EscapeVarsForHtml()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}EscapeVarsForHtml()"

    gBootDeviceIdentifierPrint=$( echo "$gBootDeviceIdentifierPrint" | sed 's/\//\\\//g' )
    mountpointPrint=$( echo "$mountpointPrint" | sed 's/\//\\\//g' )
    blConfigPlistFilePathPrint=$( echo "$blConfigPlistFilePathPrint" | sed 's/\//\\\//g' )
    blNvramReadFromPrint=$( echo "$blNvramReadFrom" | sed 's/\//\\\//g' )
    blThemeUsedPathPrint=$( echo "$blThemeUsedPath" | sed 's/\//\\\//g' )
    blThemeAskedForPathPrint=$( echo "$blThemeAskedForPathPrint" | sed 's/\//\\\//g' )
}

# ---------------------------------------------------------------------------------------
CheckNvramIsWorking()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}CheckNvramIsWorking()"

    # $blBootType (either UEFI or Legacy) / $gNvramWorkingType (either Fake or Native)
    if [ "$blBootType" != "" ] && [ "$gNvramWorkingType" != "" ]; then

        if [ "$blBootType" == "Legacy" ] || [[ "$blBootType" == "UEFI" && $blEmuVariable -eq 0 ]]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Boot type=$blBootType | blEmuVariable=$blEmuVariable"
            # Check for necessary files to save nvram.plist file to disk
            if [ -f /Library/LaunchDaemons/com.projectosx.clover.daemon.plist ]; then
                local checkState=$( grep -A1 "RunAtLoad" /Library/LaunchDaemons/com.projectosx.clover.daemon.plist)
                if [[ "$checkState" == *true* ]]; then
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/Library/LaunchDaemons/com.projectosx.clover.daemon.plist exists and set to RunAtLoad"
                    if [ -f "/Library/Application Support/Clover/CloverDaemon" ]; then
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/Library/Application Support/Clover/CloverDaemon exists"
                        if [ -f /private/etc/rc.clover.lib ]; then
                            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/private/etc/rc.clover.lib exists"
                            if [ -f /private/etc/rc.shutdown.d/80.save_nvram_plist.local ]; then
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/private/etc/rc.shutdown.d/80.save_nvram_plist.local exists"
                                local checkMd5=$( md5 /private/etc/rc.shutdown.d/80.save_nvram_plist.local )
                                checkMd5="${checkMd5##*= }"
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/private/etc/rc.shutdown.d/80.save_nvram_plist.local md5=$checkMd5"
                                #if [ "$checkMd5" != "" ] && [[ "$checkMd5" = "44b326ce35acbbeb223031a941baf1a8" || "$checkMd5" = "0cf4ee82fd2da0aa20621c289e70939c" ]]; then
                                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Fake nvram should be working. Setting gNvramWorking to 0"
                                    gNvramWorking=0
                                #fi
                            else
                                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/private/etc/rc.shutdown.d/80.save_nvram_plist.local does not exist"
                            fi
                        else
                            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/private/etc/rc.clover.lib does not exist"
                        fi
                    else
                        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/Library/Application Support/Clover/CloverDaemon does not exist"
                    fi
                else
                    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/Library/LaunchDaemons/com.projectosx.clover.daemon.plist is not set to RunAtLoad"
                fi
            else
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}/Library/LaunchDaemons/com.projectosx.clover.daemon.plist does not exist"
            fi
        fi

        if [[ "$blBootType" == "UEFI" && $blEmuVariable -eq 1 ]]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}UEFI boot and EmuVariable Driver was not used."
            if [ "$gNvramWorkingType" == "Native" ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Native nvram is working. Setting gNvramWorking to 0"
                gNvramWorking=0
            fi
        fi
    else
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Skipped because blBootType=$blBootType AND gNvramWorkingType=$gNvramWorkingType"
    fi
}

# ---------------------------------------------------------------------------------------
PrintVarsToLog()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}PrintVarsToLog()"

    WriteToLog "${debugIndentTwo}Read Boot Log"
    WriteToLog "${debugIndentTwo}Clover Revision=$blCloverRevision"
    WriteToLog "${debugIndentTwo}Boot Type=$blBootType"
    WriteToLog "${debugIndentTwo}bootDevice partNo=$blBootDevicePartition"
    WriteToLog "${debugIndentTwo}bootDevice partType=$blBootDevicePartType"
    WriteToLog "${debugIndentTwo}bootDevice partSignature=$blBootDevicePartSignature"
    WriteToLog "${debugIndentTwo}bootDevice partLBA=$blBootDevicePartStart"
    WriteToLog "${debugIndentTwo}bootDevice partSize=$blBootDevicePartSize"
    WriteToLog "${debugIndentTwo}identifier: ${gBootDeviceIdentifier}"
    WriteToLog "${debugIndentTwo}mountpoint: ${gBootDeviceMountpoint}"
    WriteToLog "${debugIndentTwo}Config.plist OEM=$blConfigOem"
    WriteToLog "${debugIndentTwo}Config.plist file path: $blConfigPlistFilePath"
    WriteToLog "${debugIndentTwo}Config.plist Fast Boot? (1=No, 0=Yes): $blFastOption"
    WriteToLog "${debugIndentTwo}Config.plist Text Only? (1=No, 0=Yes): $blTextOnlyOption"
    WriteToLog "${debugIndentTwo}Config.plist theme entry: $blConfigPlistThemeEntry"
    WriteToLog "${debugIndentTwo}EmuVariable Driver used? (1=No, 0=Yes): $blEmuVariable"
    WriteToLog "${debugIndentTwo}NVRAM.plist volume location: $blNvramPlistVolume"
    WriteToLog "${debugIndentTwo}NVRAM.plist exists? (1=No, 0=Yes): $blNvramPlistExists"
    WriteToLog "${debugIndentTwo}NVRAM read from: $blNvramReadFrom"
    WriteToLog "${debugIndentTwo}NVRAM theme entry: $blNvramThemeEntry"
    WriteToLog "${debugIndentTwo}NVRAM theme absent? (1=No, 0=Yes): $blNvramThemeAbsent"
    WriteToLog "${debugIndentTwo}NVRAM theme exist? (1=No, 0=Yes): $blNvramThemeExists"
    WriteToLog "${debugIndentTwo}Using theme: $blUsingTheme"
    WriteToLog "${debugIndentTwo}Theme asked for title: $blThemeAskedForTitle"
    WriteToLog "${debugIndentTwo}Theme asked for full path: $blThemeAskedForPath"
    WriteToLog "${debugIndentTwo}Theme asked for exist: $themeExist"
    WriteToLog "${debugIndentTwo}Theme set in UI? (1=No, 0=Yes): $blGuiOverrideThemeChanged"
    WriteToLog "${debugIndentTwo}theme.plist not found? (1=No, 0=Yes): $blConfigThemePlistNotFound"
    WriteToLog "${debugIndentTwo}Random theme used? (1=No, 0=Yes):$blUsedRandomTheme"
    WriteToLog "${debugIndentTwo}Theme chosen in UI: $blGuiOverrideTheme"
    WriteToLog "${debugIndentTwo}Theme used path: $blThemeUsedPath"
    WriteToLog "${debugIndentTwo}Theme used chosen: $blThemeNameChosen"
    WriteLinesToLog
    WriteToLog "${debugIndentTwo}NVRAM working type: $gNvramWorkingType"
    WriteToLog "${debugIndentTwo}Is nvram working on currently booted system? (1=No, 0=Yes): $gNvramWorking"
    WriteLinesToLog
    WriteToLog "${debugIndentTwo}mountpointPrint=$mountpointPrint"
    WriteToLog "${debugIndentTwo}blConfigPlistFilePathPrint=$blConfigPlistFilePathPrint"
    WriteToLog "${debugIndentTwo}blNvramReadFromPrint=$blNvramReadFromPrint"
    WriteToLog "${debugIndentTwo}blThemeUsedPathPrint=$blThemeUsedPathPrint"
    WriteToLog "${debugIndentTwo}blThemeAskedForPathPrint=$blThemeAskedForPathPrint"
    WriteLinesToLog
}

# ---------------------------------------------------------------------------------------
PopulateNvramFunctionalityBand()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}PopulateNvramFunctionalityBand() option $1"

    local message=""
    local fillColour=""

    if [ "$1" == "0" ]; then

        if [ $gNvramWorking -eq 0 ]; then
            if [ "$blBootType" == "Legacy" ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Launch daemon \&amp; rc scripts appear to be working. NVRAM changes will be saved (to nvram.plist)"
                message="Launch daemon \&amp; rc scripts appear to be working. NVRAM changes will be saved (to nvram.plist)"
                fillColour="nvramFillWorking"
            elif [ "$blBootType" == "UEFI" ]; then
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Native NVRAM is functional and direct changes will be retained next boot."
                message="Native NVRAM is functional and direct changes will be retained next boot."
                fillColour="nvramFillWorking"
            fi
        elif [ $gNvramWorking -eq 1 ]; then
            if [ "$blBootType" == "Legacy" ]; then
                message="Launch Daemon \&amp; rc scripts not operational. Direct NVRAM changes won't be retained next boot. Run Clover Installer to fix."
                fillColour="nvramFillNotWorking"
            elif [ "$blBootType" == "UEFI" ]; then
                if [ "$gNvramWorkingType" == "" ]; then
                    if [ $blNvramBootArgs -eq 0 ]; then
                        message="Native NVRAM is working but not being used for default theme choice."
                        fillColour="nvramFillWorking"
                    else
                        message="Bootlog showed NVRAM is not being used for default theme choice."
                        fillColour="nvramFillNotWorking"
                    fi
                elif [ "$gNvramWorkingType" == "Fake" ] && [ $blEmuVariable -eq 0 ]; then
                    message="Launch daemon \&amp; rc scripts are not operational. Direct NVRAM changes won't be retained next boot. Run Clover Installer to fix."
                    fillColour="nvramFillNotWorking"
                fi
            fi
        fi

    elif [ "$1" == "1" ]; then

        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}This system was booted using Clover older than r2025."
        message="This system was booted using a Clover revision older than r2025."
        fillColour="nvramFillRed"

    elif [ "$1" == "2" ]; then

        if [ ! -f "$bootLogFile" ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}This system was not booted using Clover."
        else
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Bootlog file is not Clover bootlog."
        fi
        message="This system was not booted using Clover."
        fillColour="nvramFillRed"

    fi

    if [ "$message" != "" ]; then

        # Create html message
        local htmlToInsert=""
        htmlToInsert="$htmlToInsert"$(printf "    <div id=\"NvramFunctionalityBand\" class=\"${fillColour}\">\r")
        htmlToInsert="$htmlToInsert"$(printf "        <div id=\"nvramTextArea\">\r")
        htmlToInsert="$htmlToInsert"$(printf "            <span class=\"textBody\">${message}<\/span>\r")
        htmlToInsert="$htmlToInsert"$(printf "        <\/div>\r")
        htmlToInsert="$htmlToInsert"$(printf "    <\/div> <!-- End NvramFunctionalityBand -->\r")

        # Insert bootlog Html in to placeholder
        [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting nvram functionality message HTML in to managethemes.html"
        #LANG=C sed -ie "s/<!--INSERT_NVRAM_MESSAGE_BAND_HERE-->/${htmlToInsert}/g" "${PUBLIC_DIR}"/managethemes.html
        LANG=C sed -ie "s/<!--INSERT_NVRAM_MESSAGE_BAND_HERE-->/${htmlToInsert}/g" "${TEMPDIR}"/managethemes.html

        # Clean up
        if [ -f "${TEMPDIR}"/managethemes.htmle ]; then
            rm "${TEMPDIR}"/managethemes.htmle
        fi
    fi
}

# ---------------------------------------------------------------------------------------
PopulateBootLogTitleBand()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}PopulateBootLogTitleBand()"

    # Create bootlog band and title html
    bootlogBandTitleHtml=$(printf "    <div id=\"BootLogTitleBar\" class=\"bootlogBandFill\" tabindex=\"1\">\r")
    bandTitle=$(printf "        <span class=\"titleBarTextTitle\">LAST BOOT\&nbsp;\&nbsp;\&\#x25BE\&nbsp;\&nbsp;\&nbsp;\&nbsp;|<\/span>")
    bandTitleDescStart=$(printf "<span class=\"titleBarTextDescription\">")

    if [ $blTextOnlyOption -eq 0 ] && [ $blFastOption -eq 0 ]; then
        bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} - Fast boot and TextOnly options were both set in config.plist so no theme was loaded<\/span>"
    elif [ $blFastOption -eq 0 ]; then
        bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} - Fast boot option was set in config.plist so no theme was loaded<\/span>"
    elif [ $blTextOnlyOption -eq 0 ]; then
        bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} - Text Only option was set in config.plist so no theme was loaded<\/span>"
    else

        # Was the Christmas or NewYear theme used?
        if [ "$blThemeNameChosen" == "christmas" ] || [ "$blThemeNameChosen" == "newyear" ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded <span class=\"themeName\">${blThemeNameChosen}<\/span> as it's that time of year. <span class=\"themeAction\">Uninstall theme if not wanted.<\/span><\/span>"

        # No nvram theme entry and chosen theme matches config.plist entry as long as they're not blank
        elif [ "$blNvramThemeEntry" == "" ] && [ "$blThemeNameChosen" != "" ] && [ "$blConfigPlistThemeEntry" != "" ] && [ "$blThemeNameChosen" == "$blConfigPlistThemeEntry" ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded <span class=\"themeName\">${blThemeNameChosen}<\/span> as set in ${blConfigPlistFilePathPrint} on device ${gBootDeviceIdentifier}<\/span>"

        # nvram theme entry was used
        elif [ "$blNvramThemeEntry" != "" ] && [ "$blThemeNameChosen" == "$blNvramThemeEntry" ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded <span class=\"themeName\">${blThemeNameChosen}<\/span> as set in Clover.Theme var from ${blNvramReadFromPrint}<\/span>"

        # nvram theme entry points to non-existent theme AND chosen theme matches config.plist entry
        elif [ "$blNvramThemeEntry" != "" ] && [ $blNvramThemeExists -eq 1 ] && [ "$blThemeNameChosen" == "$blConfigPlistThemeEntry" ] && [ "$themeExist" == "Yes" ] && [ $blNvramThemeAbsent -eq 0 ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded <span class=\"themeName\">${blThemeNameChosen}<\/span> as set in ${blConfigPlistFilePathPrint} as NVRAM theme was absent<\/span>"

        # Any pointed to theme does not exist AND embedded theme was not used AND random theme was used
        elif [ $blNvramThemeAbsent -eq 0 ] && [ $blUsingEmbedded -eq 1 ] && [ $blUsedRandomTheme -eq 0 ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded a random theme <span class=\"themeName\">($blThemeNameChosen)<\/span> as it couldn't find the theme asked for<\/span>"

        # nvram entry was blank AND config.plist entry was blank AND embedded theme was not used AND random theme was used
        elif [ "$blNvramThemeEntry" == "" ] && [ "$blConfigPlistThemeEntry" == "" ] && [ $blUsingEmbedded -eq 1 ] && [ $blUsedRandomTheme -eq 0 ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded a random theme <span class=\"themeName\">($blThemeNameChosen)<\/span> as no theme was set<\/span>"

        # Embedded theme was used
        elif [ $blUsingEmbedded -eq 0 ] && [ "$blThemeAskedForTitle" == "embedded" ] && [ "$blUsingTheme" == "" ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded theme embedded as it couldn't find any themes<\/span>"

        # Embedded theme was used
        elif [ $blUsingEmbedded -eq 0 ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded theme embedded as it was asked for<\/span>"

        # User set random
        elif [ "$blConfigPlistThemeEntry" == "random" ] || [ "$blNvramThemeEntry" == "random" ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded <span class=\"themeName\">${blThemeNameChosen}<\/span> as a random theme was asked for<\/span>"

        # theme.plist missing so random theme chosen.
        elif [ $blConfigThemePlistNotFound -eq 0 ] && [ $blUsedRandomTheme -eq 0 ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded a random theme <span class=\"themeName\">($blThemeNameChosen)<\/span> as theme asked for didn't exist.<\/span>"

        # was theme overridden from GUI?
        elif [ $blGuiOverrideThemeChanged -eq 0 ] && [ "$blGuiOverrideTheme" != "" ] && [ "$blGuiOverrideTheme" == "$blThemeNameChosen" ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} loaded theme <span class=\"themeName\">${blThemeNameChosen}<\/span> as chosen in the GUI<\/span>"

        # was fast boot used?
        elif [ "$blUsingTheme" == "" ] && [ "$blThemeUsedPath" == "" ] && [ "$blThemeNameChosen" == "" ] && [ $blFastOption -eq 0 ]; then
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision} - Fast Boot option was set in config.plist so no theme was loaded<\/span>"

        # Something else happened
        else
            bootlogBandTitleHtml="${bootlogBandTitleHtml}${bandTitle}${bandTitleDescStart}${blBootType} Clover ${blCloverRevision}<\/span>"
        fi
    fi

    bootlogBandTitleHtml="$bootlogBandTitleHtml"$(printf "\r    <\/div> <!-- End BootLogTitleBar -->\r")

    # Insert bootlog Html in to placeholder
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting bootlog Band Title HTML in to managethemes.html"
    #LANG=C sed -ie "s/<!--INSERT_BOOTLOG_BAND_TITLE_HERE-->/${bootlogBandTitleHtml}/g" "${PUBLIC_DIR}"/managethemes.html && (( insertCount++ ))
    LANG=C sed -ie "s/<!--INSERT_BOOTLOG_BAND_TITLE_HERE-->/${bootlogBandTitleHtml}/g" "${TEMPDIR}"/managethemes.html && (( insertCount++ ))

    # Clean up
    if [ -f "${TEMPDIR}"/managethemes.htmle ]; then
        rm "${TEMPDIR}"/managethemes.htmle
    fi
}

# ---------------------------------------------------------------------------------------
PopulateBootLog()
{
    [[ DEBUG -eq 1 ]] && WriteLinesToLog
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}PopulateBootLog()"

    # Create bootlog container HTML
    bootlogHtml=$(printf "    <div id=\"BootLogContainer\" class=\"nvramFillNone\">\r")
    bootlogHtml="$bootlogHtml"$(printf "\r")

    # Add HTML for Boot Device Info / Boot Device Sections
    # If boot device was not found then present rescan button.
    if [ "$gBootDeviceIdentifier" != "Failed" ]; then
        if [ "$blBootDevicePartType" == "MBR" ]; then
            bootlogHtml="${bootlogHtml}${blcOpen}${blcLineDeviceInfoMbr}${blcLineDevice}"
        elif [ "$blBootDevicePartType" == "GPT" ]; then
            bootlogHtml="${bootlogHtml}${blcOpen}${blcLineDeviceInfoGpt}${blcLineDevice}"
        fi
    else
        if [ "$blBootDevicePartType" == "MBR" ]; then
            bootlogHtml="${bootlogHtml}${blcOpen}${blcLineDeviceInfoMbr}${blcLineDeviceRescan}"
        elif [ "$blBootDevicePartType" == "GPT" ]; then
            bootlogHtml="${bootlogHtml}${blcOpen}${blcLineDeviceInfoGpt}${blcLineDeviceRescan}"
        fi
    fi

    # Add HTML for NVRAM section
    if [ "$blNvramReadFrom" != "" ]; then
        if [ "$blNvramThemeEntry" != "" ]; then
            bootlogHtml="${bootlogHtml}${blcLineNvram}"
        else
            bootlogHtml="${bootlogHtml}${blcLineNvramNoTheme}"
        fi
    fi

    # Add HTML for Config.plist section
    bootlogHtml="${bootlogHtml}${blcLineConfig}"

    # Add HTML for Theme asked for section (providing a theme was asked for), and Text Only option was not used
    if [ "$blThemeAskedForPath" != "" ] && [ "$blThemeAskedForTitle" != "" ] && [ $blTextOnlyOption -eq 1 ]; then
        bootlogHtml="${bootlogHtml}${blcLineThemeAsked}"
    fi

    # If GUI was used to override theme then add this HTML section
    if [ $blGuiOverrideThemeChanged -eq 0 ] && [ "$blGuiOverrideTheme" != "" ]; then
        bootlogHtml="${bootlogHtml}${blcLineOverrideUi}"
    fi

    # If fast boot and text only options were not used then add HTML for Theme used section
    if ([ "$blUsingTheme" != "" ] || [ "$blThemeUsedPath" != "" ] || [ "$blThemeNameChosen" != "" ]) && [ $blFastOption -eq 1 ] && [ $blTextOnlyOption -eq 1 ]; then
        bootlogHtml="${bootlogHtml}${blcLineThemeUsed}"
    fi

    # Add ending HTML
    bootlogHtml="${bootlogHtml}    <\/div> <!-- End BootLogContainer -->"

    # Insert bootlog Html in to placeholder
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Inserting bootlog HTML in to managethemes.html"
    #LANG=C sed -ie "s/<!--INSERT_BOOTLOG_INFO_HERE-->/${bootlogHtml}/g" "${PUBLIC_DIR}"/managethemes.html && (( insertCount++ ))
    LANG=C sed -ie "s/<!--INSERT_BOOTLOG_INFO_HERE-->/${bootlogHtml}/g" "${TEMPDIR}"/managethemes.html && (( insertCount++ ))

    # Clean up
    #if [ -f "${PUBLIC_DIR}"/managethemes.htmle ]; then
    #    rm "${PUBLIC_DIR}"/managethemes.htmle
    #fi
    if [ -f "${TEMPDIR}"/managethemes.htmle ]; then
        rm "${TEMPDIR}"/managethemes.htmle
    fi
}

# Resolve path
SELF_PATH=$(cd -P -- "$(dirname -- "$0")" && pwd -P) && SELF_PATH=$SELF_PATH/$(basename -- "$0")
source "${SELF_PATH%/*}"/shared.sh

# Check for missing temp dir in case of local script testing.
[[ ! -d $TEMPDIR ]] && mkdir -p $TEMPDIR

# *************************************************************************
# Copy managethemes.html.template for testing only.
# Comment these two lines out for normal use
#cp "$PUBLIC_DIR"/managethemes.html.template "$TEMPDIR"/managethemes.html && PUBLIC_DIR="$TEMPDIR"
# *************************************************************************

[[ DEBUG -eq 1 ]] && WriteLinesToLog
[[ DEBUG -eq 1 ]] && WriteToLog "${debugIndent}bootlog.sh"

gNvramWorkingType=""
gNvramWorking=1                   # Set to 0 if writing to nvram can be saved for next boot
insertCount=0                     # increments each time an html block is injected in to managethemes.html
gRunInfo="$1"                     # Will be either 'Init' or 'Rescan'
gBootDeviceIdentifier="$2"        # The boot device identifier (if found)
gBootDeviceMountpoint="$3"        # The boot device mountpoint (if found)

if [ -f "$bootLogFile" ]; then

    # Note: Clover r2025 rebranded rEFIt to Clover.
    #       So any log pre r2025 will not be read correctly.

    checkLog=$( grep -a "Starting Clover" "$bootLogFile" )
    if [ "$checkLog" != "" ]; then
        ReadBootLog
        PostProcess
        CheckNvramIsWorking
        if [ "$gRunInfo" == "Init" ]; then
            EscapeVarsForHtml
            [[ DEBUG -eq 1 ]] && PrintVarsToLog

            # Write boot device info to file
            echo "${blBootDevicePartition}@${blBootDevicePartType}@${blBootDevicePartSignature}@${blBootDevicePartStartDec}@${blBootDevicePartSizeDec}" > "$bootDeviceInfo"

            # Create NVRAM functionality band
            PopulateNvramFunctionalityBand "0"

            # Show user what happened last boot
            PopulateBootLogTitleBand
            SetBootlogTextColourClasses
            SetHtmlBootlogSectionTemplates
            PopulateBootLog

            # Add message in to log for initialise.js to detect.
            if [ $insertCount -eq 2 ]; then
                WriteToLog "CTM_BootlogOK"
            else
                [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}insertCount=$insertCount | boot.log html failed to be inserted".
            fi
        elif [ "$gRunInfo" == "Rescan" ]; then
            # Remove paths file, incase this script is run a second time.
            [[ -f "$bootlogScriptOutfile" ]] && rm "$bootlogScriptOutfile"
        fi

        # Write some vars to file for script.sh to use.
        if [ "$blNvramReadFrom" != "" ]; then
            echo "nvram‡$blNvramReadFrom" >> "$bootlogScriptOutfile"
        elif [ $blNvramBootArgs -eq 0 ]; then
            echo "nvram‡Native NVRAM" >> "$bootlogScriptOutfile"
        fi
        [[ "$blNvramThemeEntry" != "" ]] && echo "nvramThemeEntry‡$blNvramThemeEntry" >> "$bootlogScriptOutfile"
        [[ "$blConfigPlistFilePath" != "" ]] && echo "config‡$blConfigPlistFilePath" >> "$bootlogScriptOutfile"
        [[ "$blBootType" != "" ]] && echo "bootType‡$blBootType" >> "$bootlogScriptOutfile"
        echo "nvramSave‡$gNvramWorking" >> "$bootlogScriptOutfile"

    else
        checkLog=$( grep -a "Starting rEFIt" "$bootLogFile" )
        if [ "$checkLog" != "" ]; then
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found Clover boot.log but revision is older than r2025"
            # Create NVRAM functionality band
            PopulateNvramFunctionalityBand "1"
        else
            [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}Found boot.log but Not Clover."
            PopulateNvramFunctionalityBand "2"
        fi
    fi

else
    # Show message that system was booted without using Clover
    PopulateNvramFunctionalityBand "2"
    
    # Add message in to log for initialise.js to detect.
    WriteToLog "CTM_BootlogMissing"
    [[ DEBUG -eq 1 ]] && WriteToLog "${debugIndentTwo}boot.log does not exist".
fi