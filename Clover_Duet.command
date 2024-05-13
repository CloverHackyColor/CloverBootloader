#!/bin/bash
# Clover-LegacyDuet: by chris1111
# Thanks: CloverHackyColor
# Thanks: Acidanthera
apptitle="Clover-LegacyDuet"
version="1.0"
# Set Icon directory and file 
iconfile="/System/Library/CoreServices/Finder.app/Contents/Resources/Finder.icns"
printf '\e[8;47;83t'
PARENTDIR=$(dirname "$0")
cd "$PARENTDIR"
echo " "
echo "🚫 Do not use this program on a disk where Clover or OpenCore is already installed!"
rm -rf ./boot*
Sleep 1
cp -Rp ./Bootloaders/x64/{boot6,boot7} ./
cp -Rp ./BootSectors/{boot0af,boot1f32} ./

# Install Clover booter on physical disk.

#get BOOT File
if [ "$2" == "" ]; then
echo " "
echo "Choose (6 or 7) for the Booter
6 = -> Clover EFI 64-bits using SATA to access drives.
7 = -> Clover EFI 64-bit using Bios Block I/O to access drives. "
echo "Make a choice followed by Enter: "

while [ -z "$BOOT" ]; do
read BOOT
done

else
BOOT="$2"
fi

export ARCHS=$BOOT

if [ ! -f "boot${ARCHS}" ] || [ ! -f boot0af ] || [ ! -f boot1f32 ] || [ ! -f EFI/CLOVER/CLOVERX64.efi ]; then
  echo "Clover is not build?"
  echo "Or you probably have change directory on this package?"
  exit 1
fi

if [ "$(uname)" = "Linux" ]; then
  if [ "$EUID" -ne 0 ]
    then echo "Please run this script as root"
    exit
  fi
  if [ "$(which lsblk)" = "" ]; then
    echo "lsblk tool is missing! Try installing util-linux package"
    exit 1
  fi
  if [ "$(which fdisk)" = "" ]; then
    echo "fdisk tool is missing!"
    exit 1
  fi

  rm -rf ./origbs
  rm -rf ./newbs
  rm -rf ./boot*

  echo "Select the disk where you want to install boot files:"
  lsblk -d | tail -n+2 | cut -d" " -f1
  echo "Example: sda"
  read -r DRIVE

  DRIVE="/dev/${DRIVE}"

  if ! lsblk "$DRIVE"; then
    echo Disk "${DRIVE}" not found
    exit 1
  fi

  echo "Choose EFI partition on selected disk:"
  lsblk -f "${DRIVE}"
  echo "Example: sda1"
  read -r EFI_PART

  EFI_PART="/dev/${EFI_PART}"

  if ! lsblk -f "$EFI_PART" | grep -q -e FAT32 -e vfat; then
    echo "No FAT32 partition to install"
    exit 1
  fi

  # Write MBR
  dd if=boot0af of="$DRIVE" bs=1 count=446 conv=notrunc || exit 1

  umount "${EFI_PART}"

  dd if="${EFI_PART}" count=1 of=origbs
  cp -v boot1f32 newbs
  dd if=origbs of=newbs skip=3 seek=3 bs=1 count=87 conv=notrunc
  dd if=/dev/random of=newbs skip=496 seek=496 bs=1 count=14 conv=notrunc
  dd if=newbs of="${EFI_PART}"

  p=/tmp/$(uuidgen)/EFI
  mkdir -p "${p}" || exit 1
  mount -t vfat "${EFI_PART}" "${p}" -o rw,noatime,uid="$(id -u)",gid="$(id -g)" || exit 1

  cp -v "boot${ARCHS}" "${p}/boot" || exit 1
  echo "Install EFI -> /Volumes/EFI/EFI Wait. . ."
  cp -Rp "EFI" "${p}" || exit 1

  echo Check "${p}" boot drive to install Clover Duet

  DISK_SCHEME=$(fdisk -l "${DRIVE}" | sed -n 's/.*Disklabel type: *//p')
  if [ "$DISK_SCHEME" != "gpt" ]; then
    BOOT_FLAG=$(dd if="$DRIVE" bs=1 count=1 status=none skip=$((0x1BE)) | od -t x1 -A n | tr -d ' ')
    if [ "$BOOT_FLAG" != "80" ]; then
      fdisk "$DRIVE" <<END
p
a
1
w
END
    fi
  fi
else
  rm -rf ./origbs
  rm -rf ./newbs

  diskutil list
  echo "Disable SIP in the case of any problems with installation!!!"
  echo "Enter disk number to install to:"
  read -r N

  if ! diskutil info disk"${N}" |  grep -q "/dev/disk"; then
    echo Disk "$N" not found
    exit 1
  fi

  if ! diskutil info disk"${N}"s1 | grep -q -e FAT_32 -e EFI; then
    echo "No FAT32 partition to install"
    exit 1
  fi

  # Write MBR
  echo "Type your password to continue!"
  sudo fdisk -uy -f boot0af /dev/rdisk"${N}" || exit 1
  echo " "
  echo "Clover Duet boot$ARCHS$BO0T will be Install to ➤ EFI System Partition"
  echo " "
  diskutil umount disk"${N}"s1
  sudo dd if=/dev/rdisk"${N}"s1 count=1  of=origbs
  cp -v boot1f32 newbs
  sudo dd if=origbs of=newbs skip=3 seek=3 bs=1 count=87 conv=notrunc
  dd if=/dev/random of=newbs skip=496 seek=496 bs=1 count=14 conv=notrunc
  sudo dd if=newbs of=/dev/rdisk"${N}"s1
  #if [[ "$(sudo diskutil mount disk"${N}"s1)" == *"mounted" ]]
  if sudo diskutil mount disk"${N}"s1 | grep -q mounted; then
    cp -v "boot${ARCHS}" "$(diskutil info  disk"${N}"s1 |  sed -n 's/.*Mount Point: *//p')/boot"
  else
    p=/tmp/$(uuidgen)/EFI
    mkdir -p "${p}" || exit 1
    sudo mount_msdos /dev/disk"${N}"s1 "${p}" || exit 1
    cp -v "boot${ARCHS}" "${p}/boot" || exit 1
  fi

  if diskutil info  disk"${N}" |  grep -q FDisk_partition_scheme; then
    sudo fdisk -e /dev/rdisk"$N" <<-MAKEACTIVE
p
f 1
w
y
q
MAKEACTIVE
  fi
fi

Sleep 1
rm -rf ./origbs
rm -rf ./newbs
rm -rf ./boot*

while true
    do
        response=$(osascript -e 'tell app "System Events" to display dialog "Clover Duet is installed!
You can install a Clover EFI folder that you created.
You can use Generic Clover EFI folder from this release.\n\nCancel to not install EFI folder." buttons {"Cancel","Generic Clover EFI","Personal EFI"} default button 3 with title "'"$apptitle"' '"$version"'" with icon POSIX file "'"$iconfile"'"  ')

        action=$(echo $response | cut -d ':' -f2)

        # Exit if Canceled
        if [ ! "$action" ] ; then
          osascript -e 'display notification "Program closing" with title "'"$apptitle"'" subtitle "User cancelled"'
          echo "User cancelled Program Clover-LegacyDuet closing in 2 sec"
          Sleep 2
          exit 1
        fi

        if [ "$action" == "Generic Clover EFI" ] ; then
          mv ./EFI/CLOVER/config-sample.plist ./EFI/CLOVER/config.plist
          Sleep 1
          cp -Rp "EFI" "$(diskutil info  disk${N}s1 |  sed -n 's/.*Mount Point: *//p')"
          echo "Install EFI -> /Volumes/EFI/EFI Done!"
          mv ./EFI/CLOVER/config.plist ./EFI/CLOVER/config-sample.plist
          Sleep 2
          Open -R "$(diskutil info  disk"${N}"s1 |  sed -n 's/.*Mount Point: *//p')/boot"
          exit 1
        fi

        if [ "$action" == "Personal EFI" ] ; then
          echo " "
        #get EFI folder
        if [ "$2" == "" ]; then
        echo " "
        echo "Move your EFI folder here"
        echo "Followed by Enter: "

        while [ -z "$EFIfolder" ]; do
        read EFIfolder
        done

        else
        EFIfolder="$2"
        fi
        cp -Rp "$EFIfolder" "$(diskutil info  disk${N}s1 |  sed -n 's/.*Mount Point: *//p')"
        echo "Install EFI -> /Volumes/EFI/EFI Done!"
        Sleep 2
        Open -R "$(diskutil info  disk"${N}"s1 |  sed -n 's/.*Mount Point: *//p')/boot"
        exit 1
        fi
   done
