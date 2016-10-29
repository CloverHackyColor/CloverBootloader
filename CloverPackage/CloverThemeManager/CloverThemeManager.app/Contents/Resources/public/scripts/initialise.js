// A script for Clover Theme Manager
// Copyright (C) 2014-2015 Blackosx
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
//Version=0.75.4

var gTmpDir = "/tmp/CloverThemeManager/";
var gLogBashToJs = "CloverThemeManager_BashToJs.log";
var lastLine="-1";
var prevLastLine="";
var prevLastLineCount=0;
var eof=0;

//-------------------------------------------------------------------------------------
// On initial load
$(document).ready(function() {    
    macgap.app.launch("started")
    readLogFileForMessages();
});

//-------------------------------------------------------------------------------------
// Called when the process is to close.
function terminate() {
    //clearTimeout(timerCheckEof);
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

//-------------------------------------------------------------------------------------
function redirectToMainPage()
{
    var redirect=(gTmpDir + "managethemes.html" );
    window.location = (redirect);
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
function ShowMessageBox()
{
    // Read position of box and only fade in if at default off screen position.
    var position = $('#box').position();
    if (position.top = -300) {   // starting position = should match .box top in css
        $('#overlay').fadeIn('fast',function(){
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
        $('#box').animate({'top':'-300px'},500,function(){  // starting position = should match .box top in css
            $('#overlay').fadeOut('fast');
        });
    }
}

//-------------------------------------------------------------------------------------
$(function()
{
    //-----------------------------------------------------
    // On clicking the message box close button
    $('#boxclose').click(function(){
        CloseMessageBox();
        macgap.app.terminate();
    });
});


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

// Read /tmp/CloverThemeManager/CloverThemeManagerLog.txt looking for embedded 
// messages for showing the initialisation process.
//-------------------------------------------------------------------------------------
function readLogFileForMessages()
{
    eof=0;
    var splitContent="";
    fileContent=GetFileContents("CloverThemeManagerLog.txt");

    if (fileContent != 0) {

        splitContent = fileContent.split("\n");
        lastLine = splitContent[splitContent.length-2];

        if (lastLine != prevLastLine) {
            for (i = prevLastLineCount; i <= splitContent.length; i++) {

                // Does this line contain CTM_AppVersion?
                if (/CTM_Version/i.test(splitContent[i])) {
                    // remove anything before the 'CTM_Version' text.
                    version = splitContent[i].substring(splitContent[i].indexOf("CTM_Version") + 11);
                    // append version to title.
                    $("#textversions").text(version);
                
                } else if (/CTM_GitOK/i.test(splitContent[i])) {
                           $("#check_GitInstalled").append( "  \u2713" );
                           $("#status_GitInstalled").css("color","#FFF");  
                } else if (/CTM_GitFail/i.test(splitContent[i])) {
                           $("#status_GitInstalled").css("color","#DD171B");
                           ChangeMessageBoxHeaderColour("red");
                           SetMessageBoxText('Attention: git is not installed' , 'git is a requirement for this app and it cannot continue without it.<br><br>To install only git, you can download an installer from <a href="http://git-scm.com" target="_blank">http://git-scm.com</a>, double click the .dmg and run the .pkg.<br><br>Alternatively you can install the Xcode command line developer tools by loading up Terminal and typing <b>xcode-select --install</b><br><br>This app will quit once you close this box.');
                           $("#AnimatedBar").css("display","none");
                           ShowMessageBox();
                           // Send native notification
                           //sendNotification("Problem: git is not installed.");

                } else if (/CTM_HTMLTemplateOK/i.test(splitContent[i])) {
                           $("#check_HtmlTemplate").append( "  \u2713" );
                           $("#status_HtmlTemplate").css("color","#FFF"); 
                } else if (/CTM_HTMLTemplateFail/i.test(splitContent[i])) {
                           $("#status_HtmlTemplate").css("color","#DD171B");
                
                } else if (/CTM_RepositoryCheckNetwork/i.test(splitContent[i])) {
                           $("#status_Repository").text( 'Checking network gateway...' ).append( '<span class="checkMark" id="check_Repository"></span>' );
                           $("#status_Repository").css("color","#FFAE40"); 
                } else if (/CTM_RepositoryChecking/i.test(splitContent[i])) {
                           $("#status_Repository").text( 'Connecting to repository...' ).append( '<span class="checkMark" id="check_Repository"></span>' );
                           $("#status_Repository").css("color","#FFAE40"); 
                } else if (/CTM_RepositorySuccess/i.test(splitContent[i])) {
                           $("#status_Repository").text( 'Repository online' ).append( '<span class="checkMark" id="check_Repository"></span>' );
                           $("#check_Repository").append( "  \u2713" );
                           $("#status_Repository").css("color","#FFF"); 
                } else if (/CTM_RepositoryError/i.test(splitContent[i])) {
                           $("#status_Repository").css("color","#DD171B");
                           ChangeMessageBoxHeaderColour("red");
                           SetMessageBoxText('Attention: Repository' , 'The remote Clover Theme repository is not responding. This app cannot continue and will exit when you close this dialog box.<br><br> <a href="https://sourceforge.net/p/cloverefiboot/themes/ci/master/tree/" target="_blank">Check Repo in Web Browser</a>');
                           $("#AnimatedBar").css("display","none");
                           ShowMessageBox();
                           // Send native notification
                           //sendNotification("Problem: The remote Clover Theme repository is not responding.");
                           
                } else if (/CTM_SupportDirOK/i.test(splitContent[i])) {
                           $("#check_SupportDir").append( "  \u2713" );
                           $("#status_SupportDir").css("color","#FFF"); 
                } else if (/CTM_SupportDirFail/i.test(splitContent[i])) {
                           $("#status_SupportDir").css("color","#DD171B");
                           
                } else if (/CTM_SymbolicLinksOK/i.test(splitContent[i])) {
                           $("#check_SymbolicLink").append( "  \u2713" );
                           $("#status_SymbolicLink").css("color","#FFF"); 
                } else if (/CTM_SymbolicLinksFail/i.test(splitContent[i])) {
                           $("#status_SymbolicLink").css("color","#DD171B");
                           
                } else if (/CTM_IndexCloneAndCheckout/i.test(splitContent[i])) {
                           $("#status_Index").text( 'Downloading updated git index' ).append( '<span class="checkMark" id="check_Index"></span>' );
                           $("#status_Index").css("color","#FFAE40"); 
                } else if (/CTM_IndexOK/i.test(splitContent[i])) {
                           $("#check_Index").append( "  \u2713" );
                           $("#status_Index").css("color","#FFF"); 
                } else if (/CTM_IndexFail/i.test(splitContent[i])) {
                           $("#status_Index").css("color","#DD171B");
                           
                } else if (/CTM_ThemeListOK/i.test(splitContent[i])) {
                           $("#check_ThemeList").append( "  \u2713" );
                           $("#status_ThemeList").css("color","#FFF"); 
                } else if (/CTM_ThemeListFail/i.test(splitContent[i])) {
                           $("#status_ThemeList").css("color","#DD171B");
                           
                } else if (/CTM_InsertHtmlOK/i.test(splitContent[i])) {
                           $("#check_InsertHtml").append( "  \u2713" );
                           $("#status_InsertHtml").css("color","#FFF"); 
                } else if (/CTM_InsertHtmlFail/i.test(splitContent[i])) {
                           $("#status_InsertHtml").css("color","#DD171B");
                           
                } else if (/CTM_ThemeDirsScan/i.test(splitContent[i])) {
                           $("#status_ThemeDirs").text( 'Building list of Theme dir(s)' );
                           $("#status_ThemeDirs").css("color","#FFAE40"); 
                } else if (/CTM_ThemeDirsOK/i.test(splitContent[i])) {
                           $("#status_ThemeDirs").text( 'Theme directories' ).append( '<span class="checkMark" id="check_ThemeDirs">  \u2713</span>' );
                           $("#status_ThemeDirs").css("color","#FFF"); 
                } else if (/CTM_ThemeDirsFail/i.test(splitContent[i])) {
                           $("#status_ThemeDirs").css("color","#DD171B");
                           
                } else if (/CTM_DropDownListOK/i.test(splitContent[i])) {
                           $("#check_Dropdown").append( "  \u2713" );
                           $("#status_Dropdown").css("color","#FFF"); 
                } else if (/CTM_DropDownListNone/i.test(splitContent[i])) {
                           $("#check_Dropdown").append( "  \u2713" );
                           $("#status_Dropdown").css("color","#BBB"); // Change to lighter grey, not red.
                           
                } else if (/CTM_ReadPrefsOK/i.test(splitContent[i])) {
                           $("#check_ReadPrefs").append( "  \u2713" );
                           $("#status_ReadPrefs").css("color","#FFF"); 
                } else if (/CTM_ReadPrefsCreate/i.test(splitContent[i])) {
                           $("#status_ReadPrefs").text( 'Create prefs file' ).append( '<span class="checkMark" id="check_ReadPrefs">  \u2713</span>' );
                           $("#status_ReadPrefs").css("color","#FFF");
                           
                } else if (/CTM_BootDeviceMBR/i.test(splitContent[i])) {
                           ChangeMessageBoxHeaderColour("blue");
                           SetMessageBoxText('Attention: Boot Device' , 'Clover was loaded from an MBR partitioned device. This device will need to be detected before you can manage the themes.<br><br>Please enter your password for running fdisk.');
                           $("#AnimatedBar").css("display","none");
                           HideMessageBoxClose();
                           ShowMessageBox();
                } else if (/CTM_BootDeviceGPT/i.test(splitContent[i])) {
                           ChangeMessageBoxHeaderColour("blue");
                           SetMessageBoxText('Attention: Boot Device' , 'Clover was loaded from an EFI system partition which is currently unmounted. It will need to be mounted to manage the themes.<br><br>Please enter your password to mount it.');
                           $("#AnimatedBar").css("display","none");
                           HideMessageBoxClose();
                           ShowMessageBox();
                } else if (/CTM_BootDeviceCloseWindow/i.test(splitContent[i])) {
                           CloseMessageBox();
                           
                } else if (/CTM_BootlogOK/i.test(splitContent[i])) {
                           $("#check_ReadBootlog").append( "  \u2713" );
                           $("#status_ReadBootlog").css("color","#FFF"); 
                } else if (/CTM_BootlogMissing/i.test(splitContent[i])) {
                           $("#status_ReadBootlog").text( 'Bootlog not found' ).append( '<span class="checkMark" id="check_ReadBootlog">  \u2713</span>' );
                           $("#status_ReadBootlog").css("color","#BBB"); // Change to lighter grey, not red.
                } else if (/CTM_BootlogFail/i.test(splitContent[i])) {
                           $("#status_ReadBootlog").css("color","#DD171B");
                           
                } else if (/CTM_InitInterface/i.test(splitContent[i])) {
                           $("#check_InitMainUI").append( "  \u2713" );
                           $("#status_InitMainUI").css("color","#FFF"); 

                } else if (/CTM_NvramFound/i.test(splitContent[i])) {
                           $("#check_NvramVar").append( "  \u2713" );
                           $("#status_NvramVar").css("color","#FFF"); 
                } else if (/CTM_NvramNotFound/i.test(splitContent[i])) {
                           $("#check_NvramVar").append( "  \u2713" );
                           $("#status_NvramVar").css("color","#BBB"); // Change to lighter grey, not red.
                           
                }
            }
            prevLastLineCount=splitContent.length-2;
            prevLastLineCount=prevLastLineCount+1
            prevLastLine = lastLine;
        }
        
        // Is the string 'Complete!' found in fileContent?
        if (/Complete!/i.test(fileContent)) {
            // stop this timer and set to not re-iterate this function

            if(typeof timerCheckEof !== "undefined"){
              clearTimeout(timerCheckEof);
            }
            eof=1;
            
            //Redirect after 1 second
            timerReadMessageFile = setTimeout(redirectToMainPage, 1000);

        } else {
            // recursively call function providing we haven't completed.
            if(eof==0)
                timerCheckEof = setTimeout(readLogFileForMessages, 50);
        }
    }
    else
    {
        // recursively call function providing we haven't completed.
        if(eof==0)
            timerCheckEof = setTimeout(readLogFileForMessages, 250);
    }
}
