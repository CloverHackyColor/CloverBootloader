#!/bin/bash

# old=1755= ${SRCROOT}/package/buildpkg.sh ${SYMROOT}/package;
# new=1756=@${SRCROOT}/package/buildpkg.sh "$(SRCROOT)" "$(SYMROOT)" "$(PKG_BUILD_DIR)"

# 1=SRCROOT = $(CURDIR)
# 2=SYMROOT = $(SRCROOT)/sym
# 3=PKG_BUILD_DIR = $(SYMROOT)/package

# $3 Path to store built package

packagesidentity="org.Clover"

packagename="Clover"

pkgroot="${0%/*}"

COL_BLACK="\x1b[30;01m"
COL_RED="\x1b[31;01m"
COL_GREEN="\x1b[32;01m"
COL_YELLOW="\x1b[33;01m"
COL_MAGENTA="\x1b[35;01m"
COL_CYAN="\x1b[36;01m"
COL_WHITE="\x1b[37;01m"
COL_BLUE="\x1b[34;01m"
COL_RESET="\x1b[39;49;00m"

#version=$( grep FIRMWARE_VERSION Version.h | awk '{ print $3 }' | tr -d '\"' )
version=$( cat version )
stage=${version##*-}
revision=$( grep FIRMWARE_REVISION Version.h | awk '{ print $3 }' | tr -d '\"' )
builddate=$( grep FIRMWARE_BUILDDATE Version.h | awk '{ print $3,$4 }' | tr -d '\"' )
timestamp=$( date -j -f "%Y-%m-%d %H:%M:%S" "${builddate}" "+%s" )

# =================

develop=$(awk "NR==6{print;exit}" ${pkgroot}/../CREDITS)
credits=$(awk "NR==10{print;exit}" ${pkgroot}/../CREDITS)
pkgdev=$(awk "NR==14{print;exit}" ${pkgroot}/../CREDITS)
year=$(awk "NR==18{print;exit}" ${pkgroot}/../CREDITS)

# =================

distributioncount=0
xmlindent=0

indent[0]="\t"
indent[1]="\t\t"
indent[2]="\t\t\t"
indent[3]="\t\t\t\t"

main ()
{

# clean up the destination path

rm -R -f "${3}"
echo ""	
echo -e $COL_CYAN"	----------------------------------"$COL_RESET
echo -e $COL_CYAN"	Building $packagename Install Package"$COL_RESET
echo -e $COL_CYAN"	----------------------------------"$COL_RESET
echo ""

outline[$((outlinecount++))]="${indent[$xmlindent]}<choices-outline>"

# build Clover package
	echo "===================== Clover ==========================="
	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Clover\">"
	choices[$((choicescount++))]="<choice\n\tid=\"Clover\"\n\ttitle=\"Clover_title\"\n\tdescription=\"Clover_description\"\n>\n</choice>\n"

# build core package
	echo "===================== Core ============================="
	((xmlindent++))
	packagesidentity="org.Clover"
	mkdir -p ${3}/Core/Root/usr/local/bin
	mkdir -p ${3}/Core/Root/usr/standalone/i386/ia32
	mkdir -p ${3}/Core/Root/usr/standalone/i386/x64
	ditto --noextattr --noqtn ${3%/*}/i386/ia32/boot ${3}/Core/Root/usr/standalone/i386/ia32
	ditto --noextattr --noqtn ${3%/*}/i386/x64/boot ${3}/Core/Root/usr/standalone/i386/x64
	ditto --noextattr --noqtn ${3%/*}/i386/boot0 ${3}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${3%/*}/i386/boot0md ${3}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${3%/*}/i386/boot0hfs ${3}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${3%/*}/i386/boot1f32 ${3}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${3%/*}/i386/boot1f32alt ${3}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${3%/*}/i386/boot1h ${3}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${3%/*}/i386/boot1h2 ${3}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${3%/*}/i386/bootc ${3}/Core/Root/usr/standalone/i386
	ditto --noextattr --noqtn ${3%/*}/i386/fdisk440 ${3}/Core/Root/usr/local/bin
	local coresize=$( du -hkc "${3}/Core/Root" | tail -n1 | awk {'print $1'} )
	echo "	[BUILD] i386 "
	buildpackage "${3}/Core" "/" "" "start_visible=\"false\" start_selected=\"true\" start_enabled=\"false\"" >/dev/null 2>&1
# End build core package

# build core EFI folder package
	echo "===================== EFI folder ======================="
	((xmlindent++))
	packagesidentity="org.Clover"
	rm -rf ${3}/EFIfolder/Root/EFI
	mkdir -p ${3}/EFIfolder/Root/EFI
	mkdir -p ${3}/EFIfolder/Root/etc
	mkdir -p ${3}/EFIfolder/Scripts
	cp -f ${pkgroot}/Scripts/preinstall/preinstall ${3}/EFIfolder/Scripts/preinstall
	cp -Rf ${3%/*/*}/CloverV2/EFI/* ${3}/EFIfolder/Root/EFI
	cp -Rf ${3%/*/*}/CloverV2/etc/* ${3}/EFIfolder/Root/etc
	rm -f ${3}/EFIfolder/Root/EFI/config.plist >/dev/null 2>&1
	cp ${3}/EFIfolder/Root/EFI/BOOT/refit.conf ${3}/EFIfolder/Root/EFI/BOOT/refit.conf-default
	rm -f ${3}/EFIfolder/Root/EFI/BOOT/refit.conf
	local coresize=$( du -hkc "${3}/EFIfolder/Root" | tail -n1 | awk {'print $1'} )
	echo "	[BUILD] EFIfolder "
	buildpackage "${3}/EFIfolder" "/" "" "start_visible=\"false\" start_selected=\"true\" start_enabled=\"false\"" >/dev/null 2>&1
	
# End build EFI folder package	


	# build boot0 package 
		mkdir -p ${3}/boot0/Root
		mkdir -p ${3}/boot0/Scripts/Tools
		cp -f ${pkgroot}/Scripts/boot0/postinstall ${3}/boot0/Scripts
		ditto --arch i386 `which SetFile` ${3}/boot0/Scripts/Tools/SetFile
		echo "	[BUILD] boot0 "
		buildpackage "${3}/boot0" "/" "" "start_enabled=\"true\" start_selected=\"false\" selected=\"exclusive(choices['boot0hfs'])\"" >/dev/null 2>&1
	# End build boot0 package 
	
	# build boot0hfs package 
		mkdir -p ${3}/boot0hfs/Root
		mkdir -p ${3}/boot0hfs/Scripts/Tools
		cp -f ${pkgroot}/Scripts/boot0hfs/postinstall ${3}/boot0hfs/Scripts
		ditto --arch i386 `which SetFile` ${3}/boot0hfs/Scripts/Tools/SetFile
		echo "	[BUILD] boot0hfs "
		buildpackage "${3}/boot0hfs" "/" "" "start_enabled=\"true\" start_selected=\"true\" selected=\"exclusive(choices['boot0'])\"" >/dev/null 2>&1
	# End build boot0hfs package 

	# build boot1f32alt package 
		mkdir -p ${3}/boot1f32alt/Root
		mkdir -p ${3}/boot1f32alt/Scripts/Tools
		cp -f ${pkgroot}/Scripts/boot1f32alt/postinstall ${3}/boot1f32alt/Scripts
		ditto --arch i386 `which SetFile` ${3}/boot1f32alt/Scripts/Tools/SetFile
		echo "	[BUILD] boot1f32alt "
		buildpackage "${3}/boot1f32alt" "/" "" "start_enabled=\"true\" start_selected=\"false\" selected=\"exclusive(choices['boot1h']) &amp;&amp; exclusive(choices['boot1h2']) &amp;&amp; exclusive(choices['boot1no'])\"" >/dev/null 2>&1
	# End build boot1f32alt package 
	
	# build boot1h package 
		mkdir -p ${3}/boot1h/Root
		mkdir -p ${3}/boot1h/Scripts/Tools
		cp -f ${pkgroot}/Scripts/boot1h/postinstall ${3}/boot1h/Scripts
		ditto --arch i386 `which SetFile` ${3}/boot1h/Scripts/Tools/SetFile
		echo "	[BUILD] boot1h "
		buildpackage "${3}/boot1h" "/" "" "start_enabled=\"true\" start_selected=\"false\" selected=\"exclusive(choices['boot1f32alt']) &amp;&amp; exclusive(choices['boot1h2']) &amp;&amp; exclusive(choices['boot1no'])\"" >/dev/null 2>&1
	# End build boot1h package 
	
	# build boot1h2 package 
		mkdir -p ${3}/boot1h2/Root
		mkdir -p ${3}/boot1h2/Scripts/Tools
		cp -f ${pkgroot}/Scripts/boot1h2/postinstall ${3}/boot1h2/Scripts
		ditto --arch i386 `which SetFile` ${3}/boot1h2/Scripts/Tools/SetFile
		echo "	[BUILD] boot1h2 "
		buildpackage "${3}/boot1h2" "/" "" "start_enabled=\"true\" start_selected=\"true\" selected=\"exclusive(choices['boot1f32alt']) &amp;&amp; exclusive(choices['boot1h']) &amp;&amp; exclusive(choices['boot1no'])\"" >/dev/null 2>&1
	# End build boot1h2 package 

	# build boot1no package
		mkdir -p ${3}/boot1no/Root
		echo "	[BUILD] boot1no "
		buildpackage "${3}/boot1no" "/tmpcham" "" "start_visible=\"true\" start_selected=\"false\" selected=\"exclusive(choices['boot1f32alt']) &amp;&amp; exclusive(choices['boot1h']) &amp;&amp; exclusive(choices['boot1h2'])\"" >/dev/null 2>&1
	# End build boot1no package 

	# build boot32 package 
		mkdir -p ${3}/boot32/Root
		mkdir -p ${3}/boot32/Scripts/Tools
		cp -f ${pkgroot}/Scripts/boot32/postinstall ${3}/boot32/Scripts
		ditto --arch i386 `which SetFile` ${3}/boot32/Scripts/Tools/SetFile
		echo "	[BUILD] boot32 "
		buildpackage "${3}/boot32" "/" "" "start_enabled=\"true\" start_selected=\"false\" selected=\"exclusive(choices['boot64'])\"" >/dev/null 2>&1
	# End build boot32 package 

	# build boot64 package 
		mkdir -p ${3}/boot64/Root
		mkdir -p ${3}/boot64/Scripts/Tools
		cp -f ${pkgroot}/Scripts/boot64/postinstall ${3}/boot64/Scripts
		ditto --arch i386 `which SetFile` ${3}/boot64/Scripts/Tools/SetFile
		echo "	[BUILD] boot64 "
		buildpackage "${3}/boot64" "/" "" "start_enabled=\"true\" start_selected=\"true\" selected=\"exclusive(choices['boot32'])\"" >/dev/null 2>&1
	# End build boot64 package 

    ((xmlindent--))
    outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"
# End build Clover package

# build Themes package
	echo "===================== Themes ==========================="
	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Themes\">"
	choices[$((choicescount++))]="<choice\n\tid=\"Themes\"\n\ttitle=\"Themes_title\"\n\tdescription=\"Themes_description\"\n>\n</choice>\n"
	((xmlindent++))
	packagesidentity="org.Clover.Themes"

	rm -rf ${3}/black_green/Root/EFI
	mkdir -p ${3}/black_green/Root/EFI/BOOT/themes/black_green
	mkdir -p ${3}/black_green/Root/etc
	mkdir -p ${3}/black_green/Scripts
	cp -f ${3%/*/*}/CloverV2/themespkg/preinstall-black_green ${3}/black_green/Scripts/preinstall
	cp -Rf ${3%/*/*}/CloverV2/themespkg/black_green/* ${3}/black_green/Root/EFI/BOOT/themes/black_green
	cp -f ${3%/*/*}/CloverV2/themespkg/refit.conf-black_green ${3}/black_green/Root/EFI/BOOT/refit.conf-default
	cp -f ${3%/*/*}/CloverV2/themespkg/refit.conf-black_green ${3}/black_green/Root/EFI/BOOT/refit.conf-black_green
	echo "	[BUILD] black_green "
	buildpackage "${3}/black_green" "/" "" "start_visible=\"true\" start_selected=\"true\" selected=\"exclusive(choices['buttons']) &amp;&amp; exclusive(choices['metal'])\"" >/dev/null 2>&1

	rm -rf ${3}/buttons/Root/EFI
	mkdir -p ${3}/buttons/Root/EFI/BOOT/themes/buttons
	mkdir -p ${3}/buttons/Root/etc
	mkdir -p ${3}/buttons/Scripts
	cp -f ${3%/*/*}/CloverV2/themespkg/preinstall-buttons ${3}/buttons/Scripts/preinstall
	cp -Rf ${3%/*/*}/CloverV2/themespkg/buttons/* ${3}/buttons/Root/EFI/BOOT/themes/buttons
	cp -f ${3%/*/*}/CloverV2/themespkg/refit.conf-buttons ${3}/buttons/Root/EFI/BOOT/refit.conf-default
	cp -f ${3%/*/*}/CloverV2/themespkg/refit.conf-buttons ${3}/buttons/Root/EFI/BOOT/refit.conf-buttons
	echo "	[BUILD] buttons "
	buildpackage "${3}/buttons" "/" "" "start_visible=\"true\" start_selected=\"false\" selected=\"exclusive(choices['black_green']) &amp;&amp; exclusive(choices['metal'])\"" >/dev/null 2>&1

	rm -rf ${3}/metal/Root/EFI
	mkdir -p ${3}/metal/Root/EFI/BOOT/themes/metal
	mkdir -p ${3}/metal/Root/etc
	mkdir -p ${3}/metal/Scripts
	cp -f ${3%/*/*}/CloverV2/themespkg/preinstall-metal ${3}/metal/Scripts/preinstall
	cp -Rf ${3%/*/*}/CloverV2/themespkg/metal/* ${3}/metal/Root/EFI/BOOT/themes/metal
	cp -f ${3%/*/*}/CloverV2/themespkg/refit.conf-metal ${3}/metal/Root/EFI/BOOT/refit.conf-default
	cp -f ${3%/*/*}/CloverV2/themespkg/refit.conf-metal ${3}/metal/Root/EFI/BOOT/refit.conf-metal
	echo "	[BUILD] metal "
	buildpackage "${3}/metal" "/" "" "start_visible=\"true\" start_selected=\"false\" selected=\"exclusive(choices['black_green']) &amp;&amp; exclusive(choices['buttons'])\"" >/dev/null 2>&1

    ((xmlindent--))
    outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"

# End build Themes package

# build drivers-x32 packages 
	echo "===================== drivers32 ========================"
	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Drivers32\">"
	choices[$((choicescount++))]="<choice\n\tid=\"Drivers32\"\n\ttitle=\"Drivers32\"\n\tdescription=\"Drivers32\"\n>\n</choice>\n"
	((xmlindent++))
	packagesidentity="org.Clover.drivers32"
	drivers=($( find "${3%/*/*}/CloverV2/drivers-Off/drivers32" -type f -name '*.efi' -depth 1 ))
	for (( i = 0 ; i < ${#drivers[@]} ; i++ )) 
	do
		filename="${drivers[$i]##*/}"	
		mkdir -p "${3}/${filename%.efi}/Root/"
		ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${3}/${filename%.efi}/Root/"
		find "${3}/${filename%.efi}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
		fixperms "${3}/${filename%.efi}/Root/"
		chown 501:20 "${3}/${filename%.efi}/Root/"
		echo "	[BUILD] ${filename%.efi}"
		buildpackagedrivers32 "${3}/${filename%.efi}" "/EFI/drivers32" "" "start_selected=\"false\"" >/dev/null 2>&1
		rm -R -f "${3}/${filename%.efi}"
	done
	
	((xmlindent--))
	outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"

# End build drivers-x32 packages

# build drivers-x64 packages 
	echo "===================== drivers64 ========================"
	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"Drivers64\">"
	choices[$((choicescount++))]="<choice\n\tid=\"Drivers64\"\n\ttitle=\"Drivers64\"\n\tdescription=\"Drivers64\"\n>\n</choice>\n"
	((xmlindent++))
	packagesidentity="org.Clover.drivers64"
	drivers=($( find "${3%/*/*}/CloverV2/drivers-Off/drivers64" -type f -name '*.efi' -depth 1 ))
	for (( i = 0 ; i < ${#drivers[@]} ; i++ )) 
	do
		filename="${drivers[$i]##*/}"	
		mkdir -p "${3}/${filename%.efi}/Root/"
		ditto --noextattr --noqtn --arch i386 "${drivers[$i]}" "${3}/${filename%.efi}/Root/"
		find "${3}/${filename%.efi}" -name '.DS_Store' -exec rm -R -f {} \; 2>/dev/null
		fixperms "${3}/${filename%.efi}/Root/"
		chown 501:20 "${3}/${filename%.efi}/Root/"
		echo "	[BUILD] ${filename%.efi}"
		buildpackagedrivers64 "${3}/${filename%.efi}" "/EFI/drivers64" "" "start_selected=\"false\"" >/dev/null 2>&1
		rm -R -f "${3}/${filename%.efi}"
	done
	
	((xmlindent--))
	outline[$((outlinecount++))]="${indent[$xmlindent]}\t</line>"

# End build drivers-x64 packages

# build post install package
	echo "===================== Post ============================="
	packagesidentity="org.Clover"
	mkdir -p ${3}/post/Root
	mkdir -p ${3}/post/Scripts
	cp -f ${pkgroot}/Scripts/post/* ${3}/post/Scripts
	echo "	[BUILD] Post "
	buildpackage "${3}/Post" "/" "" "start_visible=\"false\" start_selected=\"true\"" >/dev/null 2>&1
	outline[$((outlinecount++))]="${indent[$xmlindent]}</choices-outline>"

# build meta package

	makedistribution "${3}" "${2}" "${3}" "${4}" "${5}"

# clean up 

	rm -R -f "${3}"

}

fixperms ()
{
	# $1 path
	find "${1}" -type f -exec chmod 644 {} \;
	find "${1}" -type d -exec chmod 755 {} \;
	chown -R 0:0 "${1}"
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

		echo "	[BUILD] ${optionName} "

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
#  $1 Path to package to build containing Root and or Scripts
#  $2 Install Location
#  $3 Size
#  $4 Options

if [ -d "${1}/Root" ] && [ "${1}/Scripts" ]; then

	local packagename="${1##*/}"
	local identifier=$( echo ${packagesidentity}.${packagename//_/.} | tr [:upper:] [:lower:] )
	find "${1}" -name '.DS_Store' -delete
	local filecount=$( find "${1}/Root" | wc -l )
	if [ "${3}" ]; then
		local installedsize="${3}"
	else
		local installedsize=$( du -hkc "${1}/Root" | tail -n1 | awk {'print $1'} )
	fi
	local header="<?xml version=\"1.0\"?>\n<pkg-info format-version=\"2\" "

	#[ "${3}" == "relocatable" ] && header+="relocatable=\"true\" "		

	header+="identifier=\"${identifier}\" "
	header+="version=\"${version}\" "

	[ "${2}" != "relocatable" ] && header+="install-location=\"${2}\" "

	header+="auth=\"root\">\n"
	header+="\t<payload installKBytes=\"${installedsize##* }\" numberOfFiles=\"${filecount##* }\"/>\n"
	rm -R -f "${1}/Temp"

	[ -d "${1}/Temp" ] || mkdir -m 777 "${1}/Temp"
	[ -d "${1}/Root" ] && mkbom "${1}/Root" "${1}/Temp/Bom"

	if [ -d "${1}/Scripts" ]; then 
		header+="\t<scripts>\n"
		for script in $( find "${1}/Scripts" -type f \( -name 'pre*' -or -name 'post*' \) )
		do
			header+="\t\t<${script##*/} file=\"./${script##*/}\"/>\n"
		done
		header+="\t</scripts>\n"
		chown -R 0:0 "${1}/Scripts"
		pushd "${1}/Scripts" >/dev/null
		find . -print | cpio -o -z -H cpio > "../Temp/Scripts"
		popd >/dev/null
	fi

	header+="</pkg-info>"
	echo -e "${header}" > "${1}/Temp/PackageInfo"
	pushd "${1}/Root" >/dev/null
	find . -print | cpio -o -z -H cpio > "../Temp/Payload"
	popd >/dev/null
	pushd "${1}/Temp" >/dev/null

	xar -c -f "${1%/*}/${packagename// /}.pkg" --compression none .

	popd >/dev/null

	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"${packagename// /}\"/>"

	if [ "${4}" ]; then
		local choiceoptions="${indent[$xmlindent]}${4}\n"	
	fi
	choices[$((choicescount++))]="<choice\n\tid=\"${packagename// /}\"\n\ttitle=\"${packagename}_title\"\n\tdescription=\"${packagename}_description\"\n${choiceoptions}>\n\t<pkg-ref id=\"${identifier}\" installKBytes='${installedsize}' version='${version}.0.0.${timestamp}' auth='root'>#${packagename// /}.pkg</pkg-ref>\n</choice>\n"

	rm -R -f "${1}"
fi
}

buildpackagedrivers32 ()
{
#  $1 Path to package to build containing Root and or Scripts
#  $2 Install Location
#  $3 Size
#  $4 Options

if [ -d "${1}/Root" ] && [ "${1}/Scripts" ]; then

	local packagename="${1##*/}"
	local identifier=$( echo ${packagesidentity}.${packagename//_/.} | tr [:upper:] [:lower:] )
	find "${1}" -name '.DS_Store' -delete
	local filecount=$( find "${1}/Root" | wc -l )
	if [ "${3}" ]; then
		local installedsize="${3}"
	else
		local installedsize=$( du -hkc "${1}/Root" | tail -n1 | awk {'print $1'} )
	fi
	local header="<?xml version=\"1.0\"?>\n<pkg-info format-version=\"2\" "

	#[ "${3}" == "relocatable" ] && header+="relocatable=\"true\" "		

	header+="identifier=\"${identifier}\" "
	header+="version=\"${version}\" "

	[ "${2}" != "relocatable" ] && header+="install-location=\"${2}\" "

	header+="auth=\"root\">\n"
	header+="\t<payload installKBytes=\"${installedsize##* }\" numberOfFiles=\"${filecount##* }\"/>\n"
	rm -R -f "${1}/Temp"

	[ -d "${1}/Temp" ] || mkdir -m 777 "${1}/Temp"
	[ -d "${1}/Root" ] && mkbom "${1}/Root" "${1}/Temp/Bom"

	if [ -d "${1}/Scripts" ]; then 
		header+="\t<scripts>\n"
		for script in $( find "${1}/Scripts" -type f \( -name 'pre*' -or -name 'post*' \) )
		do
			header+="\t\t<${script##*/} file=\"./${script##*/}\"/>\n"
		done
		header+="\t</scripts>\n"
		chown -R 0:0 "${1}/Scripts"
		pushd "${1}/Scripts" >/dev/null
		find . -print | cpio -o -z -H cpio > "../Temp/Scripts"
		popd >/dev/null
	fi

	header+="</pkg-info>"
	echo -e "${header}" > "${1}/Temp/PackageInfo"
	pushd "${1}/Root" >/dev/null
	find . -print | cpio -o -z -H cpio > "../Temp/Payload"
	popd >/dev/null
	pushd "${1}/Temp" >/dev/null

	xar -c -f "${1%/*}/${packagename// /}.pkg" --compression none .

	popd >/dev/null

	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"${packagename// /}\"/>"

	if [ "${4}" ]; then
		local choiceoptions="${indent[$xmlindent]}${4}\n"	
	fi
	choices[$((choicescount++))]="<choice\n\tid=\"${packagename// /}\"\n\ttitle=\"${packagename}\"\n\tdescription=\"Install to /EFI/drivers32/${packagename}.efi\"\n${choiceoptions}>\n\t<pkg-ref id=\"${identifier}\" installKBytes='${installedsize}' version='${version}.0.0.${timestamp}' auth='root'>#${packagename// /}.pkg</pkg-ref>\n</choice>\n"

	rm -R -f "${1}"
fi
}

buildpackagedrivers64 ()
{
#  $1 Path to package to build containing Root and or Scripts
#  $2 Install Location
#  $3 Size
#  $4 Options

if [ -d "${1}/Root" ] && [ "${1}/Scripts" ]; then

	local packagename="${1##*/}"
	local identifier=$( echo ${packagesidentity}.${packagename//_/.} | tr [:upper:] [:lower:] )
	find "${1}" -name '.DS_Store' -delete
	local filecount=$( find "${1}/Root" | wc -l )
	if [ "${3}" ]; then
		local installedsize="${3}"
	else
		local installedsize=$( du -hkc "${1}/Root" | tail -n1 | awk {'print $1'} )
	fi
	local header="<?xml version=\"1.0\"?>\n<pkg-info format-version=\"2\" "

	#[ "${3}" == "relocatable" ] && header+="relocatable=\"true\" "		

	header+="identifier=\"${identifier}\" "
	header+="version=\"${version}\" "

	[ "${2}" != "relocatable" ] && header+="install-location=\"${2}\" "

	header+="auth=\"root\">\n"
	header+="\t<payload installKBytes=\"${installedsize##* }\" numberOfFiles=\"${filecount##* }\"/>\n"
	rm -R -f "${1}/Temp"

	[ -d "${1}/Temp" ] || mkdir -m 777 "${1}/Temp"
	[ -d "${1}/Root" ] && mkbom "${1}/Root" "${1}/Temp/Bom"

	if [ -d "${1}/Scripts" ]; then 
		header+="\t<scripts>\n"
		for script in $( find "${1}/Scripts" -type f \( -name 'pre*' -or -name 'post*' \) )
		do
			header+="\t\t<${script##*/} file=\"./${script##*/}\"/>\n"
		done
		header+="\t</scripts>\n"
		chown -R 0:0 "${1}/Scripts"
		pushd "${1}/Scripts" >/dev/null
		find . -print | cpio -o -z -H cpio > "../Temp/Scripts"
		popd >/dev/null
	fi

	header+="</pkg-info>"
	echo -e "${header}" > "${1}/Temp/PackageInfo"
	pushd "${1}/Root" >/dev/null
	find . -print | cpio -o -z -H cpio > "../Temp/Payload"
	popd >/dev/null
	pushd "${1}/Temp" >/dev/null

	xar -c -f "${1%/*}/${packagename// /}.pkg" --compression none .

	popd >/dev/null

	outline[$((outlinecount++))]="${indent[$xmlindent]}\t<line choice=\"${packagename// /}\"/>"

	if [ "${4}" ]; then
		local choiceoptions="${indent[$xmlindent]}${4}\n"	
	fi
	choices[$((choicescount++))]="<choice\n\tid=\"${packagename// /}\"\n\ttitle=\"${packagename}\"\n\tdescription=\"Install to /EFI/drivers64/${packagename}.efi\"\n${choiceoptions}>\n\t<pkg-ref id=\"${identifier}\" installKBytes='${installedsize}' version='${version}.0.0.${timestamp}' auth='root'>#${packagename// /}.pkg</pkg-ref>\n</choice>\n"

	rm -R -f "${1}"
fi
}

makedistribution ()
{
	rm -f "${1%/*}/${packagename// /}"*.pkg

	find "${1}" -type f -name '*.pkg' -depth 1 | while read component
	do
		mkdir -p "${1}/${packagename}/${component##*/}"
		pushd "${1}/${packagename}/${component##*/}" >/dev/null
		xar -x -f "${1%}/${component##*/}"
		popd >/dev/null
	done

	ditto --noextattr --noqtn "${pkgroot}/Distribution" "${1}/${packagename}/Distribution"
	ditto --noextattr --noqtn "${pkgroot}/Resources" "${1}/${packagename}/Resources"

##	find "${1}/${packagename}/Resources" -type d -name '.svn' -exec rm -R -f {} \; 2>/dev/null

	for (( i=0; i < ${#outline[*]} ; i++));
		do
			echo -e "${outline[$i]}" >> "${1}/${packagename}/Distribution"
		done

	for (( i=0; i < ${#choices[*]} ; i++));
		do
			echo -e "${choices[$i]}" >> "${1}/${packagename}/Distribution"
		done

	echo "</installer-gui-script>"  >> "${1}/${packagename}/Distribution"

	perl -i -p -e "s/%CLOVERVERSION%/${version%%-*}/g" `find "${1}/${packagename}/Resources" -type f`
	perl -i -p -e "s/%CLOVERREVISION%/${revision}/g" `find "${1}/${packagename}/Resources" -type f`

#  Adding Developer and credits
	perl -i -p -e "s/%DEVELOP%/${develop}/g" `find "${1}/${packagename}/Resources" -type f`
	perl -i -p -e "s/%CREDITS%/${credits}/g" `find "${1}/${packagename}/Resources" -type f`
	perl -i -p -e "s/%PKGDEV%/${pkgdev}/g" `find "${1}/${packagename}/Resources" -type f`
	perl -i -p -e "s/%YEAR%/${year}/g" `find "${1}/${packagename}/Resources" -type f`

	stage=${stage/RC/Release Candidate }
	stage=${stage/FINAL/2.0 Final}
	perl -i -p -e "s/%CLOVERSTAGE%/${stage}/g" `find "${1}/${packagename}/Resources" -type f`

	find "${1}/${packagename}" -name '.DS_Store' -delete
	pushd "${1}/${packagename}" >/dev/null
	xar -c -f "${1%/*}/${packagename// /}_${version}_r${revision}_EFI_x32_x64.pkg" --compression none .
	popd >/dev/null

    SetFile -a C "${1%/*}/${packagename// /}_${version}_r${revision}_EFI_x32_x64.pkg"

#    DeRez -only icns "${pkgroot}/Icons/CloverPKG.icns" > tempicns.rsrc
#    Rez -append tempicns.rsrc -o "${1%/*}/${packagename// /}_${version}_r${revision}_EFI_x32_x64.pkg"
#    SetFile -a C "${1%/*}/${packagename// /}_${version}_r${revision}_EFI_x32_x64.pkg"
#    rm -f tempicns.rsrc
# End

	md5=$( md5 "${1%/*}/${packagename// /}_${version}_r${revision}_EFI_x32_x64.pkg" | awk {'print $4'} )
	echo "MD5 (${packagename// /}_${version}_r${revision}_EFI_x32_x64.pkg) = ${md5}" > "${1%/*}/${packagename// /}_${version}_r${revision}_EFI_x32_x64.pkg.md5"
	echo ""	

	echo -e $COL_GREEN"	--------------------------"$COL_RESET
	echo -e $COL_GREEN"	Building process complete!"$COL_RESET
	echo -e $COL_GREEN"	--------------------------"$COL_RESET
	echo ""	
	echo -e $COL_GREEN"	Build info."
	echo -e $COL_GREEN"	==========="
	echo -e $COL_BLUE"	Package name:	"$COL_RESET"${packagename// /}_${version}_r${revision}_EFI_x32_x64.pkg"
	echo -e $COL_BLUE"	MD5:		"$COL_RESET"$md5"
	echo -e $COL_BLUE"	Version:	"$COL_RESET"$version"
#	echo -e $COL_BLUE"	Stage:		"$COL_RESET"$stage"
	echo -e $COL_BLUE"	Date/Time:	"$COL_RESET"$builddate"
	echo ""
	echo "===================== Finish ==========================="
	echo ""
}

main "${1}" "${2}" "${3}" "${4}" "${5}"

