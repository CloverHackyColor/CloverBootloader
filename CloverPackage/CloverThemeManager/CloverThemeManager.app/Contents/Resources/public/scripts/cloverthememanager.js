// A script for Clover Theme Manager
// Copyright (C) 2014-2017 Blackosx
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//Version=0.76.8

var gTmpDir = "/tmp/CloverThemeManager";
var gLogBashToJs = "bashToJs";
var gInstalledThemeListStr="";    // This is used for drawing upper shadow on installed theme bands
var gUpdateThemeListStr="";       // This is used for drawing upper shadow on update theme bands

//-------------------------------------------------------------------------------------
// On initial load
$(document).ready(function() {

    SetListingThemesMessageAndProgressBarPosition();
    disableInterface();
    hideButtons();
    HideProgressBar();
    readBashToJsMessageFile();
    ResetButtonsAndBandsToDefault();
    HideUpdateAllButton();

});


//-------------------------------------------------------------------------------------
// Listen to keyboard
document.addEventListener('keydown', function(event) {

    // Only continue if the keystroke was not combined with the Command key
    if (!event.metaKey) {

        // Check for key press 0-9 or A-Z and scroll to nearest theme based on key pressed.
        if ((event.keyCode >= 48 && event.keyCode <= 57) || (event.keyCode >= 65 && event.keyCode <= 90 )) {

            var keyPressed = event.keyCode;
            var searchKey = String.fromCharCode(keyPressed);

            // find lowercase key also
            if (event.keyCode >= 65 && event.keyCode <= 90 ) {

                var searchKeyLowerCase = String.fromCharCode(keyPressed + 32)

            } else {

                var searchKeyLowerCase = searchKey;

            }

            // Loop through each theme name and find first
            $('.themeTitle').each(function(){

                // get text before first <br>		
                var splitter = $(this).html().split('<br>');

                // Compare first char of theme name
                if ( splitter[0].charAt(0) == searchKey || splitter[0].charAt(0) == searchKeyLowerCase ) {

                    var firstThemeEntry = $("#ThemeBand").first();
                    var posfirstThemeEntry = firstThemeEntry.position(); // in relation to bottom of header. 0 = Alienware. 52 = BGM, christmas = 390, mac = 858

                    var thisThemeEntry = $(this).closest("#ThemeBand");
                    var offset = thisThemeEntry.offset();

                    $('#content').animate({
                        scrollTop: offset.top - 99 - posfirstThemeEntry.top
                    });

                    return false; // stop. don't continue looping through the each

                }

            })

        }


        // Check for left arrow

        if ( event.keyCode == 37 ) {
    
            // Check if large expanded previews are shown

            if ($("#preview_Toggle_Button").text().indexOf("Collapse") >= 0) {

                SetShowHidePreviews("Hide");
                // Send a message to the bash script to record user choice in prefs
                macgap.app.launch("CTM_hidePreviews");

            } else {

                ChangeThumbnailSize('smaller');
        
                // Check for visible thumbnails
                var showHideButtonText = $("#BandsHeightToggleButton").text();
                var currentThumbWidth = $(".thumbnail img").first().width();
        
                if (currentThumbWidth == 100 ) {
                    SetThemeBandHeight("Hide");
                    // Send a message to the bash script to record user choice in prefs
                    macgap.app.launch("CTM_showThumbails");
                }

            }

        }


        // Check for right arrow

        if ( event.keyCode == 39 ) {

            // Check for visible thumbnails
            var showHideButtonText = $("#BandsHeightToggleButton").text();
            if (showHideButtonText.indexOf("Show") >= 0) {

                SetThemeBandHeight("Show");   

                // Send a message to the bash script to record user choice in prefs
                macgap.app.launch("CTM_hideThumbails");

            } else {

                // Are thumbnails already at largest size?
                if ( $(".thumbnail img").first().width() == 175 ) {

                    // Show Expanded previews
                    SetShowHidePreviews("Show");

                    // Send a message to the bash script to record user choice in prefs
                    macgap.app.launch("CTM_showPreviews");

                } else {

                    ChangeThumbnailSize('larger');

                }

            }
        
        }


        // End Button - Scroll to bottom

        if ( event.keyCode == 35 ) {
    
            $("#content").animate({ scrollTop: $('#content').prop("scrollHeight")}, 1000);

        }


        // Home Button - Scroll to top

        if ( event.keyCode == 36 ) {

            $('#content').animate({
               scrollTop: 0
            }, 'slow');

        }


        // Check for Enter / Return key

        if ( event.keyCode == 13 ) {

            focusedItem=$("*:focus").attr("id");

            switch(focusedItem) {

                case "BootLogTitleBar":
                ActionShowHideBootlog();
                break;

                case "RefreshButton":
                $('#partitionSelect').change();
                break;

                case "OpenButton":
                macgap.app.launch("OpenPath");
                break;

                case "EspButton":
                ActionMountESP();
                break;

                case "BandsHeightToggleButton":
                ActionShowHideThumbnailsButton();
                break;

                case "thumbSizeSmaller":
                ChangeThumbnailSize('smaller');
                break;

                case "thumbSizeLarger":
                ChangeThumbnailSize('larger');
                break;

                case "preview_Toggle_Button":
                ActionShowHideExpandedPreviews();
                break;

                case "ShowHideUpdateAllButton":
                ActionUpdateAll();
                break;

                case "ShowHideToggleButton":
                ActionShowHideInstalledThemes();
                break;

                case "content":
                break;

                case "installedThemeDropDownNvram":
                ActionChangedDropDownNvram();
                break;

                case "installedThemeDropDownNvram":
                ActionChangedDropDownConfigP();
                break;

                default:
                // code to be executed if n is different from case 1 and 2
            }
            
            // Is messagebox showing?
            // Read position of box to see where it is
            var position = $('#box').position();
            if (position.top > 0) {
                CloseMessageBox();
                enableInterface();
            }

        }

    } else {

        // Command key was also pressed

        // Check for upper case or lower case e
        if ((event.keyCode == 69) || (event.keyCode == 101)) {

            ActionShowHideExpandedPreviews();

        }

        // Check for upper case or lower case i
        if ((event.keyCode == 73) || (event.keyCode == 105)) {

            ActionShowHideInstalledThemes();

        }

        // Check for upper case or lower case r
        if ((event.keyCode == 82) || (event.keyCode == 114)) {

            $('#partitionSelect').change();

        }

    }

});

//-------------------------------------------------------------------------------------
// Called when the process is to close.
function terminate() {
    clearTimeout(timerReadMessageFile);
    macgap.notice.close("*"); // Remove all notifications sent by app
    macgap.app.terminate();    
}

//-------------------------------------------------------------------------------------
// looks for a file and if found, returns the contents
function GetFileContents(filename)
{
    xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET",gTmpDir+"/"+filename,false);
    xmlhttp.send(null);

    fileContent = xmlhttp.responseText;

    if (fileContent != "" ) {
        return fileContent;
    } else {
        return 0;
    }
}

/*
// Called when returning back from help page
//-------------------------------------------------------------------------------------
$(window).bind("pageshow", function(event) {
    if (event.originalEvent.persisted) {
        alert("From bfcache");
        // check if macgap API works or generates an error
        if ('macgap.app.launch' in window) {
            try {
                alert("trying");
            } catch (e) {
                alert("catch");
            }     
            alert("macgap working");
        } else {
            try {
                alert("trying");
                macgap.app.activate(); 
            } catch (e) {
                alert("catch");
            }
            alert("macgap not working");
            //window.location.reload();
        }
    }
});
*/

//-------------------------------------------------------------------------------------
// Check for incoming messages from bash script
function readBashToJsMessageFile()
{
    incoming=GetFileContents(gLogBashToJs);
    if (incoming != 0) {
    
        // Split settings by newline
        var incoming = incoming.split('\n');
        
        // Take first line
        var firstLine = (incoming[0]);
    
        // Split firstLine by @
        var firstLineSplit = (firstLine).split('‡');
        var firstLineCommand = (firstLineSplit[0]);
        
        // match command against known ones.
        switch(firstLineCommand) {
            case "Target":
                // Bash sends: "Target@$entry"
                macgap.app.removeMessage(firstLine);
                setTargetThemePath(firstLineSplit[1]);
                break;
            case "NotExist":
                // Bash Sends: "NotExist@${TARGET_THEME_DIR_DEVICE}@${TARGET_THEME_DIR}@entry"
                macgap.app.removeMessage(firstLine);
                presentNotExistsDialog(firstLineSplit[1],firstLineSplit[2],firstLineSplit[3]);
                break;
            case "InstalledThemes":
                // Bash sends: "InstalledThemes@${installedThemeStr}@"
                // where $installedThemeStr is a comma separated string.
                macgap.app.removeMessage(firstLine);
                updateBandsWithInstalledThemes(firstLineSplit[1]);
                // remember installed theme list
                gInstalledThemeListStr=firstLineSplit[1];
                // Honour users choice of which themes to view (All or just Installed)
                GetShowHideButtonStateAndUpdateUI();
                break;
            case "FreeSpace":
                // Bash sends: "FreeSpace@${freeSpace}@"
                macgap.app.removeMessage(firstLine);
                actOnFreeSpace(firstLineSplit[1]);
                break;
            case "UpdateAvailThemes":
                // Bash sends: "UpdateAvailThemes@${updateAvailThemeStr}@"
                // where $updateAvailThemeStr is a comma separated string.
                macgap.app.removeMessage(firstLine);
                hideCheckingThemeUpdateProgressBar();
                ShowThemeUpdatesAvailableMessage(firstLineSplit[1]);
                ShowUpdateThemesInUI(firstLineSplit[1]);
                // remember installed theme list
                gUpdateThemeListStr=firstLineSplit[1];
                break;
            case "UnversionedThemes":
                // Bash sends: "UnversionedThemes@${unversionedThemeStr}@"
                macgap.app.removeMessage(firstLine);
                displayUnversionedThemes(firstLineSplit[1]);
                break;
            case "Nvram":
                // Bash sends: "Nvram@${themeName}@"
                macgap.app.removeMessage(firstLine);
                SetCurrentThemeEntry("Nvram",firstLineSplit[1]);
                break;
            //case "NvramP":
                // Bash sends: "NvramP@${themeName}@"
                //macgap.app.removeMessage(firstLine);
                //SetCurrentThemeEntry("NvramP",firstLineSplit[1]);
                //break;
            case "ConfigP":
                // Bash sends: "ConfigP@${themeName}@"
                macgap.app.removeMessage(firstLine);
                SetCurrentThemeEntry("ConfigP",firstLineSplit[1]);
                break;
            case "Success":
                // Bash sends: "Success@${passedAction}@$themeTitleToActOn"
                macgap.app.removeMessage(firstLine);
                themeActionSuccess(firstLineSplit[1],firstLineSplit[2]);
                break;
            case "UpdateAll":
                // Bash sends: "UpdateAll@Theme@${arr[$u]}"
                macgap.app.removeMessage(firstLine);
                themeActionUpdateAll(firstLineSplit[1],firstLineSplit[2]);
                break;
            case "Fail":
                // Bash sends: "Fail@${passedAction}@$themeTitleToActOn"
                macgap.app.removeMessage(firstLine);
                themeActionFail(firstLineSplit[1],firstLineSplit[2]);
                break;
            case "NoPathSelected":
                // Bash sends: "NoPathSelected@@"
                macgap.app.removeMessage(firstLine);
                HideFreeSpace();
                break;
            case "ThumbnailSize":
                // Bash sends: "ThumbnailSize@${gThumbSizeX}@${gThumbSizeY}"
                macgap.app.removeMessage(firstLine);
                SetThumbnailSize(firstLineSplit[1],firstLineSplit[2]);
                break;
            case "UnInstalledView":
                // Bash sends: "UnInstalledView@${gUISettingViewUnInstalled}@"
                macgap.app.removeMessage(firstLine);
                SetShowHideButton(firstLineSplit[1]);
                break;
            case "ThumbnailView":
                // Bash sends: "UnInstalledView@${gUISettingViewUnInstalled}@"
                macgap.app.removeMessage(firstLine);
                SetThemeBandHeight(firstLineSplit[1]);
                break;
            case "PreviewView":
                // Bash sends: "PreviewView@${gUISettingViewPreviews}@"
                macgap.app.removeMessage(firstLine);
                SetShowHidePreviews(firstLineSplit[1]);
                break;
            case "MessageESP":
                // Bash sends: "MessageESP@Mounted@${espMountedCount}"
                macgap.app.removeMessage(firstLine);
                UpdateMessageBox(firstLineSplit[1],firstLineSplit[2]);
                break;
            case "BootDevice":
                // Bash sends: "BootDevice@Mounted@${bootDeviceIdentifier}@${gBootDeviceMountPoint}" or "BootDevice@Failed@@"
                macgap.app.removeMessage(firstLine);
                ShowMessageBootDevice(firstLineSplit[1],firstLineSplit[2],firstLineSplit[3]);
                break;
            case "NewVolumeDropDown":
                // Bash sends: "NewVolumeDropDown@${newThemeList}"
                //${newThemeList} is a comma separated list
                macgap.app.removeMessage(firstLine);
                UpdateAndRefreshPartitionSelectMenu(firstLineSplit[1]);
                break;
            case "BootlogView":
                // Bash sends: "BootlogView@${gBootlogState}@"
                macgap.app.removeMessage(firstLine);
                SetBootLogState(firstLineSplit[1]);
                // Adjust footer height also
                //SetFooterHeight("IncludeCO");
                break;
            case "SetPrediction":
                // Bash sends: "SetPrediction@${themeToSend}@"
                macgap.app.removeMessage(firstLine);
                SetPredictionText(firstLineSplit[1]);
                break;
            case "ShowHideControlOptions":
                // Bash sends: "ShowHideControlOptions@Show or Hide@"
                macgap.app.removeMessage(firstLine);
                ShowHideControlOptions(firstLineSplit[1]);
                break;
            case "UpdateThemePaths":
                // Bash sends: "UpdateThemePaths@${gNvramPlistFullPath}@${gConfigPlistFullPath}@${mountpoint}"
                macgap.app.removeMessage(firstLine);
                UpdateThemePaths(firstLineSplit[1],firstLineSplit[2],firstLineSplit[3]);
                break;
            case "EnableInterface":
                // Bash sends: "EnableInterface@@"
                macgap.app.removeMessage(firstLine);
                enableInterface(); 
                break;
            case "CheckingThemeUpdates":
                // Bash sends: "EnableInterface@@"
                macgap.app.removeMessage(firstLine);
                showCheckingThemeUpdateProgressBar();
                break;
            default:
                alert("Found else:" + firstLine + " | This is a problem and the app may not function correctly. Please report this error");
                if(firstLine == "") {
                    macgap.app.removeMessage("");
                } else {
                    macgap.app.removeMessage(firstLine);
                }
                break;
        }
	    // recursively call function as long as file exists every 1/10th second.
        timerReadMessageFile = setTimeout(readBashToJsMessageFile, 100);
    } else {
        // recursively call function as long as file exists but at 1/2 second intervals
        timerReadMessageFile = setTimeout(readBashToJsMessageFile, 500);
    }
}

//-------------------------------------------------------------------------------------
//function PopulateThemeListArray()
//{
    // Get each 'id' of a 'div' with class 'versionControl' and store everything after 'indicator_' in an array. 
    //$(".versionControl").each(function(){ gThemeList.push(this.id.replace('indicator_','')); });
//}

//-------------------------------------------------------------------------------------
function setTargetThemePath(entry)
{
    $('#partitionSelect').val(entry);
    ResetButtonsAndBandsToDefault();
    if (entry != "-") {
        showButtons();
        // Show open button beside device dropdown
        $("#OpenPathButton").css("display","block");
        ShowFreeSpace();
    } else {
        $("#OpenPathButton").css("display","none");
        ShowMessageBoxClose();
        HideProgressBar();
    }
}

//-------------------------------------------------------------------------------------
function presentNotExistsDialog(uuid,path,id)
{
    if (uuid != "" & path != "") {
        ChangeMessageBoxHeaderColour("red");
        SetMessageBoxText("Attention:" ,"Previously used:<br>" + path + "<br>on volume with UUID<br>" + uuid + "<br>is no longer mounted.<br><br>Please choose a theme path.")
        ShowMessageBox();
        
        // Remove partition entry from dropdown menu
        if ( id != "-" ) {
            $("#partitionSelect option[value=" + id + "]").remove();
        }
    }
}

//-------------------------------------------------------------------------------------
function updateBandsWithInstalledThemes(themeList)
{
    if (themeList != "") {
        splitThemeList = (themeList).split(',');
        if (splitThemeList != "-") {

            showButtons();
            var unknownThemeCount=0;
            // Update only installed themes with uninstall buttons
            for (var t = 0; t < splitThemeList.length; t++) {
                // Does theme actually exist?
                // User could have their own theme installed which is not in the repo.
                if (!$("[id='button_" + splitThemeList[t] + "']").length) {
                    //alert(splitThemeList[t] + " does not exist in the repo");
                    unknownThemeCount++;
                }
                ChangeButtonAndBandToUnInstall(splitThemeList[t]);
            }

            // Has the user chosen to view only installed themes?
            var readButton = $("#ShowHideToggleButton").text();
            if (readButton.indexOf("Show") >= 0) {
                // Hide previews of uninstalled themes
                //ClosePreviewsForUninstalledThemes();
            }

            // Update number of installed themes
            if (splitThemeList != ",") { // This check needs verifying!! - is a single comma possible?
                $("#NumInstalledThemes").html(splitThemeList.length + "/" + $('div[id^=ThemeBand]').length);
                if (unknownThemeCount > 0){
                    // Change colour of textThemeCount class to orange
                    $("#NumInstalledThemes").css("color","#FFA500");
                    $("#NumInstalledThemesQuery").css("display","inline");
                } else {
                    $("#NumInstalledThemes").css("color","#FFF");
                    $("#NumInstalledThemesQuery").css("display","none");
                }
            } else {
                $("#NumInstalledThemes").html("0/" + $('div[id^=ThemeBand]').length);
            }

            // Populate control option dropdown menus.
            UpdateAndRefreshInstalledThemeDropDown(splitThemeList);
        } else {
            // Populate control option dropdown menus even though there are no themes.
            UpdateAndRefreshInstalledThemeDropDown("");
                    
            // Update number of installed themes
            $("#NumInstalledThemes").html("-/" + $('div[id^=ThemeBand]').length);
            // Reset colours and question mark incase previously shown.
            $("#NumInstalledThemes").css("color","#FFF");
            $("#NumInstalledThemesQuery").css("display","none");
        }
    } else {
        showButtons();
        // Populate control option dropdown menus even though there are no themes.
        UpdateAndRefreshInstalledThemeDropDown("");
        // No themes installed on this volume
        $("#NumInstalledThemes").html("0/" + $('div[id^=ThemeBand]').length);
        // Reset colours and question mark incase previously shown.
        $("#NumInstalledThemes").css("color","#FFF");
        $("#NumInstalledThemesQuery").css("display","none");
    }
}

//-------------------------------------------------------------------------------------
function actOnFreeSpace(availableSpace)
{
    if (availableSpace != "") {
        // Bash sends the size read from the result of df
        // This will look like 168M 
        // Is the last character a G?
        lastChar = availableSpace.slice(-1);

        if (lastChar == "K") {
            // change colour to red
            $(".textFreeSpace").css("color","#C00000");
        }
                
        if (lastChar == "M") {
            // Remove last character of string
            number = availableSpace.slice(0,-1);
            // round down
            number = Math.floor(number);
                    
            if(parseInt(number, 10) < parseInt(10, 10)) {
                // change colour to red
                $(".textFreeSpace").css("color","#C00000");
                        
                // Show user a low space warning message
                ChangeMessageBoxHeaderColour("red");                            
                SetMessageBoxText("Warning: Low Space","You only have " + number +"MB remaining on this volume. Installing another theme may fail!");
                ShowMessageBoxClose();
                ShowMessageBox();
            }
        }

        if (lastChar == "G") {
            // set to green as defined in the .css file
            $(".textFreeSpace").css("color","#3ef14b");
        }

        $(".textFreeSpace").text(availableSpace+"B");
    }
}

//-------------------------------------------------------------------------------------
function ShowThemeUpdatesAvailableMessage(themeList)
{    
    if (themeList != "") {
        disableInterface();
        localThemeUpdates = (themeList).split(',');
        if (localThemeUpdates != "") {
        
            var printString=("<br>");
        
            // Update only installed themes with uninstall buttons
            for (var t = 0; t < localThemeUpdates.length; t++) {
            
                if(localThemeUpdates[t] != "") {
                        
                    // Prepare text for pretty print
                    printString=(printString + "<br>" + localThemeUpdates[t]);
                
                    // Send native notification
                    sendNotification("Theme update available for " + localThemeUpdates[t] + ".");
                }
            }
                    
            // Show a message to the user
            ChangeMessageBoxHeaderColour("blue");                            
            SetMessageBoxText("Theme Updates:","There is an update available for: " + printString);
            ShowMessageBoxClose();
            ShowMessageBox();
        }
    }
}

//-------------------------------------------------------------------------------------
function ShowUpdateThemesInUI(themeList)
{    
    if (themeList != "") {
        localThemeUpdates = (themeList).split(',');
        if (localThemeUpdates != "") {
        
            // Update only installed themes with update buttons
            for (var t = 0; t < localThemeUpdates.length; t++) {
                if(localThemeUpdates[t] != "") {
                    ChangeButtonAndBandToUpdate(localThemeUpdates[t]);
                }
            }

            ShowUpdateAllButton();

        }
    }
}

//-------------------------------------------------------------------------------------
function displayUnversionedThemes(themeList)
{
    if (themeList != "") {
        unVersionedThemes = (themeList).split(',');
        if (unVersionedThemes != "") {
            for (var t = 0; t < unVersionedThemes.length; t++) {
                SetUnVersionedControlIndicator(unVersionedThemes[t]);
            }
        }
    }
}

//-------------------------------------------------------------------------------------
function SetCurrentThemeEntry(textToChange,themeName)
{
    if (textToChange == "Nvram") {
        //$('#ctEntryNvram').text(themeName);
        SetDropDownNvram(themeName);
    } else if (textToChange == "ConfigP") {
        //$('#ctEntryConfig').text(themeName);
        SetDropDownConfigP(themeName);
    }
}

//-------------------------------------------------------------------------------------
function SetDropDownNvram(themeName)
{
    // Note: This menu is populated by UpdateAndRefreshInstalledThemeDropDown()
    
    var themeNotInstalled=1
    if(themeName != "") {
        $('#installedThemeDropDownNvram option').each(function(){
            if(this.value == themeName) {
                // Note: Bash script sends '-' when no theme is set
                $("#installedThemeDropDownNvram").val(themeName);
                themeNotInstalled=0
            }
        });
        if(themeNotInstalled == 1) {
            $("#installedThemeDropDownNvram").val("-");
        }
    }
    
    checkEntryAndInsertRemoveOptionIfBlank($("#installedThemeDropDownNvram"));
}

/*
//-------------------------------------------------------------------------------------
function SetDropDownNvramP(themeName)
{
    // Note: This menu is populated by UpdateAndRefreshInstalledThemeDropDown()
    
    var themeNotInstalled=1
    if(themeName != "") {
        $('#installedThemeDropDownNvramP option').each(function(){
            if(this.value == themeName) {
                // Note: Bash script sends '-' when no theme is set
                $("#installedThemeDropDownNvramP").val(themeName);
                themeNotInstalled=0
            }
        });  
        if(themeNotInstalled == 1) {
            $("#installedThemeDropDownNvramP").val("-");
        }
    }
}*/

//-------------------------------------------------------------------------------------
function SetDropDownConfigP(themeName)
{
    // Note: This menu is populated by UpdateAndRefreshInstalledThemeDropDown()
    
    var themeNotInstalled=1
    if(themeName != "") {
        $('#installedThemeDropDownConfigP option').each(function(){
            if(this.value == themeName) {
                // Note: Bash script sends '-' when no theme is set
                $("#installedThemeDropDownConfigP").val(themeName);
                themeNotInstalled=0
            }
        });  
        if(themeNotInstalled == 1) {
            $("#installedThemeDropDownConfigP").val("-");
        }
    }
    
    checkEntryAndInsertRemoveOptionIfBlank($("#installedThemeDropDownConfigP"));
    
}

//-------------------------------------------------------------------------------------
function themeActionSuccess(action,themeName)
{

    if (action != "" & themeName != "") {

        // Present dialog to the user
        ChangeMessageBoxHeaderColour("green");

        // Correct language - Install, UnInstall, Update to Installed, UnInstalled, Updated
        if (action == "Update") {
            printText="Updated";
            // theme was updated, remove theme name from global update string
            gUpdateThemeListStr = gUpdateThemeListStr.replace(themeName+',','');

        } else if (action == "UnInstall") {
            // theme was uninstalled - remove theme name from global update string (if there)
            gUpdateThemeListStr = gUpdateThemeListStr.replace(themeName+',','');

        } else {
            printText=(action + "ed");
        }

        // Print message
        HideProgressBar();
        SetMessageBoxText("Success:","The theme " + themeName + " was successfully " + printText + ".");
        ShowMessageBoxClose();
        
        // Send native notification
        sendNotification("Success: The theme " + themeName + " was successfully " + printText + ".");

        // Reset current theme list, bands and buttons
        ResetButtonsAndBandsToDefault();
        hideButtons();

        // Show Overlay Box to stop user interacting with buttons
        disableInterface();

    }
}

//-------------------------------------------------------------------------------------
function themeActionFail(action,themeName)
{
    if (action != "" & themeName != "") {
    
        // Present dialog to the user
        ChangeMessageBoxHeaderColour("red");

        // Correct language - Install, UnInstall, Update to Installed, UnInstalled, Updated
        if (action == "Update") {
            printText="Updated";
        } else {
            printText=(action + "ed");
        }
        
        // Print message
        HideProgressBar();
        SetMessageBoxText("Failure:","The theme " + themeName + " was not " + printText + ".");
        ShowMessageBoxClose();
        
        // Send native notification
        sendNotification("Failure: The theme " + themeName + " was not " + printText + ".");
    }
}

//-------------------------------------------------------------------------------------
function themeActionUpdateAll(message,themeName)
{
    if (message != "" & themeName != "") {
    
        // message box should already be showing.. Maybe add check?

        ChangeMessageBoxHeaderColour("green");
        ShowProgressBar();
        HideMessageBoxClose();

        if (message == "Theme") {

            // theme was updated, remove theme name from global update string
            gUpdateThemeListStr = gUpdateThemeListStr.replace(themeName+',','');

            // Change dialog text
            SetMessageBoxText("Success:","The theme " + themeName + " was successfully Updated.");

            // Send native notification
            sendNotification("Success: The theme " + themeName + " was successfully Updated.");

        } else if (message == "Complete"){

            HideProgressBar();

            // Change dialog text
            SetMessageBoxText("Completed:","All updates completed successfully.");

            ShowMessageBoxClose();

            // Reset current theme list, bands and buttons
            ResetButtonsAndBandsToDefault();
            hideButtons();

        } else {

            alert ("Not supposed to reach here");

        }

    }
}


//-------------------------------------------------------------------------------------
function disableInterface()
{
    // Disable path drop down menu and open button until updates have been checked.
    // Will re-enable in CheckForUpdatesThemeList();
    $("#partitionSelect").prop("disabled", true);
    $("#OpenPathButton").prop("disabled", true);
            
    // Display message to notify checking for updates
    $("#ListingThemesMessage").css("display","block");
            
    // Show Overlay Box to stop user interacting with buttons
    DisplayOverlayTwoBox();

    // Show open button beside device dropdown
    $("#OpenPathButton").css("display","block");
}      

//-------------------------------------------------------------------------------------
function enableInterface()
{
    // Re-enable drop down menu and open button
    $("#partitionSelect").prop("disabled", false);
    $("#OpenPathButton").prop("disabled", false);
            
    // Hide message to notify checking for updates
    $("#ListingThemesMessage").css("display","none");
            
    // Hide Overlay Box to allow user to interact with buttons
    HideOverlayTwoBox();    
}        

//-------------------------------------------------------------------------------------
function showCheckingThemeUpdateProgressBar()
{
    // Disable path drop down menu and open button until updates have been checked.
    // Will re-enable in CheckForUpdatesThemeList();
    $("#partitionSelect").prop("disabled", true);
    $("#OpenPathButton").prop("disabled", true);
            
    // Display message to notify checking for updates
    $("#CheckingUpdatesMessage").css("display","block");
}

//-------------------------------------------------------------------------------------
function hideCheckingThemeUpdateProgressBar()
{
    // Disable path drop down menu and open button until updates have been checked.
    // Will re-enable in CheckForUpdatesThemeList();
    $("#partitionSelect").prop("disabled", false);
    $("#OpenPathButton").prop("disabled", false);
            
    // Display message to notify checking for updates
    $("#CheckingUpdatesMessage").css("display","none");
}  


//-------------------------------------------------------------------------------------
function SetThumbnailSize(width,height)
{
    // function ChangeThumbnailSize() changes thumb size +/- 25px
    // So call it number of times necessary to achieve wanted width
        
    if (width != "" & height != "") {
        var currentThumbWidth = $(".thumbnail img").first().width();
        if (currentThumbWidth > width) {
            var timesDifference=((currentThumbWidth-width)/25)
            for (s = 0; s < timesDifference; s++) {
                 ChangeThumbnailSize('smaller');
            }
        } else if (currentThumbWidth < width) {
            var timesDifference=((width-currentThumbWidth)/25)
            for (s = 0; s < timesDifference; s++) {
                ChangeThumbnailSize('larger');
            }
        }
    }
}

//-------------------------------------------------------------------------------------
$(function()
{
    //-----------------------------------------------------
    // On changing the 'partitionSelect' dropdown menu.
    $("#partitionSelect").change(function() {
        var selectedPartition=$("#partitionSelect").val();

        // Send massage to bash script to notify change of path.
        // The bash script will get, and return, a list of installed themes for this path.
        // The bash script will then check if any of the themes have available updates.
        
        // Send a message to the bash script to fetch new theme list.
        macgap.app.launch("CTM_selectedPartition‡" + selectedPartition);
        
        // The bash script will send back:
        // 1 - A list of themes.
        // 2 - Any themes from user prefs marked with an update available.
        // 3 - Any themes flagged as orphaned (ie. not parent bare clone).
        // These will be picked up by function readBashToJsMessageFile()

        // As long as the user did not select the 'Please Choose' menu option.
        if (selectedPartition != "-") {
        
            // Show Overlay Box to stop user interacting with buttons
            disableInterface();
            
            // Show the Free Space text
            ShowFreeSpace();

        } else {
            // Hide open button beside device dropdown
            $("#OpenPathButton").css("display","none");

            // Hide the Free Space text
            HideFreeSpace();
        }
        
        // Honour users choice of which themes to view (All or just Installed)
        GetShowHideButtonStateAndUpdateUI();
        
        HideUpdateAllButton();
        
        // Reset current theme list, bands and buttons
        ResetButtonsAndBandsToDefault();
        hideButtons();
        
        // show all themes, even if asked to Show Installed
        $(".accordion").css("display","block");
        
        // Reset global var for updated themes
        gUpdateThemeListStr="";
    });
    
    //-----------------------------------------------------
    // On pressing the Rescan Boot Device button
    $("#RescanBootDeviceButton").on('click', function() {
        macgap.app.launch("RescanBootDevice");
    });

    //-----------------------------------------------------
    // On pressing the refresh path button
    $("#RefreshPathButton").on('click', function() {
        // select current dropdown selection
        $('#partitionSelect').change();
    });

    //-----------------------------------------------------
    // On pressing the open path button
    $("#OpenPathButton").on('click', function() {
        macgap.app.launch("OpenPath");
    });
    
    //-----------------------------------------------------
    // On pressing the mount ESP button
    $("#MountEspButton").on('click', function() {
        ActionMountESP();
    });
    
    //-----------------------------------------------------
    // On pressing a theme button (Install,UnInstall,Update)
    $("[id^=button_]").on('click', function() {
        var pressedButton=$(this).attr('id');
        var currentStatus=$(this).attr('class');
        RespondToButtonPress(pressedButton,currentStatus);
    });

    //-----------------------------------------------------
    // On clicking the message box close button
    $('#boxclose').click(function(){
        CloseMessageBox();
        enableInterface();
    });
    
    //-----------------------------------------------------
    // On clicking an accordion entry
    $('.accordion').click(function(checkWhat) {
        // Reveal large screenshot image only if the install/uninstall/update
        // were not clicked.
        if (checkWhat.target.id.substring(0,6) != "button") {
        
            var hidden = $(this).closest("#ThemeBand").nextAll('[class="accordionContent"]').first().is(":hidden");
            if (!hidden) {
                $(this)                                  // Start with current
                .closest("#ThemeBand")                   // Traverse up the DOM to the ThemeBand div
                .nextAll('[class="accordionContent"]')   // find the next siblings with class .accordionContent
                .first()                                 // just use first one
                .slideUp('fast');                        // Slide up
            } else {
                $(this)
                .closest("#ThemeBand")
                .nextAll('[class="accordionContent"]')
                .first()
                .slideToggle('fast');
            }
        } else {
            return;
        }
    });
    
    //-----------------------------------------------------
    // On clicking the bootlog band
    $('#BootLogTitleBar').click(function() {
        ActionShowHideBootlog();
    });
    
    //-----------------------------------------------------
    // On clicking the Hide / Show Thumbnails button
    $("#BandsHeightToggleButton").on('click', function() {
        ActionShowHideThumbnailsButton();
    });

    //-----------------------------------------------------
    // On clicking the Toggle Preview button - change to Expand/Collapse All
    $("#preview_Toggle_Button").click(function() {
        ActionShowHideExpandedPreviews();
    });	

    //-----------------------------------------------------
    // On clicking the Update All button
    $("#ShowHideUpdateAllButton").on('click', function() {
        ActionUpdateAll();
    });

    //-----------------------------------------------------
    // On clicking the Show Installed / Show All button
    $("#ShowHideToggleButton").on('click', function() {
        ActionShowHideInstalledThemes();
    });

    //-----------------------------------------------------
    // On clicking the Thumbnail Smaller button
    $("#thumbSizeSmaller").on('click', function() {
        ChangeThumbnailSize('smaller');
    });
    
    //-----------------------------------------------------
    // On clicking the Thumbnail Larger button
    $("#thumbSizeLarger").on('click', function() {
        ChangeThumbnailSize('larger');
    });
    
    //-----------------------------------------------------
    // On clicking a version X mark
    $("[id^=indicator]").on('click', function() {
        // Show a message to the user
        ChangeMessageBoxHeaderColour("blue");                            
        SetMessageBoxText("Untracked Theme","This theme was not installed by Clover Theme Manager. This means you will not be notified of any updates for this theme unless you UnInstall and then re-install it.");
        ShowMessageBoxClose();
        ShowMessageBox();
    });
    
    //-----------------------------------------------------
    // On clicking the theme count question mark
    $("#NumInstalledThemesQuery").on('click', function() {
        // Show a message to the user
        ChangeMessageBoxHeaderColour("blue");                            
        SetMessageBoxText("Unknown Theme Detected","There is a theme in this path with a name that does not match any in the Clover repository. This is not a problem, just be aware the number of installed themes shown in the main list will not match the counter.");
        ShowMessageBoxClose();
        ShowMessageBox();
    });
    
    //-----------------------------------------------------
    // On changing the 'NVRAM' dropdown menu.
    $("#installedThemeDropDownNvram").change(function() {
        ActionChangedDropDownNvram();
    });

    //-----------------------------------------------------
    // On changing the 'Config.plist' dropdown menu.
    $("#installedThemeDropDownConfigP").change(function() {
        ActionChangedDropDownConfigP();
    });

});

//-------------------------------------------------------------------------------------
function ActionShowHideBootlog()
{
    var hidden = $('#BootLogContainer').is(":hidden");
    if (!hidden) {
        SetBootLogState("Close");
        macgap.app.launch("CTM_bootlog‡Close");
    } else {
        SetBootLogState("Open");
        macgap.app.launch("CTM_bootlog‡Open");
    }
}

//-------------------------------------------------------------------------------------
function ActionMountESP()
{
    macgap.app.launch("MountESP");
    // Show a message to the user
    ChangeMessageBoxHeaderColour("blue");                            
    SetMessageBoxText("EFI System Partition(s)","All currently unmounted EFI System Partitions will now be mounted and checked for a /EFI/Clover/Themes directory. If found, the paths will appear in the volume selector.");
    HideMessageBoxClose();
    ShowProgressBar();
    ShowMessageBox();
}

//-------------------------------------------------------------------------------------
function ActionShowHideThumbnailsButton()
{
    var textState = $("#BandsHeightToggleButton").text();
    if (textState.indexOf("Hide") >= 0) {
        SetThemeBandHeight("Hide");
        // Send a message to the bash script to record user choice in prefs
        macgap.app.launch("CTM_showThumbails");
    }
    if (textState.indexOf("Show") >= 0) {     
        SetThemeBandHeight("Show");   
        // Send a message to the bash script to record user choice in prefs
        macgap.app.launch("CTM_hideThumbails");
    }
}

//-------------------------------------------------------------------------------------
function ActionShowHideExpandedPreviews()
{
    var buttonText=$("#preview_Toggle_Button").text();
    if (buttonText.indexOf("Expand") >= 0) {
        SetShowHidePreviews("Show");
        // Send a message to the bash script to record user choice in prefs
        macgap.app.launch("CTM_showPreviews");
    }
    if (buttonText.indexOf("Collapse") >= 0) {
        SetShowHidePreviews("Hide");
        // Send a message to the bash script to record user choice in prefs
        macgap.app.launch("CTM_hidePreviews");
    }
}

//-------------------------------------------------------------------------------------
function ActionUpdateAll()
{

    var themeList = "";
    var themeListPrint = "";

    $('.buttonUpdate').each(function(){

        updateButtonName = $(this).attr('id');

		// Update buttons for themes with spaces in their names had the
		// spaces replaced with five Q's. Let's put spaces back
		if (updateButtonName.indexOf('QQQQQ') >= 0) {
			updateButtonName = updateButtonName.replace(/QQQQQ/g, ' ');
		}

        // Get all after 'button_Update_'
        themeList = themeList + updateButtonName.substring(14) + ","
        themeListPrint = themeListPrint + updateButtonName.substring(14) + "<br>"

    });

    // remove last char
    themeList = themeList.slice(0, -1);

    // Send a message to the bash script
    macgap.app.launch("CTM_UpdateAll‡" + themeList);

    // Show a message to the user
    ChangeMessageBoxHeaderColour("blue");

    SetMessageBoxText("Updating:", themeListPrint);
    HideMessageBoxClose();
    ShowProgressBar();
    ShowMessageBox();
    HideUpdateAllButton();

}

//-------------------------------------------------------------------------------------
function ActionShowHideInstalledThemes()
{
    // Change text of button
    var textState = $("#ShowHideToggleButton").text();
    if (textState.indexOf("Show Installed") >= 0) {
        SetShowHideButton("Hide");
        // Send a message to the bash script to record user choice in prefs
        macgap.app.launch("CTM_hideUninstalled");
        // Close all preview images for UnInstalled themes
        ClosePreviewsForUninstalledThemes();
    }
    if (textState.indexOf("Show All") >= 0) {     
        SetShowHideButton("Show");   
        // Send a message to the bash script to record user choice in prefs
        macgap.app.launch("CTM_showUninstalled");
    }
}

//-------------------------------------------------------------------------------------
function ActionChangedDropDownNvram()
{
	var chosenOption=$("#installedThemeDropDownNvram").val();
	if(chosenOption != "-") {
		// Send message to bash script to notify change.
		macgap.app.launch("CTM_changeThemeN‡" + chosenOption);
		// Remove the Remove option from the menu
		$("#installedThemeDropDownNvram option[value='!Remove!']").remove();
		// If chosing to remove the entry then select the top (blank) option from the menu
		if(chosenOption == "!Remove!") {
		    $("#installedThemeDropDownNvram option:first").attr('selected','selected');
		}
	}

}

//-------------------------------------------------------------------------------------
function ActionChangedDropDownConfigP()
{
	var chosenOption=$("#installedThemeDropDownConfigP").val();
	if(chosenOption != "-") {
		// Send message to bash script to notify change.
		macgap.app.launch("CTM_changeThemeC‡" + chosenOption);
		// Remove the Remove option from the menu
		$("#installedThemeDropDownConfigP option[value='!Remove!']").remove();
		// If chosing to remove the entry then select the top (blank) option from the menu
		if(chosenOption == "!Remove!") {
		    $("#installedThemeDropDownConfigP option:first").attr('selected','selected');
		}
	}
}

//-------------------------------------------------------------------------------------
function SetBootLogState(state)
{
    // Note: The bootlog is not in the starting html template.
    // It's injected by the bash script if a bootlog has been found.

    var bootLogTitleBarHeight = $('#BootLogTitleBar').outerHeight();
    var bootLogContainerBandCount = $('div[id^=bandHeader]').length;
    var bootLogContainerHeight = ( bootLogContainerBandCount * 26 ); // 26px = Height of a single #BandHeader, #Band Description (inc 1px border)
    var currentHeaderHeight = $('#header').outerHeight();
    
    // Show and ShowClosed are passed by bash script if finding a bootlog.
    // ShowClosed is used if user has asked to close it and setting is in prefs.
    
    if(state == "Show") {
        // bash script has sent a command to show the bootlog
        var bootlogTotalHeight = bootLogTitleBarHeight + bootLogContainerHeight;
        var newHeaderHeight = (currentHeaderHeight + bootlogTotalHeight);
        $('#header').height(newHeaderHeight);
        $('#content').css("top","+="+bootlogTotalHeight);
        SetListingThemesMessageAndProgressBarPosition();
    } else if (state == "ShowClosed" ) {
        // bash script has sent a command to show the bootlog but collapsed
        $('#BootLogContainer').hide(function() {
            var newHeaderHeight = (currentHeaderHeight + bootLogTitleBarHeight);
            $('#header').height(newHeaderHeight);
            $('#content').css("top","+="+bootLogTitleBarHeight);
            SetListingThemesMessageAndProgressBarPosition();
        });
    } else {
        // Open and Close are passed as a result of the user clicking the #BootLogTitleBar
        
        var hidden = $('#BootLogContainer').is(":hidden"); 
        if(state == "Open") {
            // check it's not already open
            if (hidden) {
            
                // set header div z-index to 0 so header does not sit over sliding #content
                $('#header').css("z-index",0);
                
                var newHeaderHeight = (currentHeaderHeight + bootLogContainerHeight);
                $('#BootLogContainer').show(200);
                $('#content').animate({ left: '0', top: '+=' + (bootLogContainerHeight)}, 200, function () {
                        // Action after animation has completed
                        SetListingThemesMessageAndProgressBarPosition();
                        
                        // set header div z-index to 1 so header sits over #content and shadows shows.
                        $('#header').css("z-index",1);
                    });
                $('#header').height(newHeaderHeight);
                // Change arrow in bootlog header band title
                $('#BootLogTitleBar span').first().html('LAST BOOT\&nbsp;\&nbsp;\&\#x25BE\&nbsp;\&nbsp;\&nbsp;\&nbsp;|');
            }
        } else {
            // check it's not already closed and not already been clicked and currently animating to close
            if (!hidden && (!$('#content').is(':animated'))) {
            
                // set header div z-index to 0 so header does not sit over sliding #content
                $('#header').css("z-index",0);
                
                var newHeaderHeight = (currentHeaderHeight - bootLogContainerHeight);
                $('#BootLogContainer').hide(200);
                $('#content').animate({ left: '0', top: '-=' + (bootLogContainerHeight)}, 200, function () {
                        // Action after animation has completed
                        $('#header').height(newHeaderHeight);
                        SetListingThemesMessageAndProgressBarPosition();
                        
                        // set header div z-index to 1 so header sits over #content and shadows shows.
                        $('#header').css("z-index",1);
                    });
                // Change arrow in bootlog header band title
                $('#BootLogTitleBar span').first().html('LAST BOOT\&nbsp;\&nbsp;\&\#x25B8\&nbsp;\&nbsp;\&nbsp;\&nbsp;|');
            }
        }
    }
}

//-------------------------------------------------------------------------------------
function SetFooterHeight(UseControlOption)
{
    // Note: only the footer links exist in the default template.
    // The nvram status band is injected by bootlog.sh
    // The control options are injected by script.sh

    var nvramBandHeight = $('#NvramFunctionalityBand').outerHeight();
    var changeThemeContainerHeight = $('#changeThemeContainer').outerHeight();
    var footerLinksHeight = $('#FooterLinks').outerHeight();
    
    // Read text in nvram band as it has a dual purpose.
    // If not booted using Clover then it shows a message saying so.
    var nvramBandText = $('#nvramTextArea span').text();
    
    if(UseControlOption == "IncludeCO") {
    
        // Show nvram band if it's not displaying message about not being booted by Clover
        if(nvramBandText != "This system was not booted using Clover.") { // set by bootlog.sh
            $('#NvramFunctionalityBand').css("display","block");
        } 
        var newFooterHeight = (changeThemeContainerHeight + nvramBandHeight + footerLinksHeight);
        
    } else if(UseControlOption == "ExcludeCO") {

        // Hide nvram band if it's not displaying message about not being booted by Clover
        if(nvramBandText != "This system was not booted using Clover.") { // set by bootlog.sh
            $('#NvramFunctionalityBand').css("display","none");
            var newFooterHeight = (footerLinksHeight);
        } else {
            var newFooterHeight = (footerLinksHeight + nvramBandHeight);
        }
    }

    // set height of footer
    $('#footer').css("height",newFooterHeight);
    
    // Adjust bottom of #content to match footer height
    $('#content').css("bottom",newFooterHeight);
}


//-------------------------------------------------------------------------------------
function SetListingThemesMessageAndProgressBarPosition()
{
    var pathSelectorTop = $('#PathSelector').offset().top;
    $("#ListingThemesMessage").css({ top: (pathSelectorTop-5) });
    // Also adjust position of the CheckingUpdatesMessage progress meter
    $("#CheckingUpdatesMessage").css({ top: (pathSelectorTop-5) });
}

//-------------------------------------------------------------------------------------
function SetShowHidePreviews(state)
{
    var accordionBandState=$('.accordion').is(":hidden");
    if(state == "Show") {
        if (accordionBandState) {
            $(".accordionInstalled").next('[class="accordionContent"]').slideDown('normal');
            $(".accordionInstalledNoShadow").next('[class="accordionContent"]').slideDown('normal');
            $(".accordionUpdate").next('[class="accordionContent"]').slideDown('normal');
            $(".accordionUpdateNoShadow").next('[class="accordionContent"]').slideDown('normal');
        } else {
            $(".accordion").next('[class="accordionContent"]').slideDown('normal');
            $(".accordionInstalled").next('[class="accordionContent"]').slideDown('normal');
            $(".accordionInstalledNoShadow").next('[class="accordionContent"]').slideDown('normal');
            $(".accordionUpdate").next('[class="accordionContent"]').slideDown('normal');
            $(".accordionUpdateNoShadow").next('[class="accordionContent"]').slideDown('normal');
        }
        $("#preview_Toggle_Button").text("Collapse Previews");
        $("#preview_Toggle_Button").css("background-image","-webkit-linear-gradient(top, rgba(0,0,0,1) 0%,rgba(82,82,82,1) 100%)");
        $("#preview_Toggle_Button").css("border","1px solid #000");
        $("#preview_Toggle_Button").css("color","#82f3ff");
    } else if (state == "Hide") {
        if (accordionBandState) {
            $(".accordionInstalled").next('[class="accordionContent"]').slideUp('normal');
            $(".accordionInstalledNoShadow").next('[class="accordionContent"]').slideUp('normal');
            $(".accordionUpdate").next('[class="accordionContent"]').slideUp('normal');
            $(".accordionUpdateNoShadow").next('[class="accordionContent"]').slideUp('normal');
        } else {
            $(".accordion").next('[class="accordionContent"]').slideUp('normal');
            $(".accordionInstalled").next('[class="accordionContent"]').slideUp('normal');
            $(".accordionInstalledNoShadow").next('[class="accordionContent"]').slideUp('normal');
            $(".accordionUpdate").next('[class="accordionContent"]').slideUp('normal');
            $(".accordionUpdateNoShadow").next('[class="accordionContent"]').slideUp('normal');
        }
        $("#preview_Toggle_Button").text("Expand Previews");
        $("#preview_Toggle_Button").css("background-image","-webkit-linear-gradient(top, rgba(110,110,110,1) 0%,rgba(0,0,0,1) 100%)");
        $("#preview_Toggle_Button").css("border","1px solid #282828");
        $("#preview_Toggle_Button").css("color","#FFF");
    }
}

//-------------------------------------------------------------------------------------
function SetShowHideButton(state)
{
    if(state == "Hide") {
        $("#ShowHideToggleButton").text("Show All");
        $("#ShowHideToggleButton").css("background-image","-webkit-linear-gradient(top, rgba(0,0,0,1) 0%,rgba(82,82,82,1) 100%)");
        $("#ShowHideToggleButton").css("border","1px solid #000");
        $("#ShowHideToggleButton").css("color","#82f3ff");
        GetShowHideButtonStateAndUpdateUI();
    } else if (state == "Show") {
        $("#ShowHideToggleButton").text("Show Installed");
        $("#ShowHideToggleButton").css("background-image","-webkit-linear-gradient(top, rgba(110,110,110,1) 0%,rgba(0,0,0,1) 100%)");
        $("#ShowHideToggleButton").css("border","1px solid #282828");
        $("#ShowHideToggleButton").css("color","#FFF");
        GetShowHideButtonStateAndUpdateUI();
    }
}

//-------------------------------------------------------------------------------------
function SetThemeBandHeight(setting)
{
    if (setting == "Hide") {
    
            // Hide + and - buttons
            $("#thumbSizeSmaller").css("display","none");
            $("#thumbSizeLarger").css("display","none");
            // Show spacer sml buttons to retain spacing of other buttons
            $(".spacerButtonSml").css("display","block");
            // Move theme titles up
            $("[id=ThemeText]").css("top","80%");
            // Hide all theme descriptions
            $(".themeDescription").css("display","none");
            // Hide all theme authors
            $(".themeAuthor").css("display","none");
            // Hide thumbnails
            $(".thumbnail").css("display","none");
            // Adjust height of theme bands
            $(".accordion").css("height","27px");
            $(".accordionInstalled").css("height","27px");
            $(".accordionInstalledNoShadow").css("height","27px");
            $(".accordionUpdate").css("height","27px");
            $(".accordionUpdateNoShadow").css("height","27px");
            // Reduce margin top of buttons
            $(".buttonInstall").css("margin-top","3px");
            $(".buttonUnInstall").css("margin-top","3px");
            $(".buttonUpdate").css("margin-top","3px");
            // Reduce margin top of Unversioned Themes Indicator
            $(".versionControl").css("margin-top","9px");
            // Add margin left to theme titles
            $("[id=ThemeText]").css("margin-left","32px");
            // Change button text
            $("#BandsHeightToggleButton").text("Show Thumbnails");
            // Set background colour to indicate its selected
            $("#BandsHeightToggleButton").css("background-image","-webkit-linear-gradient(top, rgba(0,0,0,1) 0%,rgba(82,82,82,1) 100%)");
            $("#BandsHeightToggleButton").css("border","1px solid #000");
            $("#BandsHeightToggleButton").css("color","#82f3ff");
            
    } else if (setting == "Show") {

            // Show + and - buttons
            $("#thumbSizeSmaller").css("display","block");
            $("#thumbSizeLarger").css("display","block");
            // Hide spacer sml buttons to retain spacing of other buttons
            $(".spacerButtonSml").css("display","none");
            // Revert theme titles margin top
            $("[id=ThemeText]").css("top","50%");
            // Show all theme descriptions
            $(".themeDescription").css("display","inline");
            // Show all theme authors
            $(".themeAuthor").css("display","inline");
            // Show thumbnails
            $(".thumbnail").css("display","block");
            // Adjust height of theme bands
            var currentThumbHeight = $(".thumbnail img").first().height();
            var accordionHeight = (currentThumbHeight+14);
            $(".accordion").css("height",accordionHeight);
            $(".accordionInstalled").css("height",accordionHeight);
            $(".accordionInstalledNoShadow").css("height",accordionHeight);
            $(".accordionUpdate").css("height",accordionHeight);
            $(".accordionUpdateNoShadow").css("height",accordionHeight);
            // Revert margin top of buttons
            // Note: When thumb=100px wide, default button top=24px
            var currentThumbWidth = $(".thumbnail img").first().width();
            var buttonMarginAdjustment = (((currentThumbWidth-100)/25)*7);
            var buttonMarginTop = (24 + buttonMarginAdjustment);
            $(".buttonInstall").css("margin-top",buttonMarginTop);
            $(".buttonUnInstall").css("margin-top",buttonMarginTop);
            $(".buttonUpdate").css("margin-top",buttonMarginTop);
            // Revert margin top of Unversioned Themes Indicatpor
            // Note: When thumb=100px wide, default margin top=28px
            var versionControlMarginTop = (28 + buttonMarginAdjustment);
            $(".versionControl").css("margin-top",versionControlMarginTop);
            // Remove added margin left to theme titles
            $("[id=ThemeText]").css("margin-left","0px");
            // Change button text
            $("#BandsHeightToggleButton").text("Hide Thumbnails");
            // Revert background colour
            $("#BandsHeightToggleButton").css("background-image","-webkit-linear-gradient(top, rgba(110,110,110,1) 0%,rgba(0,0,0,1) 100%)");
            $("#BandsHeightToggleButton").css("border","1px solid #282828");
            $("#BandsHeightToggleButton").css("color","#FFF");
    }
}

//-------------------------------------------------------------------------------------
function UpdateMessageBox(messageOne,messageTwo)
{
    HideProgressBar();
    if (messageOne != "" && messageTwo != "") {
        // Show a message to the user
        ChangeMessageBoxHeaderColour("blue"); 
        if (messageOne == 'Mounted') {
            if (messageTwo == '0') {
                SetMessageBoxText("EFI System Partition(s)","There are no unmounted EFI system partitions with an existing /EFI/Clover/Themes directory.");
            } else {
                ChangeMessageBoxHeaderColour("green");
                if (messageTwo == '1') {
                    var msgWrd1="partition"
                    var msgWrd2="was"
                } else {
                    var msgWrd1="partitions"
                    var msgWrd2="were"
                }
                SetMessageBoxText("EFI System Partition(s)",messageTwo + " EFI system " + msgWrd1 + " with an existing /EFI/Clover/Themes directory " + msgWrd2 + " mounted.");

                // Honour users choice of which themes to view (All or just Installed)
                GetShowHideButtonStateAndUpdateUI();
        
                // Reset current theme list, bands and buttons
                ResetButtonsAndBandsToDefault();
                hideButtons();
        
                // show all themes, even if asked to Show Installed
                $(".accordion").css("display","block");
            }
        } else if (messageOne == "Cancelled") {
            ChangeMessageBoxHeaderColour("red"); 
            SetMessageBoxText("EFI System Partition(s)","Password dialog cancelled.");
        }
        ShowMessageBoxClose();
        ShowMessageBox();
    }
}

//-------------------------------------------------------------------------------------
function ShowMessageBootDevice(result,deviceId,mountPoint)
{
    // Show a message to the user
    if (result == "Failed") {
        ChangeMessageBoxHeaderColour("red"); 
        SetMessageBoxText("Boot Device","The boot device failed to be detected. A Rescan button is available in the bootlog region.");
        ShowMessageBoxClose();
        ShowMessageBox();
    } else if (result == "Mounted") {
        // Update bootlog device text
        $("#bandIdentifer span").last().html("<span class=\"infoBody\">" + deviceId + "<\/span>");
        $("#bandMountpoint span").last().html("<span class=\"infoBody\">" + mountPoint + "<\/span>");
        // Hide Rescan button
        $("#RescanButton").hide();
        // show message box confirming device found
        ChangeMessageBoxHeaderColour("blue");                            
        SetMessageBoxText("Boot Device",deviceId + " on mountpoint " + mountPoint + " was detected as the boot device ");
        ShowMessageBoxClose();
        ShowMessageBox();
    }
}

//-------------------------------------------------------------------------------------
function ChangeThumbnailSize(action)
{
    // Adjust the width of each thumbnail image by 25px each time this is called.
    // Each adjustment also alters:
    // - thumbnail height and theme band height.
    // - Y position of buttons and version control indicator.
    // - width of title, author and description text box.
    
    var currentThumbWidth = $(".thumbnail img").first().width();
    var currentThumbHeight = $(".thumbnail img").first().height();
    var currentAccordionHeight = $(".accordion").first().height();
    var currentThemeTextWidth = $("#ThemeText").first().width();
    if (action=='larger' && currentThumbWidth <= 175) {
        var newAccordionHeight=(currentAccordionHeight+14);
        var newThumbWidth=(currentThumbWidth+25);
        var newThumbHeight=(currentThumbHeight+14);
        var newThemeTextWidth=(currentThemeTextWidth-30);
    } else if (action=='smaller' && currentThumbWidth >= 125) {
        var newAccordionHeight=(currentAccordionHeight-14);
        var newThumbWidth=(currentThumbWidth-25);
        var newThumbHeight=(currentThumbHeight-14);
        var newThemeTextWidth=(currentThemeTextWidth+30);
    } else {
        newThemeTextWidth=currentThemeTextWidth;
    }
    
    //alert(currentThumbWidth+","+action+","+currentThemeTextWidth+","+newThemeTextWidth);     
    
    // Only make changes if thumbnail width has changed
    if (newThumbWidth != currentThumbWidth) {
        // Adjust height of theme bands
        $(".accordion").css("height",newAccordionHeight);
        $(".accordionInstalled").css("height",newAccordionHeight);
        $(".accordionInstalledNoShadow").css("height",newAccordionHeight);
        $(".accordionUpdate").css("height",newAccordionHeight);
        $(".accordionUpdateNoShadow").css("height",newAccordionHeight);
            
        // Change thumbnail size
        $(".thumbnail img").css("width",newThumbWidth);
        $(".thumbnail img").css("height",newThumbHeight);
            
        // Change margin top of buttons
        var buttonHeight = $(".buttonInstall").first().outerHeight();
        var newButtonTop = ((newAccordionHeight-buttonHeight)/2);
        $(".buttonInstall").css("margin-top",newButtonTop);
        $(".buttonUnInstall").css("margin-top",newButtonTop);
        $(".buttonUpdate").css("margin-top",newButtonTop);
            
        // Change margin top of version control indicator
        $(".versionControl").css("margin-top",newButtonTop+3);
            
        // Reduce width of theme text (by 30px) to retain space for update button
        $("[id^=ThemeText]").width(newThemeTextWidth);
            
        // Send a message to the bash script to record thumbnail width
        if (newThumbWidth >= 100 && newThumbWidth <= 200)
            macgap.app.launch("CTM_thumbSize‡" + newThumbWidth + " " + newThumbHeight);
    }
}

//-------------------------------------------------------------------------------------
function GetShowHideButtonStateAndUpdateUI()
{
    var showHideState=$("[id='ShowHideToggleButton']").text();
    var expandCollapseState=$("[id='preview_Toggle_Button']").text();
    if (showHideState.indexOf("Show Installed") >= 0) {
        showHideState="Hide";
    } else if (showHideState.indexOf("Show All") >= 0) {
        showHideState="Show";
    }
    ShowHideUnInstalledThemes(showHideState,expandCollapseState);
}

//-------------------------------------------------------------------------------------
function FindLineInString(CompleteString,SearchString)
{
    var splitLines = (CompleteString).split(/\r\n|\r|\n/);    
    for (l = 0; l < splitLines.length; l++) {
        if ((splitLines[l]).indexOf(SearchString) >= 0)
            return splitLines[l];
    }
    return "0";
}

//-------------------------------------------------------------------------------------
function RespondToButtonPress(button,status)
{
    // Update buttons have a class name 'button_Update_' and not 'button_'
    // The bash script matches against the string 'button_'
    // Remove 'Update_' from string.
    // Note: Could cause issues if a theme has 'Update_' in it's title.

    var button = button.replace('Update_', '');
    
    // Update buttons for themes with spaces in their names had the
    // spaces replaced with five Q's. Let's put spaces back
    if (button.indexOf('QQQQQ') >= 0) {
        button = button.replace(/QQQQQ/g, ' ');
    }

    // Notify bash script. Send button name and it's current state.
    macgap.app.launch("CTM_ThemeAction‡" + button + "‡" + status);

    // Prepare vars for legible user message
    // PresedButton will begin with "button_"
    button=button.substring(7);

    // PresedButton will begin with "button"
    status=status.substring(6);
    if (status == "Install") {
        headerText="Downloading & Installing:";
        printText="Installed";
    }
    if (status == "UnInstall") {
        headerText="Un-Installing:";
        printText="Un-Installed";
    }
    if (status == "Update") {
        headerText="Updating:";
        printText="Updated";
    }
 
    // Show a message to the user
    ChangeMessageBoxHeaderColour("blue");
        
    SetMessageBoxText(headerText, "Please wait while theme " + button + " is " + printText + ".");
    HideMessageBoxClose();
    ShowProgressBar();
    ShowMessageBox();
    HideUpdateAllButton();
}

//-------------------------------------------------------------------------------------
// hide all option buttons
function hideButtons()
{
    $(".buttonInstall").css("display","none");
    $(".buttonUnInstall").css("display","none");
    $(".buttonUpdate").css("display","none");
}

//-------------------------------------------------------------------------------------
// show all option buttons
function showButtons()
{
    $(".buttonInstall").css("display","block");
    $(".buttonUnInstall").css("display","block");
    $(".buttonUpdate").css("display","block");
}

//-------------------------------------------------------------------------------------
function SetMessageBoxText(title,message)
{
    $(".box h1").html(title);
    $(".box p").html(message);
}

//-------------------------------------------------------------------------------------
function HideMessageBoxClose()
{
    $("a.boxclose").css("display","none");
}

//-------------------------------------------------------------------------------------
function ShowMessageBoxClose()
{
    $("a.boxclose").css("display","block");
}

//-------------------------------------------------------------------------------------
function HideFreeSpace()
{
    $("#FreeSpace").css("display","none");
}

//-------------------------------------------------------------------------------------
function ShowFreeSpace()
{
    $("#FreeSpace").css("display","block");
}

//-------------------------------------------------------------------------------------
function HideProgressBar()
{
    $("#AnimatedBar").css("display","none");
}

//-------------------------------------------------------------------------------------
function ShowProgressBar()
{
    $("#AnimatedBar").css("display","block");
}

//-------------------------------------------------------------------------------------
function DisplayOverlayBox()
{
    $('#overlay').fadeIn('fast');
}

//-------------------------------------------------------------------------------------
function HideOverlayBox()
{
    $('#overlay').fadeOut('fast');
}

//-------------------------------------------------------------------------------------
function DisplayOverlayTwoBox()
{
    $('#overlayTwo').fadeIn('fast');
}

//-------------------------------------------------------------------------------------
function HideOverlayTwoBox()
{
    $('#overlayTwo').fadeOut('fast');
}

//-------------------------------------------------------------------------------------
// From a tutorial by Mary Lou
// http://tympanus.net/codrops/2009/12/03/css-and-jquery-tutorial-overlay-with-slide-out-box/
function ShowMessageBox()
{
    // Read position of box and only fade in if at default off screen position.
    var position = $('#box').position();
    if (position.top = -300) {   // starting position = should match .box top in css
        $('#overlay').fadeIn('fast',function(){
             // move box from current position so top=150px
             $('#box').animate({'top':'150px'},500, function(){
                 // Bounce
                 doBounce($('.box'), 2, '10px', 100);   
             }); 
        });
    }
}

//-------------------------------------------------------------------------------------
// from http://stackoverflow.com/questions/10363671/jquery-bounce-effect-on-click-no-jquery-ui
function doBounce(element, times, distance, speed) {
    for(var i = 0; i < times; i++) {
        element.animate({marginTop: '-='+distance}, speed)
            .animate({marginTop: '+='+distance}, speed);
    }        
}

//-------------------------------------------------------------------------------------
function CloseMessageBox()
{
    // Read position of box and only fade out if at calculated top position is 150px which is set in ShowMessageBox()
    var position = $('#box').position();
    if (position.top = 150) {
    
        var messageBoxHeight = $("#box").outerHeight();
        var distanceToMove = 300; // matches top of .box in css
        if (messageBoxHeight > distanceToMove) {
            distanceToMove = messageBoxHeight;
        }
        $('#box').animate({'top':'-'+distanceToMove+'px'},500,function(){  // starting position = should match .box top in css
            $('#overlay').fadeOut('fast');
        });
    }
}

//-------------------------------------------------------------------------------------
function ChangeMessageBoxHeaderColour(colour)
{
    if(colour == "blue") {
        $("#box h1").css("background-color","#1e8ec6");
        $("#box h1").css("color","#c4e0ee");
    }
        
    if(colour == "red") {
        $("#box h1").css("background-color","#b43239");
        $("#box h1").css("color","#f2d6d8");
    }
        
    if(colour == "green") {
        $("#box h1").css("background-color","#8db035");
        $("#box h1").css("color","#e4ecce");
    }
}

//-------------------------------------------------------------------------------------
function ResetButtonsAndBandsToDefault()
{
    // Set class of all buttons to Install
    $(".buttonInstall").attr('class', 'buttonInstall');
    $(".buttonUnInstall").attr('class', 'buttonInstall');
    
    // Set all button text to Install
    $(".buttonInstall").html("Install");
    $(".buttonUnInstall").html("Install");

    // Change all installed band backgrounds to normal
    $(".accordionInstalled").attr("class","accordion");
    $(".accordionInstalledNoShadow").attr("class","accordion");
    
    // Change all update band backgrounds to normal
    $(".accordionUpdate").attr("class","accordion");
    $(".accordionUpdateNoShadow").attr("class","accordion");
    
    // Remove any update buttons
    $("[id^=button_Update]").remove();
    
    // Remove any unVersioned indicators
    $("[id^=indicator_]").html("&nbsp;&nbsp;&nbsp;");
    $("[id^=indicator_]").css("pointer-events","none");
}

//-------------------------------------------------------------------------------------
function ChangeButtonAndBandToUnInstall(installedThemeName)
{
    // Set class of this themes' button to UnInstall
    // Use an attribute selector to deal with themes with spaces in their name
    $("[id='button_" + installedThemeName + "']").attr('class', 'buttonUnInstall');
    
    // Set class of this themes' button text to UnInstall
    $("[id='button_" + installedThemeName + "']").html("UnInstall");

    // Note: bash script supplies this theme list in alphabetical order.
    //       If themes are not called in alphabetical order this does not work.
    // Work out if band above is accordionInstalled. If yes then fill with accordionInstalledNoShadow
    var currentThemeBand = $("[id='button_" + installedThemeName + "']").closest("#ThemeBand");
    var currentBandClass = currentThemeBand.attr('class');
    var aboveBandClass = currentThemeBand.prevAll("#ThemeBand").first().attr('class');
    if(typeof aboveBandClass != 'undefined') { // not at top band
        if(aboveBandClass == "accordion") {
            $("[id='button_" + installedThemeName + "']").closest('div[class="accordion"]').attr("class","accordionInstalled");
        } else if(aboveBandClass == "accordionInstalled") {
            $("[id='button_" + installedThemeName + "']").closest('div[class="accordion"]').attr("class","accordionInstalledNoShadow");
        } else if(aboveBandClass == "accordionInstalledNoShadow") {
            $("[id='button_" + installedThemeName + "']").closest('div[class="accordion"]').attr("class","accordionInstalledNoShadow");
        } else if(aboveBandClass == "accordionUpdate") {
            $("[id='button_" + installedThemeName + "']").closest('div[class="accordion"]').attr("class","accordionInstalledNoShadow");
        } else if(aboveBandClass == "accordionUpdateNoShadow") {
            $("[id='button_" + installedThemeName + "']").closest('div[class="accordion"]').attr("class","accordionInstalledNoShadow");
        }
    } else { // at top band
        $("[id='button_" + installedThemeName + "']").closest('div[class="accordion"]').attr("class","accordionInstalledNoShadow");
    }
}

//-------------------------------------------------------------------------------------
function ChangeButtonAndBandToUpdate(themeName)
{
    var themeNameForUpdate = themeName;

    // Check for any spaces in the theme name as I've just found out that
    // an id with spaces is invalid html. Ooops! This happens for each 
    // theme with a name containing spaces.
    
    // For now, replace replace a space with five Q's.
    // This will only be used internally and the user won't see anything.
    if (themeName.indexOf(' ') >= 0) {
        themeNameForUpdate = themeName.replace(/ /g, 'QQQQQ');
    }

    // Check we have not already added an update button for this theme.
    var buttonExistCheck=$("#button_Update_"+themeNameForUpdate);

    if (!jQuery.contains(document, buttonExistCheck[0])) {
        // Add a new 'update' button beside the current 'UnInstall' button
        // http://stackoverflow.com/questions/12618214/binding-jquery-events-before-dom-insertion
        $( '<div class="buttonUpdate" id="button_Update_' + themeNameForUpdate + '"></div>' ).on('click', function(e) {
            e.preventDefault();
            var pressedButton=$(this).attr('id');
            var currentStatus=$(this).attr('class');
            RespondToButtonPress(pressedButton,currentStatus);
        }).insertAfter("[id='button_" + themeName + "']");
        $("[id='button_Update_" + themeNameForUpdate + "']").html("Update");

        // set the vertical position to match the uninstall button
        var uninstallButtonTop=$("[id='button_" + themeName + "']").css("margin-top");
        $("[id='button_Update_" + themeNameForUpdate + "']").css("margin-top",uninstallButtonTop);
    }

    // Change band background for themes with updates available   
    // Note: bash script supplies this theme list in alphabetical order.
    //       If themes are not called in alphabetical order this does not work.
    var currentThemeBand = $("[id='button_" + themeName + "']").closest("#ThemeBand");
    var currentBandClass = currentThemeBand.attr('class');
    
    // if UI is currently set to 'Show Installed' themes
    // then we only need to show update bands without shadows.
    var viewState = $(ShowHideToggleButton).text();
    if (viewState.indexOf("Show All") >= 0) {
        $(currentThemeBand).attr("class","accordionUpdateNoShadow");
    } else {
        // UI is set to 'Show all' themes so we need to figure out
        // if shadows are needed for the update bands.
        var aboveBandClass = currentThemeBand.prevAll("#ThemeBand").first().attr('class');
        if(typeof aboveBandClass != 'undefined') { // not at top band
            if(aboveBandClass == "accordion") {
                $(currentThemeBand).attr("class","accordionUpdate");
            } else {
                $(currentThemeBand).attr("class","accordionUpdateNoShadow");
            }
        } else { // at top band
            $(currentThemeBand).attr("class","accordionUpdateNoShadow");
        }
    }
}

//-------------------------------------------------------------------------------------
function ChangeButtonsToUpdate()
{
    $(".buttonInstall").attr('class', 'buttonUpdate');
    $(".buttonInstall").html("Update");
}

//-------------------------------------------------------------------------------------
function SetUnVersionedControlIndicator(themeName)
{
    // themeName will be the name of an installed theme

    // Display indicator to show theme is unversioned
    //$("[id='indicator_" + themeName + "']").html("\u2715");
    $("[id='indicator_" + themeName + "']").html("?");
    $("[id='indicator_" + themeName + "']").css("pointer-events","auto");
}

//-------------------------------------------------------------------------------------
function UpdateAndRefreshPartitionSelectMenu(list)
{
    // Clear existing entries
    $(partitionSelect).empty();
      
    // Add title menu option
    $("#partitionSelect").append("<option value=\"-\" disabled=\"disabled\">Select your target theme directory:</option>");
    
    // Add list sent from bash script
    if (list != "") {
        splitList = (list).split(',');
        for (var t = 0; t < splitList.length; t++) {
            var parts = (splitList[t]).split(';');
            var id = (parts[0]);
            var desc = (parts[1]);     
            $("#partitionSelect").append("<option value=\"" + id + "\">" + desc + "</option>");
        }
    }
}

//-------------------------------------------------------------------------------------
function UpdateAndRefreshInstalledThemeDropDown(themeList)
{
    // If menus exist in the DOM then populate them
    var test=$("#installedThemeDropDownNvram");
    if (jQuery.contains(document, test[0])) {
        $(installedThemeDropDownNvram).empty();
        populateDropDownMenu(installedThemeDropDownNvram,themeList);
    }

    //var test=$("#installedThemeDropDownNvramP");
    //if (jQuery.contains(document, test[0])) {
    //    $(installedThemeDropDownNvramP).empty();
    //    populateDropDownMenu(installedThemeDropDownNvramP,themeList);
    //}
        
    var test=$("#installedThemeDropDownConfigP");
    if (jQuery.contains(document, test[0])) {
        $(installedThemeDropDownConfigP).empty();
        populateDropDownMenu(installedThemeDropDownConfigP,themeList);
    }
}

//-------------------------------------------------------------------------------------
function populateDropDownMenu(menu,themeList)
{
    // Note: Bash script sends '-' when no theme is set.
    //       This then sets the menu to '-'
    $(menu).append("<option value=\"-\" disabled=\"disabled\"> </option>");
    //$(menu).append("<option value=\"!Remove!\">Remove Current Entry</option>");
    $(menu).append("<option value=\"-1\" disabled=\"disabled\">--------------------</option>");
    $(menu).append("<option value=\"0\" disabled=\"disabled\">Installed Themes</option>");
    for (var t = 0; t < themeList.length; t++) {
        if (themeList[t] != "")
            $(menu).append("<option value=\"" + themeList[t] + "\">" + themeList[t] + "</option>");
    }
    $(menu).append("<option value=\"-1\" disabled=\"disabled\">--------------------</option>");
    $(menu).append("<option value=\"0\" disabled=\"disabled\">Other Choices</option>");
    $(menu).append("<option value=\"random\">random</option>");
    $(menu).append("<option value=\"embedded\">embedded</option>");
}

//-------------------------------------------------------------------------------------
function checkEntryAndInsertRemoveOptionIfBlank(menu)
{
    var isThemeSet=$(menu).val();
    var menuId=$(menu).attr('id');

	if(isThemeSet != null && isThemeSet != " ") {

        // If remove option is not already in list then add the 'Remove Current Entry' option from the menu
        if ( $("#" + menuId + " option[value='!Remove!']").length == 0 ) {
            $("<option value=\"!Remove!\">Remove Current Entry</option>").insertAfter($("#" + menuId + " option:first"));
        }

	}
}

//-------------------------------------------------------------------------------------
function imgErrorThumb(image){
    image.onerror="";
    image.src="assets/thumb_noimage.png";
    return true;
}

//-------------------------------------------------------------------------------------
function imgErrorPreview(image){
    image.onerror="";
    image.src="assets/preview_noimage.png";
    return true;
}

//-------------------------------------------------------------------------------------
function AddQuitButton(){

    // Add button No
    $( '<div class="feedbackButton" id="feedback_Button_Quit"></div>' ).on('click', function(e) {
        e.preventDefault();
        // Terminate the app
        macgap.app.terminate(); 
        
    }).insertAfter("[id='FeedbackButtons']");

    // Set button text
    $("[id='feedback_Button_Quit']").html("Quit");
}

//-------------------------------------------------------------------------------------
function ShowHideUnInstalledThemes(showHide,expandCollapse)
{
    if (showHide.indexOf("Show") >= 0) {
        if (expandCollapse.indexOf("Expand") >= 0) {
            $(".accordion").css("display","none");
        }
        if (expandCollapse.indexOf("Collapse") >= 0) {
            $(".accordion").css("display","none");
            $(".accordion").next('[class="accordionContent"]').css("display","none");
        }

        // Remove all installed theme band shadows
        $(".accordionInstalled").attr("class","accordionInstalledNoShadow");
        
        // Remove all update theme band shadows
        $(".accordionUpdate").attr("class","accordionUpdateNoShadow");
        
    } else if (showHide.indexOf("Hide") >= 0) {  
     
        if (expandCollapse.indexOf("Expand") >= 0) {
            $(".accordion").css("display","block");
        }
        if (expandCollapse.indexOf("Collapse") >= 0) {
            $(".accordion").css("display","block");
            $(".accordion").next('[class="accordionContent"]').css("display","block");
        }
        
        // Reset all theme bands
        ResetButtonsAndBandsToDefault();
                
        // Set all installed theme bands
        updateBandsWithInstalledThemes(gInstalledThemeListStr);
    }

    // Set all update theme bands
    if(gUpdateThemeListStr != "" )
        ShowUpdateThemesInUI(gUpdateThemeListStr);
}

//-------------------------------------------------------------------------------------
function ClosePreviewsForUninstalledThemes()
{
    // Close all preview images for UnInstalled themes
    $('.buttonInstall').closest("#ThemeBand").next('[class="accordionContent"]').slideUp('normal');
}

//-------------------------------------------------------------------------------------
function sendNotification(messageBody)
{
    // INSERT_NOTIFICATION_CODE_HERE
}

//-------------------------------------------------------------------------------------
function SetPredictionText(message)
{
    $("#predictionTheme").text(message);
}

//-------------------------------------------------------------------------------------
function ShowHideControlOptions(state)
{
    // Does the changeThemeContainer div exist in the DOM?
    var ctcDiv=$("#changeThemeContainer");
    if (jQuery.contains(document, ctcDiv[0])) {
        var ctcDivExist=1;
    }
    
    var controlOptionsHidden = $('#changeThemeContainer').is(":hidden"); 
    if (state == "Show") {
        // check it's not already showing
        if (ctcDivExist == 1 && controlOptionsHidden) {
            $('#changeThemeContainer').show(function() {
                SetFooterHeight("IncludeCO");
            });
        } else {
            // #changeThemeContainer does not exist.
            SetFooterHeight("IncludeCO");
        }
    } else if (state == "Hide"){
        // check it's not already hidden
        if (ctcDivExist == 1 && !controlOptionsHidden) {
            $('#changeThemeContainer').hide(function() {
                SetFooterHeight("ExcludeCO");
            });
        } else {
            // #changeThemeContainer does not exist.
            SetFooterHeight("ExcludeCO");
        }
    }
}

//-------------------------------------------------------------------------------------
function UpdateThemePaths(nvramPlistPath,configPlistPath,mountpoint)
{
    // Replace paths in theme control option area
    //$('#ctTitleNvramP span').text(nvramPlistPath);
    $('#ctTitleConfig span').text(configPlistPath);
    // Unset any disabled control option drop down menus
    //$('#installedThemeDropDownNvramP').prop("disabled", false);
    $('#installedThemeDropDownConfigP').prop("disabled", false);
    
    // Update paths in bootlog
    $('.infoBodyReplaceable').each(function(){
        var str = $(this).text();
        var strTest = str.toUpperCase();
        if (strTest.match("^/EFI")) {
            $(this).text(mountpoint+str);
        }
    });
}


//-------------------------------------------------------------------------------------
function ShowUpdateAllButton()
{

    $("#ShowHideUpdateAllButton").show();
    $(".spacerButtonUpdateAll").hide();

}


//-------------------------------------------------------------------------------------
function HideUpdateAllButton()
{

    $("#ShowHideUpdateAllButton").hide();
    $(".spacerButtonUpdateAll").show();

}
