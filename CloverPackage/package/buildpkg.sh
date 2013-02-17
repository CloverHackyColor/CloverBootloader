#!/bin/bash

# old=1755= ${SRCROOT}/package/buildpkg.sh ${SYMROOT}/package;
# new=1756=@${SRCROOT}/package/buildpkg.sh "$(SRCROOT)" "$(SYMROOT)" "$(PKG_BUILD_DIR)"

# 1=SRCROOT = $(CURDIR)
# 2=SYMROOT = $(SRCROOT)/sym
# 3=PKG_BUILD_DIR = $(SYMROOT)/package

# $3 Path to store built package

# Prevent the script from doing bad things
set -u  # Abort with unset variables

packagename="Clover"

declare -r PKGROOT="${0%/*}"    # ie. edk2/Clover/CloverPackage/package
declare -r SRCROOT="${1}"       # ie. edk2/Clover/CloverPackage
declare -r SYMROOT="${2}"       # ie. edk2/Clover/CloverPackage/sym
declare -r PKG_BUILD_DIR="${3}" # ie. edk2/Clover/CloverPackage/sym/package
declare -r SCPT_TPL_DIR="${PKGROOT}/Scripts.templates"

if [[ $# -lt 3 ]];then
    echo "Too few arguments. Aborting..." >&2 && exit 1
fi

if [[ ! -d "$SYMROOT" ]];then
    echo "Directory ${SYMROOT} doesn't exit. Aborting..." >&2 && exit 1
fi

# ====== LANGUAGE SETUP ======
export LANG='en_US.UTF-8'
export LC_COLLATE='C'
export LC_CTYPE='C'

# ====== COLORS ======
COL_BLACK="\x1b[30;01m"
COL_RED="\x1b[31;01m"
COL_GREEN="\x1b[32;01m"
COL_YELLOW="\x1b[33;01m"
COL_MAGENTA="\x1b[35;01m"
COL_CYAN="\x1b[36;01m"
COL_WHITE="\x1b[37;01m"
COL_BLUE="\x1b[34;01m"
COL_RESET="\x1b[39;49;00m"

# ====== REVISION/VERSION ======
declare -r CLOVER_VERSION=$( cat version )
# stage
CLOVER_STAGE=${CLOVER_VERSION##*-}
CLOVER_STAGE=${CLOVER_STAGE/RC/Release Candidate }
CLOVER_STAGE=${CLOVER_STAGE/FINAL/2.1 Final}
declare -r CLOVER_STAGE
declare -r CLOVER_REVISION=$( cat revision )
declare -r CLOVER_BUILDDATE=$( sed -n 's/.*FIRMWARE_BUILDDATE *\"\(.*\)\".*/\1/p' "${PKGROOT}/../../Version.h" )
declare -r CLOVER_TIMESTAMP=$( date -j -f "%Y-%m-%d %H:%M:%S" "${CLOVER_BUILDDATE}" "+%s" )

# =================
declare -r CLOVER_DEVELOP=$(awk "NR==6{print;exit}"  ${PKGROOT}/../CREDITS)
declare -r CLOVER_CREDITS=$(awk "NR==10{print;exit}" ${PKGROOT}/../CREDITS)
declare -r CLOVER_PKGDEV=$(awk "NR==14{print;exit}"  ${PKGROOT}/../CREDITS)
declare -r CLOVER_CPRYEAR=$(awk "NR==18{print;exit}" ${PKGROOT}/../CREDITS)
whoami=$(whoami | awk '{print $1}' | cut -d ":" -f3)
if [[ "$whoami" == "admin" ]];then
    declare -r CLOVER_WHOBUILD="VoodooLabs BuildBot"
else
    declare -r CLOVER_WHOBUILD="$whoami"
fi

# ====== GLOBAL VARIABLES ======
declare -r LOG_FILENAME="Clover_Installer_Log.txt"

declare -a pkgrefs
declare -a choice_key
declare -a choice_options
declare -a choice_title
declare -a choive_description
declare -a choice_pkgrefs
declare -a choice_parent_group_index
declare -a choice_group_items
declare -a choice_group_exclusive

# Init Main Group
choice_key[0]=""
choice_options[0]=""
choice_title[0]=""
choice_description[0]=""
choices_pkgrefs[0]=""
choice_group_items[0]=""
choice_group_exclusive[0]=""

# =================

add_ia32=0

# Package identifiers
declare -r clover_package_identity="org.clover"

# ====== FUNCTIONS ======
trim () {
    local result="${1#"${1%%[![:space:]]*}"}"   # remove leading whitespace characters
    echo "${result%"${result##*[![:space:]]}"}" # remove trailing whitespace characters
}

function makeSubstitutions () {
    # Substition is like: Key=Value
    #
    # Optional arguments:
    #    --subst=<substition> : add a new substitution
    #
    # Last argument(s) is/are file(s) where substitutions must be made

    local ownSubst=""

    function addSubst () {
        local mySubst="$1"
        case "$mySubst" in
            *=*) keySubst=${mySubst%%=*}
                 valSubst=${mySubst#*=}
                 ownSubst=$(printf "%s\n%s" "$ownSubst" "s&@$keySubst@&$valSubst&g;t t")
                 ;;
            *) echo "Invalid substitution $mySubst" >&2
               exit 1
               ;;
        esac
    }

    # Check the arguments.
    while [[ $# -gt 0 ]];do
        local option="$1"
        case "$option" in
            --subst=*) shift; addSubst "${option#*=}" ;;
            -*)
                echo "Unrecognized makeSubstitutions option '$option'" >&2
                exit 1
                ;;
            *)  break ;;
        esac
    done

    if [[ $# -lt 1 ]];then
        echo "makeSubstitutions invalid number of arguments: at least one file needed" >&2
        exit 1
    fi

    local cloverSubsts="
s&%CLOVERVERSION%&${CLOVER_VERSION%%-*}&g
s&%CLOVERREVISION%&${CLOVER_REVISION}&g
s&%CLOVERSTAGE%&${CLOVER_STAGE}&g
s&%DEVELOP%&${CLOVER_DEVELOP}&g
s&%CREDITS%&${CLOVER_CREDITS}&g
s&%PKGDEV%&${CLOVER_PKGDEV}&g
s&%CPRYEAR%&${CLOVER_CPRYEAR}&g
s&%WHOBUILD%&${CLOVER_WHOBUILD}&g
:t
/@[a-zA-Z_][a-zA-Z_0-9]*@/!b
s&@LOG_FILENAME@&${LOG_FILENAME}&g;t t"

    local allSubst="
$cloverSubsts
$ownSubst"

    for file in "$@";do
        cp -pf "$file" "${file}.in"
        sed "$allSubst" "${file}.in" > "${file}"
        rm -f "${file}.in"
    done
}


addTemplateScripts () {
    # Arguments:
    #    --pkg-rootdir=<pkg_rootdir> : path of the pkg root dir
    #
    # Optional arguments:
    #    --subst=<substition> : add a new substitution
    #
    # Substition is like: Key=Value
    #
    # $n : Name of template(s) (templates are in package/Scripts.templates

    local pkgRootDir=""
    declare -a allSubst

    # Check the arguments.
    while [[ $# -gt 0 ]];do
        local option="$1"
        case "$option" in
            --pkg-rootdir=*)   shift; pkgRootDir="${option#*=}" ;;
            --subst=*) shift; allSubst[${#allSubst[*]}]="${option}" ;;
            -*)
                echo "Unrecognized addTemplateScripts option '$option'" >&2
                exit 1
                ;;
            *)  break ;;
        esac
    done
    if [[ $# -lt 1 ]];then
        echo "addTemplateScripts invalid number of arguments: you must specify a template name" >&2
        exit 1
    fi
    [[ -z "$pkgRootDir" ]] && { echo "Error addTemplateScripts: --pkg-rootdir option is needed" >&2 ; exit 1; }
    [[ ! -d "$pkgRootDir" ]] && { echo "Error addTemplateScripts: directory '$pkgRootDir' doesn't exists" >&2 ; exit 1; }

    for templateName in "$@";do
        local templateRootDir="${SCPT_TPL_DIR}/${templateName}"
        [[ ! -d "$templateRootDir" ]] && {
            echo "Error addTemplateScripts: template '$templateName' doesn't exists" >&2; exit 1; }

        # Copy files to destination
        rsync -pr --exclude=.svn --exclude="*~" "$templateRootDir/" "$pkgRootDir/Scripts/"
    done

    files=$( find "$pkgRootDir/Scripts/" -type f )
    if [[ ${#allSubst[*]} -gt 0 ]];then
        makeSubstitutions "${allSubst[@]}" $files
    else
        makeSubstitutions $files
    fi
}

getPackageRefId () {
    echo ${1//_/.}.${2//_/.} | tr [:upper:] [:lower:]
}

# Return index of a choice
getChoiceIndex () {
    # $1 Choice Id
    local found=0
    for (( idx=0 ; idx < ${#choice_key[*]}; idx++ ));do
        if [[ "${1}" == "${choice_key[$idx]}" ]];then
            found=1
            break
        fi
    done
    echo "$idx"
    return $found
}

# Add a new choice
addChoice () {
    # Optional arguments:
    #    --title=<title> : Force the title
    #    --description=<description> : Force the description
    #    --group=<group> : Group Choice Id
    #    --start-selected=<javascript code> : Specifies whether this choice is initially selected or unselected
    #    --start-enabled=<javascript code>  : Specifies the initial enabled state of this choice
    #    --start-visible=<javascript code>  : Specifies whether this choice is initially visible
    #    --pkg-refs=<pkgrefs> : List of package reference(s) id (separate by spaces)
    #
    # $1 Choice Id

    local option
    local title=""
    local description=""
    local groupChoice=""
    local choiceOptions=""
    local pkgrefs=""

    # Check the arguments.
    for option in "${@}";do
        case "$option" in
            --title=*)
                       shift; title="${option#*=}" ;;
            --description=*)
                       shift; description="${option#*=}" ;;
            --group=*)
                       shift; groupChoice=${option#*=} ;;
            --start-selected=*)
                         shift; choiceOptions="$choiceOptions start_selected=\"${option#*=}\"" ;;
            --start-enabled=*)
                         shift; choiceOptions="$choiceOptions start_enabled=\"${option#*=}\"" ;;
            --start-visible=*)
                         shift; choiceOptions="$choiceOptions start_visible=\"${option#*=}\"" ;;
            --pkg-refs=*)
                          shift; pkgrefs=${option#*=} ;;
            -*)
                echo "Unrecognized addChoice option '$option'" >&2
                exit 1
                ;;
            *)  break ;;
        esac
    done

    if [[ $# -ne 1 ]];then
        echo "addChoice invalid number of arguments: ${@}" >&2
        exit 1
    fi

    local choiceId="${1}"

    # Add choice in the group
    idx_group=$(getChoiceIndex "$groupChoice")
    found_group=$?
    if [[ $found_group -ne 1 ]];then
        # No group exist
        echo "Error can't add choice '$choiceId' to group '$groupChoice': group choice '$groupChoice' doesn't exists." >&2
        exit 1
    else
        set +u; oldItems=${choice_group_items[$idx_group]}; set -u
        choice_group_items[$idx_group]="$oldItems $choiceId"
    fi

    # Check that the choice doesn't already exists
    idx=$(getChoiceIndex "$choiceId")
    found=$?
    if [[ $found -ne 0 ]];then
        # Choice already exists
        echo "Error can't add choice '$choiceId': a choice with same name already exists." >&2
        exit 1
    fi

    # Record new node
    choice_key[$idx]="$choiceId"
    choice_title[$idx]="${title:-${choiceId}_title}"
    choice_description[$idx]="${description:-${choiceId}_description}"
    choice_options[$idx]=$(trim "${choiceOptions}") # Removing leading and trailing whitespace(s)
    choice_parent_group_index[$idx]=$idx_group
    choice_pkgrefs[$idx]="$pkgrefs"

    return $idx
}

# Add a group choice
addGroupChoices() {
    # Optional arguments:
    #    --title=<title> : Force the title
    #    --description=<description> : Force the description
    #    --parent=<parent> : parent group choice id
    #    --exclusive_zero_or_one_choice : only zero or one choice can be selected in the group
    #    --exclusive_one_choice : only one choice can be selected in the group
    #
    # $1 Choice Id

    local option
    local title=""
    local description=""
    local groupChoice=""
    local exclusive_function=""

    for option in "${@}";do
        case "$option" in
            --title=*)
                       shift; title="${option#*=}" ;;
            --description=*)
                       shift; description="${option#*=}" ;;
            --exclusive_zero_or_one_choice)
                       shift; exclusive_function="exclusive_zero_or_one_choice" ;;
            --exclusive_one_choice)
                       shift; exclusive_function="exclusive_one_choice" ;;
            --parent=*)
                       shift; groupChoice=${option#*=} ;;
            -*)
                echo "Unrecognized addGroupChoices option '$option'" >&2
                exit 1
                ;;
            *)  break ;;
        esac
    done

    if [[ $# -ne 1 ]];then
        echo "addGroupChoices invalid number of arguments: ${@}" >&2
        exit 1
    fi

    addChoice --group="$groupChoice" --title="$title" --description="$description" "${1}"
    local idx=$? # index of the new created choice

    choice_group_exclusive[$idx]="$exclusive_function"
}

exclusive_one_choice () {
    # $1 Current choice (ie: test1)
    # $2..$n Others choice(s) (ie: "test2" "test3"). Current can or can't be in the others choices
    local myChoice="${1}"
    local result="";
    local separator=' || ';
    for choice in ${@:2};do
        if [[ "$choice" != "$myChoice" ]];then
            result="${result}choices['$choice'].selected${separator}";
        fi
    done
    if [[ -n "$result" ]];then
        echo "!(${result%$separator})"
    fi
}

exclusive_zero_or_one_choice () {
    # $1 Current choice (ie: test1)
    # $2..$n Others choice(s) (ie: "test2" "test3"). Current can or can't be in the others choices
    local myChoice="${1}"
    local result;
    echo "(my.choice.selected &amp;&amp; $(exclusive_one_choice ${@}))"
}

main ()
{

# clean up the destination path

    rm -R -f "${PKG_BUILD_DIR}"
    echo ""
    echo -e $COL_CYAN"  ----------------------------------"$COL_RESET
    echo -e $COL_CYAN"  Building $packagename Install Package"$COL_RESET
    echo -e $COL_CYAN"  ----------------------------------"$COL_RESET
    echo ""

# # Add pre install choice
#     echo "================= Preinstall ================="
#     packagesidentity="${clover_package_identity}"
#     choiceId="Pre"
#
#     packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
#     addChoice --start-visible="false" --start-selected="true"  --pkg-refs="$packageRefId" "${choiceId}"
#
#     # Package will be built at the end
# # End pre install choice

# build Core package
    echo "===================== Core ============================="
    packagesidentity="$clover_package_identity"
    choiceId="Core"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin
    if [[ "$add_ia32" -eq 1 ]]; then
        mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/ia32
        ditto --noextattr --noqtn ${SYMROOT}/i386/ia32/boot ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/ia32
    fi
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/x64
    ditto --noextattr --noqtn ${SYMROOT}/i386/x64/boot    ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/x64
#   ditto --noextattr --noqtn ${SYMROOT}/i386/x64/boot7   ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/x64
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot0       ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot0md     ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot0hfs    ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot1f32    ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot1f32alt ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot1h      ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/boot1h2     ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
#   ditto --noextattr --noqtn ${SYMROOT}/i386/bootc       ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386
    ditto --noextattr --noqtn ${SYMROOT}/i386/fdisk440    ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin

    fixperms "${PKG_BUILD_DIR}/${choiceId}/Root/"
    chmod 755 "${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin/fdisk440"

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="false" --start-selected="true" --pkg-refs="$packageRefId" "${choiceId}"
# End build core package

# build core EFI folder package
    echo "===================== EFI folder ======================="
    packagesidentity="$clover_package_identity"
    choiceId="EFIfolder"
    rm -rf   ${PKG_BUILD_DIR}/${choiceId}/Root/EFI
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/EFI
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}
    rsync -r --exclude=.svn --exclude="*~" ${SRCROOT}/CloverV2/EFI/ ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/
    [[ "$add_ia32" -ne 1 ]] && rm -rf ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/drivers32
    # config.plist
    rm -f ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/config.plist &>/dev/null
    # refit.conf
    mv -f ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/BOOT/refit.conf \
     ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/BOOT/refit-default.conf
    fixperms "${PKG_BUILD_DIR}/${choiceId}/Root/"

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="false" --start-selected="true" --pkg-refs="$packageRefId" "${choiceId}"
# End build EFI folder package

# Create Clover Node
    addGroupChoices --exclusive_one_choice "Clover"
    echo "===================== BootLoaders ======================="
# build boot0 package
    packagesidentity="$clover_package_identity".bootloader
    choiceId="boot0"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="Clover" --start-selected="false" --pkg-refs="$packageRefId" "${choiceId}"
# End build boot0 package

# build boot0hfs package
    choiceId="boot0hfs"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="Clover" --start-selected="true" --pkg-refs="$packageRefId" "${choiceId}"
# End build boot0hfs package

# build boot0EFI package
    choiceId="boot0EFI"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="Clover" --start-selected="false" --pkg-refs="$packageRefId" "${choiceId}"
# End build boot0EFI package

# build boot1no package
    choiceId="boot1no"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="Clover" --start-selected="false" --pkg-refs="$packageRefId" "${choiceId}"
# End build boot1no package

# build boot1UEFI package
    choiceId="boot1UEFI"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="Clover" --start-selected="false" --pkg-refs="$packageRefId" "${choiceId}"
# End build boot1UEFI package

if [[ "$add_ia32" -eq 1 ]]; then
    # Create Boot Arch Node
    addGroupChoices --parent="Clover" --exclusive_one_choice "BootArch"

    # build boot32 package
    packagesidentity="$clover_package_identity".bootarch
    choiceId="boot32"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --group="BootArch" --start-selected="false" --pkg-refs="$packageRefId" "${choiceId}"
    # End build boot32 package
fi
# build boot64 package
    packagesidentity="$clover_package_identity".bootarch
    choiceId="boot64"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    # Only add the choice if we have ia32 arch
    if [[ "$add_ia32" -eq 1 ]]; then
        addChoice --group="BootArch" --start-selected="true" --pkg-refs="$packageRefId" "${choiceId}"
    else
        addChoice --start-visible="false" --start-selected="true" --pkg-refs="$packageRefId" "${choiceId}"
    fi
# End build boot64 package


# build theme packages
    echo "================= Themes ================="
    addGroupChoices "Themes"

    # Using themes section from Azi's/package branch.
    packagesidentity="${clover_package_identity}".themes
    artwork="${SRCROOT}/CloverV2/themespkg/"
    themes=($( find "${artwork}" -type d -depth 1 -not -name '.svn' ))
    for (( i = 0 ; i < ${#themes[@]} ; i++ )); do
        themeName=${themes[$i]##*/}
        themeDir="$themeName"
        mkdir -p "${PKG_BUILD_DIR}/${themeName}/Root/"
        rsync -r --exclude=.svn --exclude="*~" "${themes[$i]}/" "${PKG_BUILD_DIR}/${themeName}/Root/${themeName}"
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${themeName}" \
                           --subst="themeName=$themeName"                \
                           InstallTheme

        packageRefId=$(getPackageRefId "${packagesidentity}" "${themeName}")
        buildpackage "$packageRefId" "${themeName}" "${PKG_BUILD_DIR}/${themeName}" "/EFI/BOOT/themes"
        addChoice --group="Themes"  --start-selected="false"  --pkg-refs="$packageRefId"  "${themeName}"
    done
# End build theme packages

# build drivers-x32 packages
if [[ "$add_ia32" -eq 1 ]]; then
    echo "===================== drivers32 ========================"
    addGroupChoices --title="Drivers32" --description="Drivers32" "Drivers32"
    packagesidentity="${clover_package_identity}".drivers32
    drivers=($( find "${SRCROOT}/CloverV2/drivers-Off/drivers32" -type f -name '*.efi' -depth 1 ))
    for (( i = 0 ; i < ${#drivers[@]} ; i++ )); do
        driver="${drivers[$i]##*/}"
        driverName="${driver%.efi}"
        mkdir -p "${PKG_BUILD_DIR}/${driverName}/Root/"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")
        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "/EFI/drivers32"
        addChoice --group="Drivers32"  --title="$driverName"  --description="Install to /EFI/drivers32/$driver" \
         --start-selected="false"  --pkg-refs="$packageRefId"  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build drivers-x32 packages

# build drivers-x64 packages
    echo "===================== drivers64 ========================"
    addGroupChoices --title="Drivers64" --description="Drivers64" "Drivers64"
    packagesidentity="${clover_package_identity}".drivers64
    drivers=($( find "${SRCROOT}/CloverV2/drivers-Off/drivers64" -type f -name '*.efi' -depth 1 ))
    for (( i = 0 ; i < ${#drivers[@]} ; i++ )); do
        driver="${drivers[$i]##*/}"
        driverName="${driver%.efi}"
        mkdir -p "${PKG_BUILD_DIR}/${driverName}/Root/"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")
        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "/EFI/drivers64"
        addChoice --group="Drivers64"  --title="$driverName"  --description="Install to /EFI/drivers64/$driver" \
         --start-selected="false"  --pkg-refs="$packageRefId"  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
# End build drivers-x64 packages

# build drivers-x64UEFI packages 
    echo "===================== drivers64UEFI ========================"
    addGroupChoices --title="Drivers64UEFI" --description="Drivers64UEFI" "Drivers64UEFI"
    packagesidentity="${clover_package_identity}".drivers64UEFI
    drivers=($( find "${SRCROOT}/CloverV2/drivers-Off/drivers64UEFI" -type f -name '*.efi' -depth 1 ))
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        driver="${drivers[$i]##*/}"
        driverName="${driver%.efi}"
        mkdir -p "${PKG_BUILD_DIR}/${driverName}/Root/"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")
        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "/EFI/drivers64UEFI"
        addChoice --group="Drivers64UEFI"  --title="$driverName"  --description="Install to /EFI/drivers64UEFI/$driver" \
         --start-selected="false"  --pkg-refs="$packageRefId"  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
# End build drivers-x64UEFI packages

# build rc scripts package
    echo "===================== RC Scripts ======================="
    packagesidentity="$clover_package_identity"
    choiceId="rc.scripts"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/etc
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" RcScripts
    rsync -r --exclude=.svn --exclude="*~" ${SRCROOT}/CloverV2/etc/ ${PKG_BUILD_DIR}/${choiceId}/Root/etc/
    fixperms "${PKG_BUILD_DIR}/${choiceId}/Root/"
    chmod 755 "${PKG_BUILD_DIR}/${choiceId}/Root/etc"/rc*.local
    chmod 755 "${PKG_BUILD_DIR}/${choiceId}/Scripts/postinstall"

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="false" --start-selected="true" --pkg-refs="$packageRefId" "${choiceId}"
# End build rc scripts package

# build post install package
    echo "================= Post ================="
    packagesidentity="${clover_package_identity}"
    choiceId="Post"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}
    # cp -f ${PKGROOT}/Scripts/Sub/UnMountEFIvolumes.sh ${PKG_BUILD_DIR}/${choiceId}/Scripts

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice  --start-visible="false" --start-selected="true"  --pkg-refs="$packageRefId" "${choiceId}"
# End build post install package

}

fixperms ()
{
    # $1 path
    find "${1}" -type f -exec chmod 644 {} \;
    find "${1}" -type d -exec chmod 755 {} \;
}

buildoptionalsettings()
{
    # $1 Path to package to build containing Root and or Scripts
    # $2 = exclusiveFlag
    # S3 = exclusiveName

    # ------------------------------------------------------
    # if exclusiveFlag=1 then re-build array
    # adding extra boot option at beginning to give
    # user a chance to choose none of them.
    # ------------------------------------------------------
    if [ ${2} = "1" ]; then
        tempArray=("${availableOptions[@]}")
        availableOptions=()
        availableOptions[0]="ChooseNone-"$3":DONT=ADD"
        position=0
        totalItems="${#tempArray[@]}"
        for (( position = 0 ; position < $totalItems ; position++ ))
        do
            availableOptions[$position+1]=${tempArray[${position}]}
        done
    fi

    # ------------------------------------------------------
    # Loop through options in array and process each in turn
    # ------------------------------------------------------
    for (( c = 0 ; c < ${#availableOptions[@]} ; c++ ))
    do
        textLine=${availableOptions[c]}
        # split line - taking all before ':' as option name
        # and all after ':' as key/value
        optionName=${textLine%:*}
        keyValue=${textLine##*:}

        # create folders required for each boot option
        mkdir -p "${1}/$optionName/Root/"

        # create dummy file with name of key/value
        echo "" > "${1}/$optionName/Root/${keyValue}"

        echo "  [BUILD] ${optionName} "

        # ------------------------------------------------------
        # Before calling buildpackage, add exclusive options
        # to buildpackage call if requested.
        # ------------------------------------------------------
        if [ $2 = "1" ]; then

            # Prepare individual string parts
            stringStart="selected=\""
            stringBefore="exclusive(choices['"
            stringAfter="']) &amp;&amp; "
            stringEnd="'])\""
            x=${stringStart}${stringBefore}

            # build string for sending to buildpackage
            totalItems="${#availableOptions[@]}"
            lastItem=$((totalItems-1))

            for (( r = 0 ; r < ${totalItems} ; r++ ))
            do
                textLineTemp=${availableOptions[r]}
                optionNameTemp=${textLineTemp%:*}
                if [ "${optionNameTemp}" != "${optionName}" ]; then
                     x="${x}${optionNameTemp}"
                     # Only add these to end of string up to the one before the last item
                    if [ $r -lt $lastItem ]; then
                        x="${x}${stringAfter}${stringBefore}"
                    fi
                fi
            done
            x="${x}${stringEnd}"

            # First exclusive option is the 'no choice' option, so let's make that selected by default.
            if [ $c = 0 ]; then
                initialChoice="true"
            else
                initialChoice="false"
            fi

            buildpackage "${1}/${optionName}" "/tmpcham/chamTemp/options" "" "start_selected=\"${initialChoice}\" ${x}" >/dev/null 2>&1
        else
            buildpackage "${1}/${optionName}" "/tmpcham/chamTemp/options" "" "start_selected=\"false\"" >/dev/null 2>&1
        fi
    done
}

buildpackage ()
{
    #  $1 Package Reference Id (ie: org.clover.themes.default)
    #  $2 Package Name (ie: Default)
    #  $3 Path to package to build containing Root and/or Scripts
    #  $4 Target install location
    #  $5 Size (optional)
    if [[ -d "${3}/Root" ]]; then
        local packageRefId="$1"
        local packageName="$2"
        local packagePath="$3"
        local targetPath="$4"
        set +u # packageSize is optional
        local packageSize="$5"
        set -u

        echo -e "\t[BUILD] ${packageName}"

        find "${packagePath}" -name '.DS_Store' -delete
        local filecount=$( find "${packagePath}/Root" | wc -l )
        if [ "${packageSize}" ]; then
            local installedsize="${packageSize}"
        else
            local installedsize=$( du -hkc "${packagePath}/Root" | tail -n1 | awk {'print $1'} )
        fi
        local header="<?xml version=\"1.0\"?>\n<pkg-info format-version=\"2\" "

        #[ "${3}" == "relocatable" ] && header+="relocatable=\"true\" "

        header+="identifier=\"${packageRefId}\" "
        header+="version=\"${CLOVER_VERSION}\" "

        [ "${targetPath}" != "relocatable" ] && header+="install-location=\"${targetPath}\" "

        header+="auth=\"root\">\n"
        header+="\t<payload installKBytes=\"${installedsize##* }\" numberOfFiles=\"${filecount##* }\"/>\n"
        rm -R -f "${packagePath}/Temp"

        [ -d "${packagePath}/Temp" ] || mkdir -m 777 "${packagePath}/Temp"
        [ -d "${packagePath}/Root" ] && mkbom "${packagePath}/Root" "${packagePath}/Temp/Bom"

        if [ -d "${packagePath}/Scripts" ]; then
            header+="\t<scripts>\n"
            for script in $( find "${packagePath}/Scripts" -type f \( -name 'pre*' -or -name 'post*' \) ); do
                header+="\t\t<${script##*/} file=\"./${script##*/}\"/>\n"
            done
            header+="\t</scripts>\n"
            # Create the Script archive file (cpio format)
            (cd "${packagePath}/Scripts" && find . -print |                                    \
                cpio -o -z -R root:wheel --format cpio > "${packagePath}/Temp/Scripts") 2>&1 | \
                grep -vE '^[0-9]+\s+blocks?$' # to remove cpio stderr messages
        fi

        header+="</pkg-info>"
        echo -e "${header}" > "${packagePath}/Temp/PackageInfo"

        # Create the Payload file (cpio format)
        (cd "${packagePath}/Root" && find . -print |                                       \
            cpio -o -z -R root:wheel --format cpio > "${packagePath}/Temp/Payload") 2>&1 | \
            grep -vE '^[0-9]+\s+blocks?$' # to remove cpio stderr messages

        # Create the package
        (cd "${packagePath}/Temp" && xar -c -f "${packagePath}/../${packageName}.pkg" --compression none .)

        # Add the package to the list of build packages
        pkgrefs[${#pkgrefs[*]}]="\t<pkg-ref id=\"${packageRefId}\" installKBytes='${installedsize}' version='${CLOVER_VERSION}.0.0.${CLOVER_TIMESTAMP}'>#${packageName}.pkg</pkg-ref>"

        rm -rf "${packagePath}"
    fi
}

generateOutlineChoices() {
    # $1 Main Choice
    # $2 indent level
    local idx=$(getChoiceIndex "$1")
    local indentLevel="$2"
    local indentString=""
    for ((level=1; level <= $indentLevel ; level++)); do
        indentString="\t$indentString"
    done
    set +u; subChoices="${choice_group_items[$idx]}"; set -u
    if [[ -n "${subChoices}" ]]; then
        # Sub choices exists
        echo -e "$indentString<line choice=\"$1\">"
        for subChoice in $subChoices;do
            generateOutlineChoices $subChoice $(($indentLevel+1))
        done
        echo -e "$indentString</line>"
    else
        echo -e "$indentString<line choice=\"$1\"/>"
    fi
}

generateChoices() {
    for (( idx=1; idx < ${#choice_key[*]} ; idx++)); do
        local choiceId=${choice_key[$idx]}
        local choiceTitle=${choice_title[$idx]}
        local choiceDescription=${choice_description[$idx]}
        local choiceOptions=${choice_options[$idx]}
        local choiceParentGroupIndex=${choice_parent_group_index[$idx]}
        set +u; local group_exclusive=${choice_group_exclusive[$choiceParentGroupIndex]}; set -u

        # Create the node and standard attributes
        local choiceNode="\t<choice\n\t\tid=\"${choiceId}\"\n\t\ttitle=\"${choiceTitle}\"\n\t\tdescription=\"${choiceDescription}\""

        # Add options like start_selected, etc...
        [[ -n "${choiceOptions}" ]] && choiceNode="${choiceNode}\n\t\t${choiceOptions}"

        # Add the selected attribute if options are mutually exclusive
        if [[ -n "$group_exclusive" ]];then
            local group_items="${choice_group_items[$choiceParentGroupIndex]}"
            case $group_exclusive in
                exclusive_one_choice)
                    local selected_option=$(exclusive_one_choice "$choiceId" "$group_items") ;;
                exclusive_zero_or_one_choice)
                    local selected_option=$(exclusive_zero_or_one_choice "$choiceId" "$group_items") ;;
                *) echo "Error: unknown function to generate exclusive mode '$group_exclusive' for group '${choice_key[$choiceParentGroupIndex]}'" >&2
                   exit 1
                   ;;
            esac
            choiceNode="${choiceNode}\n\t\tselected=\"$selected_option\""
        fi

        choiceNode="${choiceNode}>"

        # Add the package references
        for pkgRefId in ${choice_pkgrefs[$idx]};do
            choiceNode="${choiceNode}\n\t\t<pkg-ref id=\"${pkgRefId}\"/>"
        done

        # Close the node
        choiceNode="${choiceNode}\n\t</choice>\n"

        echo -e "$choiceNode"
    done
}

makedistribution ()
{
    declare -r distributionDestDir="${SYMROOT}"
    declare -r distributionFilename="${packagename// /}_${CLOVER_VERSION}_r${CLOVER_REVISION}.pkg"
    declare -r distributionFilePath="${distributionDestDir}/${distributionFilename}"

    rm -f "${distributionDestDir}/${packagename// /}"*.pkg

    mkdir -p "${PKG_BUILD_DIR}/${packagename}"

    find "${PKG_BUILD_DIR}" -type f -name '*.pkg' -depth 1 | while read component
    do
        pkg="${component##*/}" # ie: EFI.pkg
        pkgdir="${PKG_BUILD_DIR}/${packagename}/${pkg}"
        # expand individual packages
        pkgutil --expand "${PKG_BUILD_DIR}/${pkg}" "$pkgdir"
        rm -f "${PKG_BUILD_DIR}/${pkg}"
    done

    # Create the Distribution file
    ditto --noextattr --noqtn "${PKGROOT}/Distribution" "${PKG_BUILD_DIR}/${packagename}/Distribution"
    makeSubstitutions "${PKG_BUILD_DIR}/${packagename}/Distribution"

    local start_indent_level=2
    echo -e "\n\t<choices-outline>" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
    for main_choice in ${choice_group_items[0]};do
        generateOutlineChoices $main_choice $start_indent_level >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
    done
    echo -e "\t</choices-outline>\n" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"

    generateChoices >> "${PKG_BUILD_DIR}/${packagename}/Distribution"

    for (( i=0; i < ${#pkgrefs[*]} ; i++)); do
        echo -e "${pkgrefs[$i]}" >> "${PKG_BUILD_DIR}/${packagename}/Distribution"
    done

    echo -e "\n</installer-gui-script>"  >> "${PKG_BUILD_DIR}/${packagename}/Distribution"

    # Create the Resources directory
    ditto --noextattr --noqtn "${PKGROOT}/Resources" "${PKG_BUILD_DIR}/${packagename}/Resources"
    find "${PKG_BUILD_DIR}/${packagename}/Resources" -type d -name '.svn' -exec rm -R -f {} \; 2>/dev/null

# #   Make the translation
#     echo ""
#     echo "========= Translating Resources ========"
#     (cd "${PKGROOT}" &&  PERLLIB=${PKGROOT}/bin/po4a/lib                  \
#         bin/po4a/po4a                                                     \
#         --package-name 'Chameleon'                                        \
#         --package-version "${CHAMELEON_VERSION}-r${CHAMELEON_REVISION}"   \
#         --msgmerge-opt '--lang=$lang'                                     \
#         --variable PODIR="po"                                             \
#         --variable TEMPLATES_DIR="Resources/templates"                    \
#         --variable OUTPUT_DIR="${PKG_BUILD_DIR}/${packagename}/Resources" \
#         ${PKGROOT}/po4a-chameleon.cfg)
#
#    # Copy common files in english localisation directory
#    ditto --noextattr --noqtn "${PKGROOT}/Resources/common" "${PKG_BUILD_DIR}/${packagename}/Resources/en.lproj"

    # CleanUp the directory
    find "${PKG_BUILD_DIR}/${packagename}" \( -type d -name '.svn' \) -o -name '.DS_Store' -depth -exec rm -rf {} \;
    find "${PKG_BUILD_DIR}/${packagename}" -type d -depth -empty -exec rmdir {} \; # Remove empty directories

    # Make substitutions for version, revision, stage, developers, credits, etc..
    makeSubstitutions $( find "${PKG_BUILD_DIR}/${packagename}/Resources" -type f )

    # Create the final package
    pkgutil --flatten "${PKG_BUILD_DIR}/${packagename}" "${distributionFilePath}"

    #   Here is the place to assign an icon to the pkg
    #   Icon pkg reworked by ErmaC
    ditto -xk "${PKGROOT}/Icon.zip" "${PKG_BUILD_DIR}/Icons/"
    DeRez -only icns "${PKG_BUILD_DIR}/Icons/Icon.icns" > "${PKG_BUILD_DIR}/Icons/tempicns.rsrc"
    Rez -append "${PKG_BUILD_DIR}/Icons/tempicns.rsrc" -o "${distributionFilePath}"
    SetFile -a C "${distributionFilePath}"
    rm -rf "${PKG_BUILD_DIR}/Icons"

    md5=$( md5 "${distributionFilePath}" | awk {'print $4'} )
    echo "MD5 (${distributionFilePath}) = ${md5}" > "${distributionFilePath}.md5"
    echo ""

    echo -e $COL_GREEN" --------------------------"$COL_RESET
    echo -e $COL_GREEN" Building process complete!"$COL_RESET
    echo -e $COL_GREEN" --------------------------"$COL_RESET
    echo ""
    echo -e $COL_GREEN" Build info."
    echo -e $COL_GREEN" ==========="
    echo -e $COL_BLUE"  Package name: "$COL_RESET"${distributionFilename}"
    echo -e $COL_BLUE"  MD5:          "$COL_RESET"$md5"
    echo -e $COL_BLUE"  Version:      "$COL_RESET"$CLOVER_VERSION"
    echo -e $COL_BLUE"  Stage:        "$COL_RESET"$CLOVER_STAGE"
    echo -e $COL_BLUE"  Date/Time:    "$COL_RESET"$CLOVER_BUILDDATE"
    echo -e $COL_BLUE"  Built by:     "$COL_RESET"$CLOVER_WHOBUILD"
    echo -e $COL_BLUE"  Copyright     "$COL_RESET"$CLOVER_CPRYEAR"
    echo ""
}

# build packages
main

# build meta package
makedistribution


# Local Variables:      #
# mode: ksh             #
# tab-width: 4          #
# indent-tabs-mode: nil #
# End:                  #
#
# vi: set expandtab ts=4 sw=4 sts=4: #
