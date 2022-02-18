#!/bin/bash

# **************************************************
# CLOVER builds only
# **************************************************
# postinstall for OpenRuntime.efi
# **************************************************
# Users please do not Modify
# for Personalized scripts use "Scripts" folder
# **************************************************
cd "$(dirname $0)"

export CLOVERROOT=${CLOVERROOT:-$WORKSPACE}
export OR_DIR=${OR_DIR:-$CLOVERROOT/CloverPackage/CloverV2/EFI/CLOVER/drivers/off/UEFI/MemoryFix}
version=v12

if [[ ! -x "${CLOVERROOT}"/ebuild.sh ]]; then
  echo "can't find Clover"
fi

if [[ -d "${OR_DIR}" ]]; then
	mv -f "${OR_DIR}"/OpenRuntime.efi "${OR_DIR}"/OpenRuntime-${version}.efi
else
  echo "" && echo "ERROR: nothing to be done, driver not in place!"
  sleep 2
fi
# **************************************************