#!/bin/bash

CURRENT_FOLDER=$PWD

GENERATE_NAME="update_image.sh"

MKRESCUE_NAME=

MKIMAGE_NAME=

XORRISO_NAME=

echo "Check for grub-mkrescue..."
if [ -f /usr/bin/grub-mkrescue  ] ;
then
	echo "found.";
	MKRESCUE_NAME=/usr/bin/grub-mkrescue;
else
	echo "not found.";
	MKRESCUE_NAME=$(find ~/ | grep grub-mkrescue | grep -v grub-mkrescue.);
	echo "Please enter path to grub-mkrescue [$MKRESCUE_NAME]:";
	read a;
	if [ ! -z "$a" ] ; then
		if [ "$a" != "$MKRESCUE_NAME" ] ; then
			MKRESCUE_NAME=$a;
		fi
	fi
fi

echo "Check for grub-mkimage..."
if [ -f /usr/bin/grub-mkimage ] ;
then
	echo "found.";
	MKIMAGE_NAME=/usr/bin/grub-mkimage;
else
	echo "not found.";
	MKIMAGE_NAME=$(find ~/ | grep grub-mkimage | grep -v grub-mkimage.);
	echo "Please enter path to grub-mkimage [$MKIMAGE_NAME]:";
	read a;
	if [! -z "$a" ] ; then
		if [ "$a" != "$MKIMAGE_NAME" ] ; then
			MKIMAGE_NAME=$a;
		fi
	fi
fi

echo "Check for xorriso..."
if [ -f /usr/bin/xorriso ] ;
then
	echo "found.";
else
	echo "You must install libisoburn for xorriso!";
	exit 1;
fi

echo "Check your settings:"
echo "mkrescue path: ${MKRESCUE_NAME}";
echo "mkimage path: ${MKIMAGE_NAME}";

touch $GENERATE_NAME;
echo "#!/bin/bash" >> $GENERATE_NAME;
echo "MKRESCUE=${MKRESCUE_NAME}" >> $GENERATE_NAME;
echo "MKIMAGE=${MKIMAGE_NAME}" >> $GENERATE_NAME;
echo "" >> $GENERATE_NAME;
cat isotmpl.sh.tmpl >> $GENERATE_NAME;
chmod +x $GENERATE_NAME;

exit 0

