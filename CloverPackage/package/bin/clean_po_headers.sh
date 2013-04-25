#!/bin/bash

if [[ ! -d "po" ]];then
    echo "There no 'po' directory in the current directory !" >&2
    exit 1
fi

for pofile in po/*.po; do
	echo $pofile
    lang=${pofile##*/}
    lang=${lang%.*}
    gsed -i 's/; charset=CHARSET/; charset=UTF-8/g' $pofile
    gsed -i 's/^\"Project-Id-Version: PACKAGE VERSION/\"Project-Id-Version: Clover 2.0/' $pofile
    gsed -i "s/^\"Language:.*\"/\"Language: $lang\\\n\"/" $pofile
    gsed -i "s/^\"Language-Team:.*\"/\"Language-Team: $lang <$lang@li.org>\\\n\"/" $pofile
done
