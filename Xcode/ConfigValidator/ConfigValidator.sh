#!/bin/sh
#Credit: chris1111
#Credit: Jief_Machak
#Vars
apptitle="Config Validator"
version="1.0"
sleep 1
rm -rf /Private/tmp/CloverConfigPlistValidator.zip
#Dialog Text
read -r -d '' applescriptCode <<'EOF'
   set dialogText to text returned of (display dialog "
Type the numer of the CloverConfigPlistValidator you whant to use?
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The lowest you can start is ➢ 5143
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Type the number then click Continue" default answer "" with icon 0 buttons {"Continue"} default button "Continue")
   return dialogText
EOF

dialogText=$(osascript -e "$applescriptCode");
echo " "
echo Your choice is CloverConfigPlistValidator $dialogText;

echo " "
echo "
******************************************************
Clover Config.Plist Validator
Drag config.plist file to verify it
******************************************************"
#Set Droping directory and file
rm -rf /Private/tmp/CloverConfigPlistValidator.zip
sleep 1
rm -rf /Private/tmp/CloverConfigPlistValidator
curl -L https://github.com/CloverHackyColor/CloverBootloader/releases/download/$dialogText/CloverConfigPlistValidator_$dialogText.zip -s -o /Private/tmp/CloverConfigPlistValidator.zip
Sleep 1
ditto -x -k --sequesterRsrc --rsrc /Private/tmp/CloverConfigPlistValidator.zip /Private/tmp
sleep 1
rm -rf /Private/tmp/CloverConfigPlistValidator.zip
#get config path
if [ "$2" == "" ]; then
while [ -z "$config" ]; do
read config
echo "Start of verification:"
sleep 1
done
else
config="$2"
fi
/Private/tmp/CloverConfigPlistValidator_$dialogText -v "$config"
