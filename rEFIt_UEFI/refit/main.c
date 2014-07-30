/*
 * refit/main.c
 * Main code for the boot menu
 *
 * Copyright (c) 2006-2010 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "entry_scan.h"
//#include "Platform.h"
//#include "../include/Handle.h"

#include "Version.h"

#ifndef DEBUG_ALL
#define DEBUG_MAIN 1
#else
#define DEBUG_MAIN DEBUG_ALL
#endif

#if DEBUG_MAIN == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_MAIN, __VA_ARGS__)
#endif

#ifndef HIBERNATE
#define HIBERNATE 0
#endif

// variables

//static CHAR8 FirmwareRevisionStr[] = FIRMWARE_REVISION_STR;

BOOLEAN                 gGuiIsReady = FALSE;
BOOLEAN                 gThemeNeedInit = TRUE;
BOOLEAN                 DoHibernateWake = FALSE;
EFI_HANDLE              gImageHandle;
EFI_SYSTEM_TABLE*       gST;
EFI_BOOT_SERVICES*		  gBS; 
EFI_RUNTIME_SERVICES*	  gRS;
EFI_DXE_SERVICES*       gDS;

static REFIT_MENU_ENTRY MenuEntryOptions  = { L"Options", TAG_OPTIONS, 1, 0, 'O', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone, NULL };
static REFIT_MENU_ENTRY MenuEntryAbout    = { L"About Clover", TAG_ABOUT, 1, 0, 'A', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone,  NULL };
static REFIT_MENU_ENTRY MenuEntryReset    = { L"Restart Computer", TAG_RESET, 1, 0, 'R', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionSelect, ActionEnter, ActionNone, ActionNone,  NULL };
static REFIT_MENU_ENTRY MenuEntryShutdown = { L"Exit Clover", TAG_SHUTDOWN, 1, 0, 'U', NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionSelect, ActionEnter, ActionNone, ActionNone,  NULL };
REFIT_MENU_ENTRY MenuEntryReturn   = { L"Return", TAG_RETURN, 0, 0, 0, NULL, NULL, NULL,
  {0, 0, 0, 0}, ActionEnter, ActionEnter, ActionNone, ActionNone,  NULL };

REFIT_MENU_SCREEN        MainMenu    = {1, L"Main Menu", NULL, 0, NULL, 0, NULL, 0, L"Automatic boot", NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL};
static REFIT_MENU_SCREEN AboutMenu   = {2, L"About",     NULL, 0, NULL, 0, NULL, 0, NULL,              NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL};
static REFIT_MENU_SCREEN HelpMenu    = {3, L"Help",      NULL, 0, NULL, 0, NULL, 0, NULL,              NULL, FALSE, FALSE, 0, 0, 0, 0, {0, 0, 0, 0}, NULL};

DRIVERS_FLAGS gDriversFlags = {FALSE, FALSE, FALSE, FALSE, FALSE};  //MemFixLoaded

EMU_VARIABLE_CONTROL_PROTOCOL *gEmuVariableControl = NULL;

static VOID AboutRefit(VOID)
{
  //  CHAR8* Revision = NULL;
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    AboutMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_ABOUT);
  } else {
    AboutMenu.TitleImage = NULL;
  }
  if (AboutMenu.EntryCount == 0) {
    AddMenuInfoLine(&AboutMenu, L"Clover Version 2k"); // by Slice, dmazar, apianti, JrCs, pene and others");
#ifdef FIRMWARE_BUILDDATE
    AddMenuInfoLine(&AboutMenu, PoolPrint(L" Build: %a", FIRMWARE_BUILDDATE));
#else
    AddMenuInfoLine(&AboutMenu, L" Build: unknown");
#endif
    AddMenuInfoLine(&AboutMenu, L"");
    AddMenuInfoLine(&AboutMenu, L"Based on rEFIt (c) 2006-2010 Christoph Pfisterer");
    AddMenuInfoLine(&AboutMenu, L"Portions Copyright (c) Intel Corporation");
    AddMenuInfoLine(&AboutMenu, L"Developers:");
    AddMenuInfoLine(&AboutMenu, L"  Slice, dmazar, apianti, JrCs, pene, usrsse2, SoThOr");
    AddMenuInfoLine(&AboutMenu, L"Credits also:");
    AddMenuInfoLine(&AboutMenu, L"  Kabyl, pcj, jadran, Blackosx, STLVNUB, ycr.ru");
    AddMenuInfoLine(&AboutMenu, L"  FrodoKenny, skoczy, crazybirdy, Oscar09, xsmile");
    AddMenuInfoLine(&AboutMenu, L"  cparm, rehabman");
    AddMenuInfoLine(&AboutMenu, L"  projectosx.com, applelife.ru");
    AddMenuInfoLine(&AboutMenu, L"");
    AddMenuInfoLine(&AboutMenu, L"Running on:");
    AddMenuInfoLine(&AboutMenu, PoolPrint(L" EFI Revision %d.%02d",
                                          gST->Hdr.Revision >> 16, gST->Hdr.Revision & ((1 << 16) - 1)));
#if defined(MDE_CPU_IA32)
    AddMenuInfoLine(&AboutMenu, L" Platform: i386 (32 bit)");
#elif defined(MDE_CPU_X64)
    AddMenuInfoLine(&AboutMenu, L" Platform: x86_64 (64 bit)");
#else
    AddMenuInfoLine(&AboutMenu, L" Platform: unknown");
#endif
#ifdef FIRMWARE_REVISION
    AddMenuInfoLine(&AboutMenu, PoolPrint(L" Firmware: %s rev %s", gST->FirmwareVendor, FIRMWARE_REVISION));
#else
    AddMenuInfoLine(&AboutMenu, PoolPrint(L" Firmware: %s rev %d", gST->FirmwareVendor, gST->FirmwareRevision));
#endif
    AddMenuInfoLine(&AboutMenu, PoolPrint(L" Screen Output: %s", egScreenDescription()));
    AboutMenu.AnimeRun = GetAnime(&AboutMenu);
    AddMenuEntry(&AboutMenu, &MenuEntryReturn);
  }  else {
    FreePool(AboutMenu.InfoLines[AboutMenu.InfoLineCount-1]);
    AboutMenu.InfoLines[AboutMenu.InfoLineCount-1]=PoolPrint(L" Screen Output: %s", egScreenDescription());
  }

  RunMenu(&AboutMenu, NULL);
}

static VOID HelpRefit(VOID)
{
  if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_MENU_TITLE_IMAGE)) {
    HelpMenu.TitleImage = BuiltinIcon(BUILTIN_ICON_FUNC_HELP);
  } else {
    HelpMenu.TitleImage = NULL;
  }
  if (HelpMenu.EntryCount == 0) {
    switch (gLanguage)
    {
      case russian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Выход из подменю, обновление главного меню");
        AddMenuInfoLine(&HelpMenu, L"F1  - Помощь по горячим клавишам");
        AddMenuInfoLine(&HelpMenu, L"F2  - Сохранить отчет в preboot.log (только если FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Показать скрытые значки в меню");
        AddMenuInfoLine(&HelpMenu, L"F4  - Родной DSDT сохранить в EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Патченный DSDT сохранить в EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Сохранить ВидеоБиос в EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Снимок экрана в папку EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Извлечь указанный DVD");
        AddMenuInfoLine(&HelpMenu, L"Пробел - Подробнее о выбранном пункте");
        AddMenuInfoLine(&HelpMenu, L"Цифры 1-9 - Быстрый запуск тома по порядку в меню");
        AddMenuInfoLine(&HelpMenu, L"A - О загрузчике");
        AddMenuInfoLine(&HelpMenu, L"O - Дополнительные настройки");
        AddMenuInfoLine(&HelpMenu, L"R - Теплый перезапуск");
        AddMenuInfoLine(&HelpMenu, L"U - Завершить работу в Кловере");
        break;
      case ukrainian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Вийти з меню, оновити головне меню");
        AddMenuInfoLine(&HelpMenu, L"F1  - Ця довідка");
        AddMenuInfoLine(&HelpMenu, L"F2  - Зберегти preboot.log (тiльки FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Відображати приховані розділи");
        AddMenuInfoLine(&HelpMenu, L"F4  - Зберегти OEM DSDT в EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Зберегти патчений DSDT в EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Зберегти VideoBios в EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Зберегти знімок екрану в EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Відкрити обраний диск (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Пробіл - докладніше про обраний пункт меню");
        AddMenuInfoLine(&HelpMenu, L"Клавіші 1-9 -  клавіші пунктів меню");
        AddMenuInfoLine(&HelpMenu, L"A - Про систему");
        AddMenuInfoLine(&HelpMenu, L"O - Опції меню");
        AddMenuInfoLine(&HelpMenu, L"R - Перезавантаження");
        AddMenuInfoLine(&HelpMenu, L"U - Відключити ПК");
        break;
      case spanish:
        AddMenuInfoLine(&HelpMenu, L"ESC - Salir de submenu o actualizar el menu principal");
        AddMenuInfoLine(&HelpMenu, L"F1  - Esta Ayuda");
        AddMenuInfoLine(&HelpMenu, L"F2  - Guardar preboot.log (Solo FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Guardar DSDT oem en EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Guardar DSDT parcheado en EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Guardar VideoBios en EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Guardar Captura de pantalla en EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Expulsar volumen seleccionado (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Espacio - Detalles acerca selected menu entry");
        AddMenuInfoLine(&HelpMenu, L"Digitos 1-9 - Atajo a la entrada del menu");
        AddMenuInfoLine(&HelpMenu, L"A - Menu Acerca de");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Optiones");
        AddMenuInfoLine(&HelpMenu, L"R - Reiniciar Equipo");
        AddMenuInfoLine(&HelpMenu, L"U - Apagar");
        break;
      case portuguese:
      case brasil:
        AddMenuInfoLine(&HelpMenu, L"ESC - Sai do submenu, atualiza o menu principal");
        AddMenuInfoLine(&HelpMenu, L"F1  - Esta ajuda");
        AddMenuInfoLine(&HelpMenu, L"F2  - Salva preboot.log (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Salva oem DSDT em EFI/CLOVER/ACPI/origin/ (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Salva DSDT corrigido em EFI/CLOVER/ACPI/origin/ (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Salva VideoBios em EFI/CLOVER/misc/ (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Salva screenshot em EFI/CLOVER/misc/ (somente FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Ejeta o volume selecionado (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Espaco - Detalhes sobre a opcao do menu selecionada");
        AddMenuInfoLine(&HelpMenu, L"Tecle 1-9 - Atalho para as entradas do menu");
        AddMenuInfoLine(&HelpMenu, L"A - Sobre o Menu");
        AddMenuInfoLine(&HelpMenu, L"O - Opcoes do Menu");
        AddMenuInfoLine(&HelpMenu, L"R - Reiniciar");
        AddMenuInfoLine(&HelpMenu, L"U - Desligar");
        break;
      case italian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Esci dal submenu, Aggiorna menu principale");
        AddMenuInfoLine(&HelpMenu, L"F1  - Aiuto");
        AddMenuInfoLine(&HelpMenu, L"F2  - Salva il preboot.log (solo su FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Salva il DSDT oem in EFI/CLOVER/ACPI/origin/ (solo suFAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Salva il patched DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Salva il VideoBios in EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Salva screenshot in EFI/CLOVER/misc/ (solo su FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Espelli il volume selezionato (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spazio - Dettagli sul menu selezionato");
        AddMenuInfoLine(&HelpMenu, L"Digita 1-9 - Abbreviazioni per il menu");
        AddMenuInfoLine(&HelpMenu, L"A - Informazioni");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Opzioni");
        AddMenuInfoLine(&HelpMenu, L"R - Riavvio");
        AddMenuInfoLine(&HelpMenu, L"U - Spegnimento");
        break;
      case german:
        AddMenuInfoLine(&HelpMenu, L"ESC - Zurueck aus Untermenue, Hauptmenue erneuern");
        AddMenuInfoLine(&HelpMenu, L"F1  - Diese Hilfe");
        AddMenuInfoLine(&HelpMenu, L"F2  - Sichere preboot.log (nur mit FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Sichere OEM DSDT in EFI/CLOVER/ACPI/origin/ (nur mit FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Sichere gepatchtes DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Sichere VideoBios in EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Sichere Bildschirmfoto in EFI/CLOVER/misc/ (nur mit FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Volume auswerfen (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Leertaste - Details über den gewählten Menue Eintrag");
        AddMenuInfoLine(&HelpMenu, L"Zahlen 1-9 - Kurzwahl zum Menue Eintrag");
        AddMenuInfoLine(&HelpMenu, L"A - Menue Informationen");
        AddMenuInfoLine(&HelpMenu, L"O - Menue Optionen");
        AddMenuInfoLine(&HelpMenu, L"R - Neustart");
        AddMenuInfoLine(&HelpMenu, L"U - Ausschalten");
        break;		
      case dutch:
        AddMenuInfoLine(&HelpMenu, L"ESC - Verlaat submenu, Vernieuwen hoofdmenu");
        AddMenuInfoLine(&HelpMenu, L"F1  - Onderdeel hulp");
        AddMenuInfoLine(&HelpMenu, L"F2  - preboot.log opslaan (FAT32 only)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Opslaan oem DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Opslaan gepatchte DSDT in EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Opslaan VideoBios in EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Opslaan schermafdruk in EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Uitwerpen geselecteerd volume (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spatie - Details over geselecteerd menuoptie");
        AddMenuInfoLine(&HelpMenu, L"Cijfers 1-9 - Snelkoppeling naar menuoptie");
        AddMenuInfoLine(&HelpMenu, L"A - Menu Over");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Opties");
        AddMenuInfoLine(&HelpMenu, L"R - Soft Reset");
        AddMenuInfoLine(&HelpMenu, L"U - Verlaten");
        break;
     case french:
        AddMenuInfoLine(&HelpMenu, L"ESC - Quitter sous-menu, Retour menu principal");
        AddMenuInfoLine(&HelpMenu, L"F1  - Aide");
        AddMenuInfoLine(&HelpMenu, L"F2  - Enregistrer preboot.log (FAT32 only)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Enregistrer oem DSDT dans EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Enregistrer DSDT modifié dans EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Enregistrer VideoBios dans EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Enregistrer la capture d'écran dans EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Ejecter le volume (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - Détails a propos du menu selectionné");
        AddMenuInfoLine(&HelpMenu, L"Digits 1-9 - Raccourci vers entrée menu");
        AddMenuInfoLine(&HelpMenu, L"A - A propos");
        AddMenuInfoLine(&HelpMenu, L"O - Options Menu");
        AddMenuInfoLine(&HelpMenu, L"R - Redémarrer");
        AddMenuInfoLine(&HelpMenu, L"U - Eteindre");
        break;
      case indonesian:
        AddMenuInfoLine(&HelpMenu, L"ESC - Keluar submenu, Refresh main menu");
        AddMenuInfoLine(&HelpMenu, L"F1  - Help");
        AddMenuInfoLine(&HelpMenu, L"F2  - Simpan preboot.log (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Simpan oem DSDT ke EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Simpan patched DSDT ke EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Simpan VideoBios ke EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Simpan screenshot ke EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Eject volume (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spasi - Detail dari menu yang dipilih");
        AddMenuInfoLine(&HelpMenu, L"Tombol 1-9 - Shortcut pilihan menu");
        AddMenuInfoLine(&HelpMenu, L"A - About");
        AddMenuInfoLine(&HelpMenu, L"O - Opsi");
        AddMenuInfoLine(&HelpMenu, L"R - Soft Reset");
        AddMenuInfoLine(&HelpMenu, L"U - Shutdown");
        break;
      case polish:
/*        AddMenuInfoLine(&HelpMenu, L"ESC - Wyjście z podmenu, Odświeżenie głównego menu");
        AddMenuInfoLine(&HelpMenu, L"F1  - Pomoc");
        AddMenuInfoLine(&HelpMenu, L"F2  - Zapis preboot.log (tylko dla FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F4  - Zapis czystych tabel DSDT do EFI/CLOVER/ACPI/origin/ (tylko dla FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Zapis zmodyfikowanego DSDT do EFI/CLOVER/ACPI/origin/ (tylko dla FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Zapis BIOSu karty graficznej do EFI/CLOVER/misc/ (tylko dla FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Zapis zrzutu ekranu do EFI/CLOVER/misc/ (tylko dla FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Wysunięcie zaznaczonego dysku (tylko dla DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spacja - Informacje na temat dostępnych opcji dla zaznaczonego dysku");
        AddMenuInfoLine(&HelpMenu, L"Znaki 1-9 - Skróty do opcji dla zaznaczonego dysku");
        AddMenuInfoLine(&HelpMenu, L"A - Menu Informacyjne");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Opcje");
        AddMenuInfoLine(&HelpMenu, L"R - Restart komputera");
        AddMenuInfoLine(&HelpMenu, L"U - Wyłączenie komputera");*/
        
        AddMenuInfoLine(&HelpMenu, L"ESC - Wyjscie z podmenu, Odswiezenie glownego menu");
        AddMenuInfoLine(&HelpMenu, L"F1  - Pomoc");
        AddMenuInfoLine(&HelpMenu, L"F2  - Zapis preboot.log (tylko FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Zapis DSDT do EFI/CLOVER/ACPI/origin/ (tylko FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Zapis poprawionego DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Zapis BIOSu k. graficznej do EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Zapis zrzutu ekranu do EFI/CLOVER/misc/ (tylko FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Wysuniecie zaznaczonego dysku (tylko dla DVD)");
        AddMenuInfoLine(&HelpMenu, L"Spacja - Informacje nt. dostepnych opcji dla zaznaczonego dysku");
        AddMenuInfoLine(&HelpMenu, L"Znaki 1-9 - Skroty opcji dla wybranego dysku");
        AddMenuInfoLine(&HelpMenu, L"A - Menu Informacyjne");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Opcje");
        AddMenuInfoLine(&HelpMenu, L"R - Restart komputera");
        AddMenuInfoLine(&HelpMenu, L"U - Wylaczenie komputera");
        break;
      case croatian:
        AddMenuInfoLine(&HelpMenu, L"ESC - izlaz iz podizbornika, Osvježi glavni izbornik");
        AddMenuInfoLine(&HelpMenu, L"F1  - Ovaj izbornik");
        AddMenuInfoLine(&HelpMenu, L"F2  - Spremi preboot.log (samo na FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Spremi oem DSDT u EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Spremi patched DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Spremi VideoBios into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Spremi screenshot into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Izbaci izabrai (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - Detalji o odabranom sistemu");
        AddMenuInfoLine(&HelpMenu, L"Brojevi 1 do 9 su prečac do izbora");
        AddMenuInfoLine(&HelpMenu, L"A - Izbornik o meni");
        AddMenuInfoLine(&HelpMenu, L"O - Izbornik opcije");
        AddMenuInfoLine(&HelpMenu, L"R - Restart računala");
        AddMenuInfoLine(&HelpMenu, L"U - Isključivanje računala");
        break;
      case czech:
        AddMenuInfoLine(&HelpMenu, L"ESC - Vrátit se do hlavní nabídky");
        AddMenuInfoLine(&HelpMenu, L"F1  - Tato Nápověda");
        AddMenuInfoLine(&HelpMenu, L"F2  - Uložit preboot.log (FAT32 only)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Uložit oem DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Uložit patchnuté DSDT do EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Uložit VideoBios do EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Uložit snímek obrazovky do EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Vysunout vybranou mechaniku (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Mezerník - Podrobnosti o vybraném disku");
        AddMenuInfoLine(&HelpMenu, L"čísla 1-9 - Klávesové zkratky pro disky");
        AddMenuInfoLine(&HelpMenu, L"A - Menu O Programu");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Možnosti");
        AddMenuInfoLine(&HelpMenu, L"R - Částečný restart");
        AddMenuInfoLine(&HelpMenu, L"U - Odejít");
        break;
      case korean:
        AddMenuInfoLine(&HelpMenu, L"ESC - 하위메뉴에서 나감, 메인메뉴 새로 고침");
        AddMenuInfoLine(&HelpMenu, L"F1  - 이 도움말");
        AddMenuInfoLine(&HelpMenu, L"F2  - preboot.log를 저장합니다. (FAT32방식에만 해당됨)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - oem DSDT를 EFI/CLOVER/ACPI/origin/에 저장합니다. (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - 패치된 DSDT를 EFI/CLOVER/ACPI/origin/에 저장합니다. (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - VideoBios를 EFI/CLOVER/misc/에 저장합니다. (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - 스크린샷을 EFI/CLOVER/misc/에 저장합니다. (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - 선택한 볼륨을 제거합니다. (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - 선택한 메뉴의 상세 설명");
        AddMenuInfoLine(&HelpMenu, L"Digits 1-9 - 메뉴 단축 번호");
        AddMenuInfoLine(&HelpMenu, L"A - 단축키 - 이 부트로더에 관하여");
        AddMenuInfoLine(&HelpMenu, L"O - 단축키 - 부트 옵션");
        AddMenuInfoLine(&HelpMenu, L"R - 단축키 - 리셋");
        AddMenuInfoLine(&HelpMenu, L"U - 단축키 - 시스템 종료");
        break;
      case english:
      default:
        AddMenuInfoLine(&HelpMenu, L"ESC - Escape from submenu, Refresh main menu");
        AddMenuInfoLine(&HelpMenu, L"F1  - This help");
        AddMenuInfoLine(&HelpMenu, L"F2  - Save preboot.log (FAT32 only)");
        AddMenuInfoLine(&HelpMenu, L"F3  - Show hidden entries");
        AddMenuInfoLine(&HelpMenu, L"F4  - Save oem DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F5  - Save patched DSDT into EFI/CLOVER/ACPI/origin/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F6  - Save VideoBios into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F10 - Save screenshot into EFI/CLOVER/misc/ (FAT32)");
        AddMenuInfoLine(&HelpMenu, L"F12 - Eject selected volume (DVD)");
        AddMenuInfoLine(&HelpMenu, L"Space - Details about selected menu entry");
        AddMenuInfoLine(&HelpMenu, L"Digits 1-9 - Shortcut to menu entry");
        AddMenuInfoLine(&HelpMenu, L"A - Menu About");
        AddMenuInfoLine(&HelpMenu, L"O - Menu Options");
        AddMenuInfoLine(&HelpMenu, L"R - Soft Reset");
        AddMenuInfoLine(&HelpMenu, L"U - Exit");
        break;
    }
    HelpMenu.AnimeRun = GetAnime(&HelpMenu);
    AddMenuEntry(&HelpMenu, &MenuEntryReturn);
  }
  
  RunMenu(&HelpMenu, NULL);
}


static EFI_STATUS LoadEFIImageList(IN EFI_DEVICE_PATH **DevicePaths,
                                    IN CHAR16 *ImageTitle,
                                    OUT UINTN *ErrorInStep,
                                    OUT EFI_HANDLE *NewImageHandle)
{
  EFI_STATUS              Status, ReturnStatus;
  EFI_HANDLE              ChildImageHandle = 0;
  UINTN                   DevicePathIndex;
  CHAR16                  ErrorInfo[256];
  
  DBG("Loading %s\n", ImageTitle);
  if (ErrorInStep != NULL) {
    *ErrorInStep = 0;
  }
  if (NewImageHandle != NULL) {
    *NewImageHandle = NULL;
  }
  
  // load the image into memory
  ReturnStatus = Status = EFI_NOT_FOUND;  // in case the list is empty
  for (DevicePathIndex = 0; DevicePaths[DevicePathIndex] != NULL; DevicePathIndex++) {
    ReturnStatus = Status = gBS->LoadImage(FALSE, SelfImageHandle, DevicePaths[DevicePathIndex], NULL, 0, &ChildImageHandle);
    if (ReturnStatus != EFI_NOT_FOUND)
      break;
  }
  UnicodeSPrint(ErrorInfo, 512, L"while loading %s", ImageTitle);
  if (CheckError(Status, ErrorInfo)) {
    if (ErrorInStep != NULL)
      *ErrorInStep = 1;
    PauseForKey(L"press any key");
    goto bailout;
  }
  
  if (!EFI_ERROR(ReturnStatus)) { //why unload driver?!
    if (NewImageHandle != NULL) {
      *NewImageHandle = ChildImageHandle;
    }
    goto bailout;
  }
  
  // unload the image, we don't care if it works or not...
  Status = gBS->UnloadImage(ChildImageHandle);
bailout:
  return ReturnStatus;
}


static EFI_STATUS StartEFILoadedImage(IN EFI_HANDLE ChildImageHandle,
                                    IN CHAR16 *LoadOptions, IN CHAR16 *LoadOptionsPrefix,
                                    IN CHAR16 *ImageTitle,
                                    OUT UINTN *ErrorInStep)
{
  EFI_STATUS              Status, ReturnStatus;
  EFI_LOADED_IMAGE        *ChildLoadedImage;
  CHAR16                  ErrorInfo[256];
  CHAR16                  *FullLoadOptions = NULL;
  
//  DBG("Starting %s\n", ImageTitle);
  if (ErrorInStep != NULL) {
    *ErrorInStep = 0;
  }
  ReturnStatus = Status = EFI_NOT_FOUND;  // in case no image handle was specified
  if (ChildImageHandle == NULL) {
    if (ErrorInStep != NULL) *ErrorInStep = 1;
    goto bailout;
  }
  
  // set load options
  if (LoadOptions != NULL) {
    ReturnStatus = Status = gBS->HandleProtocol(ChildImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &ChildLoadedImage);
    if (CheckError(Status, L"while getting a LoadedImageProtocol handle")) {
      if (ErrorInStep != NULL)
        *ErrorInStep = 2;
      goto bailout_unload;
    }
    
    if (LoadOptionsPrefix != NULL) {
      FullLoadOptions = PoolPrint(L"%s %s ", LoadOptionsPrefix, LoadOptions);
      // NOTE: That last space is also added by the EFI shell and seems to be significant
      //  when passing options to Apple's boot.efi...
      LoadOptions = FullLoadOptions;
    }
    // NOTE: We also include the terminating null in the length for safety.
    ChildLoadedImage->LoadOptions = (VOID *)LoadOptions;
    ChildLoadedImage->LoadOptionsSize = (UINT32)StrSize(LoadOptions);
    //((UINT32)StrLen(LoadOptions) + 1) * sizeof(CHAR16);
//    DBG("Using load options '%s'\n", LoadOptions);
  }
  //DBG("Image loaded at: %p\n", ChildLoadedImage->ImageBase);
  //PauseForKey(L"continue");
  
  // close open file handles
  UninitRefitLib();
  
  // turn control over to the image
  //
  // Before calling the image, enable the Watchdog Timer for
  // the 5 Minute period - Slice - NO! 60seconds is enough
  //  
  gBS->SetWatchdogTimer (180, 0x0000, 0x00, NULL);
  
  ReturnStatus = Status = gBS->StartImage(ChildImageHandle, NULL, NULL);
  //
  // Clear the Watchdog Timer after the image returns
  //
  gBS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);
  
  //PauseForKey(L"Returned from StartImage\n");
  
  // control returns here when the child image calls Exit()
  if (ImageTitle) {
    UnicodeSPrint(ErrorInfo, 512, L"returned from %s", ImageTitle);
  }
  
  if (CheckError(Status, ErrorInfo)) {
    if (ErrorInStep != NULL)
      *ErrorInStep = 3;
  }
  if (!EFI_ERROR(ReturnStatus)) { //why unload driver?!
    goto bailout;
  }
  
bailout_unload:
  // unload the image, we don't care if it works or not...
  Status = gBS->UnloadImage(ChildImageHandle);
  if (FullLoadOptions != NULL)
    FreePool(FullLoadOptions);
bailout:
  return ReturnStatus;
}


static EFI_STATUS LoadEFIImage(IN EFI_DEVICE_PATH *DevicePath,
                                IN CHAR16 *ImageTitle,
                                OUT UINTN *ErrorInStep,
                                OUT EFI_HANDLE *NewImageHandle)
{
  EFI_DEVICE_PATH *DevicePaths[2];

#ifdef ENABLE_SECURE_BOOT
  // Verify secure boot policy
  if (gSettings.SecureBoot && gSettings.SecureBootSetupMode) {
    // Only verify if in forced secure boot mode
    EFI_STATUS Status = VerifySecureBootImage(DevicePath);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
#endif // ENABLE_SECURE_BOOT

  // Load the image now
  DevicePaths[0] = DevicePath;
  DevicePaths[1] = NULL;
  return LoadEFIImageList(DevicePaths, ImageTitle, ErrorInStep, NewImageHandle);
}


static EFI_STATUS StartEFIImage(IN EFI_DEVICE_PATH *DevicePath,
                                IN CHAR16 *LoadOptions, IN CHAR16 *LoadOptionsPrefix,
                                IN CHAR16 *ImageTitle,
                                OUT UINTN *ErrorInStep,
                                OUT EFI_HANDLE *NewImageHandle)
{
  EFI_STATUS Status;
  EFI_HANDLE ChildImageHandle = NULL;

  Status = LoadEFIImage(DevicePath, ImageTitle, ErrorInStep, &ChildImageHandle);  
  if (!EFI_ERROR(Status)) {
    Status = StartEFILoadedImage(ChildImageHandle, LoadOptions, LoadOptionsPrefix, ImageTitle, ErrorInStep);
  }

  if (NewImageHandle != NULL) {
      *NewImageHandle = ChildImageHandle;
  }
  return Status;
}


static EFI_STATUS StartEFIImageList(IN EFI_DEVICE_PATH **DevicePaths,
                                IN CHAR16 *LoadOptions, IN CHAR16 *LoadOptionsPrefix,
                                IN CHAR16 *ImageTitle,
                                OUT UINTN *ErrorInStep,
                                OUT EFI_HANDLE *NewImageHandle)
{
  EFI_STATUS Status;
  EFI_HANDLE ChildImageHandle = NULL;

  Status = LoadEFIImageList(DevicePaths, ImageTitle, ErrorInStep, &ChildImageHandle);
  if (!EFI_ERROR(Status)) {
    Status = StartEFILoadedImage(ChildImageHandle, LoadOptions, LoadOptionsPrefix, ImageTitle, ErrorInStep);
  }

  if (NewImageHandle != NULL) {
      *NewImageHandle = ChildImageHandle;
  }
  return Status;
}


static CHAR8 *SearchString (
  IN  CHAR8       *Source,
  IN  UINT64      SourceSize,
  IN  CHAR8       *Search,
  IN  UINTN       SearchSize
  )
{
  CHAR8 *End = Source + SourceSize;

  while (Source < End) {
    if (CompareMem(Source, Search, SearchSize) == 0) {
      return Source;
    } else {
      Source++;
    }
  }
  return NULL;
}


//
// Null ConOut OutputString() implementation - for blocking
// text output from boot.efi when booting in graphics mode
//
EFI_STATUS EFIAPI
NullConOutOutputString(IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN CHAR16 *String) {
	return EFI_SUCCESS;
}

//
// EFI OS loader functions
//
EG_PIXEL DarkBackgroundPixel  = { 0x0, 0x0, 0x0, 0xFF };

static VOID StartLoader(IN LOADER_ENTRY *Entry)
{
  EFI_STATUS              Status;
  EFI_TEXT_STRING         ConOutOutputString = 0;
  EFI_HANDLE              ImageHandle = NULL;
  EFI_LOADED_IMAGE        *LoadedImage;
  CHAR8                   *InstallerVersion;
  
  DBG("StartLoader() start\n");
  MsgLog("Finally: Bus=%ldkHz CPU=%ldMHz\n",
         DivU64x32(gCPUStructure.FSBFrequency, kilo),
         gCPUStructure.MaxSpeed);

//  MsgLog("Turbo=%c\n", gSettings.Turbo?'Y':'N');
//  MsgLog("PatchAPIC=%c\n", gSettings.PatchNMI?'Y':'N');
//  MsgLog("PatchVBios=%c\n", gSettings.PatchVBios?'Y':'N');
//  DBG("KillMouse\n");

  // Load image into memory (will be started later) 
  Status = LoadEFIImage(Entry->DevicePath, Basename(Entry->LoaderPath), NULL, &ImageHandle);
  if (EFI_ERROR(Status)) {
    return; // no reason to continue if loading image failed
  }

  egClearScreen(Entry->BootBgColor ? Entry->BootBgColor : &DarkBackgroundPixel);
  KillMouse();

//  if (Entry->LoaderType == OSTYPE_OSX) {
  if (OSTYPE_IS_OSX(Entry->LoaderType) ||
      OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType) ||
      OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {

//DBG("GetOSVersion\n");

    // Correct OSVersion if it was not found
    // This should happen only for 10.7-10.9 OSTYPE_OSX_INSTALLER 
    // For these cases, take OSVersion from loaded boot.efi image in memory
    if (Entry->LoaderType == OSTYPE_OSX_INSTALLER || !Entry->OSVersion) {
      Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
      if (!EFI_ERROR(Status)) {
        // version in boot.efi appears as "Mac OS X 10.?"
        InstallerVersion = SearchString(LoadedImage->ImageBase, LoadedImage->ImageSize, "Mac OS X ", 9);
        if (InstallerVersion != NULL) { // string was found
          InstallerVersion += 9; // advance to version location
          if (AsciiStrnCmp(InstallerVersion, "10.7", 4) &&
              AsciiStrnCmp(InstallerVersion, "10.8", 4) &&
              AsciiStrnCmp(InstallerVersion, "10.9", 4) &&
              AsciiStrnCmp(InstallerVersion, "10.10", 4)) {   //xxx
            InstallerVersion = NULL; // flag known version was not found
          }
          if (InstallerVersion != NULL) { // known version was found in image
            if (Entry->OSVersion != NULL) {
              FreePool(Entry->OSVersion);
            }
            Entry->OSVersion = AllocateCopyPool(AsciiStrLen(InstallerVersion)+1, InstallerVersion);
            Entry->OSVersion[AsciiStrLen(InstallerVersion)] = '\0';
            DBG("Corrected OSVersion: %a\n", Entry->OSVersion);
          }
        }
      }
    }
    
    // if "InjectKexts if no FakeSMC" and OSFLAG_WITHKEXTS is not set
    // then user selected submenu entry and requested no injection.
    // we'll turn off global "InjectKexts if no FakeSMC" to avoid unnecessary
    // FakeSMC scanning.
    if (OSFLAG_ISUNSET(Entry->Flags, OSFLAG_WITHKEXTS)) {
      gSettings.WithKextsIfNoFakeSMC = FALSE;
    }

    //we are booting OSX - restore emulation if it's not installed before starting boot.efi
    if (gEmuVariableControl != NULL) {
        gEmuVariableControl->InstallEmulation(gEmuVariableControl);
    }

    // first patchACPI and find PCIROOT and RTC
    // but before ACPI patch we need smbios patch
    PatchSmbios();
//    DBG("PatchACPI\n");
    PatchACPI(Entry->Volume, Entry->OSVersion);

    // Prepare boot arguments
//    if ((StrCmp(gST->FirmwareVendor, L"CLOVER") != 0) &&
//        (StrCmp(gST->FirmwareVendor, L"EDKII") != 0)) {
/*    if (gDriversFlags.AptioFixLoaded) {
      // Add slide=0 argument for ML and Mavericks if not present
      CHAR16 *TempOptions = AddLoadOption(Entry->LoadOptions, L"slide=0");
      FreePool(Entry->LoadOptions);
      Entry->LoadOptions = TempOptions;
    }
*/
    // If KPDebug is true boot in verbose mode to see the debug messages
    if (gSettings.KPDebug) {
      CHAR16 *TempOptions = AddLoadOption(Entry->LoadOptions, L"-v");
      FreePool(Entry->LoadOptions);
      Entry->LoadOptions = TempOptions;
    }

//    DBG("SetDevices\n");
    SetDevices(Entry->OSVersion);
//    DBG("SetFSInjection\n");
    SetFSInjection(Entry);
    //PauseForKey(L"SetFSInjection");
//    DBG("SetVariablesForOSX\n");
    SetVariablesForOSX();
//    DBG("SetVariablesForOSX\n");
    EventsInitialize(Entry);
//    DBG("FinalizeSmbios\n");
    FinalizeSmbios();
//    DBG("SetupDataForOSX\n");
    SetupDataForOSX();

    SetCPUProperties();
    
    if (OSFLAG_ISSET(Entry->Flags, OSFLAG_HIBERNATED)) {
      DoHibernateWake = PrepareHibernation(Entry->Volume);
    }
    if (gDriversFlags.AptioFixLoaded && !DoHibernateWake) {
      // Add slide=0 argument for ML and Mavericks if not present
      CHAR16 *TempOptions = AddLoadOption(Entry->LoadOptions, L"slide=0");
      FreePool(Entry->LoadOptions);
      Entry->LoadOptions = TempOptions;
    }

//    DBG("Set FakeCPUID: 0x%x\n", gSettings.FakeCPUID);
//    DBG("LoadKexts\n");
    // LoadKexts writes to DataHub, where large writes can prevent hibernate wake (happens when several kexts present in Clover's kexts dir)
    if (!DoHibernateWake) {
      LoadKexts(Entry);
    }
    
    // blocking boot.efi output if -v is not specified
    // note: this blocks output even if -v is specified in
    // /Library/Preferences/SystemConfiguration/com.apple.Boot.plist
    // which is wrong
    // apianti - only block console output if using graphics
    //           but don't block custom boot logo
    if ((Entry->LoadOptions != NULL) &&
        ((StrStr(Entry->LoadOptions, L"-v") != NULL) ||
         (StrStr(Entry->LoadOptions, L"-V") != NULL))) {
      Entry->Flags = OSFLAG_UNSET(Entry->Flags, OSFLAG_USEGRAPHICS);
    }
  }
  else if (OSTYPE_IS_WINDOWS(Entry->LoaderType)) {
    
    if (gEmuVariableControl != NULL) {
      gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }
    
    PatchACPI_OtherOS(L"Windows", FALSE);
    //PauseForKey(L"continue");
      
  }
  else if (OSTYPE_IS_LINUX(Entry->LoaderType) || (Entry->LoaderType == OSTYPE_LINEFI)) {
    if (gEmuVariableControl != NULL) {
      gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }
    //FinalizeSmbios();
    PatchACPI_OtherOS(L"Linux", FALSE);
    //PauseForKey(L"continue");
  }
  
  SetStartupDiskVolume(Entry->Volume, Entry->LoaderType == OSTYPE_OSX ? NULL : Entry->LoaderPath);
  
//    DBG("BeginExternalScreen\n");
  BeginExternalScreen(OSFLAG_ISSET(Entry->Flags, OSFLAG_USEGRAPHICS), L"Booting OS");

  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_USEGRAPHICS)) {
    // save orig OutputString and replace it with
    // null implementation
    ConOutOutputString = gST->ConOut->OutputString;
    gST->ConOut->OutputString = NullConOutOutputString;
  }

  // Initialize the boot screen
  if (EFI_ERROR(Status = InitBootScreen(Entry))) {
    DBG("Failed to initialize boot screen: %r!\n", Status);
  }
  else if (EFI_ERROR(Status = LockBootScreen())) {
    DBG("Failed to lock boot screen: %r!\n", Status);
  }

  if (OSTYPE_IS_OSX(Entry->LoaderType) ||
      OSTYPE_IS_OSX_RECOVERY(Entry->LoaderType) ||
      OSTYPE_IS_OSX_INSTALLER(Entry->LoaderType)) {
    
    if (DoHibernateWake) {
      DBG("Closing events\n");
      gBS->CloseEvent (OnReadyToBootEvent);
      gBS->CloseEvent (ExitBootServiceEvent);
      gBS->CloseEvent (mSimpleFileSystemChangeEvent);
//      gBS->CloseEvent (mVirtualAddressChangeEvent);
    } else {
      // delete boot-switch-vars if exists
      Status = gRT->SetVariable(L"boot-switch-vars", &gEfiAppleBootGuid,
                                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                0, NULL);

    }
    DBG("Closing log\n");
    if (0 /* DoHibernateWake */) {
      Status = SaveBooterLog(SelfRootDir, PREWAKE_LOG);
      if (EFI_ERROR(Status)) {
        /*Status = */SaveBooterLog(NULL, PREWAKE_LOG);
      }
    } else {
      // When doing hibernate wake, save to DataHub only up to initial size of log
      /*Status = */SetupBooterLog(!DoHibernateWake);
    }
  }

//  DBG("StartEFIImage\n");
//  StartEFIImage(Entry->DevicePath, Entry->LoadOptions,
//                Basename(Entry->LoaderPath), Basename(Entry->LoaderPath), NULL, NULL);

//  DBG("StartEFILoadedImage\n");
  StartEFILoadedImage(ImageHandle, Entry->LoadOptions, 
                Basename(Entry->LoaderPath), Basename(Entry->LoaderPath), NULL);
  // Unlock boot screen
  if (EFI_ERROR(Status = UnlockBootScreen())) {
    DBG("Failed to unlock boot screen: %r!\n", Status);
  }
  if (OSFLAG_ISSET(Entry->Flags, OSFLAG_USEGRAPHICS)) {
    // return back orig OutputString
    gST->ConOut->OutputString = ConOutOutputString;
  }
  
//  PauseForKey(L"FinishExternalScreen");
  FinishExternalScreen();
//  PauseForKey(L"System started?!");
}

// early 2006 Core Duo / Core Solo models
static UINT8 LegacyLoaderDevicePath1Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xF9, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
// mid-2006 Mac Pro (and probably other Core 2 models)
static UINT8 LegacyLoaderDevicePath2Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xF7, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
// mid-2007 MBP ("Santa Rosa" based models)
static UINT8 LegacyLoaderDevicePath3Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xF8, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
// early-2008 MBA
static UINT8 LegacyLoaderDevicePath4Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xF8, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
// late-2008 MB/MBP (NVidia chipset)
static UINT8 LegacyLoaderDevicePath5Data[] = {
    0x01, 0x03, 0x18, 0x00, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x40, 0xCB, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xBF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x06, 0x14, 0x00, 0xEB, 0x85, 0x05, 0x2B,
    0xB8, 0xD8, 0xA9, 0x49, 0x8B, 0x8C, 0xE2, 0x1B,
    0x01, 0xAE, 0xF2, 0xB7, 0x7F, 0xFF, 0x04, 0x00,
};
static EFI_DEVICE_PATH *LegacyLoaderList[] = {
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath1Data,
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath2Data,
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath3Data,
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath4Data,
    (EFI_DEVICE_PATH *)LegacyLoaderDevicePath5Data,
    NULL
};

#define MAX_DISCOVERED_PATHS (16)
//#define PREBOOT_LOG L"EFI\\CLOVER\\misc\\preboot.log"

static VOID StartLegacy(IN LEGACY_ENTRY *Entry)
{
    EFI_STATUS          Status = EFI_UNSUPPORTED;
    EG_IMAGE            *BootLogoImage;
    UINTN               ErrorInStep = 0;
    EFI_DEVICE_PATH     *DiscoveredPathList[MAX_DISCOVERED_PATHS];

    // Unload EmuVariable before booting legacy.
    // This is not needed in most cases, but it seems to interfere with legacy OS
    // booted on some UEFI bioses, such as Phoenix UEFI 2.0
    if (gEmuVariableControl != NULL) {
      gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }  

    SetStartupDiskVolume(Entry->Volume, NULL);
  
    egClearScreen(&DarkBackgroundPixel);
    BeginExternalScreen(TRUE, L"Booting Legacy OS");
    
    BootLogoImage = LoadOSIcon(Entry->Volume->LegacyOS->IconName, L"legacy", 128, TRUE, TRUE);
    if (BootLogoImage != NULL)
        BltImageAlpha(BootLogoImage,
                      (UGAWidth  - BootLogoImage->Width) >> 1,
                      (UGAHeight - BootLogoImage->Height) >> 1,
                      &StdBackgroundPixel, 16);
  
    if (StrCmp(gSettings.LegacyBoot, L"Apple") != 0) { // not Apple-style LegacyBoot
      //try my LegacyBoot
      switch (Entry->Volume->BootType) {
        case BOOTING_BY_CD:
          Status = bootElTorito(Entry->Volume);
          break;
        case BOOTING_BY_MBR:
          Status = bootMBR(Entry->Volume);
          break;
        case BOOTING_BY_PBR:
          if (StrCmp(gSettings.LegacyBoot, L"LegacyBiosDefault") == 0) {
            Status = bootLegacyBiosDefault(gSettings.LegacyBiosDefaultEntry);
          } else if (StrCmp(gSettings.LegacyBoot, L"PBRtest") == 0) {
            Status = bootPBRtest(Entry->Volume);
          } else {
            // default
            Status = bootPBR(Entry->Volume);
          }
          break;
        default:
          break;
      }
      CheckError(Status, L"while LegacyBoot");
    } else { // Apple-style LegacyBoot
//      if (0 && Entry->Volume->IsMbrPartition && !Entry->Volume->HasBootCode)
//         ActivateMbrPartition(Entry->Volume->WholeDiskBlockIO, Entry->Volume->MbrPartitionIndex);

      Status = ExtractLegacyLoaderPaths(DiscoveredPathList, MAX_DISCOVERED_PATHS, LegacyLoaderList);
      if (!EFI_ERROR(Status)) {
        Status = StartEFIImageList(DiscoveredPathList, Entry->LoadOptions, NULL, L"legacy loader", &ErrorInStep, NULL);
      } 
      if (Status == EFI_NOT_FOUND) {
        if (ErrorInStep == 1) {
          Print(L"\nPlease make sure that you have the latest firmware update installed.\n");
        } else if (ErrorInStep == 3) {
          Print(L"\nThe firmware refused to boot from the selected volume. Note that external\n"
                L"hard drives are not well-supported by Apple's firmware for legacy OS booting.\n");
        }
      }
    }
    FinishExternalScreen();
}

//
// pre-boot tool functions
//

static VOID StartTool(IN LOADER_ENTRY *Entry)
{
  egClearScreen(&DarkBackgroundPixel);
    BeginExternalScreen(OSFLAG_ISSET(Entry->Flags, OSFLAG_USEGRAPHICS), Entry->me.Title + 6);  // assumes "Start <title>" as assigned below
    StartEFIImage(Entry->DevicePath, Entry->LoadOptions, Basename(Entry->LoaderPath),
                  Basename(Entry->LoaderPath), NULL, NULL);
    FinishExternalScreen();
//  ReinitSelfLib();
}

//
// pre-boot driver functions
//

static VOID ScanDriverDir(IN CHAR16 *Path, OUT EFI_HANDLE **DriversToConnect, OUT UINTN *DriversToConnectNum)
{
  EFI_STATUS              Status;
  REFIT_DIR_ITER          DirIter;
  EFI_FILE_INFO           *DirEntry;
  CHAR16                  FileName[256];
  EFI_HANDLE              DriverHandle;
  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding;
  UINTN                   DriversArrSize;
  UINTN                   DriversArrNum;
  EFI_HANDLE              *DriversArr;
  INTN                    i;
  BOOLEAN                 Skip;
  
  DriversArrSize = 0;
  DriversArrNum = 0;
  DriversArr = NULL;
  
  // look through contents of the directory
  DirIterOpen(SelfRootDir, Path, &DirIter);
  while (DirIterNext(&DirIter, 2, L"*.EFI", &DirEntry)) {
    Skip = (DirEntry->FileName[0] == '.');
//    if (DirEntry->FileName[0] == '.')
//      continue;   // skip this    
    for (i=0; i<gSettings.BlackListCount; i++) {
      if (StrStr(DirEntry->FileName, gSettings.BlackList[i]) != NULL) {
        Skip = TRUE;   // skip this
        break;
      }
    }
    if (Skip) {
      continue;
    }
    // either AptioFix, AptioFix2 or LowMemFix
    if (StrStr(DirEntry->FileName, L"AptioFixDrv") != NULL) {
      if (gDriversFlags.MemFixLoaded || gDriversFlags.AptioFix2Loaded) {
        continue; //if other driver loaded then skip new one
      }
      gDriversFlags.AptioFixLoaded = TRUE;
    } else if (StrStr(DirEntry->FileName, L"AptioFix2Drv") != NULL) {
      if (gDriversFlags.MemFixLoaded || gDriversFlags.AptioFixLoaded) {
        continue; //if other driver loaded then skip new one
      }
      gDriversFlags.AptioFix2Loaded = TRUE;
    } else if (StrStr(DirEntry->FileName, L"LowMemFix") != NULL) {
      if (gDriversFlags.AptioFixLoaded || gDriversFlags.AptioFix2Loaded) {
        continue; //if other driver loaded then skip new one
      }
      gDriversFlags.MemFixLoaded = TRUE;
    }

    UnicodeSPrint(FileName, 512, L"%s\\%s", Path, DirEntry->FileName);
    Status = StartEFIImage(FileDevicePath(SelfLoadedImage->DeviceHandle, FileName),
                           L"", DirEntry->FileName, DirEntry->FileName, NULL, &DriverHandle);
    if (EFI_ERROR(Status)) {
      continue;
    }
    if (StrStr(FileName, L"EmuVariable") != NULL) {
      gDriversFlags.EmuVariableLoaded = TRUE;
    } else if (StrStr(FileName, L"Video") != NULL) {
      gDriversFlags.VideoLoaded = TRUE;
    } else if (StrStr(FileName, L"Partition") != NULL) {
      gDriversFlags.PartitionLoaded = TRUE;
    }
    if (DriverHandle != NULL && DriversToConnectNum != NULL && DriversToConnect != NULL) {
      // driver loaded - check for EFI_DRIVER_BINDING_PROTOCOL
      Status = gBS->HandleProtocol(DriverHandle, &gEfiDriverBindingProtocolGuid, (VOID **) &DriverBinding);
      if (!EFI_ERROR(Status) && DriverBinding != NULL) {
        DBG(" - driver needs connecting\n");
        // standard UEFI driver - we would reconnect after loading - add to array
        if (DriversArrSize == 0) {
          // new array
          DriversArrSize = 16;
          DriversArr = AllocateZeroPool(sizeof(EFI_HANDLE) * DriversArrSize);
        } else if (DriversArrNum + 1 == DriversArrSize) {
          // extend array
          DriversArr = ReallocatePool(DriversArrSize, DriversArrSize + 16, DriversArr);
          DriversArrSize += 16;
        }
        DriversArr[DriversArrNum] = DriverHandle;
        DriversArrNum++;
        // we'll make array terminated
        DriversArr[DriversArrNum] = NULL;
      }
    }
  }
  Status = DirIterClose(&DirIter);
  if (Status != EFI_NOT_FOUND) {
    UnicodeSPrint(FileName, 512, L"while scanning the %s directory", Path);
    CheckError(Status, FileName);
  }
  
  if (DriversToConnectNum != NULL && DriversToConnect != NULL) {
    *DriversToConnectNum = DriversArrNum;
    *DriversToConnect = DriversArr;
  }
//release memory for BlackList
  for (i=0; i<gSettings.BlackListCount; i++) {
    if (gSettings.BlackList[i]) {
      FreePool(gSettings.BlackList[i]);
      gSettings.BlackList[i] = NULL;
    }
  }
}


/**
 * Some UEFI's (like HPQ EFI from HP notebooks) have DiskIo protocols
 * opened BY_DRIVER (by Partition driver in HP case) even when no file system
 * is produced from this DiskIo. This then blocks our FS drivers from connecting
 * and producing file systems.
 * To fix it: we will disconnect drivers that connected to DiskIo BY_DRIVER
 * if this is partition volume and if those drivers did not produce file system.
 */
VOID DisconnectInvalidDiskIoChildDrivers(VOID)
{
  EFI_STATUS                            Status;
  UINTN                                 HandleCount = 0;
  UINTN                                 Index;
  UINTN                                 OpenInfoIndex;
  EFI_HANDLE                            *Handles = NULL;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Fs;
  EFI_BLOCK_IO_PROTOCOL                 *BlockIo;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY   *OpenInfo;
  UINTN                                 OpenInfoCount;
  BOOLEAN                               Found;
  
  DBG("Searching for invalid DiskIo BY_DRIVER connects:");
  
  //
  // Get all DiskIo handles
  //
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiDiskIoProtocolGuid, NULL, &HandleCount, &Handles);
  if (EFI_ERROR(Status) || HandleCount == 0) {
    DBG(" no DiskIo handles\n");
    return;
  }
  
  //
  // Check every DiskIo handle
  //
  Found = FALSE;
  for (Index = 0; Index < HandleCount; Index++) {
    //DBG("\n");
    //DBG(" - Handle %p:", Handles[Index]);
    //
    // If this is not partition - skip it.
    // This is then whole disk and DiskIo
    // should be opened here BY_DRIVER by Partition driver
    // to produce partition volumes.
    //
    Status = gBS->HandleProtocol (
                                  Handles[Index],
                                  &gEfiBlockIoProtocolGuid,
                                  (VOID **) &BlockIo
                                  );
    if (EFI_ERROR (Status)) {
      //DBG(" BlockIo: %r - skipping\n", Status);
      continue;
    }
    if (BlockIo->Media == NULL) {
      //DBG(" BlockIo: no media - skipping\n");
      continue;
      
    }
    if (!BlockIo->Media->LogicalPartition) {
      //DBG(" BlockIo: whole disk - skipping\n");
      continue;
      
    }
    //DBG(" BlockIo: partition");
    
    //
    // If SimpleFileSystem is already produced - skip it, this is ok
    //
    Status = gBS->HandleProtocol (
                                  Handles[Index],
                                  &gEfiSimpleFileSystemProtocolGuid,
                                  (VOID **) &Fs
                                  );
    if (Status == EFI_SUCCESS) {
      //DBG(" FS: ok - skipping\n");
      continue;
    }
    //DBG(" FS: no");
    
    //
    // If no SimpleFileSystem on this handle but DiskIo is opened BY_DRIVER
    // then disconnect this connection
    //
    Status = gBS->OpenProtocolInformation (
                                           Handles[Index],
                                           &gEfiDiskIoProtocolGuid,
                                           &OpenInfo,
                                           &OpenInfoCount
                                           );
    if (EFI_ERROR (Status)) {
      //DBG(" OpenInfo: no - skipping\n");
      continue;
    }
    //DBG(" OpenInfo: %d", OpenInfoCount);
    for (OpenInfoIndex = 0; OpenInfoIndex < OpenInfoCount; OpenInfoIndex++) {
      if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) == EFI_OPEN_PROTOCOL_BY_DRIVER) {
        if (!Found) {
          DBG("\n");
        }
        Found = TRUE;
        Status = gBS->DisconnectController (Handles[Index], OpenInfo[OpenInfoIndex].AgentHandle, NULL);
        //DBG(" BY_DRIVER Agent: %p, Disconnect: %r", OpenInfo[OpenInfoIndex].AgentHandle, Status);
        DBG(" - Handle %p with DiskIo, is Partition, no Fs, BY_DRIVER Agent: %p, Disconnect: %r\n", Handles[Index], OpenInfo[OpenInfoIndex].AgentHandle, Status);
      }
    }
    FreePool (OpenInfo);
  }
  FreePool(Handles);
  
  if (!Found) {
    DBG(" not found, all ok\n");
  }
}

VOID DisconnectSomeDevices(VOID)
{
  EFI_STATUS              Status;
  UINTN                   HandleCount = 0;
  UINTN                   Index;
  EFI_HANDLE              *Handles = NULL;
	EFI_BLOCK_IO_PROTOCOL   *BlockIo	= NULL;
	EFI_PCI_IO_PROTOCOL     *PciIo	= NULL;
	PCI_TYPE00              Pci;
  
  if (gDriversFlags.PartitionLoaded) {
    DBG("Partition driver loaded: ");
    // get all BlockIo handles
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &Handles);
    if (Status == EFI_SUCCESS) {
      for (Index = 0; Index < HandleCount; Index++) {
        Status = gBS->HandleProtocol(Handles[Index], &gEfiBlockIoProtocolGuid, (VOID **) &BlockIo);
        if (EFI_ERROR(Status)) {
          continue;
        }
        if (BlockIo->Media->BlockSize == 2048) {
          // disconnect CD driver
          Status = gBS->DisconnectController(Handles[Index], NULL, NULL);
          DBG("CD disconnect %r", Status);
        }
      }
      FreePool(Handles);
    }
    DBG("\n");
  }
  
  if (gDriversFlags.VideoLoaded) {
    DBG("Video driver loaded: ");
    // get all PciIo handles
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPciIoProtocolGuid, NULL, &HandleCount, &Handles);
    if (Status == EFI_SUCCESS) {
      for (Index = 0; Index < HandleCount; Index++) {
        Status = gBS->HandleProtocol(Handles[Index], &gEfiPciIoProtocolGuid, (VOID **) &PciIo);
        if (EFI_ERROR(Status)) {
          continue;
        }
        Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
        if (!EFI_ERROR (Status))
        {
          if(IS_PCI_VGA(&Pci) == TRUE)
          {
            // disconnect VGA
            Status = gBS->DisconnectController(Handles[Index], NULL, NULL);
            DBG("disconnect %r", Status);
          }
        }
      }
      FreePool(Handles);
    }
    DBG("\n");
  }
  
  if (!gFirmwareClover) {
    DisconnectInvalidDiskIoChildDrivers();
  }
}

//UINT8 *mCurrentEdid;
/*
UINT8* getCurrentEdid (VOID)
{
  EFI_STATUS                      Status;
  EFI_EDID_ACTIVE_PROTOCOL        *EdidProtocol;
  UINT8                           *Edid;
  
  DBG ("Edid:");
  Edid = NULL;
  Status = gBS->LocateProtocol (&gEfiEdidActiveProtocolGuid, NULL, (VOID**)&EdidProtocol);
  if (!EFI_ERROR (Status)) {
    DBG(" size=%d", EdidProtocol->SizeOfEdid);
    if (EdidProtocol->SizeOfEdid > 0) {
      Edid = AllocateCopyPool (EdidProtocol->SizeOfEdid, EdidProtocol->Edid);
    }
  }
  DBG(" %a\n", Edid != NULL ? "found" : "not found");
  
  return Edid;
}
*/

VOID PatchVideoBios(UINT8 *Edid)
{

  if (gSettings.PatchVBiosBytesCount > 0 && gSettings.PatchVBiosBytes != NULL) {
    VideoBiosPatchBytes(gSettings.PatchVBiosBytes, gSettings.PatchVBiosBytesCount);
  }

  if (gSettings.PatchVBios) {
    VideoBiosPatchNativeFromEdid(Edid);
  }
}


static VOID LoadDrivers(VOID)
{
  EFI_STATUS  Status;
  EFI_HANDLE  *DriversToConnect = NULL;
  UINTN       DriversToConnectNum = 0;
  UINT8       *Edid;
  UINTN       VarSize = 0;
  BOOLEAN     VBiosPatchNeeded;
  
    // load drivers from /efi/drivers
#if defined(MDE_CPU_X64)
  if (gFirmwareClover) {
    ScanDriverDir(L"\\EFI\\CLOVER\\drivers64", &DriversToConnect, &DriversToConnectNum);
  } else
    ScanDriverDir(L"\\EFI\\CLOVER\\drivers64UEFI", &DriversToConnect, &DriversToConnectNum);
#else
  ScanDriverDir(L"\\EFI\\CLOVER\\drivers32", &DriversToConnect, &DriversToConnectNum);
#endif
  
  VBiosPatchNeeded = gSettings.PatchVBios || (gSettings.PatchVBiosBytesCount > 0 && gSettings.PatchVBiosBytes != NULL);
  if (VBiosPatchNeeded) {
    // check if it is already done in CloverEFI BiosVideo
    Status = gRT->GetVariable (
                               L"CloverVBiosPatchDone",
                               &gEfiGlobalVariableGuid,
                               NULL,
                               &VarSize,
                               NULL
                               );
    if (Status == EFI_BUFFER_TOO_SMALL) {
      // var exists - it's done - let's not do it again
      VBiosPatchNeeded = FALSE;
    }
  }
  
  if (((gSettings.CustomEDID != NULL) && gFirmwareClover) || (VBiosPatchNeeded && !gDriversFlags.VideoLoaded)) {
    // we have video bios patch - force video driver reconnect
    DBG("Video bios patch requested or CustomEDID - forcing video reconnect\n");
    gDriversFlags.VideoLoaded = TRUE;
    DriversToConnectNum++;
  }
  
  if (DriversToConnectNum > 0) {
    DBG("%d drivers needs connecting ...\n", DriversToConnectNum);
    // note: our platform driver protocol
    // will use DriversToConnect - do not release it
    RegisterDriversToHighestPriority(DriversToConnect);
    if (VBiosPatchNeeded) {
      if (gSettings.CustomEDID != NULL) {
        Edid = gSettings.CustomEDID;
      } else {
        Edid = getCurrentEdid();
      }
      DisconnectSomeDevices();
      PatchVideoBios(Edid);
      if (gSettings.CustomEDID == NULL) {
        FreePool(Edid);
      }
    } else {
      DisconnectSomeDevices();
    }
    BdsLibConnectAllDriversToAllControllers();
    
    // Boot speedup: remove temporary "BiosVideoBlockSwitchMode" RT var
    // to unlock mode switching in CsmVideo
    gRT->SetVariable(L"BiosVideoBlockSwitchMode", &gEfiGlobalVariableGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS, 0, NULL);
  }
}


INTN FindDefaultEntry(VOID)
{
  INTN                Index = -1;
  REFIT_VOLUME        *Volume;
  LOADER_ENTRY        *Entry;
  BOOLEAN             SearchForLoader;
  
//  DBG("FindDefaultEntry ...\n");
  
  //
  // try to detect volume set by Startup Disk or previous Clover selection
  // with broken nvram this requires emulation to be installed
  // enabled emulation to determin efi-boot-device-data
  if (gEmuVariableControl != NULL) {
    gEmuVariableControl->InstallEmulation(gEmuVariableControl);
  }
    
  if (!GlobalConfig.IgnoreNVRAMBoot) {
    Index = FindStartupDiskVolume(&MainMenu);
  }
  
  if (Index >= 0) {
    DBG("Boot redirected to Entry %d. '%s'\n", Index, MainMenu.Entries[Index]->Title);
    // we got boot-device-data, no need to keep emulating anymore
    if (gEmuVariableControl != NULL) {
        gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
    }
    return Index;
  }
  
  //
  // if not found, then try DefaultVolume from config.plist
  // if not null or empty, search volume that matches gSettings.DefaultVolume
  //
  if (gSettings.DefaultVolume != NULL && gSettings.DefaultVolume[0] != L'\0') {
    
    // if not null or empty, also search for loader that matches gSettings.DefaultLoader
    SearchForLoader = (gSettings.DefaultLoader != NULL && gSettings.DefaultLoader[0] != L'\0');
    
    if (SearchForLoader) {
      DBG("Searching for DefaultVolume '%s', DefaultLoader '%s' ...\n", gSettings.DefaultVolume, gSettings.DefaultLoader);
    } else {
      DBG("Searching for DefaultVolume '%s' ...\n", gSettings.DefaultVolume);
    }
    
    for (Index = 0; ((Index < (INTN)MainMenu.EntryCount) && (MainMenu.Entries[Index]->Row == 0)); Index++) {
      
      Entry = (LOADER_ENTRY*)MainMenu.Entries[Index];
      if (!Entry->Volume) {
        continue;
      }
      
      Volume = Entry->Volume;
      if ((Volume->VolName == NULL || StrCmp(Volume->VolName, gSettings.DefaultVolume) != 0) && !StrStr(Volume->DevicePathString, gSettings.DefaultVolume)) {
        continue;
      }
      
      if (SearchForLoader && (Entry->me.Tag != TAG_LOADER || !StrStriBasic(Entry->LoaderPath, gSettings.DefaultLoader))) {
        continue;
      }
      
      DBG(" found entry %d. '%s', Volume '%s', DevicePath '%s'\n", Index, Entry->me.Title, Volume->VolName, Entry->DevicePathString);
      // if first method failed and second succeeded - uninstall emulation
      if (gEmuVariableControl != NULL) {
        gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
      }
      return Index;
    }
    
  }
  
  DBG("Default boot entry not found\n");
 // if both methods to determine default boot entry have failed - uninstall emulation before GUI
 if (gEmuVariableControl != NULL) {
    gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
 }
  return -1;
}

VOID SetVariablesFromNvram()
{
  CHAR16  UStr[80];
  CHAR8  *tmpString;
  UINTN   Size = 0;
  UINTN   index = 0, index2, len, i;
  CHAR8  *arg = NULL;
  
  tmpString = GetNvramVariable(L"Clover.LogLineCount", &gEfiAppleBootGuid, NULL, &Size);
  ZeroMem(UStr, 10);
  if (tmpString) {
    gSettings.LogLineCount = (UINT32)AsciiStrDecimalToUintn(tmpString);
  }
  
  tmpString = GetNvramVariable(L"Clover.MountEFI", &gEfiAppleBootGuid, NULL, &Size);
  if (tmpString) {
    gSettings.MountEFI = AllocateCopyPool(AsciiStrSize(tmpString), tmpString);
  }
  
  tmpString = GetNvramVariable(L"Clover.LogEveryBoot", &gEfiAppleBootGuid, NULL, &Size);
  if (tmpString) {
    gSettings.LogEveryBoot = AllocateCopyPool(AsciiStrSize(tmpString), tmpString);
  }

  tmpString = GetNvramVariable(L"boot-args", &gEfiAppleBootGuid, NULL, &Size);
  if (tmpString && (Size <= 0x1000) && (Size > 0)) {
    DBG("found boot-args in NVRAM:%a, size=%d\n", tmpString, Size);
    Size = AsciiStrLen(tmpString); // some EFI implementations include '\0' in Size, and others don't, so update Size to string length
    arg = AllocatePool(Size+1);
    //first we will find new args that is not present in main args
    index = 0;
    while ((index < Size) && (tmpString[index] != 0x0)) {
      ZeroMem(arg, Size+1);
      index2 = 0;
      if (tmpString[index] != '\"') {
 //       DBG("search space index=%d\n", index);
        while ((index < Size) && (tmpString[index] != 0x20) && (tmpString[index] != 0x0)) {
          arg[index2++] = tmpString[index++];
        }
        DBG("...found arg:%a\n", arg);
      } else {
        index++;
//        DBG("search quote index=%d\n", index);
        while ((index < Size) && (tmpString[index] != '\"') && (tmpString[index] != 0x0)) {
          arg[index2++] = tmpString[index++];
        }     
        if (tmpString[index] == '\"') {
          index++;
        }
        DBG("...found quoted arg:\n", arg);
      }
      while (tmpString[index] == 0x20) {
        index++;
      }
      // For the moment only arg -s must be ignored
      if (AsciiStrCmp(arg, "-s") == 0) {
          DBG("...ignoring arg:%a\n", arg);
        continue;
      }
      if (!AsciiStrStr(gSettings.BootArgs, arg)) {
        //this arg is not present will add
        DBG("...adding arg:%a\n", arg); 
        len = iStrLen(gSettings.BootArgs, 256);
        if (len + index2 > 256) {
          DBG("boot-args overflow... bytes=%d+%d\n", len, index2);
          break;
        }
        gSettings.BootArgs[len++] = 0x20;
        for (i = 0; i < index2; i++) {
          gSettings.BootArgs[len++] = arg[i];
        }
        gSettings.BootArgs[len++] = 0x20;
      }
    }
    FreePool(arg);
  }
}

VOID SetOEMPath(CHAR16 *ConfName)
  {
    if (ConfName == NULL) {
      OEMPath = L"EFI\\CLOVER";
    } else if (!gFirmwareClover &&
               FileExists(SelfRootDir,
                          PoolPrint(L"EFI\\CLOVER\\OEM\\%a\\UEFI\\%s.plist", gSettings.OEMBoard, ConfName))) {
      OEMPath = PoolPrint(L"EFI\\CLOVER\\OEM\\%a\\UEFI", gSettings.OEMBoard);
    } else if (FileExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a\\%s.plist", gSettings.OEMProduct, ConfName))) {
      OEMPath = PoolPrint(L"EFI\\CLOVER\\OEM\\%a", gSettings.OEMProduct);
    } else if (FileExists(SelfRootDir, PoolPrint(L"EFI\\CLOVER\\OEM\\%a\\%s.plist", gSettings.OEMBoard, ConfName))) {
      OEMPath = PoolPrint(L"EFI\\CLOVER\\OEM\\%a", gSettings.OEMBoard);
    } else {
      OEMPath = L"EFI\\CLOVER";
    }
  }

//
// main entry point
//
EFI_STATUS
EFIAPI
RefitMain (IN EFI_HANDLE           ImageHandle,
           IN EFI_SYSTEM_TABLE     *SystemTable)
{
  EFI_STATUS Status;
  BOOLEAN           MainLoopRunning = TRUE;
  BOOLEAN           ReinitDesktop = TRUE;
  BOOLEAN           AfterTool = FALSE;
  REFIT_MENU_ENTRY  *ChosenEntry = NULL;
  REFIT_MENU_ENTRY  *DefaultEntry = NULL;
  REFIT_MENU_ENTRY  *OptionEntry = NULL;
  INTN              DefaultIndex;
  UINTN             MenuExit;
  UINTN             Size, i;
//  UINT64            TscDiv;
//  UINT64            TscRemainder = 0;
  LOADER_ENTRY      *LoaderEntry;
  CHAR16            *ConfName = NULL;
  TagPtr            smbiosTags = NULL;
  TagPtr            UniteTag = NULL;
  BOOLEAN           UniteConfigs = FALSE;
  EFI_TIME          Now;
  
  // CHAR16            *InputBuffer; //, *Y;
  //  EFI_INPUT_KEY Key;
  
  // get TSC freq and init MemLog if needed
  gCPUStructure.TSCCalibr = GetMemLogTscTicksPerSecond(); //ticks for 1second
  //GlobalConfig.TextOnly = TRUE;
  
  // bootstrap
	gST				= SystemTable;
	gImageHandle	= ImageHandle;
	gBS				= SystemTable->BootServices;
	gRS				= SystemTable->RuntimeServices;
	/*Status = */EfiGetSystemConfigurationTable (&gEfiDxeServicesTableGuid, (VOID **) &gDS);

  gRS->GetTime(&Now, NULL);

  // firmware detection
  gFirmwareClover = StrCmp(gST->FirmwareVendor, L"CLOVER") == 0;
  InitializeConsoleSim();
  InitBooterLog();
  DBG("\n");
  DBG("Now is %d.%d.%d,  %d:%d:%d (GMT+%d)\n",
      Now.Day, Now.Month, Now.Year, Now.Hour, Now.Minute, Now.Second, Now.TimeZone);
  DBG("Starting Clover rev %s on %s EFI\n", FIRMWARE_REVISION, gST->FirmwareVendor);

  Status = InitRefitLib(gImageHandle);
  if (EFI_ERROR(Status))
    return Status;
  
  // disable EFI watchdog timer
  gBS->SetWatchdogTimer(0x0000, 0x0000, 0x0000, NULL);
  ZeroMem((VOID*)&gSettings, sizeof(SETTINGS_DATA));
  
  Status = InitializeUnicodeCollationProtocol();
  DBG("UnicodeCollation Status=%r\n", Status);
  PrepatchSmbios();
  
#ifdef REVISION_STR
  MsgLog(REVISION_STR); 
#endif
  //replace / with _
  Size = iStrLen(gSettings.OEMProduct, 64);
  for (i=0; i<Size; i++) {
    if (gSettings.OEMProduct[i] == 0x2F) {
      gSettings.OEMProduct[i] = 0x5F;
    }
  }
  Size = iStrLen(gSettings.OEMBoard, 64);
  for (i=0; i<Size; i++) {
    if (gSettings.OEMBoard[i] == 0x2F) {
      gSettings.OEMBoard[i] = 0x5F;
    }
  }
  DBG("  running on %a\n",   gSettings.OEMProduct);
  DBG("... with board %a\n", gSettings.OEMBoard);
  
  // LoadOptions Parsing
  DBG("Clover load options size = %d bytes\n", SelfLoadedImage->LoadOptionsSize);
    ParseLoadOptions(&ConfName, &gConfigDict[1]);
  if (ConfName) {
    if (StrLen(ConfName) == 0) {
      gConfigDict[1] = NULL;
      FreePool(ConfName);
      ConfName = NULL;
    } else {
      SetOEMPath(ConfName);
      Status = LoadUserSettings(SelfRootDir, ConfName, &gConfigDict[1]);
      DBG("%s\\%s.plist%s loaded with name from LoadOptions: %r\n",
                OEMPath, ConfName, EFI_ERROR(Status) ? L" not" : L"", Status);
      if (EFI_ERROR(Status)) {
        gConfigDict[1] = NULL;
        FreePool(ConfName);
        ConfName = NULL;
      }
    }
  }
  if (gConfigDict[1]) {
    UniteTag = GetProperty(gConfigDict[1], "Unite");
    if(UniteTag) {
      UniteConfigs =  (UniteTag->type == kTagTypeTrue) ||
                      ((UniteTag->type == kTagTypeString) &&
                      ((UniteTag->string[0] == 'y') || (UniteTag->string[0] == 'Y')));
      DBG("UniteConfigs = %s", UniteConfigs ? L"TRUE\n": L"FALSE\n" );
    }
  }
  if (!gConfigDict[1] || UniteConfigs) {
    SetOEMPath(L"config");
    Status = LoadUserSettings(SelfRootDir, L"config", &gConfigDict[0]);
      DBG("%s\\config.plist%s loaded: %r\n", OEMPath, EFI_ERROR(Status) ? L" not" : L"", Status);
  }
  gSettings.ConfigName = PoolPrint(L"%s%s%s",
                                   gConfigDict[0] ? L"config": L"",
                                   (gConfigDict[0] && gConfigDict[1]) ? L" + ": L"",
                                   !gConfigDict[1] ? L"": (ConfName ? ConfName : L"Load Options"));
  gSettings.MainConfigName = EfiStrDuplicate(gSettings.ConfigName);

  gSettings.PointerEnabled = TRUE;
  gSettings.PointerSpeed = 2;
  gSettings.DoubleClickTime = 500;

#ifdef ENABLE_SECURE_BOOT
  // Initialize secure boot
  InitializeSecureBoot();
#endif // ENABLE_SECURE_BOOT

#if HIBERNATE
  {
    UINT32                    machineSignature		= 0;
    EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE		  *FadtPointer = NULL;
    EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE	*Facs = NULL;
//    EFI_DEVICE_PATH_PROTOCOL* bootImagePath       = NULL;
    
    DBG("---dump hibernations data---\n");
    FadtPointer = GetFadt();
    if (FadtPointer != NULL) {
      Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);
      DBG("  Firmware wake address=%08lx\n", Facs->FirmwareWakingVector);
      DBG("  Firmware wake 64 addr=%16llx\n",  Facs->XFirmwareWakingVector);
      DBG("  Hardware signature   =%08lx\n", Facs->HardwareSignature);
      DBG("  GlobalLock           =%08lx\n", Facs->GlobalLock);
      DBG("  Flags                =%08lx\n", Facs->Flags);
      machineSignature = Facs->HardwareSignature;
    }
//------------------------------------------------------
    //DoHibernateWake = DumpVariable(L"Boot0082", &gEfiGlobalVariableGuid, 8);
    DumpVariable(L"boot-switch-vars", &gEfiAppleBootGuid, -1);
    DumpVariable(L"boot-signature",   &gEfiAppleBootGuid, -1);
    DumpVariable(L"boot-image-key",   &gEfiAppleBootGuid, -1);
    DumpVariable(L"boot-image",       &gEfiAppleBootGuid, 0);
//-----------------------------------------------------------
  }
#endif //

  if (!GlobalConfig.FastBoot) {
    GetListOfThemes();
  }

  for (i=0; i<2; i++) {
    if (gConfigDict[i]) {
      Status = GetEarlyUserSettings(SelfRootDir, gConfigDict[i]);
      if (EFI_ERROR(Status)) {
        DBG("Error in Early settings%d: %r\n", i, Status);
      }
    }
  }

#ifdef ENABLE_SECURE_BOOT
  // Install secure boot shim
  if (EFI_ERROR(Status = InstallSecureBoot())) {
    PauseForKey(L"Secure boot failure!\n");
    return Status;
  }
#endif // ENABLE_SECURE_BOOT

  MainMenu.TimeoutSeconds = GlobalConfig.Timeout >= 0 ? GlobalConfig.Timeout : 0;
  
  DBG("LoadDrivers() start\n");
  LoadDrivers();
  DBG("LoadDrivers() end\n");
  
  // for DUET or Ovmf
/*  if (StrCmp(gST->FirmwareVendor, L"EDK II") == 0) {
    gDriversFlags.EmuVariableLoaded = TRUE;
  }*/
  
  Status = gBS->LocateProtocol (&gEmuVariableControlProtocolGuid, NULL, (VOID**)&gEmuVariableControl);
  if (EFI_ERROR(Status)) {
    gEmuVariableControl = NULL;
  }
  if (gEmuVariableControl != NULL) {
    gEmuVariableControl->InstallEmulation(gEmuVariableControl);
  }

  if (!GlobalConfig.FastBoot) {
    // init screen and dump video modes to log
    if (gDriversFlags.VideoLoaded) {
      InitScreen(FALSE);
    } else {
      InitScreen(!gFirmwareClover); // ? FALSE : TRUE);
    }
    SetupScreen();
    //  DBG("InitScreen\n");
  } else {
    InitScreen(FALSE);
  }
  //Now we have to reinit handles
  Status = ReinitSelfLib();
  if (EFI_ERROR(Status)){
    DebugLog(2, " %r", Status);
    PauseForKey(L"Error reinit refit\n");
#ifdef ENABLE_SECURE_BOOT
    UninstallSecureBoot();
#endif // ENABLE_SECURE_BOOT
    return Status;
  }
  
//        DBG("reinit OK\n");
//  ZeroMem((VOID*)&gSettings, sizeof(SETTINGS_DATA));
  ZeroMem((VOID*)&gGraphics[0], sizeof(GFX_PROPERTIES) * 4);
  
//  DumpBiosMemoryMap();

  GuiEventsInitialize();
  
  GetCPUProperties();
  if (!gSettings.EnabledCores) {
    gSettings.EnabledCores = gCPUStructure.Cores;
  }
//  DBG("GetCPUProperties OK\n");
  GetDevices();
 //     DBG("GetDevices OK\n");
 // if (!GlobalConfig.FastBoot) {
  DBG("ScanSPD() start\n");
  ScanSPD();
  DBG("ScanSPD() end\n");
 // }
 //       DBG("ScanSPD OK\n");
  SetPrivateVarProto();
//        DBG("SetPrivateVarProto OK\n");
  GetDefaultSettings();
  DBG("Calibrated TSC frequency =%ld =%ldMHz\n", gCPUStructure.TSCCalibr, DivU64x32(gCPUStructure.TSCCalibr, Mega));
  if (gCPUStructure.TSCCalibr > 200000000ULL) {  //200MHz
    gCPUStructure.TSCFrequency = gCPUStructure.TSCCalibr;
  }
  
  gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
  gCPUStructure.FSBFrequency = DivU64x32(MultU64x32(gCPUStructure.CPUFrequency, 10),
                                         (gCPUStructure.MaxRatio == 0) ? 1 : gCPUStructure.MaxRatio);
  gCPUStructure.ExternalClock = (UINT32)DivU64x32(gCPUStructure.FSBFrequency, kilo);
  gCPUStructure.MaxSpeed = (UINT32)DivU64x32(gCPUStructure.TSCFrequency, Mega);

  //Second step. Load config.plist into gSettings
  for (i=0; i<2; i++) {
    if (gConfigDict[i]) {
      Status = GetUserSettings(SelfRootDir, gConfigDict[i]);
      if (EFI_ERROR(Status)) {
        DBG("Error in Second part of settings%d: %r\n", i, Status);
      }
    }
  }
 //       DBG("GetUserSettings OK\n");
  dropDSM = 0xFFFF; //by default we drop all OEM _DSM. They have no sense for us.
  if (defDSM) {
    dropDSM = gSettings.DropOEM_DSM;   //if set by user
  }  
  // Load any extra SMBIOS information
  if (!EFI_ERROR(LoadUserSettings(SelfRootDir, L"smbios", &smbiosTags)) && (smbiosTags != NULL)) {
    TagPtr dictPointer = GetProperty(smbiosTags,"SMBIOS");
    if (dictPointer) {
      ParseSMBIOSSettings(dictPointer);
    } else {
      DBG("Invalid smbios.plist, not overriding config.plist!\n");
    }
  } else {
    DBG("smbios.plist not found, not overriding config.plist\n");
  }
  
  if (!gFirmwareClover && 
      !gDriversFlags.EmuVariableLoaded &&
      !GlobalConfig.IgnoreNVRAMBoot &&
      GlobalConfig.Timeout == 0 && !ReadAllKeyStrokes()) {
// UEFI boot: get gEfiBootDeviceGuid from NVRAM.
// if present, ScanVolumes() will skip scanning other volumes
// in the first run.
// this speeds up loading of default OSX volume.
     GetEfiBootDeviceFromNvram();
//    DBG("GetEfiBootDeviceFromNvram()\n");
  }
  AfterTool = FALSE;
  gGuiIsReady = TRUE;
  do {
    //     PauseForKey(L"Enter main cycle");
    //    DBG("Enter main cycle\n");

    MainMenu.EntryCount = 0;
    OptionMenu.EntryCount = 0;
    ScanVolumes();
    //   DBG("ScanVolumes()\n");

    // as soon as we have Volumes, find latest nvram.plist and copy it to RT vars
    if (!AfterTool) {
      if (gFirmwareClover || gDriversFlags.EmuVariableLoaded) {
        PutNvramPlistToRtVars();
      }
    }

    if (!GlobalConfig.FastBoot) {
 //     GetListOfThemes();
      if (gThemeNeedInit) {
        InitTheme(TRUE, &Now);
        gThemeNeedInit = FALSE;
      } else if (gThemeChanged) {
        InitTheme(FALSE, NULL);
        FreeMenu(&OptionMenu);
      }
      gThemeChanged = FALSE;
      DBG("Choosing theme %s\n", GlobalConfig.Theme); 

      //now it is a time to set RtVariables
      SetVariablesFromNvram();
      FillInputs();
      // scan for loaders and tools, add then to the menu
      if (GlobalConfig.LegacyFirst){
        //DBG("scan legacy first\n");
        AddCustomLegacy();
        if (!GlobalConfig.NoLegacy) {
          ScanLegacy();
        }
      }
    }

    // Add custom entries
    AddCustomEntries();

    if (gSettings.DisableEntryScan) {
      DBG("Entry scan disabled\n");
    } else {
      //ScanUEFIBootOptions(FALSE);
      ScanLoader();
      //        DBG("ScanLoader OK\n");
    }

    if (!GlobalConfig.FastBoot) {

      if (!GlobalConfig.LegacyFirst) {
        //      DBG("scan legacy second\n");
        AddCustomLegacy();
        if (!GlobalConfig.NoLegacy) {
          ScanLegacy();
        }
        //      DBG("ScanLegacy()\n");
      }

      // fixed other menu entries
      //               DBG("FillInputs OK\n");

      if (!(GlobalConfig.DisableFlags & HIDEUI_FLAG_TOOLS)) {
        AddCustomTool();
        if (!gSettings.DisableToolScan) {
          ScanTool();
#ifdef ENABLE_SECURE_BOOT
          // Check for secure boot setup mode
          AddSecureBootTool();
#endif // ENABLE_SECURE_BOOT
        }
        //      DBG("ScanTool()\n");
      }

      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_FUNCS)) {
        MenuEntryOptions.Image = BuiltinIcon(BUILTIN_ICON_FUNC_OPTIONS);
        AddMenuEntry(&MainMenu, &MenuEntryOptions);
        MenuEntryAbout.Image = BuiltinIcon(BUILTIN_ICON_FUNC_ABOUT);
        AddMenuEntry(&MainMenu, &MenuEntryAbout);
      }

      if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_FUNCS) || MainMenu.EntryCount == 0) {
        MenuEntryReset.Image = BuiltinIcon(BUILTIN_ICON_FUNC_RESET);
        //    DBG("Reset.Image->Width=%d\n", MenuEntryReset.Image->Width);
        AddMenuEntry(&MainMenu, &MenuEntryReset);
        MenuEntryShutdown.Image = BuiltinIcon(BUILTIN_ICON_FUNC_SHUTDOWN);
        //    DBG("Shutdown.Image->Width=%d\n", MenuEntryShutdown.Image->Width);
        AddMenuEntry(&MainMenu, &MenuEntryShutdown);
      }
    }
    // wait for user ACK when there were errors
    FinishTextScreen(FALSE);
    //   DBG("FinishTextScreen()\n");

    DefaultIndex = FindDefaultEntry();
    //    DBG("FindDefaultEntry()\n");
      DBG("DefaultIndex=%d and MainMenu.EntryCount=%d\n", DefaultIndex, MainMenu.EntryCount);
    if ((DefaultIndex >= 0) && (DefaultIndex < (INTN)MainMenu.EntryCount)) {
      DefaultEntry = MainMenu.Entries[DefaultIndex];
    } else {
      DefaultEntry = NULL;
    }

    //    MainMenu.TimeoutSeconds = GlobalConfig.Timeout >= 0 ? GlobalConfig.Timeout : 0;
    if (GlobalConfig.FastBoot && DefaultEntry) {
      StartLoader((LOADER_ENTRY *)DefaultEntry);
      MainLoopRunning = FALSE;
      GlobalConfig.FastBoot = FALSE; //Hmm... will never be here
    }
    MainAnime = GetAnime(&MainMenu);
    MainLoopRunning = TRUE;

    // PauseForKey(L"Enter main loop");

    AfterTool = FALSE;
    gEvent = 0; //clear to cancel loop
    while (MainLoopRunning) {
      if (GlobalConfig.Timeout == 0 && DefaultEntry != NULL && !ReadAllKeyStrokes()) {
        // go strait to DefaultVolume loading
        MenuExit = MENU_EXIT_TIMEOUT;
      } else {
        MainMenu.AnimeRun = MainAnime;
        MenuExit = RunMainMenu(&MainMenu, DefaultIndex, &ChosenEntry);
      }

      // disable default boot - have sense only in the first run
      GlobalConfig.Timeout = -1;

      //    DBG("out from menu\n");
      if ((DefaultEntry != NULL) && (MenuExit == MENU_EXIT_TIMEOUT)) {
        //      DBG("boot by timeout\n");
        if (DefaultEntry->Tag == TAG_LOADER) {
          StartLoader((LOADER_ENTRY *)DefaultEntry);
        } else if (DefaultEntry->Tag == TAG_LEGACY){
          StartLegacy((LEGACY_ENTRY *)DefaultEntry);
        }
        // if something goes wrong - break main loop to reinit volumes
        break;
      }

      if (MenuExit == MENU_EXIT_OPTIONS){
        gBootArgsChanged = FALSE;
        OptionsMenu(&OptionEntry);
        if (gBootArgsChanged) {
          AfterTool = TRUE;
          MainLoopRunning = FALSE;
          break;
        }
        continue;
      }

      if (MenuExit == MENU_EXIT_HELP){
        HelpRefit();
        continue;
      }

      // EjectVolume
      if (MenuExit == MENU_EXIT_EJECT){
        if ((ChosenEntry->Tag == TAG_LOADER) ||
            (ChosenEntry->Tag == TAG_LEGACY)) {
          Status = EjectVolume(((LOADER_ENTRY *)ChosenEntry)->Volume);
          if (!EFI_ERROR(Status)) {
            break; //main loop is broken so Reinit all
          }
        }
        continue;
      }

      // Hide toggle
      if (MenuExit == MENU_EXIT_HIDE_TOGGLE) {
        gSettings.ShowHiddenEntries = !gSettings.ShowHiddenEntries;
        AfterTool = TRUE;
        break;
      }

      // We don't allow exiting the main menu with the Escape key.
      if (MenuExit == MENU_EXIT_ESCAPE){
  //      AfterTool = TRUE;
        break;   //refresh main menu
        //           continue;
      }

      switch (ChosenEntry->Tag) {

        case TAG_RESET:    // Restart
          // Attempt warm reboot
          gRS->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
          // Warm reboot may not be supported attempt cold reboot
          gRS->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
          // Terminate the screen and just exit
          TerminateScreen();
          MainLoopRunning = FALSE;
          ReinitDesktop = FALSE;
          AfterTool = TRUE;
          break;

        case TAG_SHUTDOWN: // It is not Shut Down, it is Exit from Clover
          TerminateScreen();
          //         gRS->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          MainLoopRunning = FALSE;   // just in case we get this far
          ReinitDesktop = FALSE;
          AfterTool = TRUE;
          break;

        case TAG_OPTIONS:    // Options like KernelFlags, DSDTname etc.
          gBootArgsChanged = FALSE;
          OptionsMenu(&OptionEntry);
          if (gBootArgsChanged)
            AfterTool = TRUE;
          if (gBootArgsChanged || gThemeChanged) // If theme has changed reinit the desktop
            MainLoopRunning = FALSE;
          break;

        case TAG_ABOUT:    // About rEFIt
          AboutRefit();
          break;

        case TAG_LOADER:   // Boot OS via .EFI loader
          StartLoader((LOADER_ENTRY *)ChosenEntry);
          break;

        case TAG_LEGACY:   // Boot legacy OS
          if (StrCmp(gST->FirmwareVendor, L"Phoenix Technologies Ltd.") == 0 &&
              gST->Hdr.Revision >> 16 == 2 && (gST->Hdr.Revision & ((1 << 16) - 1)) == 0){
            // Phoenix SecureCore Tiano 2.0 can't properly initiate LegacyBios protocol when called externally
            // which results in "Operating System not found" message coming from BIOS
            // in this case just quit Clover to enter BIOS again
            TerminateScreen();
            MainLoopRunning = FALSE;
            ReinitDesktop = FALSE;
            AfterTool = TRUE;           
          } else {
            StartLegacy((LEGACY_ENTRY *)ChosenEntry);
          }
          break;

        case TAG_TOOL:     // Start a EFI tool
          StartTool((LOADER_ENTRY *)ChosenEntry);
          TerminateScreen(); //does not happen
          //   return EFI_SUCCESS;
          //  BdsLibConnectAllDriversToAllControllers();
          //    PauseForKey(L"Returned from StartTool\n");
          MainLoopRunning = FALSE;
          AfterTool = TRUE;
          break;

#ifdef ENABLE_SECURE_BOOT
        case TAG_SECURE_BOOT: // Try to enable secure boot
          EnableSecureBoot();
          MainLoopRunning = FALSE;
          AfterTool = TRUE;
          break;

        case TAG_SECURE_BOOT_CONFIG: // Configure secure boot
          MainLoopRunning = !ConfigureSecureBoot();
          AfterTool = TRUE;
          break;
#endif // ENABLE_SECURE_BOOT

        case TAG_CLOVER:     // Clover options
          LoaderEntry = (LOADER_ENTRY *)ChosenEntry;
          if (LoaderEntry->LoadOptions != NULL) {

            // we are uninstalling in case user selected Clover Options and EmuVar is installed
            // because adding bios boot option requires access to real nvram
            if (gEmuVariableControl != NULL) {
              gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
            }

            if (StrStr(LoaderEntry->LoadOptions, L"BO-ADD") != NULL) {
              PrintBootOptions(FALSE);
              Status = AddBootOptionForFile (LoaderEntry->Volume->DeviceHandle,
                                             LoaderEntry->LoaderPath,
                                             TRUE,
                                             L"Clover OS X Boot",
                                             0,
                                             NULL
                                             );
              PrintBootOptions(FALSE);
            } else if (StrStr(LoaderEntry->LoadOptions, L"BO-REMOVE-ALL") != NULL) {
              PrintBootOptions(FALSE);
              DeleteBootOptionsContainingFile (L"CLOVERX64.efi");
              DeleteBootOptionsContainingFile (L"CLOVERIA32.efi");
              PrintBootOptions(FALSE);
            } else if (StrStr(LoaderEntry->LoadOptions, L"BO-REMOVE") != NULL) {
              PrintBootOptions(FALSE);
              Status = DeleteBootOptionForFile (LoaderEntry->Volume->DeviceHandle,
                                                LoaderEntry->LoaderPath
                                                );
              PrintBootOptions(FALSE);
            } else if (StrStr(LoaderEntry->LoadOptions, L"BO-PRINT") != NULL) {
              PrintBootOptions(TRUE);
            }

/* we don't need to restore emulation after user has finished with this menu
            if (gEmuVariableControl != NULL) {
              gEmuVariableControl->InstallEmulation(gEmuVariableControl);
            }
*/
          }
          MainLoopRunning = FALSE;
          AfterTool = TRUE;
          break;

      } //switch
    } //MainLoopRunning
    UninitRefitLib();
    if (!AfterTool) {
      //   PauseForKey(L"After uninit");
      //reconnectAll
      if (!gFirmwareClover) {
        BdsLibConnectAllEfi();
      }
      else {
        DBG("ConnectAll after refresh menu\n");
        BdsLibConnectAllDriversToAllControllers();
      }
      //  ReinitRefitLib();
      //    PauseForKey(L"After ReinitRefitLib");
    }
    if (ReinitDesktop) {
      ReinitSelfLib();
    }
    //    PauseForKey(L"After ReinitSelfLib");
  } while (ReinitDesktop);
  
  // If we end up here, things have gone wrong. Try to reboot, and if that
  // fails, go into an endless loop.
  //Slice - NO!!! Return to EFI GUI
  //   gRS->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
  //   EndlessIdleLoop();

#ifdef ENABLE_SECURE_BOOT
  UninstallSecureBoot();
#endif // ENABLE_SECURE_BOOT

  // Unload EmuVariable before returning to EFI GUI, as it should not be present when booting other Operating Systems.
  // This seems critical in some UEFI implementations, such as Phoenix UEFI 2.0
  if (gEmuVariableControl != NULL) {
    gEmuVariableControl->UninstallEmulation(gEmuVariableControl);
  }  
  return EFI_SUCCESS;
}
