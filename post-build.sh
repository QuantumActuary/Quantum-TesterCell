#!/bin/bash
DIR=$1
FILE=$2
DEST="/Applications/Quantum.app/Contents/Resources/.kivy/extensions/plugins/"
cp ${DIR}/${FILE} ${DEST}
install_name_tool -id ./Contents/Resources/.kivy/extensions/plugins/${FILE} ${DEST}${FILE}
#install_name_tool -change /Library/Frameworks/Python.framework/Versions/3.5/Python @executable_path/../Frameworks/python/3.5.0/Python ${DEST}${FILE}
#install_name_tool -change /usr/lib/libc++.1.dylib @executable_path/lib/libc++.1.dylib ${DEST}${FILE}