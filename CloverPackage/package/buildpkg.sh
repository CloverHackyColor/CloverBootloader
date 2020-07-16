#!/bin/bash

# old=1755= ${SRCROOT}/package/buildpkg.sh ${SYMROOT}/package;
# new=1756=@${SRCROOT}/package/buildpkg.sh "$(SRCROOT)" "$(SYMROOT)" "$(PKG_BUILD_DIR)"

# 1=SRCROOT = $(CURDIR)
# 2=SYMROOT = $(SRCROOT)/sym
# 3=PKG_BUILD_DIR = $(SYMROOT)/package

# $3 Path to store built package

# Prevent the script from doing bad things
set -u  # Abort with unset variables
#set -x

usage () {
printf "\n\e[1m%s\e[0m" "Usage: $0 --srcroot <path> --symroot <name> --builddir <path> [flag1 flag2...]"
echo
printf "\n%s" "The following options are mandatory:"
echo
printf "\n\e[1m%s\e[0m\t%s \e[1m(%s)\e[0m." "--srcroot" "Specifies the Clover Package root dir" "usually edk2/Clover/CloverPackage"
printf "\n\e[1m%s\e[0m\t%s \e[1m(%s)\e[0m." "--symroot" "Specifies the name for the subfolder that will contain the final product" "usually sym"
printf "\n\e[1m%s\e[0m\t%s \e[1m(%s)\e[0m." "--builddir" "Specifies the Clover Package temp build dir" "usually edk2/Clover/CloverPackage/sym/package"
echo
printf "\n%s" "The following flags are optional and exclude options (subpackages) from the final Clover package."
printf "\n%s" "Possible values:"
echo
printf "\n\e[1m%s\e[0m\t%s" "--nothemes" "Excludes the Themes subpackage."
printf "\n\e[1m%s\e[0m\t%s" "--noprefpane" "Excludes the Clover Prefpane / Clover Updater subpackage."
printf "\n\e[1m%s\e[0m\t\t%s" "--norc" "Excludes the RC scripts subpackage."
printf "\n\e[1m%s\e[0m\t%s\n\n" "--nolegacy" "Excludes the CloverEFI subpackages."
}

if [[ $# -eq 0 ]]; then usage; exit 1; fi

declare NOEXTRAS=""

while [[ $# -gt 0 ]]; do
	case "${1}" in
		--srcroot ) declare -r SRCROOT="${2}"; shift;; # ie. edk2/Clover/CloverPackage
		--symroot ) declare -r SYMROOT="${2}"; shift;; # ie. edk2/Clover/CloverPackage/sym
		--builddir ) declare -r PKG_BUILD_DIR="${2}"; shift;; # ie. edk2/Clover/CloverPackage/sym/package
		--nothemes ) NOEXTRAS+=", Clover Themes";;
		--noprefpane ) NOEXTRAS+=", Clover Prefpane/Clover Updater";;
		--norc ) NOEXTRAS+=", RC scripts";;
		--nolegacy ) NOEXTRAS+=", CloverEFI";;
		* ) printf "\e[1m%s\e[0m\n" "Invalid option: ${1} !" >&2; usage; exit 1;;
	esac
	shift
done

if [[ -z "$SRCROOT" || -z "$SYMROOT" || -z "$PKG_BUILD_DIR" ]]; then
	printf "\e[1m%s\e[0m\n" "Too few arguments provided. Aborting..." >&2
	usage
	exit 1
fi

if [[ ! -d "$SYMROOT" ]]; then printf "\e[1m%s\e[0m\n" "Directory ${SYMROOT} doesn't exit. Aborting..." >&2; exit 1; fi

packagename="Clover"

declare -r PKGROOT="${0%/*}"    # ie. edk2/Clover/CloverPackage/package
declare -r SCPT_TPL_DIR="${PKGROOT}/Scripts.templates"
declare -r SCPT_LIB_DIR="${PKGROOT}/Scripts.libraries"

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
# stage
CLOVER_STAGE=${RC/Release Candidate }
CLOVER_STAGE=${CLOVER_STAGE/FINAL/2.2 Final}
declare -r CLOVER_STAGE
declare -r CLOVER_REVISION=$( cat revision )
if [[ -d "$(dirname ${SRCROOT})"/.git ]];then
  declare -r CLOVER_SHA1=$( git -C $(dirname "${SRCROOT}") rev-parse --short HEAD )
else
  declare -r CLOVER_SHA1="N/A"
fi

declare -r CLOVER_BUILDDATE=$( sed -n 's/.*FIRMWARE_BUILDDATE *\"\(.*\)\".*/\1/p' "${PKGROOT}/../../Version.h" )
declare -r CLOVER_TIMESTAMP=$( date -j -f "%Y-%m-%d %H:%M:%S" "${CLOVER_BUILDDATE}" "+%s" )

# =================
declare -r CLOVER_DEVELOP=$(awk "NR==6{print;exit}"  ${PKGROOT}/../CREDITS)
declare -r CLOVER_CREDITS=$(awk "NR==10{print;exit}" ${PKGROOT}/../CREDITS)
declare -r CLOVER_PKGDEV=$(awk "NR==14{print;exit}"  ${PKGROOT}/../CREDITS)
declare -r CLOVER_CPRYEAR=$(awk "NR==18{print;exit}" ${PKGROOT}/../CREDITS)"-"$( date +"%Y" )
whoami=$(whoami | awk '{print $1}' | cut -d ":" -f3)
if [[ "$whoami" == "admin" ]];then
    declare -r CLOVER_WHOBUILD="VoodooLabs BuildBot"
else
    declare -r CLOVER_WHOBUILD="$whoami"
fi

# ====== GLOBAL VARIABLES ======
declare -r DRIVERS_LEGACY="BIOS" # same in ebuild.sh/makeiso
declare -r DRIVERS_UEFI="UEFI"   # same in ebuild.sh/makeiso
declare -r DRIVERS_OFF="off"     # same in ebuild.sh/makeiso
declare -r LOG_FILENAME="Clover_Installer_Log.txt"
declare -r CLOVER_INSTALLER_PLIST="/Library/Preferences/com.projectosx.clover.installer.plist"

declare -a pkgrefs
declare -a choice_key
declare -a choice_options
declare -a choice_selected
declare -a choice_force_selected
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

# Package identifiers
declare -r clover_package_identity="org.clover"

# ====== FUNCTIONS ======
trim () {
    local result="${1#"${1%%[![:space:]]*}"}"   # remove leading whitespace characters
    echo "${result%"${result##*[![:space:]]}"}" # remove trailing whitespace characters
}

# Check if an element is in an array
# Arguments:
#    $1: string to search
#    $2+: array elements
#
# Return the string if found else return an empty string
function inArray() {
  local element
  for element in "${@:2}"; do [[ "$element" == "$1" ]] && echo "$element" && return 0; done
  return 1
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
s&%CLOVERREVISION%&${CLOVER_REVISION}&g
s&%CLOVERSHA1%&${CLOVER_SHA1}&g
s&%CLOVERSTAGE%&${CLOVER_STAGE}&g
s&%DEVELOP%&${CLOVER_DEVELOP}&g
s&%CREDITS%&${CLOVER_CREDITS}&g
s&%PKGDEV%&${CLOVER_PKGDEV}&g
s&%CPRYEAR%&${CLOVER_CPRYEAR}&g
s&%WHOBUILD%&${CLOVER_WHOBUILD}&g
:t
/@[a-zA-Z_][a-zA-Z_0-9]*@/!b
s&@CLOVER_INSTALLER_PLIST_NEW@&${CLOVER_INSTALLER_PLIST}.new&g
s&@CLOVER_INSTALLER_PLIST@&${CLOVER_INSTALLER_PLIST}&g
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
    local choiceSelected=""
    local choiceForceSelected=""
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
            --enabled=*)
                       shift; choiceOptions="$choiceOptions enabled=\"${option#*=}\"" ;;
            --selected=*)
                       shift; choiceSelected="${option#*=}" ;;
            --force-selected=*)
                       shift; choiceForceSelected="${option#*=}" ;;
            --visible=*)
                       shift; choiceOptions="$choiceOptions visible=\"${option#*=}\"" ;;
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

    # check if we have a localization ready in Localizable.strings
    local ltitle=""
    local ldescription=""
    if grep -q "\"${choiceId}_title\"" "${PKGROOT}/Resources/templates/Localizable.strings"; then
      ltitle="${choiceId}_title"
    else
      ltitle="${choiceId%.UEFI}"
    fi

    if grep -q "\"${choiceId}_description\"" "${PKGROOT}/Resources/templates/Localizable.strings"; then
      ldescription="${choiceId}_description"
    else
      ldescription="${choiceId%.UEFI}"
    fi

    # Record new node
    choice_key[$idx]="$choiceId"
    choice_title[$idx]="$ltitle"
    choice_description[$idx]="${ldescription}"

    choice_options[$idx]=$(trim "${choiceOptions}") # Removing leading and trailing whitespace(s)
    choice_selected[$idx]=$(trim "${choiceSelected}") # Removing leading and trailing whitespace(s)
    choice_force_selected[$idx]=$(trim "${choiceForceSelected}") # Removing leading and trailing whitespace(s)
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
    local choiceOptions=

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
            --start-selected=*)
                       shift; choiceOptions+=("--start-selected=${option#*=}") ;;
            --start-enabled=*)
                       shift; choiceOptions+=("--start-enabled=${option#*=}") ;;
            --start-visible=*)
                       shift; choiceOptions+=("--start-visible=${option#*=}") ;;
            --enabled=*)
                       shift; choiceOptions+=("--enabled=${option#*=}") ;;
            --selected=*)
                       shift; choiceOptions+=("--selected=${option#*=}") ;;
            --visible=*)
                       shift; choiceOptions+=("--visible=${option#*=}") ;;
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

    addChoice --group="$groupChoice" --title="$title" --description="$description" ${choiceOptions[*]} "${1}"
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
    else
        echo "choices['$myChoice'].selected"
    fi
}

exclusive_zero_or_one_choice () {
    # $1 Current choice (ie: test1)
    # $2..$n Others choice(s) (ie: "test2" "test3"). Current can or can't be in the others choices
    local myChoice="${1}"
    local result;
    local exclusive_one_choice_code="$(exclusive_one_choice ${@})"
    echo "(my.choice.selected &amp;&amp; $(exclusive_one_choice ${@}))"
}

main ()
{

# clean up the destination path

    rm -R -f "${PKG_BUILD_DIR}"
    echo ""
    echo -e $COL_CYAN"  -------------------------------"$COL_RESET
    echo -e $COL_CYAN"  Building $packagename Install Package"$COL_RESET
    echo -e $COL_CYAN"  -------------------------------"$COL_RESET
    echo ""
	if [[ ! -z ${NOEXTRAS} ]]; then printf "$COL_CYAN  %s$COL_RESET\n\n" "Excluded packages: ${NOEXTRAS:2}"; fi

# build Pre package
    echo "====================== Preinstall ======================"
    packagesidentity="${clover_package_identity}"
    choiceId="Pre"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="false"  --start-selected="true"  --pkg-refs="$packageRefId" "${choiceId}"
# End pre install choice

# build UEFI only
    echo "===================== Installation ====================="
    packagesidentity="$clover_package_identity"
    choiceId="UEFI.only"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/EFI
    rsync -r --exclude=.svn --exclude="*~" --exclude='drivers*'   \
     ${SRCROOT}/CloverV2/EFI/BOOT ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                       --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/EFIROOTDIR"
    if [[ ${NOEXTRAS} != *"CloverEFI"* ]]; then
        addChoice --start-visible="true" --start-selected="choicePreviouslySelected('$packageRefId')"  \
                  --pkg-refs="$packageRefId" "${choiceId}"
    else
        addChoice --start-visible="false" --start-selected="true"  \
                  --pkg-refs="$packageRefId" "${choiceId}"
    fi
# End UEFI only

# build EFI target
    echo "================== Target ESP =========================="
    packagesidentity="$clover_package_identity"
    choiceId="Target.ESP"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    installer_target_esp_refid=$packageRefId
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                       --subst="INSTALLER_CHOICE=$installer_target_esp_refid" MarkChoice
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="true" --start-selected="choicePreviouslySelected('$packageRefId')"  \
              --selected="choices['UEFI.only'].selected || choices['Target.ESP'].selected"         \
              --pkg-refs="$packageRefId" "${choiceId}"
# End build EFI target

# build BiosBoot package
if [[ ${NOEXTRAS} != *"CloverEFI"* ]]; then
    echo "=================== BiosBoot ==========================="
    packagesidentity="$clover_package_identity"
    choiceId="BiosBoot"

    ls "${SRCROOT}"/CloverV2/Bootloaders/x64/boot? &>/dev/null && \
     ditto --noextattr --noqtn ${SRCROOT}/CloverV2/Bootloaders/x64/boot?  ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/x64/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/boot0af     ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/boot0ss     ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/boot1f32    ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/boot1f32alt ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/boot1h      ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/boot1h2     ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/boot1x      ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/boot1xalt   ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/

#    ditto --noextattr --noqtn ${SRCROOT}/utils/fdisk440/fdisk440.8        ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/man/man8/
#    ditto --noextattr --noqtn ${SYMROOT}/utils/fdisk440                   ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin/
    ditto --noextattr --noqtn ${SYMROOT}/utils/boot1-install              ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin/

    # Add some documentation
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/Description.txt  ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/Installation.txt ${PKG_BUILD_DIR}/${choiceId}/Root/usr/standalone/i386/
    ditto --noextattr --noqtn ${SRCROOT}/CloverV2/BootSectors/Installation.txt ${PKG_BUILD_DIR}/${choiceId}/Root/EFIROOTDIR/EFI/CLOVER/doc/

    fixperms "${PKG_BUILD_DIR}/${choiceId}/Root/"
#    chmod 755 "${PKG_BUILD_DIR}/${choiceId}"/Root/usr/local/bin/{fdisk440,boot1-install}
    chmod 755 "${PKG_BUILD_DIR}/${choiceId}"/Root/usr/local/bin/boot1-install
    
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" ${choiceId}

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    packageBiosBootRefId=$packageRefId
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="false" --start-selected="false" --pkg-refs="$packageRefId" "${choiceId}"
# End build BiosBoot package
fi
# build Utils package
    echo "===================== Utils ============================"
    packagesidentity="$clover_package_identity"
    choiceId="Utils"
    # Utils
    ditto --noextattr --noqtn ${SYMROOT}/utils/bdmesg            ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin/
   # ditto --noextattr --noqtn ${SYMROOT}/utils/clover-genconfig  ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin/
    ditto --noextattr --noqtn ${SYMROOT}/utils/partutil          ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin/
    ditto --noextattr --noqtn ${SYMROOT}/utils/espfinder         ${PKG_BUILD_DIR}/${choiceId}/Root/usr/local/bin/
    fixperms "${PKG_BUILD_DIR}/${choiceId}/Root/"
    #chmod 755 "${PKG_BUILD_DIR}/${choiceId}"/Root/usr/local/bin/{bdmesg,clover-genconfig,partutil,espfinder}
    chmod 755 "${PKG_BUILD_DIR}/${choiceId}"/Root/usr/local/bin/{bdmesg,partutil,espfinder}
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    packageUtilsRefId=$packageRefId
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="false" --start-selected="true"  \
              --pkg-refs="$packageRefId" "${choiceId}"
# End build Utils package

# build core EFI folder package
    echo "===================== EFI folder ======================="
    packagesidentity="$clover_package_identity"
    choiceId="EFIFolder"
    rm -rf   ${PKG_BUILD_DIR}/${choiceId}/Root/EFI
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/EFI
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts
    # Add the partutil binary as a helper to mount ESP
    ditto --noextattr --noqtn ${SYMROOT}/utils/partutil  ${PKG_BUILD_DIR}/${choiceId}/Scripts/
    # Add the espfinder binary as a helper to mount ESP when target is apfs, corestorage, Fusion or RAID
    ditto --noextattr --noqtn ${SYMROOT}/utils/espfinder  ${PKG_BUILD_DIR}/${choiceId}/Scripts/
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"                     \
                       --subst="CLOVER_PACKAGE_IDENTITY=$clover_package_identity"       \
                       --subst="INSTALLER_TARGET_ESP_REFID=$installer_target_esp_refid" \
                       --subst="CLOVER_DRIVERS_LEGACY=$DRIVERS_LEGACY" \
                       --subst="CLOVER_DRIVERS_UEFI=$DRIVERS_UEFI" \
                       ${choiceId}
    rsync -r --exclude=.svn --exclude="*~" --exclude='drivers*'   \
     ${SRCROOT}/CloverV2/EFI/BOOT ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/
    rsync -r --exclude=.svn --exclude="*~" --exclude='drivers*'   \
     ${SRCROOT}/CloverV2/EFI/CLOVER ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/

    # config.plist
    rm -f ${PKG_BUILD_DIR}/${choiceId}/Root/EFI/CLOVER/config.plist &>/dev/null
    fixperms "${PKG_BUILD_DIR}/${choiceId}/Root/"

    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/EFIROOTDIR"
    addChoice --start-visible="false" --start-selected="true" --pkg-refs="$packageRefId" "${choiceId}"
# End build EFI folder package

# build off drivers package
    echo "===================== off drivers ======================"
    packagesidentity="$clover_package_identity"
    choiceId="off"
    offPath="${PKG_BUILD_DIR}/${choiceId}/Root/EFI/CLOVER/drivers/$DRIVERS_OFF"
    rm -rf   "${offPath}"
    mkdir -p "${offPath}"
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts
    fixperms "${PKG_BUILD_DIR}/${choiceId}/Root/"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice

    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                           --subst="DRIVER_DIR=$DRIVERS_LEGACY" off


    # Regroup off drivers
    find "${SRCROOT}"/CloverV2/EFI/CLOVER/drivers -type f -name '*.efi' -exec cp -R {} "${offPath}"/ \;



    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/EFIROOTDIR"
    addChoice --start-visible="true" \
              --start-selected="choicePreviouslySelected('$packageRefId')" \
              --pkg-refs="$packageRefId" "${choiceId}"
# End build off drivers package

# Create Bootloader Node
if [[ ${NOEXTRAS} != *"CloverEFI"* ]]; then
    addGroupChoices --enabled="!choices['UEFI.only'].selected" --visible="!choices['UEFI.only'].selected" --exclusive_one_choice "Bootloader"
    echo "===================== BootLoaders ======================"
    packagesidentity="$clover_package_identity".bootloader

    # build alternative booting package
    choiceId="AltBoot"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    altbootRefId=$packageRefId
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root

    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                       --subst="INSTALLER_CHOICE=$packageRefId"     \
                       MarkChoice
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"  ${choiceId}

    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/EFIROOTDIR"
    addChoice --start-selected="choicePreviouslySelected('$packageRefId')"                          \
              --selected="!choices['UEFI.only'].selected &amp;&amp; choices['$choiceId'].selected"  \
              --visible="choices['boot0af'].selected || choices['boot0ss'].selected"                \
              --pkg-refs="$packageBiosBootRefId $packageUtilsRefId $packageRefId" "${choiceId}"
    # End alternative booting package

    # build bootNo package
    choiceId="bootNo"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"     \
                       --subst="INSTALLER_CHOICE=$packageRefId"         \
                       --subst="INSTALLER_ALTBOOT_REFID=$altbootRefId"  \
                       ${choiceId}
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/EFIROOTDIR"
    addChoice --group="Bootloader"                                                                \
              --enabled="!choices['UEFI.only'].selected"                                          \
              --start-selected="choicePreviouslySelected('$packageRefId') || checkBootFromUEFI()" \
              --force-selected="choices['UEFI.only'].selected"                                    \
              --pkg-refs="$packageRefId" "${choiceId}"
    # End build bootNo package

    # build boot0af package
    choiceId="boot0af"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"     \
                       --subst="INSTALLER_CHOICE=$packageRefId"         \
                       --subst="INSTALLER_ALTBOOT_REFID=$altbootRefId"  \
                       --subst="MBR_SECTOR_FILE"=boot0af                \
                       InstallBootsectors
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/EFIROOTDIR"
    addChoice --group="Bootloader"                                         \
              --enabled="!choices['UEFI.only'].selected"                   \
              --start-selected="choicePreviouslySelected('$packageRefId')" \
              --pkg-refs="$packageBiosBootRefId $packageUtilsRefId $packageRefId" "${choiceId}"
    # End build boot0af package

    # build boot0ss package
    choiceId="boot0ss"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"     \
                       --subst="INSTALLER_CHOICE=$packageRefId"         \
                       --subst="INSTALLER_ALTBOOT_REFID=$altbootRefId"  \
                       --subst="MBR_SECTOR_FILE"=boot0ss                \
                       InstallBootsectors
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/EFIROOTDIR"
    addChoice --group="Bootloader"                                          \
              --enabled="!choices['UEFI.only'].selected"                    \
              --start-selected="choicePreviouslySelected('$packageRefId')"  \
              --pkg-refs="$packageBiosBootRefId $packageUtilsRefId $packageRefId" "${choiceId}"
    # End build boot0ss package

    # Create CloverEFI Node
    echo "====================== CloverEFI ======================="
    nb_cloverEFI=$(find "${SRCROOT}"/CloverV2/Bootloaders -type f -name 'boot?' | wc -l)
    local cloverEFIGroupOption=(--exclusive_one_choice)
    [[ "$nb_cloverEFI" -lt 2 ]] && cloverEFIGroupOption=(--selected="!choices['UEFI.only'].selected")
    addGroupChoices --enabled="!choices['UEFI.only'].selected" --visible="!choices['UEFI.only'].selected" ${cloverEFIGroupOption[@]} "CloverEFI"

    # build cloverEFI.64.sata package
    if [[ -f "${SRCROOT}/CloverV2/Bootloaders/x64/boot6" ]]; then
        packagesidentity="$clover_package_identity"
        choiceId="cloverEFI.64.sata"
        packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
        mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"  \
                        --subst="CLOVER_EFI_ARCH=x64"                 \
                        --subst="CLOVER_BOOT_FILE=boot6"              \
                        --subst="INSTALLER_CHOICE=$packageRefId"      \
                        CloverEFI
        buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
        local choiceOptions=(--group="CloverEFI" --enabled="!choices['UEFI.only'].selected")
        [[ "$nb_cloverEFI" -ge 2 ]] && \
        choiceOptions+=(--start-selected="choicePreviouslySelected('$packageRefId')")
        choiceOptions+=(--selected="!choices['UEFI.only'].selected")
        addChoice ${choiceOptions[@]} --pkg-refs="$packageBiosBootRefId $packageRefId" "${choiceId}"
    fi
    # End build boot64 package

    # build cloverEFI.64.blockio package
    if [[ -f "${SRCROOT}/CloverV2/Bootloaders/x64/boot7" ]]; then
        packagesidentity="$clover_package_identity"
        choiceId="cloverEFI.64.blockio"
        packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
        mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"  \
                        --subst="CLOVER_EFI_ARCH=x64"                 \
                        --subst="CLOVER_BOOT_FILE=boot7"              \
                        --subst="INSTALLER_CHOICE=$packageRefId"      \
                        CloverEFI
        buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
        local choiceOptions=(--group="CloverEFI" --enabled="!choices['UEFI.only'].selected")
        [[ "$nb_cloverEFI" -ge 2 ]] && \
        choiceOptions+=(--start-selected="choicePreviouslySelected('$packageRefId')")
        choiceOptions+=(--selected="!choices['UEFI.only'].selected")
        addChoice ${choiceOptions[@]} --pkg-refs="$packageBiosBootRefId $packageRefId" "${choiceId}"
    fi
    # End build cloverEFI.64.blockio package

    # build for chipset only NVIDIA NFORCE-MCP79 cloverEFI.64.blockio2 package
    if [[ -f "${SRCROOT}/CloverV2/Bootloaders/x64/boot7-MCP79" ]]; then
        packagesidentity="$clover_package_identity"
        choiceId="cloverEFI.64.blockio2"
        packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
        mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"  \
                        --subst="CLOVER_EFI_ARCH=x64"                 \
                        --subst="CLOVER_BOOT_FILE=boot7-MCP79"              \
                        --subst="INSTALLER_CHOICE=$packageRefId"      \
                        CloverEFI
        buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
        local choiceOptions=(--group="CloverEFI" --enabled="!choices['UEFI.only'].selected")
        [[ "$nb_cloverEFI" -ge 2 ]] && \
        choiceOptions+=(--start-selected="choicePreviouslySelected('$packageRefId')")
        choiceOptions+=(--selected="!choices['UEFI.only'].selected")
        addChoice ${choiceOptions[@]} --pkg-refs="$packageBiosBootRefId $packageRefId" "${choiceId}"
    fi
    # End for chipset only NVIDIA NFORCE-MCP79 cloverEFI.64.blockio2 package
fi

# build mandatory drivers-x64 packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_LEGACY" && ${NOEXTRAS} != *"CloverEFI"* ]]; then
    echo "================= drivers64 mandatory =================="
    addGroupChoices --title="Drivers64" --description="Drivers64"  \
      --enabled="!choices['UEFI.only'].selected"     \
      --visible="!choices['UEFI.only'].selected"     \
      "Drivers64"

    addGroupChoices --title="Recommended64" --description="Recommended64"  \
      --enabled="!choices['UEFI.only'].selected"     \
      --visible="!choices['UEFI.only'].selected"     \
      --parent=Drivers64 \
      "Recommended64"

    packagesidentity="${clover_package_identity}".drivers64.mandatory
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_LEGACY" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_LEGACY"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverChoice="${driver%.efi}"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverChoice}/Root/"
        find "${PKG_BUILD_DIR}/${driverChoice}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverChoice}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverChoice}")

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverChoice}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice

        # Add postinstall script for SMCHelper drivers to remove it if VirtualSMC driver exists
        if [[ "$driver" == SMCHelper* ]]; then
         addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverChoice}"  \
                            --subst="DRIVER_NAME=$driver"                     \
                            --subst="DRIVER_DIR=$(basename $driverDestDir)"   \
                            "VirtualSMC"
        fi
        # mandatory drivers starts all selected only if /Library/Preferences/com.projectosx.clover.installer.plist does not exist
        # (i.e. Clover package never run on that target partition).
        # Otherwise each single choice start selected only for legacy Clover and only if you previously selected it
        buildpackage "$packageRefId" "${driverChoice}" "${PKG_BUILD_DIR}/${driverChoice}" "${driverDestDir}"
        addChoice --group="Recommended64" \
                  --start-visible="true" \
                  --enabled="!choices['UEFI.only'].selected" \
                  --start-selected="!choices['UEFI.only'].selected &amp;&amp; (cloverPackageFirstRun() || choicePreviouslySelected('$packageRefId'))"  \
                  --visible="!choices['UEFI.only'].selected"     \
                  --pkg-refs="$packageRefId"  "${driverChoice}"
        rm -R -f "${PKG_BUILD_DIR}/${driverChoice}"
    done
fi
# End mandatory drivers-x64 packages

# build drivers-x64 packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY" && ${NOEXTRAS} != *"CloverEFI"* ]]; then
    echo "===================== drivers64 ========================"
    packagesidentity="${clover_package_identity}".drivers64
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_LEGACY"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ )); do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice
        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"
        addChoice --group="Drivers64" --title="$driverName"                                               \
                  --enabled="!choices['UEFI.only'].selected"                                              \
                  --start-selected="!choices['UEFI.only'].selected &amp;&amp; choicePreviouslySelected('$packageRefId')"                            \
                  --selected="choices['$driverName'].selected"  \
                  --pkg-refs="$packageRefId"                                                              \
                  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build drivers-x64 packages

# build FileSystem drivers-x64 packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileSystem"  && ${NOEXTRAS} != *"CloverEFI"* ]]; then
    echo "=============== drivers64 FileSystem ==================="
    addGroupChoices --title="FileSystem64" --description="FileSystem64" --parent=Drivers64 "FileSystem64"
    packagesidentity="${clover_package_identity}".drivers64
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileSystem" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_LEGACY"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="DRIVER_DIR=$DRIVERS_LEGACY" \
                           --subst="DRIVER_NAME=$driver" \
                           FileSystem

        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"

        if [[ $driver == VBoxHfs* || $driver == HFSPlus* ]] && \
           [[ -f "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileSystem/VBoxHfs.efi" ]] && \
           [[ -f "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileSystem/HFSPlus.efi" ]]; then
          if [[ $driver == VBoxHfs* ]]; then
            addChoice --group="FileSystem64"  --title="$driverName"                \
                      --start-selected="choicePreviouslySelected('$packageRefId')"  \
                      --selected="!choices['HFSPlus'].selected" \
                      --pkg-refs="$packageRefId"  "${driverName}"
          else
            addChoice --group="FileSystem64"  --title="$driverName"                \
                      --start-selected="choicePreviouslySelected('$packageRefId')"  \
                      --selected="!choices['VBoxHfs'].selected" \
                      --pkg-refs="$packageRefId"  "${driverName}"
          fi
        elif [[ $driver == ApfsDriverLoader* ]]; then
          addChoice --group="FileSystem64"  --title="$driverName"                \
                    --start-selected="cloverPackageFirstRun() || choicePreviouslySelected('$packageRefId')"  \
                    --pkg-refs="$packageRefId"  "${driverName}"
        else
          addChoice --group="FileSystem64"  --title="$driverName"                \
                    --start-selected="choicePreviouslySelected('$packageRefId')"  \
                    --pkg-refs="$packageRefId"  "${driverName}"
        fi
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build FileSystem drivers-x64 packages

# build FileVault drivers-x64 packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileVault2" && ${NOEXTRAS} != *"CloverEFI"* ]]; then
    echo "=============== drivers64 FileVault2 ==================="
      addGroupChoices --title="Drivers64FV2" --description="Drivers64FV2"  \
      --enabled="!choices['UEFI.only'].selected"     \
      --visible="!choices['UEFI.only'].selected"     \
       --parent=Drivers64 \
      "Drivers64FV2"
    packagesidentity="${clover_package_identity}".fv2.drivers64
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileVault2" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_LEGACY"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ )); do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")
        # Add postinstall script for old FileVault2 drivers to remove it if AppleUiSupport driver exists
        if [[ "$driver" == AppleImageCodec* || "$driver" == AppleKeyAggregator* ||
                "$driver" == AppleUITheme* || "$driver" == FirmwareVolume* || "$driver" == HashServiceFix* ]]; then
         addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}"  \
                            --subst="DRIVER_NAME=$driver"                     \
                            --subst="DRIVER_DIR=$(basename $driverDestDir)"   \
                            "AppleUiSupport"
        fi

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice
        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"
        addChoice --group="Drivers64FV2" --title="$driverName"                                               \
                  --enabled="!choices['UEFI.only'].selected"                                              \
                  --start-selected="!choices['UEFI.only'].selected &amp;&amp; choicePreviouslySelected('$packageRefId')"                            \
                  --selected="choices['$driverName'].selected"  \
                  --pkg-refs="$packageRefId"                                                              \
                  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build FileVault drivers-x64 packages

# build mandatory drivers-x64UEFI packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_UEFI" ]]; then
    echo "=============== drivers64 UEFI mandatory ==============="
    addGroupChoices --title="Drivers64UEFI" --description="Drivers64UEFI" "Drivers64UEFI"
    addGroupChoices --title="Recommended64UEFI" --description="Recommended64UEFI" --parent=Drivers64UEFI "Recommended64UEFI"
    packagesidentity="${clover_package_identity}".drivers64UEFI.mandatory
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_UEFI" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_UEFI"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverChoice="${driver%.efi}.UEFI"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverChoice}/Root/"
        find "${PKG_BUILD_DIR}/${driverChoice}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverChoice}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverChoice}")

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverChoice}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice

        # Add postinstall script for SMCHelper drivers to remove it if VirtualSMC driver exists
        if [[ "$driver" == SMCHelper* ]]; then
         addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverChoice}"  \
                            --subst="DRIVER_NAME=$driver"                     \
                            --subst="DRIVER_DIR=$(basename $driverDestDir)"   \
                            "VirtualSMC"
        fi

        buildpackage "$packageRefId" "${driverChoice}" "${PKG_BUILD_DIR}/${driverChoice}" "${driverDestDir}"
        addChoice --group="Recommended64UEFI" \
                  --start-visible="true" \
                  --start-selected="cloverPackageFirstRun() || choicePreviouslySelected('$packageRefId')" \
                  --pkg-refs="$packageRefId"  "${driverChoice}"
        rm -R -f "${PKG_BUILD_DIR}/${driverChoice}"
    done
fi
# End mandatory drivers-x64UEFI packages

# build drivers-x64UEFI packages 
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI" ]]; then
    echo "=================== drivers64 UEFI ====================="
    packagesidentity="${clover_package_identity}".drivers64UEFI
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_UEFI"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}.UEFI"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice
        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"
        addChoice --group="Drivers64UEFI"  --title="$driverName"                \
                  --start-selected="choicePreviouslySelected('$packageRefId')"  \
                  --pkg-refs="$packageRefId"  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build drivers-x64UEFI packages

# build HID drivers-x64UEFI packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/HID" ]]; then
    echo "============= drivers64 UEFI HID ========================"
    addGroupChoices --title="HID64UEFI" --description="HID64UEFI" --parent=Drivers64UEFI "HID64UEFI"
    packagesidentity="${clover_package_identity}".drivers64UEFI
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/HID" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_UEFI"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}.UEFI"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice

        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"

        addChoice --group="HID64UEFI"  --title="$driverName"                \
                    --start-selected="choicePreviouslySelected('$packageRefId')"  \
                    --pkg-refs="$packageRefId"  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build HID drivers-x64UEFI packages

# build FileSystem drivers-x64UEFI packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileSystem" ]]; then
    echo "============= drivers64 UEFI FileSystem ================="
    addGroupChoices --title="FileSystem64UEFI" --description="FileSystem64UEFI" --parent=Drivers64UEFI "FileSystem64UEFI"
    packagesidentity="${clover_package_identity}".drivers64UEFI
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileSystem" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_UEFI"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}.UEFI"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="DRIVER_DIR=$DRIVERS_UEFI" \
                           --subst="DRIVER_NAME=$driver" \
                           FileSystem

        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"

        if [[ $driver == VBoxHfs* || $driver == HFSPlus* ]] && \
           [[ -f "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileSystem/VBoxHfs.efi" ]] && \
           [[ -f "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileSystem/HFSPlus.efi" ]]; then


          if [[ $driver == VBoxHfs* ]]; then
            addChoice --group="FileSystem64UEFI"  --title="$driverName"                \
                      --start-selected="choicePreviouslySelected('$packageRefId')"  \
                      --selected="!choices['HFSPlus.UEFI'].selected" \
                      --pkg-refs="$packageRefId"  "${driverName}"
          else
            addChoice --group="FileSystem64UEFI"  --title="$driverName"                \
                      --start-selected="choicePreviouslySelected('$packageRefId')"  \
                      --selected="!choices['VBoxHfs.UEFI'].selected" \
                      --pkg-refs="$packageRefId"  "${driverName}"
          fi
        elif [[ $driver == ApfsDriverLoader* ]]; then
          addChoice --group="FileSystem64UEFI"  --title="$driverName"                \
                    --start-selected="cloverPackageFirstRun() || choicePreviouslySelected('$packageRefId')"  \
                    --pkg-refs="$packageRefId"  "${driverName}"
        else
          addChoice --group="FileSystem64UEFI"  --title="$driverName"                \
                    --start-selected="choicePreviouslySelected('$packageRefId')"  \
                    --pkg-refs="$packageRefId"  "${driverName}"
        fi
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build FileSystem drivers-x64UEFI packages

# build MemoryFix drivers-x64UEFI packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/MemoryFix" ]]; then
    echo "============= drivers64 UEFI MemoryFix ================="
    addGroupChoices --title="MemoryFix64UEFI" --description="MemoryFix64UEFI" --parent=Drivers64UEFI --exclusive_zero_or_one_choice "MemoryFix64UEFI"
    packagesidentity="${clover_package_identity}".drivers64UEFI
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/MemoryFix" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_UEFI"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}.UEFI"
   
        if [[ "$driver" != OpenRuntime.efi ]]; then
          if [[ "$driver" == OcQuirks.efi ]]; then
            ditto --noextattr --noqtn --arch i386 \
            "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/MemoryFix/OpenRuntime.efi" \
            "${PKG_BUILD_DIR}/${driverName}/Root/"
          fi
          ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
          find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
          fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

          packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")

          addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                             --subst="DRIVER_DIR=$DRIVERS_UEFI" \
                             --subst="DRIVER_NAME=$driver" \
                             MemoryFix

          addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                             --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice

          buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"
          addChoice --group="MemoryFix64UEFI"  --title="$driverName"                \
                    --start-selected="choicePreviouslySelected('$packageRefId')"  \
                    --pkg-refs="$packageRefId"  "${driverName}"
          rm -R -f "${PKG_BUILD_DIR}/${driverName}"
        fi
    done
fi
# End build FileVault drivers-x64UEFI packages

# build FileVault drivers-x64UEFI packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileVault2" ]]; then
    echo "============= drivers64 UEFI FileVault2 ================"
    addGroupChoices --title="Drivers64UEFIFV2" --description="Drivers64UEFIFV2"  --parent=Drivers64UEFI "Drivers64UEFIFV2"
    packagesidentity="${clover_package_identity}".fv2.drivers64UEFI
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileVault2" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_UEFI"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}.UEFI"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")
        # Add postinstall script for old FileVault2 drivers to remove it if AppleUiSupport driver exists
        if [[ "$driver" == AppleImageCodec* || "$driver" == AppleKeyAggregator* ||
                "$driver" == AppleUITheme* || "$driver" == FirmwareVolume* || "$driver" == HashServiceFix* ]]; then
         addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}"  \
                            --subst="DRIVER_NAME=$driver"                     \
                            --subst="DRIVER_DIR=$(basename $driverDestDir)"   \
                            "AppleUiSupport"
        fi
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice
        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"
        addChoice --group="Drivers64UEFIFV2"  --title="$driverName"                \
                  --start-selected="choicePreviouslySelected('$packageRefId')"  \
                  --pkg-refs="$packageRefId"  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build FileVault drivers-x64UEFI packages

# build 'Other' drivers-x64UEFI packages
if [[ -d "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/Other" ]]; then
    echo "============= drivers64 UEFI Other ======================"
    addGroupChoices --title="Other64UEFI" --description="Other64UEFI" --parent=Drivers64UEFI "Other64UEFI"
    packagesidentity="${clover_package_identity}".drivers64UEFI
    local drivers=($( find "${SRCROOT}/CloverV2/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/Other" -type f -name '*.efi' -depth 1 | sort -f ))
    local driverDestDir="/EFIROOTDIR/EFI/CLOVER/drivers/$DRIVERS_UEFI"
    for (( i = 0 ; i < ${#drivers[@]} ; i++ ))
    do
        local driver="${drivers[$i]##*/}"
        local driverName="${driver%.efi}.UEFI"
        ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${PKG_BUILD_DIR}/${driverName}/Root/"
        find "${PKG_BUILD_DIR}/${driverName}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
        fixperms "${PKG_BUILD_DIR}/${driverName}/Root/"

        packageRefId=$(getPackageRefId "${packagesidentity}" "${driverName}")

        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${driverName}" \
                           --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice

        buildpackage "$packageRefId" "${driverName}" "${PKG_BUILD_DIR}/${driverName}" "${driverDestDir}"

        addChoice --group="Other64UEFI"  --title="$driverName"                \
                    --start-selected="choicePreviouslySelected('$packageRefId')"  \
                    --pkg-refs="$packageRefId"  "${driverName}"
        rm -R -f "${PKG_BUILD_DIR}/${driverName}"
    done
fi
# End build Other drivers-x64UEFI packages

# build rc scripts package
if [[ ${NOEXTRAS} != *"RC scripts"* ]]; then
    echo "===================== RC Scripts ======================="
    packagesidentity="$clover_package_identity"


    choiceId="rc.scripts.on.target"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    rcScriptsOnTargetPkgRefId=$packageRefId
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                       --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="true" \
              --start-selected="checkFileExists('/System/Library/CoreServices/boot.efi') &amp;&amp; choicePreviouslySelected('$packageRefId')" \
              --start-enabled="checkFileExists('/System/Library/CoreServices/boot.efi')" \
              --pkg-refs="$packageRefId" "${choiceId}"

    choiceId="rc.scripts.on.all.volumes"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    rcScriptsOnAllColumesPkgRefId=$packageRefId
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                       --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="true" --start-selected="choicePreviouslySelected('$packageRefId')" \
              --pkg-refs="$packageRefId" "${choiceId}"

    choiceIdRcScriptsCore="rc.scripts.core"
    choiceId=$choiceIdRcScriptsCore
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/Library/LaunchDaemons
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/Library/Application\ Support/Clover
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root/etc
    mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Scripts
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"                            \
                       --subst="INSTALLER_ON_TARGET_REFID=$rcScriptsOnTargetPkgRefId"          \
                       --subst="INSTALLER_ON_ALL_VOLUMES_REFID=$rcScriptsOnAllColumesPkgRefId" \
                       RcScripts
    # Add the rc script library
    cp -f "$SCPT_LIB_DIR"/rc_scripts.lib "${PKG_BUILD_DIR}/${choiceId}"/Scripts
    rsync -r --exclude=.* --exclude="*~" ${SRCROOT}/CloverV2/rcScripts/ ${PKG_BUILD_DIR}/${choiceId}/Root/
    local toolsdir="${PKG_BUILD_DIR}/${choiceId}"/Scripts/Tools
    mkdir -p "$toolsdir"
    (cd "${PKG_BUILD_DIR}/${choiceId}"/Root && find {etc,Library} -type f > "$toolsdir"/rc.files)
    fixperms "${PKG_BUILD_DIR}/${choiceId}/Root/"
    chmod 644 "${PKG_BUILD_DIR}/${choiceId}/Root/Library/LaunchDaemons/com.projectosx.clover.daemon.plist"
    chmod 744 "${PKG_BUILD_DIR}/${choiceId}/Root/Library/Application Support/Clover/CloverDaemon"
    chmod 744 "${PKG_BUILD_DIR}/${choiceId}/Root/Library/Application Support/Clover/CloverDaemon-stopservice"
    chmod 755 "${PKG_BUILD_DIR}/${choiceId}/Root/etc"/rc.*.d/*.{local,local.disabled}
    chmod 755 "${PKG_BUILD_DIR}/${choiceId}/Scripts/postinstall"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-visible="false" \
              --selected="choices['rc.scripts.on.target'].selected || choices['rc.scripts.on.all.volumes'].selected" \
              --pkg-refs="$packageRefId" "${choiceId}"
# End build rc scripts package

# build optional rc scripts package
    echo "================= Optional RC Scripts =================="
    packagesidentity="$clover_package_identity".optional.rc.scripts
    addGroupChoices --title="Optional RC Scripts" --description="Optional RC Scripts" \
                    --enabled="choices['$choiceIdRcScriptsCore'].selected"            \
                    "OptionalRCScripts"
    local scripts=($( find "${SRCROOT}/CloverV2/rcScripts/etc" -type f -name '*.disabled' -depth 2 ))
    for (( i = 0 ; i < ${#scripts[@]} ; i++ ))
    do
        local script_rel_path=etc/"${scripts[$i]##*/etc/}" # ie: etc/rc.boot.d/70.xx_yy_zz.local.disabled
        local script="${script_rel_path##*/}" # ie: 70.xx_yy_zz.local.disabled
        local choiceId=$(echo "$script" | sed -E 's/^[0-9]*[.]?//;s/\.local\.disabled//') # ie: xx_yy_zz
        local title=${choiceId//_/ } # ie: xx yy zz
        packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
        mkdir -p ${PKG_BUILD_DIR}/${choiceId}/Root
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}"                           \
                          --subst="RC_SCRIPT=$script_rel_path"                                    \
                          --subst="INSTALLER_ON_TARGET_REFID=$rcScriptsOnTargetPkgRefId"          \
                          --subst="INSTALLER_ON_ALL_VOLUMES_REFID=$rcScriptsOnAllColumesPkgRefId" \
                          --subst="INSTALLER_CHOICE=$packageRefId"                                \
                          OptRcScripts
        # Add the rc script library
        cp -f "$SCPT_LIB_DIR"/rc_scripts.lib "${PKG_BUILD_DIR}/${choiceId}"/Scripts
        fixperms  "${PKG_BUILD_DIR}/${choiceId}/Root/"
        chmod 755 "${PKG_BUILD_DIR}/${choiceId}/Scripts/postinstall"
        buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
        addChoice --group="OptionalRCScripts" --title="$title"                  \
                  --start-selected="choicePreviouslySelected('$packageRefId')"  \
                  --enabled="choices['OptionalRCScripts'].enabled"              \
                  --pkg-refs="$packageRefId" "${choiceId}"
    done
# End build optional rc scripts package
fi

# build theme packages
if [[ ${NOEXTRAS} != *"Clover Themes"* ]]; then
    echo "======================== Themes ========================"
    addGroupChoices "Themes"
    local specialThemes=('christmas' 'newyear')

    # Using themes section from Azi's/package branch.
    packagesidentity="${clover_package_identity}".themes
    local artwork="${SRCROOT}/CloverV2/themespkg/"
    local themes=($( find "${artwork}" -type d -depth 1 -not -name '.svn' ))
    local themeDestDir='/EFIROOTDIR/EFI/CLOVER/themes'
    local defaultTheme=  # $(trim $(sed -n 's/^theme *//p' "${SRCROOT}"/CloverV2/EFI/CLOVER/refit.conf))
    for (( i = 0 ; i < ${#themes[@]} ; i++ )); do
        local themeName=${themes[$i]##*/}
        [[ -n $(inArray "$themeName" ${specialThemes[@]}) ]] && continue # it is a special theme
        mkdir -p "${PKG_BUILD_DIR}/${themeName}/Root/"
        rsync -r --exclude=.svn --exclude="*~" "${themes[$i]}/" "${PKG_BUILD_DIR}/${themeName}/Root/${themeName}"
        packageRefId=$(getPackageRefId "${packagesidentity}" "${themeName}")
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${themeName}" \
                           --subst="themeName=$themeName"                \
                           --subst="INSTALLER_CHOICE=$packageRefId"      \
                           InstallTheme

        buildpackage "$packageRefId" "${themeName}" "${PKG_BUILD_DIR}/${themeName}" "${themeDestDir}"

        # local selectTheme="checkFileExists('${themeDestDir}/$themeName/icons/func_clover.png')"
        local selectTheme="choicePreviouslySelected('$packageRefId')"
        # Select the default theme
        [[ "$themeName" == "$defaultTheme" ]] && selectTheme='true'
        addChoice --group="Themes"  --start-selected="$selectTheme"  --pkg-refs="$packageRefId"  "${themeName}"
    done

    # Special themes
    packagesidentity="${clover_package_identity}".special.themes
    local artwork="${SRCROOT}/CloverV2/themespkg/"
    local themeDestDir='/EFIROOTDIR/EFI/CLOVER/themes'
    local currentMonth=$(date -j +'%-m')
    for (( i = 0 ; i < ${#specialThemes[@]} ; i++ )); do
        local themeName=${specialThemes[$i]##*/}
        # Don't add christmas and newyear themes if month < 11
        [[ $currentMonth -lt 11 ]] && [[ "$themeName" == christmas || "$themeName" == newyear ]] && continue
        mkdir -p "${PKG_BUILD_DIR}/${themeName}/Root/"
        rsync -r --exclude=.svn --exclude="*~" "$artwork/${specialThemes[$i]}/" "${PKG_BUILD_DIR}/${themeName}/Root/${themeName}"
        packageRefId=$(getPackageRefId "${packagesidentity}" "${themeName}")
        addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${themeName}" \
                           --subst="themeName=$themeName"                \
                           --subst="INSTALLER_CHOICE=$packageRefId"      \
                           InstallTheme

        buildpackage "$packageRefId" "${themeName}" "${PKG_BUILD_DIR}/${themeName}" "${themeDestDir}"
        addChoice --start-visible="false"  --start-selected="true"  --pkg-refs="$packageRefId" "${themeName}"
    done
fi

# build CloverThemeManager package
#if [[ -d "${SRCROOT}"/CloverThemeManager && ${NOEXTRAS} != *"Clover Themes"* ]]; then
#    local CTM_Dir="${SRCROOT}"/CloverThemeManager
#    local CTM_Dest='/Applications'

#    packagesidentity="${clover_package_identity}".CTM.themes
#    choiceId="CloverThemeManager"
#    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")

#    ditto --noextattr --noqtn "$CTM_Dir"  \
#     "${PKG_BUILD_DIR}/${choiceId}/Root/${CTM_Dest}"/
#    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
#                       --subst="INSTALLER_CHOICE=$packageRefId"      \
#                       CloverThemeManager
#    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    # CloverThemeManager.app can update it-self,
    # so there's no need to record the choice as 'previously selected'.
#    addChoice --group="Themes" --start-selected="false" \
#              --start-enabled="checkFileExists('/Users')" \
#              --start-visible="checkFileExists('/Users')" \
#              --pkg-refs="$packageRefId" "${choiceId}"
    # end CloverThemeManager package
# End build theme packages
#fi

local cloverUpdaterDir="${SRCROOT}"/CloverUpdater
local cloverPrefpaneDir="${SRCROOT}"/CloverPrefpane
if [[ -x "$cloverPrefpaneDir"/build/Clover.prefPane/Contents/MacOS/Clover && ${NOEXTRAS} != *"Clover Prefpane"* ]]; then
# build CloverPrefpane package
    echo "==================== Clover Prefpane ==================="
    packagesidentity="$clover_package_identity"
    choiceId="CloverPrefpane"
    packageRefId=$(getPackageRefId "${packagesidentity}" "${choiceId}")
    # ditto --noextattr --noqtn "$cloverUpdaterDir"/CloverUpdaterUtility.plist  \
    #  "${PKG_BUILD_DIR}/${choiceId}"/Root/Library/LaunchAgents/com.projectosx.Clover.Updater.plist
    ditto --noextattr --noqtn "$cloverUpdaterDir"/CloverUpdaterUtility  \
     "${PKG_BUILD_DIR}/${choiceId}/Root/Library/Application Support/Clover"/
    ditto --noextattr --noqtn "$cloverUpdaterDir"/build/CloverUpdater.app  \
     "${PKG_BUILD_DIR}/${choiceId}/Root/Library/Application Support/Clover"/CloverUpdater.app
    ditto --noextattr --noqtn "$cloverPrefpaneDir"/build/Clover.prefPane  \
     "${PKG_BUILD_DIR}/${choiceId}/Root/Library/PreferencePanes/"/Clover.prefPane
    addTemplateScripts --pkg-rootdir="${PKG_BUILD_DIR}/${choiceId}" \
                       --subst="INSTALLER_CHOICE=$packageRefId" MarkChoice
    buildpackage "$packageRefId" "${choiceId}" "${PKG_BUILD_DIR}/${choiceId}" "/"
    addChoice --start-selected="checkFileExists('/bin/launchctl') &amp;&amp; choicePreviouslySelected('$packageRefId')" \
              --start-enabled="checkFileExists('/bin/launchctl')"                                                       \
              --pkg-refs="$packageRefId" "${choiceId}"
# end CloverUpdater package
fi

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

        find "${packagePath}" \( -name '.DS_Store' -o -name '.svn' \) -print0 | xargs -0 rm -rf
        local filecount=$( find "${packagePath}/Root" | wc -l )
        if [ "${packageSize}" ]; then
            local installedsize="${packageSize}"
        else
            local installedsize=$( du -hkc "${packagePath}/Root" | tail -n1 | awk {'print $1'} )
        fi
        local header="<?xml version=\"1.0\"?>\n<pkg-info format-version=\"2\" "

        #[ "${3}" == "relocatable" ] && header+="relocatable=\"true\" "

        header+="identifier=\"${packageRefId}\" "
        header+="version=\"${CLOVER_REVISION}\" "

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
            # Copy Scripts with out compression as We are going to use pkgutil
            # ..that will do it for Us .. and with the correct compression format :-)
            cp -R "${packagePath}/Scripts" "${packagePath}/Temp/"
        fi

        header+="</pkg-info>"
        echo -e "${header}" > "${packagePath}/Temp/PackageInfo"

        # Create the Payload file (cpio format)
        (cd "${packagePath}/Root" && find . -print |                                       \
            cpio -o -z -R root:wheel --format cpio > "${packagePath}/Temp/Payload") 2>&1 | \
            grep -vE '^[0-9]+\s+blocks?$' # to remove cpio stderr messages

        # Create the package
        # (cd "${packagePath}/Temp" && xar -c -f "${packagePath}/../${packageName}.pkg" --compression none .)
        (pkgutil --flatten "${packagePath}/Temp" "${packagePath}/../${packageName}.pkg")

        # Add the package to the list of build packages
        pkgrefs[${#pkgrefs[*]}]="\t<pkg-ref id=\"${packageRefId}\" installKBytes='${installedsize}' version='${CLOVER_REVISION}.0.0.${CLOVER_TIMESTAMP}'>#${packageName}.pkg</pkg-ref>"

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
        local selected_option="${choice_selected[$idx]}"
        local exclusive_option=""

        # Create the node and standard attributes
        local choiceNode="\t<choice\n\t\tid=\"${choiceId}\"\n\t\ttitle=\"${choiceTitle}\"\n\t\tdescription=\"${choiceDescription}\""

        # Add options like start_selected, etc...
        [[ -n "${choiceOptions}" ]] && choiceNode="${choiceNode}\n\t\t${choiceOptions}"

        # Add the selected attribute if options are mutually exclusive
        if [[ -n "$group_exclusive" ]];then
            local group_items="${choice_group_items[$choiceParentGroupIndex]}"
            case $group_exclusive in
                exclusive_one_choice)
                    local exclusive_option=$(exclusive_one_choice "$choiceId" "$group_items")
                    if [[ -n "$selected_option" ]]; then
                        selected_option="($selected_option) &amp;&amp; $exclusive_option"
                    else
                        selected_option="$exclusive_option"
                    fi
                    ;;
                exclusive_zero_or_one_choice)
                    local exclusive_option=$(exclusive_zero_or_one_choice "$choiceId" "$group_items")
                    if [[ -n "$selected_option" ]]; then
                        selected_option="($selected_option) &amp;&amp; $exclusive_option"
                    else
                        selected_option="$exclusive_option"
                    fi
                    ;;
                *) echo "Error: unknown function to generate exclusive mode '$group_exclusive' for group '${choice_key[$choiceParentGroupIndex]}'" >&2
                   exit 1
                   ;;
            esac
        fi

        if [[ -n "${choice_force_selected[$idx]}" ]]; then
            if [[ -n "$selected_option" ]]; then
                selected_option="(${choice_force_selected[$idx]}) || $selected_option"
            else
                selected_option="${choice_force_selected[$idx]}"
            fi
        fi

        [[ -n "$selected_option" ]] && choiceNode="${choiceNode}\n\t\tselected=\"$selected_option\""

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
    declare -r distributionFilename="${packagename// /}_r${CLOVER_REVISION}.pkg"
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
    ditto --noextattr --noqtn "${PKGROOT}/Resources/background.tiff" "${PKG_BUILD_DIR}/${packagename}"/Resources/
    ditto --noextattr --noqtn "${SYMROOT}/Resources/${packagename}"/ "${PKG_BUILD_DIR}/${packagename}"/

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
    echo -e $COL_BLUE"  Revision:     "$COL_RESET"$CLOVER_REVISION"
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
