/*
 * File: HdaController.c
 *
 * Copyright (c) 2018 John Davis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Library/HdaModels.h>
#include "HdaController.h"
#include "HdaControllerComponentName.h"

VOID
EFIAPI
HdaControllerStreamPollTimerHandler(
    IN EFI_EVENT Event,
    IN VOID *Context)
{
    // Create variables.
    EFI_STATUS Status;
    HDA_STREAM *HdaStream = (HDA_STREAM*)Context;
    EFI_PCI_IO_PROTOCOL *PciIo = HdaStream->HdaControllerDev->PciIo;
    UINT8 HdaStreamSts = 0;
    UINT32 HdaStreamDmaPos;
    UINTN HdaSourceLength;
    UINTN HdaCurrentBlock;
    UINTN HdaNextBlock;

    // Get stream status.
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthFifoUint8, PCI_HDA_BAR, HDA_REG_SDNSTS(HdaStream->Index), 1, &HdaStreamSts);
    ASSERT_EFI_ERROR(Status);

    // If there was a FIFO error or DESC error, halt.
    ASSERT ((HdaStreamSts & (HDA_REG_SDNSTS_FIFOE | HDA_REG_SDNSTS_DESE)) == 0);

    // Has the completion bit been set?
    if (HdaStreamSts & HDA_REG_SDNSTS_BCIS) {
        // Are we done playing the stream? If so we can stop now.
        if (HdaStream->BufferSourceDone) {
            // Stop stream.
            Status = HdaControllerSetStream(HdaStream, FALSE);
            ASSERT_EFI_ERROR(Status);

            // Stop timer.
            Status = gBS->SetTimer(HdaStream->PollTimer, TimerCancel, 0);
            ASSERT_EFI_ERROR(Status);

            // Trigger callback.
            if (HdaStream->Callback)
                HdaStream->Callback(HdaStream->Output ? EfiHdaIoTypeOutput : EfiHdaIoTypeInput,
                    HdaStream->CallbackContext1, HdaStream->CallbackContext2, HdaStream->CallbackContext3);
            goto CLEAR_BIT;
        }

        // Get stream DMA position.
        HdaStreamDmaPos = HdaStream->HdaControllerDev->DmaPositions[HdaStream->Index].Position;
        HdaCurrentBlock = HdaStreamDmaPos / HDA_BDL_BLOCKSIZE;
        HdaNextBlock = HdaCurrentBlock + 1;
        HdaNextBlock %= HDA_BDL_ENTRY_COUNT;

        // Have we reached the end of the source buffer? If so the stream will stop on the next block.
        if (HdaStream->BufferSourcePosition >= HdaStream->BufferSourceLength) {
            // Zero out next block.
            SetMem(HdaStream->BufferData + (HdaNextBlock * HDA_BDL_BLOCKSIZE), HDA_BDL_BLOCKSIZE, 0);

            // Set flag to stop stream on the next block.
            HdaStream->BufferSourceDone = TRUE;
 //           DEBUG((DEBUG_INFO, "Block %u of %u is the last! (current position 0x%X, buffer 0x%X)\n",
 //               HdaStreamDmaPos / HDA_BDL_BLOCKSIZE, HDA_BDL_ENTRY_COUNT, HdaStreamDmaPos, HdaStream->BufferSourcePosition));
            goto CLEAR_BIT;
        }

        // Determine number of bytes to pull from or push to source data.
        HdaSourceLength = HDA_BDL_BLOCKSIZE;
        if ((HdaStream->BufferSourcePosition + HdaSourceLength) > HdaStream->BufferSourceLength)
            HdaSourceLength = HdaStream->BufferSourceLength - HdaStream->BufferSourcePosition;

        // Is this an output stream (copy data to)?
        if (HdaStream->Output) {
            // Copy data to DMA buffer.
            if (HdaSourceLength < HDA_BDL_BLOCKSIZE)
                SetMem(HdaStream->BufferData + (HdaNextBlock * HDA_BDL_BLOCKSIZE), HDA_BDL_BLOCKSIZE, 0);
            CopyMem(HdaStream->BufferData + (HdaNextBlock * HDA_BDL_BLOCKSIZE), HdaStream->BufferSource + HdaStream->BufferSourcePosition, HdaSourceLength);
        } else { // Input stream (copy data from).
            // Copy data from DMA buffer.
            CopyMem(HdaStream->BufferSource + HdaStream->BufferSourcePosition, HdaStream->BufferData + (HdaNextBlock * HDA_BDL_BLOCKSIZE), HdaSourceLength);
        }

        // Increase source position.
        HdaStream->BufferSourcePosition += HdaSourceLength;
//        DEBUG((DEBUG_INFO, "Block %u of %u filled! (current position 0x%X, buffer 0x%X)\n",
//            HdaStreamDmaPos / HDA_BDL_BLOCKSIZE, HDA_BDL_ENTRY_COUNT, HdaStreamDmaPos, HdaStream->BufferSourcePosition));

CLEAR_BIT:
        // Reset completion bit.
        HdaStreamSts = HDA_REG_SDNSTS_BCIS;
        Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint8, PCI_HDA_BAR, HDA_REG_SDNSTS(HdaStream->Index), 1, &HdaStreamSts);
        ASSERT_EFI_ERROR(Status);
    }
}

EFI_STATUS
EFIAPI
HdaControllerInitPciHw(
    IN HDA_CONTROLLER_DEV *HdaControllerDev)
{
//    DEBUG((DEBUG_INFO, "HdaControllerInitPciHw(): start\n"));

    // Create variables.
    EFI_STATUS Status;
    EFI_PCI_IO_PROTOCOL *PciIo = HdaControllerDev->PciIo;
    UINT64 PciSupports = 0;
    UINT8 HdaTcSel;
    UINT16 HdaDevC;

    // Get original PCI I/O attributes.
    Status = PciIo->Attributes(PciIo, EfiPciIoAttributeOperationGet, 0, &HdaControllerDev->OriginalPciAttributes);
    if (EFI_ERROR(Status))
        return Status;
    HdaControllerDev->OriginalPciAttributesSaved = TRUE;

    // Get currently supported PCI I/O attributes.
    Status = PciIo->Attributes(PciIo, EfiPciIoAttributeOperationSupported, 0, &PciSupports);
    if (EFI_ERROR(Status))
        return Status;

    // Enable the PCI device.
    PciSupports &= EFI_PCI_DEVICE_ENABLE;
    Status = PciIo->Attributes(PciIo, EfiPciIoAttributeOperationEnable, PciSupports, NULL);
    if (EFI_ERROR(Status))
        return Status;

    // Get vendor and device IDs of PCI device.
    Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, PCI_VENDOR_ID_OFFSET, 1, &HdaControllerDev->VendorId);
    if (EFI_ERROR (Status))
        return Status;

//    DEBUG((DEBUG_INFO, "HdaControllerInitPciHw(): controller %4X:%4X\n",
//        GET_PCI_VENDOR_ID(HdaControllerDev->VendorId), GET_PCI_DEVICE_ID(HdaControllerDev->VendorId)));

    // Is this an Intel controller?
    if (GET_PCI_VENDOR_ID(HdaControllerDev->VendorId) == VEN_INTEL_ID) {
        // Set TC0 in TCSEL register.
        Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, PCI_HDA_TCSEL_OFFSET, 1, &HdaTcSel);
        if (EFI_ERROR (Status))
            return Status;
        HdaTcSel &= PCI_HDA_TCSEL_TC0_MASK;
        Status = PciIo->Pci.Write(PciIo, EfiPciIoWidthUint8, PCI_HDA_TCSEL_OFFSET, 1, &HdaTcSel);
        if (EFI_ERROR (Status))
            return Status;
    }

    // Get device control PCI register.
    Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCI_HDA_DEVC_OFFSET, 1, &HdaDevC);
    if (EFI_ERROR (Status))
        return Status;

    // If No Snoop is currently enabled, disable it.
    if (HdaDevC & PCI_HDA_DEVC_NOSNOOPEN) {
        HdaDevC &= ~PCI_HDA_DEVC_NOSNOOPEN;
        Status = PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCI_HDA_DEVC_OFFSET, 1, &HdaDevC);
        if (EFI_ERROR (Status))
            return Status;
    }

    // Get major/minor version.
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint8, PCI_HDA_BAR, HDA_REG_VMAJ, 1, &HdaControllerDev->MajorVersion);
    if (EFI_ERROR(Status))
        return Status;
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint8, PCI_HDA_BAR, HDA_REG_VMIN, 1, &HdaControllerDev->MinorVersion);
    if (EFI_ERROR(Status))
        return Status;

    // Validate version. If invalid abort.
//    DEBUG((DEBUG_INFO, "HdaControllerInitPciHw(): controller version %u.%u\n",
//        HdaControllerDev->MajorVersion, HdaControllerDev->MinorVersion));
    if (HdaControllerDev->MajorVersion < HDA_VERSION_MIN_MAJOR) {
        Status = EFI_UNSUPPORTED;
        return Status;
    }

    // Get capabilities.
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint16, PCI_HDA_BAR, HDA_REG_GCAP, 1, &HdaControllerDev->Capabilities);
    if (EFI_ERROR(Status))
        return Status;
//    DEBUG((DEBUG_INFO, "HdaControllerInitPciHw(): capabilities:\n  64-bit: %s  Serial Data Out Signals: %u\n",
//        HdaControllerDev->Capabilities & HDA_REG_GCAP_64OK ? L"Yes" : L"No",
//        HDA_REG_GCAP_NSDO(HdaControllerDev->Capabilities)));
//    DEBUG((DEBUG_INFO, "  Bidir streams: %u  Input streams: %u  Output streams: %u\n",
//        HDA_REG_GCAP_BSS(HdaControllerDev->Capabilities), HDA_REG_GCAP_ISS(HdaControllerDev->Capabilities),
//        HDA_REG_GCAP_OSS(HdaControllerDev->Capabilities)));

    // Success.
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
HdaControllerReset(
    IN HDA_CONTROLLER_DEV *HdaControllerDev)
{
//    DEBUG((DEBUG_INFO, "HdaControllerReset(): start\n"));

    // Create variables.
    EFI_STATUS Status;
    EFI_PCI_IO_PROTOCOL *PciIo = HdaControllerDev->PciIo;
    UINT32 HdaGCtl = 0;
    UINT64 Tmp = 0;

    // Get value of CRST bit.
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, PCI_HDA_BAR, HDA_REG_GCTL, 1, &HdaGCtl);
    if (EFI_ERROR(Status))
        return Status;

    // Check if the controller is already in reset. If not, clear bit.
    if (!(HdaGCtl & HDA_REG_GCTL_CRST)) {
        HdaGCtl &= ~HDA_REG_GCTL_CRST;
        Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, PCI_HDA_BAR, HDA_REG_GCTL, 1, &HdaGCtl);
        if (EFI_ERROR(Status))
            return Status;
    }

    // Set CRST bit to begin the process of coming out of reset.
    HdaGCtl |= HDA_REG_GCTL_CRST;
    Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, PCI_HDA_BAR, HDA_REG_GCTL, 1, &HdaGCtl);
    if (EFI_ERROR(Status))
        return Status;

    // Wait for bit to be set. Once bit is set, the controller is ready.
    Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint32, PCI_HDA_BAR, HDA_REG_GCTL,
        HDA_REG_GCTL_CRST, HDA_REG_GCTL_CRST, MS_TO_NANOSECOND(100), &Tmp);
    if (EFI_ERROR(Status))
        return Status;

    // Wait 100ms to ensure all codecs have also reset.
    gBS->Stall(MS_TO_MICROSECOND(100));

    // Controller is reset.
//    DEBUG((DEBUG_INFO, "HdaControllerReset(): done\n"));
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HdaControllerScanCodecs(
    IN HDA_CONTROLLER_DEV *HdaControllerDev)
{
//    DEBUG((DEBUG_INFO, "HdaControllerScanCodecs(): start\n"));

    // Create variables.
    EFI_STATUS Status;
    EFI_PCI_IO_PROTOCOL *PciIo;
    UINT16 HdaStatests = 0;
    EFI_HDA_IO_VERB_LIST HdaCodecVerbList;
    UINT32 VendorVerb;
    UINT32 VendorResponse;
    UINT8 i;

    // Streams.
    UINTN CurrentOutputStreamIndex = 0;
    UINTN CurrentInputStreamIndex = 0;

    // Protocols.
    HDA_IO_PRIVATE_DATA *HdaIoPrivateData;
    VOID *TmpProtocol;

    if (!HdaControllerDev) {
        return EFI_INVALID_PARAMETER;
    }

    PciIo = HdaControllerDev->PciIo;

    // Get STATESTS register.
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint16, PCI_HDA_BAR, HDA_REG_STATESTS, 1, &HdaStatests);
    if (EFI_ERROR(Status))
        return Status;

    // Create verb list with single item.
    VendorVerb = HDA_CODEC_VERB(HDA_VERB_GET_PARAMETER, HDA_PARAMETER_VENDOR_ID);
    SetMem(&HdaCodecVerbList, sizeof(EFI_HDA_IO_VERB_LIST), 0);
    HdaCodecVerbList.Count = 1;
    HdaCodecVerbList.Verbs = &VendorVerb;
    HdaCodecVerbList.Responses = &VendorResponse;

    // Iterate through register looking for active codecs.
    for (i = 0; i < HDA_MAX_CODECS; i++) {
        // Do we have a codec at this address?
        if ((HdaStatests & (1 << i))) {
//            DEBUG((DEBUG_INFO, "HdaControllerScanCodecs(): found codec @ 0x%X\n", i));

            // Try to get the vendor ID. If this fails, ignore the codec.
            VendorResponse = 0;
            Status = HdaControllerSendCommands(HdaControllerDev, i, HDA_NID_ROOT, &HdaCodecVerbList);
            if ((EFI_ERROR(Status)) || (VendorResponse == 0))
                continue;

            // Create HDA I/O protocol private data structure.
            HdaIoPrivateData = AllocateZeroPool(sizeof(HDA_IO_PRIVATE_DATA));
            if (HdaIoPrivateData == NULL) {
                Status = EFI_OUT_OF_RESOURCES;
                goto FREE_CODECS;
            }

            // Fill HDA I/O protocol private data structure.
            HdaIoPrivateData->Signature = HDA_CONTROLLER_PRIVATE_DATA_SIGNATURE;
            HdaIoPrivateData->HdaCodecAddress = i;
            HdaIoPrivateData->HdaControllerDev = HdaControllerDev;
            HdaIoPrivateData->HdaIo.GetAddress = HdaControllerHdaIoGetAddress;
            HdaIoPrivateData->HdaIo.SendCommand = HdaControllerHdaIoSendCommand;
            HdaIoPrivateData->HdaIo.SetupStream = HdaControllerHdaIoSetupStream;
            HdaIoPrivateData->HdaIo.CloseStream = HdaControllerHdaIoCloseStream;
            HdaIoPrivateData->HdaIo.GetStream = HdaControllerHdaIoGetStream;
            HdaIoPrivateData->HdaIo.StartStream = HdaControllerHdaIoStartStream;
            HdaIoPrivateData->HdaIo.StopStream = HdaControllerHdaIoStopStream;

            // Assign output stream.
            if (CurrentOutputStreamIndex < HdaControllerDev->OutputStreamsCount) {
 //               DEBUG((DEBUG_INFO, "Assigning output stream %u to codec\n", CurrentOutputStreamIndex));
                HdaIoPrivateData->HdaOutputStream = HdaControllerDev->OutputStreams + CurrentOutputStreamIndex;
                CurrentOutputStreamIndex++;
            }

            // Assign input stream.
            if (CurrentInputStreamIndex < HdaControllerDev->InputStreamsCount) {
//                DEBUG((DEBUG_INFO, "Assigning input stream %u to codec\n", CurrentInputStreamIndex));
                HdaIoPrivateData->HdaInputStream = HdaControllerDev->InputStreams + CurrentInputStreamIndex;
                CurrentInputStreamIndex++;
            }

            // Add to array.
            HdaControllerDev->HdaIoChildren[i].PrivateData = HdaIoPrivateData;
        }
    }

    // Clear STATESTS register.
    HdaStatests = HDA_REG_STATESTS_CLEAR;
    Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint16, PCI_HDA_BAR, HDA_REG_STATESTS, 1, &HdaStatests);
    if (EFI_ERROR(Status))
        return Status;

    // Install protocols on each codec.
  
    for (i = 0; i < HDA_MAX_CODECS; i++) {
        // Do we have a codec at this address?
        if (HdaControllerDev->HdaIoChildren[i].PrivateData != NULL) {
            // Create Device Path for codec.
            EFI_HDA_IO_DEVICE_PATH HdaIoDevicePathNode; //EFI_HDA_IO_DEVICE_PATH_TEMPLATE;
            HdaIoDevicePathNode.Header.Type = MESSAGING_DEVICE_PATH;
            HdaIoDevicePathNode.Header.SubType = MSG_VENDOR_DP;
            HdaIoDevicePathNode.Header.Length[0] = (UINT8)(sizeof(EFI_HDA_IO_DEVICE_PATH));
            HdaIoDevicePathNode.Header.Length[1] = (UINT8)((sizeof(EFI_HDA_IO_DEVICE_PATH)) >> 8);
          HdaIoDevicePathNode.Guid = gEfiHdaIoDevicePathGuid;
//            CopyMem((VOID*)&HdaIoDevicePathNode.Guid, (VOID*)&gEfiHdaIoDevicePathGuid, sizeof(EFI_GUID));
            HdaIoDevicePathNode.Address = i;
            HdaControllerDev->HdaIoChildren[i].DevicePath = AppendDevicePathNode(HdaControllerDev->DevicePath, (EFI_DEVICE_PATH_PROTOCOL*)&HdaIoDevicePathNode);
            if (HdaControllerDev->HdaIoChildren[i].DevicePath == NULL) {
                Status = EFI_INVALID_PARAMETER;
                goto FREE_CODECS;
            }

            // Install protocols for the codec. The codec driver will later bind to this.
            HdaControllerDev->HdaIoChildren[i].Handle = NULL;
            Status = gBS->InstallMultipleProtocolInterfaces(&HdaControllerDev->HdaIoChildren[i].Handle,
                &gEfiDevicePathProtocolGuid, HdaControllerDev->HdaIoChildren[i].DevicePath,
                &gEfiHdaIoProtocolGuid, &HdaControllerDev->HdaIoChildren[i].PrivateData->HdaIo, NULL);
            if (EFI_ERROR(Status))
                goto FREE_CODECS;

            // Connect child to parent.
            Status = gBS->OpenProtocol(HdaControllerDev->ControllerHandle, &gEfiPciIoProtocolGuid, &TmpProtocol,
                HdaControllerDev->DriverBinding->DriverBindingHandle, HdaControllerDev->HdaIoChildren[i].Handle, EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);
            if (EFI_ERROR(Status))
                goto FREE_CODECS;
        }
    }

    return EFI_SUCCESS;

FREE_CODECS:
    //DEBUG((DEBUG_INFO, "HdaControllerScanCodecs(): failed to load driver for codec @ 0x%X\n", i));

    return Status;
}

EFI_STATUS
EFIAPI
HdaControllerSendCommands(
    IN HDA_CONTROLLER_DEV *HdaDev,
    IN UINT8 CodecAddress,
    IN UINT8 Node,
    IN EFI_HDA_IO_VERB_LIST *Verbs)
{
    //DEBUG((DEBUG_INFO, "HdaControllerSendCommands(): start\n"));

    // Create variables.
    EFI_STATUS Status;
    EFI_PCI_IO_PROTOCOL *PciIo;
    UINT32 *HdaCorb;
    UINT64 *HdaRirb;
    UINT32 RemainingVerbs;
    UINT32 RemainingResponses;
    UINT16 HdaCorbReadPointer = 0;
    UINT16 HdaRirbWritePointer = 0;
    BOOLEAN ResponseReceived;
    UINT8 ResponseTimeout;
    UINT64 RirbResponse;
    UINT32 VerbCommand;
    BOOLEAN Retry = FALSE;

    // Ensure parameters are valid.
    if (!HdaDev || (CodecAddress >= HDA_MAX_CODECS) || !Verbs || (Verbs->Count < 1))
        return EFI_INVALID_PARAMETER;

    // Get pointers to CORB and RIRB.
    HdaCorb = HdaDev->CorbBuffer;
    HdaRirb = HdaDev->RirbBuffer;

    PciIo = HdaDev->PciIo;

    // Lock.
    AcquireSpinLock(&HdaDev->SpinLock);

START:
    RemainingVerbs = Verbs->Count;
    RemainingResponses = Verbs->Count;
    do {
        // Keep sending verbs until they are all sent.
        if (RemainingVerbs) {
            // Get current CORB read pointer.
            Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint16, PCI_HDA_BAR, HDA_REG_CORBRP, 1, &HdaCorbReadPointer);
            if (EFI_ERROR(Status))
                goto DONE;
            //DEBUG((DEBUG_INFO, "old RP: 0x%X\n", HdaCorbReadPointer));

            // Add verbs to CORB until all of them are added or the CORB becomes full.
            while (RemainingVerbs && ((HdaDev->CorbWritePointer + 1 % HdaDev->CorbEntryCount) != HdaCorbReadPointer)) {
                // Move write pointer and write verb to CORB.
                HdaDev->CorbWritePointer++;
                HdaDev->CorbWritePointer %= HdaDev->CorbEntryCount;
                VerbCommand = HDA_CORB_VERB(CodecAddress, Node, Verbs->Verbs[Verbs->Count - RemainingVerbs]);
                HdaCorb[HdaDev->CorbWritePointer] = VerbCommand;

                // Move to next verb.
                RemainingVerbs--;
            }

            // Set CORB write pointer.
            Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint16, PCI_HDA_BAR, HDA_REG_CORBWP, 1, &HdaDev->CorbWritePointer);
            if (EFI_ERROR(Status))
                goto DONE;
        }

        // Get responses from RIRB.
        ResponseReceived = FALSE;
        ResponseTimeout = 10;
        while (!ResponseReceived) {
            // Get current RIRB write pointer.
            Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint16, PCI_HDA_BAR, HDA_REG_RIRBWP, 1, &HdaRirbWritePointer);
            if (EFI_ERROR(Status))
                goto DONE;

            // If the read and write pointers differ, there are responses waiting.
            while (HdaDev->RirbReadPointer != HdaRirbWritePointer) {
                // Increment RIRB read pointer.
                HdaDev->RirbReadPointer++;
                HdaDev->RirbReadPointer %= HdaDev->RirbEntryCount;

                // Get response and ensure it belongs to the current codec.
                RirbResponse = HdaRirb[HdaDev->RirbReadPointer];
                if (HDA_RIRB_CAD(RirbResponse) != CodecAddress || HDA_RIRB_UNSOL(RirbResponse)) {
                    DEBUG((DEBUG_INFO, "Unknown response!\n"));
                    continue;
                }

                // Add response to list.
                Verbs->Responses[Verbs->Count - RemainingResponses] = HDA_RIRB_RESP(RirbResponse);
                RemainingResponses--;
                ResponseReceived = TRUE;
            }

            // If no response still, wait a bit.
            if (!ResponseReceived) {
                // If timeout reached, fail.
                if (!ResponseTimeout) {
      //              DEBUG((DEBUG_INFO, "Command: 0x%X\n", VerbCommand));
                    Status = EFI_TIMEOUT;
                    goto TIMEOUT;
                }

                ResponseTimeout--;
                gBS->Stall(MS_TO_MICROSECOND(5));
              if (ResponseTimeout < 5) {
                    DEBUG((DEBUG_INFO, "%u timeouts reached while waiting for response!\n", ResponseTimeout));
              }
            }
        }

    } while (RemainingVerbs || RemainingResponses);
    Status = EFI_SUCCESS;
    goto DONE;

TIMEOUT:
//    DEBUG((DEBUG_INFO, "Timeout!\n"));
    if (!Retry) {
//        DEBUG((DEBUG_INFO, "Stall detected, restarting CORB and RIRB!\n"));
        Status = HdaControllerSetCorb(HdaDev, FALSE);
        if (EFI_ERROR(Status))
            goto DONE;
        Status = HdaControllerSetRirb(HdaDev, FALSE);
        if (EFI_ERROR(Status))
            goto DONE;
        Status = HdaControllerSetCorb(HdaDev, TRUE);
        if (EFI_ERROR(Status))
            goto DONE;
        Status = HdaControllerSetRirb(HdaDev, TRUE);
        if (EFI_ERROR(Status))
            goto DONE;

        // Try again.
        Retry = TRUE;
        goto START;
    }

DONE:
    ReleaseSpinLock(&HdaDev->SpinLock);
    return Status;
}

EFI_STATUS
EFIAPI
HdaControllerInstallProtocols(
    IN HDA_CONTROLLER_DEV *HdaControllerDev)
{
//    DEBUG((DEBUG_INFO, "HdaControllerInstallProtocols(): start\n"));

    // Create variables.
    HDA_CONTROLLER_INFO_PRIVATE_DATA *HdaControllerInfoData;

    // Allocate space for info protocol data.
    HdaControllerInfoData = AllocateZeroPool(sizeof(HDA_CONTROLLER_INFO_PRIVATE_DATA));
    if (HdaControllerInfoData == NULL)
        return EFI_OUT_OF_RESOURCES;

    // Populate data.
    HdaControllerInfoData->Signature = HDA_CONTROLLER_PRIVATE_DATA_SIGNATURE;
    HdaControllerInfoData->HdaControllerDev = HdaControllerDev;
    HdaControllerInfoData->HdaControllerInfo.GetName = HdaControllerInfoGetName;

    // Install protocols.
    HdaControllerDev->HdaControllerInfoData = HdaControllerInfoData;
    return gBS->InstallMultipleProtocolInterfaces(&HdaControllerDev->ControllerHandle,
        &gEfiHdaControllerInfoProtocolGuid, &HdaControllerInfoData->HdaControllerInfo,
        &gEfiCallerIdGuid, HdaControllerDev, NULL);
}

VOID
EFIAPI
HdaControllerCleanup(
    IN HDA_CONTROLLER_DEV *HdaControllerDev)
{
//    DEBUG((DEBUG_INFO, "HdaControllerCleanup(): start\n"));

    // Create variables.
    EFI_STATUS Status;
    EFI_PCI_IO_PROTOCOL *PciIo;
    UINT32 HdaGCtl = 0;
    UINT8 i;

  // If controller device is already free, we are done.
  if (HdaControllerDev == NULL)
      return;

  PciIo = HdaControllerDev->PciIo;

    // Clean HDA Controller info protocol.
    if (HdaControllerDev->HdaControllerInfoData != NULL) {
        // Uninstall protocol.
//        DEBUG((DEBUG_INFO, "HdaControllerCleanup(): clean HDA Controller Info\n"));
        Status = gBS->UninstallProtocolInterface(HdaControllerDev->ControllerHandle,
            &gEfiHdaControllerInfoProtocolGuid, &HdaControllerDev->HdaControllerInfoData->HdaControllerInfo);
        ASSERT_EFI_ERROR(Status);

        // Free data.
        FreePool(HdaControllerDev->HdaControllerInfoData);
    }

    // Clean HDA I/O children.
    for (i = 0; i < HDA_MAX_CODECS; i++) {
        // Clean Device Path protocol.
        if (HdaControllerDev->HdaIoChildren[i].DevicePath != NULL) {
            // Uninstall protocol.
//            DEBUG((DEBUG_INFO, "HdaControllerCleanup(): clean Device Path index %u\n", i));
            Status = gBS->UninstallProtocolInterface(HdaControllerDev->HdaIoChildren[i].Handle,
                &gEfiDevicePathProtocolGuid, HdaControllerDev->HdaIoChildren[i].DevicePath);
            ASSERT_EFI_ERROR(Status);

            // Free Device Path.
            FreePool(HdaControllerDev->HdaIoChildren[i].DevicePath);
        }

        // Clean HDA I/O protocol.
        if (HdaControllerDev->HdaIoChildren[i].PrivateData != NULL) {
            // Uninstall protocol.
//            DEBUG((DEBUG_INFO, "HdaControllerCleanup(): clean HDA I/O index %u\n", i));
            Status = gBS->UninstallProtocolInterface(HdaControllerDev->HdaIoChildren[i].Handle,
                &gEfiHdaIoProtocolGuid, &HdaControllerDev->HdaIoChildren[i].PrivateData->HdaIo);
            ASSERT_EFI_ERROR(Status);

            // Free private data.
            FreePool(HdaControllerDev->HdaIoChildren[i].PrivateData);
        }
    }

    // Cleanup streams.
    HdaControllerCleanupStreams(HdaControllerDev);

    // Stop and cleanup CORB and RIRB.
    HdaControllerCleanupCorb(HdaControllerDev);
    HdaControllerCleanupRirb(HdaControllerDev);

    // Get value of CRST bit.
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, PCI_HDA_BAR, HDA_REG_GCTL, 1, &HdaGCtl);

    // Place controller into a reset state to stop it.
    if (!(EFI_ERROR(Status))) {
        HdaGCtl &= ~HDA_REG_GCTL_CRST;
        Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, PCI_HDA_BAR, HDA_REG_GCTL, 1, &HdaGCtl);
    }

    // Free controller device.
    gBS->UninstallProtocolInterface(HdaControllerDev->ControllerHandle,
        &gEfiCallerIdGuid, HdaControllerDev);
    FreePool(HdaControllerDev);
}

EFI_STATUS
EFIAPI
HdaControllerDriverBindingSupported(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE ControllerHandle,
    IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL)
{

    // Create variables.
    EFI_STATUS Status;
    EFI_PCI_IO_PROTOCOL *PciIo = 0;
    HDA_PCI_CLASSREG HdaClassReg;

    // Open PCI I/O protocol. If this fails, it's not a PCI device.
    Status = gBS->OpenProtocol(ControllerHandle, &gEfiPciIoProtocolGuid, (VOID**)&PciIo,
        This->DriverBindingHandle, ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER);
    if (EFI_ERROR(Status))
        return Status;

    // Read class code from PCI.
    Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, PCI_CLASSCODE_OFFSET,
        sizeof(HDA_PCI_CLASSREG) / sizeof(UINT8), &HdaClassReg);
    if (EFI_ERROR(Status))
        goto CLOSE_PCIIO;

    // Check class code. If not an HDA controller, we cannot support it.
  //may check also PCI_CLASS_MEDIA_AUDIO
    if ((HdaClassReg.Class != PCI_CLASS_MEDIA) || (HdaClassReg.SubClass != PCI_CLASS_MEDIA_HDA)) {
        Status = EFI_UNSUPPORTED;
        goto CLOSE_PCIIO;
    }
    Status = EFI_SUCCESS;

CLOSE_PCIIO:
    // Close PCI I/O protocol and return status.
    gBS->CloseProtocol(ControllerHandle, &gEfiPciIoProtocolGuid, This->DriverBindingHandle, ControllerHandle);
    return Status;
}

EFI_STATUS
EFIAPI
HdaControllerDriverBindingStart(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE ControllerHandle,
    IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL)
{
//    DEBUG((DEBUG_INFO, "HdaControllerDriverBindingStart(): start\n"));

    // Create variables.
    EFI_STATUS Status;
    EFI_PCI_IO_PROTOCOL *PciIo;
    EFI_DEVICE_PATH_PROTOCOL *HdaControllerDevicePath;
    HDA_CONTROLLER_DEV *HdaControllerDev;

    // Open PCI I/O protocol.
    Status = gBS->OpenProtocol(ControllerHandle, &gEfiPciIoProtocolGuid, (VOID**)&PciIo,
        This->DriverBindingHandle, ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER);
    if (EFI_ERROR (Status))
        return Status;

    // Open Device Path protocol.
    Status = gBS->OpenProtocol(ControllerHandle, &gEfiDevicePathProtocolGuid, (VOID**)&HdaControllerDevicePath,
        This->DriverBindingHandle, ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER);
    if (EFI_ERROR (Status))
        goto CLOSE_PCIIO;

    // Allocate controller device.
    HdaControllerDev = AllocateZeroPool(sizeof(HDA_CONTROLLER_DEV));
    if (HdaControllerDev == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
        goto CLOSE_PCIIO;
    }

    // Fill controller device data.
    HdaControllerDev->Signature = HDA_CONTROLLER_PRIVATE_DATA_SIGNATURE;
    HdaControllerDev->PciIo = PciIo;
    HdaControllerDev->DevicePath = HdaControllerDevicePath;
    HdaControllerDev->DriverBinding = This;
    HdaControllerDev->ControllerHandle = ControllerHandle;
    InitializeSpinLock(&HdaControllerDev->SpinLock);

    // Setup PCI hardware.
    do {
      Status = HdaControllerInitPciHw(HdaControllerDev);
      if (EFI_ERROR(Status))
        break;

      // Get controller name.
      HdaControllerGetName(HdaControllerDev->VendorId, &HdaControllerDev->Name);

      // Reset controller.
      Status = HdaControllerReset(HdaControllerDev);
      if (EFI_ERROR(Status))
        break;

      // Install info protocol.
      Status = HdaControllerInstallProtocols(HdaControllerDev);
      if (EFI_ERROR(Status))
        break;

      // Initialize CORB and RIRB.
      Status = HdaControllerInitCorb(HdaControllerDev);
      if (EFI_ERROR(Status))
        break;
      Status = HdaControllerInitRirb(HdaControllerDev);
      if (EFI_ERROR(Status))
        break;

      // needed for QEMU.
#ifdef QEMU
      UINT16 dd = 0xFF;
      PciIo->Mem.Write(PciIo, EfiPciIoWidthUint16, PCI_HDA_BAR, HDA_REG_RINTCNT, 1, &dd);
#endif

      // Start CORB and RIRB
      Status = HdaControllerSetCorb(HdaControllerDev, TRUE);
      if (EFI_ERROR(Status))
        break;
      Status = HdaControllerSetRirb(HdaControllerDev, TRUE);
      if (EFI_ERROR(Status))
        break;

      // Init streams.
      Status = HdaControllerInitStreams(HdaControllerDev);
      if (EFI_ERROR(Status))
        break;

      // Scan for codecs.
      Status = HdaControllerScanCodecs(HdaControllerDev);
//      ASSERT_EFI_ERROR(Status);

      //    DEBUG((DEBUG_INFO, "HdaControllerDriverBindingStart(): done\n"));
      return Status;
    } while (FALSE);

//FREE_CONTROLLER:
    // Restore PCI attributes if needed.
    if (HdaControllerDev->OriginalPciAttributesSaved)
        PciIo->Attributes(PciIo, EfiPciIoAttributeOperationSet, HdaControllerDev->OriginalPciAttributes, NULL);

    // Free controller device.
    HdaControllerCleanup(HdaControllerDev);

CLOSE_PCIIO:
    // Close protocols.
    gBS->CloseProtocol(ControllerHandle, &gEfiDevicePathProtocolGuid, This->DriverBindingHandle, ControllerHandle);
    gBS->CloseProtocol(ControllerHandle, &gEfiPciIoProtocolGuid, This->DriverBindingHandle, ControllerHandle);
    return Status;
}

EFI_STATUS
EFIAPI
HdaControllerDriverBindingStop(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE ControllerHandle,
    IN UINTN NumberOfChildren,
    IN EFI_HANDLE *ChildHandleBuffer OPTIONAL)
{
//    DEBUG((DEBUG_INFO, "HdaControllerDriverBindingStop(): start\n"));
    // Create variables.
    EFI_STATUS Status;
    HDA_CONTROLLER_DEV *HdaControllerDev = NULL;

    // Get codec device.
    Status = gBS->OpenProtocol(ControllerHandle, &gEfiCallerIdGuid, (VOID**)&HdaControllerDev,
        This->DriverBindingHandle, ControllerHandle, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (!(EFI_ERROR(Status))) {
        // Ensure controller device is valid.
        if (HdaControllerDev->Signature != HDA_CONTROLLER_PRIVATE_DATA_SIGNATURE)
            return EFI_INVALID_PARAMETER;

        // Restore PCI attributes if needed.
        if (HdaControllerDev->OriginalPciAttributesSaved)
            HdaControllerDev->PciIo->Attributes(HdaControllerDev->PciIo, EfiPciIoAttributeOperationSet,
                HdaControllerDev->OriginalPciAttributes, NULL);

        // Cleanup controller.
        HdaControllerCleanup(HdaControllerDev);
    }

    // Close protocols.
    gBS->CloseProtocol(ControllerHandle, &gEfiDevicePathProtocolGuid, This->DriverBindingHandle, ControllerHandle);
    gBS->CloseProtocol(ControllerHandle, &gEfiPciIoProtocolGuid, This->DriverBindingHandle, ControllerHandle);
    return EFI_SUCCESS;
}
