// //
// Copyright 2005 SurveySite. All rights reserved.

// Name: Microsoft-www
// Date: 2006-06-15


// Multiple script protection.
if (!window.SiteRecruit_Globals) {

// Create the configuration, globals, and constants namespaces.
var SiteRecruit_Config = new Object();
var SiteRecruit_Globals = new Object();
var SiteRecruit_Constants = new Object();

// Validation variables.
SiteRecruit_Globals.parseFlag = false;
SiteRecruit_Globals.empty = false;

// Browser information.
SiteRecruit_Constants.browser = new Object();
SiteRecruit_Constants.browser.internetExplorer = 'Microsoft Internet Explorer';
SiteRecruit_Constants.browser.mozilla = 'Netscape';

// Check browser information.
SiteRecruit_Globals.browserName = navigator.appName; 
SiteRecruit_Globals.browserVersion = parseInt(navigator.appVersion);

// Initialize browser flags.
SiteRecruit_Globals.isInternetExplorer = false;
SiteRecruit_Globals.isMozilla = false;
SiteRecruit_Globals.isInternetExplorer7 = false;

// Check for Internet Explorer based browsers.
if (SiteRecruit_Globals.browserName == SiteRecruit_Constants.browser.internetExplorer)
{
    if (SiteRecruit_Globals.browserVersion > 3)
    {
        // Only 5.5 and above.
        var a = navigator.userAgent.toLowerCase();
        if (a.indexOf("msie 5.0") == -1 && a.indexOf("msie 5.0") == -1)
        {
            SiteRecruit_Globals.isInternetExplorer = true;
        }
        
        // Check for 7.
        if (a.indexOf("msie 7") != -1)
        {
            SiteRecruit_Globals.isInternetExplorer7 = true;
        }
    }
}

// Check for Mozilla based browsers.
if (SiteRecruit_Globals.browserName == SiteRecruit_Constants.browser.mozilla)
{
    if (SiteRecruit_Globals.browserVersion > 4)
    {
        SiteRecruit_Globals.isMozilla = true;
    }
}

// Cookie lifetime.
SiteRecruit_Constants.cookieLifetimeType = new Object();
SiteRecruit_Constants.cookieLifetimeType.duration = 1;
SiteRecruit_Constants.cookieLifetimeType.expireDate = 2;
    
// Invitation type.
SiteRecruit_Constants.invitationType = new Object();
SiteRecruit_Constants.invitationType.standard = 0;
SiteRecruit_Constants.invitationType.email = 1;
SiteRecruit_Constants.invitationType.domainDeparture = 2;
    
// Cookie type flags.
SiteRecruit_Constants.cookieType = new Object();
SiteRecruit_Constants.cookieType.alreadyAsked = 1;
SiteRecruit_Constants.cookieType.inProgress = 2;

// Alignment types.
SiteRecruit_Constants.horizontalAlignment = new Object();
SiteRecruit_Constants.horizontalAlignment.left = 0;
SiteRecruit_Constants.horizontalAlignment.middle = 1;
SiteRecruit_Constants.horizontalAlignment.right = 2;
SiteRecruit_Constants.verticalAlignment = new Object();
SiteRecruit_Constants.verticalAlignment.top = 0;
SiteRecruit_Constants.verticalAlignment.middle = 1;
SiteRecruit_Constants.verticalAlignment.bottom = 2;

// Survey cookie configuration.
SiteRecruit_Config.cookieName = 'msresearch';
SiteRecruit_Config.cookieDomain = '.microsoft.com';
SiteRecruit_Config.cookiePath = '/';

// Cookie element join character.
SiteRecruit_Constants.cookieJoinChar = ':';

// Settings for cookie lifetime.
SiteRecruit_Config.cookieLifetimeType = 1;

    // Duration of the cookie in days.
    SiteRecruit_Config.cookieDuration = 90;

// Duration of the rapid cookie.
SiteRecruit_Config.rapidCookieDuration = 0;
// //
// Copyright 2005 SurveySite. All rights reserved.

// KeepAlive class definition.
function SiteRecruit_KeepAlive()
{
    // Time between page checks.
    this.keepAlivePollDelay = 1000;

    // Unique (well, sorta) ID for this page.
    this.id = Math.random();

    // Attach methods.
    this.attemptStart = KeepAlive_attemptStart;
    this.checkCookie = KeepAlive_checkCookie;
    this.inProgressCookieExists = KeepAlive_inProgressCookieExists;

    // Start the KeepAlive if an in-progress cookie exists.
    function KeepAlive_attemptStart()
    {
        if (this.inProgressCookieExists())
        {
            setInterval('SiteRecruit_Globals.keepAlive.checkCookie()', this.keepAlivePollDelay);
        }
    }
    
    // Check and update the cookie.
    function KeepAlive_checkCookie()
    {
        if (this.inProgressCookieExists())
        {
            var j = SiteRecruit_Constants.cookieJoinChar;
            
            // Update the cookie with the current time and location.
            var c = SiteRecruit_Config.cookieName + '=' + SiteRecruit_Constants.cookieType.inProgress
                + j + escape(document.location)
                + j + (new Date()).getTime()
                + j + this.id
                + '; path=' + SiteRecruit_Config.cookiePath;
            
            if (SiteRecruit_Config.cookieDomain != '')
            {
                c += '; domain=' + SiteRecruit_Config.cookieDomain;
            }
            
            document.cookie = c;
        }
    }

    // Return true if an in-progress cookie exists.
    function KeepAlive_inProgressCookieExists()
    {
        var c = SiteRecruit_Config.cookieName + '=' + SiteRecruit_Constants.cookieType.inProgress;
    
        if (document.cookie.indexOf(c) != -1)
        {
            return true;
        }
        
        return false;
    }
}

// Create the KeepAlive if a suitable cookie exists.
SiteRecruit_Globals.keepAlive = new SiteRecruit_KeepAlive();
SiteRecruit_Globals.keepAlive.attemptStart();
// //
// Copyright 2005 SurveySite. All rights reserved.

// Broker class definition.
function SiteRecruit_PageConfigurationBroker()
{
    this.urls = new Array();
    this.pages = new Array();
    this.priorities = new Array();
        
    // Attach methods.
    this.start = PageConfigurationBroker_start;
    this.initializeMapping = PageConfigurationBroker_initializeMapping;
    this.getConfigurationForPage = PageConfigurationBroker_getConfigurationForPage;
    this.loadConfiguration = PageConfigurationBroker_loadConfiguration;

    // Start the Broker.
    function PageConfigurationBroker_start(url)
    {
        this.initializeMapping();
        
        // Decide if there are surveys to run for this page.
        var configuration = this.getConfigurationForPage(url);
        
        // If there is, load id up.
        if (configuration != null)
        {
            this.loadConfiguration(configuration);
        }
    }
    
    // Initializes the page mapping.
    function PageConfigurationBroker_initializeMapping()
    {
        var u = this.urls;
        var p = this.pages;
        var x = this.priorities;
        
                     
                            // http://www.microsoft.com/athome...
                u[0] = '//[\\w\\.-]+/athome';
                p[0] = '/library/svy/SiteRecruit_PageConfiguration_2943mt16-3089mt.js';
                x[0] = 0;
                                        
             
                            // http://www.microsoft.com/athome/security...
                u[1] = '//[\\w\\.-]+/athome/security';
                p[1] = '/library/svy/SiteRecruit_PageConfiguration_2943mt12-2943mt16-3089mt.js';
                x[1] = 1;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows2000/antispy.mspx
                u[2] = '//[\\w\\.-]+/athome/security/protect/windows2000/antispy\\.mspx$';
                p[2] = '/library/svy/SiteRecruit_PageConfiguration_2503mt1-2943mt16-12.js';
                x[2] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows2000/antivirus.mspx
                u[3] = '//[\\w\\.-]+/athome/security/protect/windows2000/antivirus\\.mspx$';
                p[3] = '/library/svy/SiteRecruit_PageConfiguration_2503mt2-2943mt12-16.js';
                x[3] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows2000/default.mspx
                u[4] = '//[\\w\\.-]+/athome/security/protect/windows2000/default\\.mspx$';
                p[4] = '/library/svy/SiteRecruit_PageConfiguration_2503mt3-2943mt16-12.js';
                x[4] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows2000/firewall.mspx
                u[5] = '//[\\w\\.-]+/athome/security/protect/windows2000/firewall\\.mspx$';
                p[5] = '/library/svy/SiteRecruit_PageConfiguration_2503mt4-2943mt16-12.js';
                x[5] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows2000/updates.mspx
                u[6] = '//[\\w\\.-]+/athome/security/protect/windows2000/updates\\.mspx$';
                p[6] = '/library/svy/SiteRecruit_PageConfiguration_2503mt5-2943mt16-12.js';
                x[6] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows9x/antispy.mspx
                u[7] = '//[\\w\\.-]+/athome/security/protect/windows9x/antispy\\.mspx$';
                p[7] = '/library/svy/SiteRecruit_PageConfiguration_2503mt6-2943mt16-12.js';
                x[7] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows9x/antivirus.mspx
                u[8] = '//[\\w\\.-]+/athome/security/protect/windows9x/antivirus\\.mspx$';
                p[8] = '/library/svy/SiteRecruit_PageConfiguration_2503mt7-2943mt16-12.js';
                x[8] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows9x/default.mspx
                u[9] = '//[\\w\\.-]+/athome/security/protect/windows9x/default\\.mspx$';
                p[9] = '/library/svy/SiteRecruit_PageConfiguration_2503mt8-2946mt16-12.js';
                x[9] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows9x/firewall.mspx
                u[10] = '//[\\w\\.-]+/athome/security/protect/windows9x/firewall\\.mspx$';
                p[10] = '/library/svy/SiteRecruit_PageConfiguration_2503mt9-2943mt16-12.js';
                x[10] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windows9x/updates.mspx
                u[11] = '//[\\w\\.-]+/athome/security/protect/windows9x/updates\\.mspx$';
                p[11] = '/library/svy/SiteRecruit_PageConfiguration_2503mt10-2943mt16-12.js';
                x[11] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsme/antispy.mspx
                u[12] = '//[\\w\\.-]+/athome/security/protect/windowsme/antispy\\.mspx$';
                p[12] = '/library/svy/SiteRecruit_PageConfiguration_2503mt11-2943mt16-12.js';
                x[12] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsme/antivirus.mspx
                u[13] = '//[\\w\\.-]+/athome/security/protect/windowsme/antivirus\\.mspx$';
                p[13] = '/library/svy/SiteRecruit_PageConfiguration_2503mt12-2943mt16-12.js';
                x[13] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsme/default.mspx
                u[14] = '//[\\w\\.-]+/athome/security/protect/windowsme/default\\.mspx$';
                p[14] = '/library/svy/SiteRecruit_PageConfiguration_2503mt13-2943mt16-12.js';
                x[14] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsme/firewall.mspx
                u[15] = '//[\\w\\.-]+/athome/security/protect/windowsme/firewall\\.mspx$';
                p[15] = '/library/svy/SiteRecruit_PageConfiguration_2503mt14-2943mt16-12.js';
                x[15] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsme/updates.mspx
                u[16] = '//[\\w\\.-]+/athome/security/protect/windowsme/updates\\.mspx$';
                p[16] = '/library/svy/SiteRecruit_PageConfiguration_2503mt15-2943mt16-12.js';
                x[16] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsxp/antispy.mspx
                u[17] = '//[\\w\\.-]+/athome/security/protect/windowsxp/antispy\\.mspx$';
                p[17] = '/library/svy/SiteRecruit_PageConfiguration_2503mt16-2943mt16-12.js';
                x[17] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsxp/default.mspx
                u[18] = '//[\\w\\.-]+/athome/security/protect/windowsxp/default\\.mspx$';
                p[18] = '/library/svy/SiteRecruit_PageConfiguration_2503mt17-2943mt16-12.js';
                x[18] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsxp/getsp2.mspx
                u[19] = '//[\\w\\.-]+/athome/security/protect/windowsxp/getsp2\\.mspx$';
                p[19] = '/library/svy/SiteRecruit_PageConfiguration_2503mt18-2943mt16-12.js';
                x[19] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsxpsp2/antispy.mspx
                u[20] = '//[\\w\\.-]+/athome/security/protect/windowsxpsp2/antispy\\.mspx$';
                p[20] = '/library/svy/SiteRecruit_PageConfiguration_2503mt19-2943mt16-12.js';
                x[20] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsxpsp2/default.mspx
                u[21] = '//[\\w\\.-]+/athome/security/protect/windowsxpsp2/default\\.mspx$';
                p[21] = '/library/svy/SiteRecruit_PageConfiguration_2503mt20-2943mt16-12.js';
                x[21] = 2;
                                        
             
                            // http://www.microsoft.com/athome/security/protect/windowsxpsp2/wsc.mspx
                u[22] = '//[\\w\\.-]+/athome/security/protect/windowsxpsp2/wsc\\.mspx$';
                p[22] = '/library/svy/SiteRecruit_PageConfiguration_2503mt21-2943mt16-12.js';
                x[22] = 2;
                                        
             
                            // http://www.microsoft.com/atwork...
                u[23] = '//[\\w\\.-]+/atwork';
                p[23] = '/library/svy/SiteRecruit_PageConfiguration_2943mt2-3089mt.js';
                x[23] = 0;
                                        
             
                            // http://www.microsoft.com/australia/smallbusiness...
                u[24] = '//[\\w\\.-]+/australia/smallbusiness(/(?!sbrp)|$)';
                p[24] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_Australia.js';
                x[24] = 0;
                                        
             
                            // http://www.microsoft.com/brasil/pequenasempresas...
                u[25] = '//[\\w\\.-]+/brasil/pequenasempresas';
                p[25] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_Brazil.js';
                x[25] = 0;
                                        
             
                            // http://www.microsoft.com/business...
                u[26] = '//[\\w\\.-]+/business(/(?!executivecircle)|$)';
                p[26] = '/library/svy/SiteRecruit_PageConfiguration_2943mt8.js';
                x[26] = 0;
                                        
             
                            // http://www.microsoft.com/canada/smallbiz...
                u[27] = '//[\\w\\.-]+/canada/smallbiz(/(?!sbplus)|$)';
                p[27] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_EnglishCanada.js';
                x[27] = 0;
                                        
             
                            // http://www.microsoft.com/china/smallbusiness...
                u[28] = '//[\\w\\.-]+/china/smallbusiness';
                p[28] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_China.js';
                x[28] = 0;
                                        
             
                            // http://www.microsoft.com/china/windowsserversystem/facts
                u[29] = '//[\\w\\.-]+/china/windowsserversystem/facts((/)|(/((default)|(index))\\.((html?)|(aspx?)|(mspx))))?$';
                p[29] = '/library/svy/SiteRecruit_PageConfiguration_3201mt-China.js';
                x[29] = 0;
                                        
             
                            // http://www.microsoft.com/communities...
                u[30] = '//[\\w\\.-]+/communities';
                p[30] = '/library/svy/SiteRecruit_PageConfiguration_2943mt4.js';
                x[30] = 0;
                                        
             
                            // http://www.microsoft.com/communities/chats/chatrooms/mvp687.aspx
                u[31] = '//[\\w\\.-]+/communities/chats/chatrooms/mvp687\\.aspx$';
                p[31] = '/library/svy/SiteRecruit_PageConfiguration_3056mt-communities-2943mt4.js';
                x[31] = 1;
                                        
             
                            // http://www.microsoft.com/communities/forums/default.mspx
                u[32] = '//[\\w\\.-]+/communities/forums/default\\.mspx$';
                p[32] = '/library/svy/SiteRecruit_PageConfiguration_3056mt-forums-2943mt4.js';
                x[32] = 1;
                                        
             
                            // http://www.microsoft.com/downloads...
                u[33] = '//[\\w\\.-]+/downloads';
                p[33] = '/library/svy/SiteRecruit_PageConfiguration_2943mt30-2944mt1.js';
                x[33] = 0;
                                        
             
                            // http://www.microsoft.com/downloads/genuineValidation.aspx?familyid=****&displaylang=de
                u[34] = '//[\\w\\.-]+/downloads/genuineValidation\\.aspx\\?familyid=.*&displaylang=de$';
                p[34] = '/library/svy/SiteRecruit_PageConfiguration_2917mt.js';
                x[34] = 1;
                                        
             
                            // http://www.microsoft.com/downloads/genuineValidation.aspx?familyid=****&displaylang=en
                u[35] = '//[\\w\\.-]+/downloads/genuineValidation\\.aspx\\?familyid=.*&displaylang=en$';
                p[35] = '/library/svy/SiteRecruit_PageConfiguration_2914mt.js';
                x[35] = 1;
                                        
             
                            // http://www.microsoft.com/downloads/genuineValidation.aspx?familyid=****&displaylang=es
                u[36] = '//[\\w\\.-]+/downloads/genuineValidation\\.aspx\\?familyid=.*&displaylang=es$';
                p[36] = '/library/svy/SiteRecruit_PageConfiguration_2919mt.js';
                x[36] = 1;
                                        
             
                            // http://www.microsoft.com/downloads/genuineValidation.aspx?familyid=****&displaylang=fr
                u[37] = '//[\\w\\.-]+/downloads/genuineValidation\\.aspx\\?familyid=.*&displaylang=fr$';
                p[37] = '/library/svy/SiteRecruit_PageConfiguration_2918mt.js';
                x[37] = 1;
                                        
             
                            // http://www.microsoft.com/downloads/genuineValidation.aspx?familyid=****&displaylang=ja
                u[38] = '//[\\w\\.-]+/downloads/genuineValidation\\.aspx\\?familyid=.*&displaylang=ja$';
                p[38] = '/library/svy/SiteRecruit_PageConfiguration_2921mt.js';
                x[38] = 1;
                                        
             
                            // http://www.microsoft.com/downloads/genuineValidation.aspx?familyid=****&displaylang=ko
                u[39] = '//[\\w\\.-]+/downloads/genuineValidation\\.aspx\\?familyid=.*&displaylang=ko$';
                p[39] = '/library/svy/SiteRecruit_PageConfiguration_2920mt.js';
                x[39] = 1;
                                        
             
                            // http://www.microsoft.com/downloads/genuineValidation.aspx?familyid=****&displaylang=pt-br
                u[40] = '//[\\w\\.-]+/downloads/genuineValidation\\.aspx\\?familyid=.*&displaylang=pt-br$';
                p[40] = '/library/svy/SiteRecruit_PageConfiguration_2916mt.js';
                x[40] = 1;
                                        
             
                            // http://www.microsoft.com/downloads/genuineValidation.aspx?familyid=****&displaylang=zh-cn
                u[41] = '//[\\w\\.-]+/downloads/genuineValidation\\.aspx\\?familyid=.*&displaylang=zh-cn$';
                p[41] = '/library/svy/SiteRecruit_PageConfiguration_2915mt.js';
                x[41] = 1;
                                        
             
                            // http://www.microsoft.com/dynamics...
                u[42] = '//[\\w\\.-]+/dynamics';
                p[42] = '/library/svy/SiteRecruit_PageConfiguration_2943mt3-3089mt.js';
                x[42] = 0;
                                        
             
                            // http://www.microsoft.com/education...
                u[43] = '//[\\w\\.-]+/education';
                p[43] = '/library/svy/SiteRecruit_PageConfiguration_2943mt5.js';
                x[43] = 0;
                                        
             
                            // http://www.microsoft.com/events...
                u[44] = '//[\\w\\.-]+/events';
                p[44] = '/library/svy/SiteRecruit_PageConfiguration_2943mt6.js';
                x[44] = 0;
                                        
             
                            // http://www.microsoft.com/exchange...
                u[45] = '//[\\w\\.-]+/exchange';
                p[45] = '/library/svy/SiteRecruit_PageConfiguration_2943mt7.js';
                x[45] = 0;
                                        
             
                            // http://www.microsoft.com/france/entrepreneur...
                u[46] = '//[\\w\\.-]+/france/entrepreneur(/(?!plus)|$)';
                p[46] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_France.js';
                x[46] = 0;
                                        
             
                            // http://www.microsoft.com/france/msdn...
                u[47] = '//[\\w\\.-]+/france/msdn';
                p[47] = '/library/svy/SiteRecruit_PageConfiguration_2944mt4-msdn.js';
                x[47] = 0;
                                        
             
                            // http://www.microsoft.com/france/technet...
                u[48] = '//[\\w\\.-]+/france/technet';
                p[48] = '/library/svy/SiteRecruit_PageConfiguration_2944mt4-technet.js';
                x[48] = 0;
                                        
             
                            // http://www.microsoft.com/france/windows...
                u[49] = '//[\\w\\.-]+/france/windows(/|$)';
                p[49] = '/library/svy/SiteRecruit_PageConfiguration_2944mt4-windows.js';
                x[49] = 0;
                                        
             
                            // http://www.microsoft.com/germany/diefakten
                u[50] = '//[\\w\\.-]+/germany/diefakten((/)|(/((default)|(index))\\.((html?)|(aspx?)|(mspx))))?$';
                p[50] = '/library/svy/SiteRecruit_PageConfiguration_3202mt-Germany.js';
                x[50] = 0;
                                        
             
                            // http://www.microsoft.com/germany/kleinunternehmen...
                u[51] = '//[\\w\\.-]+/germany/kleinunternehmen(/(?!small-business-plus)|$)';
                p[51] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_Germany.js';
                x[51] = 0;
                                        
             
                            // http://www.microsoft.com/germany/msdn/...
                u[52] = '//[\\w\\.-]+/germany/msdn';
                p[52] = '/library/svy/SiteRecruit_PageConfiguration_2944mt3-msdn.js';
                x[52] = 0;
                                        
             
                            // http://www.microsoft.com/germany/server...
                u[53] = '//[\\w\\.-]+/germany/server';
                p[53] = '/library/svy/SiteRecruit_PageConfiguration_2944mt3-wss.js';
                x[53] = 0;
                                        
             
                            // http://www.microsoft.com/germany/windows/...
                u[54] = '//[\\w\\.-]+/germany/windows(/|$)';
                p[54] = '/library/svy/SiteRecruit_PageConfiguration_2944mt3-windows.js';
                x[54] = 0;
                                        
             
                            // http://www.microsoft.com/hardware
                u[55] = '//[\\w\\.-]+/hardware((/)|(/default\\.asp)|(/default\\.mspx)|(/default\\.aspx))?$';
                p[55] = '/library/svy/SiteRecruit_PageConfiguration_2546mt_Home.js';
                x[55] = 0;
                                        
             
                            // http://www.microsoft.com/hardware/broadbandnetworking
                u[56] = '//[\\w\\.-]+/hardware/broadbandnetworking((/)|(/default\\.asp)|(/default\\.mspx)|(/default\\.aspx))?$';
                p[56] = '/library/svy/SiteRecruit_PageConfiguration_2546mt_BroadbandNetworking.js';
                x[56] = 0;
                                        
             
                            // http://www.microsoft.com/hardware/mouseandkeyboard/default.mspx
                u[57] = '//[\\w\\.-]+/hardware/mouseandkeyboard/default\\.mspx$';
                p[57] = '/library/svy/SiteRecruit_PageConfiguration_2546mt_MNKB.js';
                x[57] = 0;
                                        
             
                            // http://www.microsoft.com/india/communities/chat/default.aspx
                u[58] = '//[\\w\\.-]+/india/communities/chat/default\\.aspx$';
                p[58] = '/library/svy/SiteRecruit_PageConfiguration_3056mt-indiacomm.js';
                x[58] = 0;
                                        
             
                            // http://www.microsoft.com/india/msdn/chat
                u[59] = '//[\\w\\.-]+/india/msdn/chat((/)|(/((default)|(index))\\.((html?)|(aspx?)|(mspx))))?$';
                p[59] = '/library/svy/SiteRecruit_PageConfiguration_3056mt-indiamsdn.js';
                x[59] = 0;
                                        
             
                            // http://www.microsoft.com/industry/financialservices...
                u[60] = '//[\\w\\.-]+/industry/financialservices';
                p[60] = '/library/svy/SiteRecruit_PageConfiguration_2943mt50.js';
                x[60] = 0;
                                        
             
                            // http://www.microsoft.com/industry/government...
                u[61] = '//[\\w\\.-]+/industry/government';
                p[61] = '/library/svy/SiteRecruit_PageConfiguration_2943mt51.js';
                x[61] = 0;
                                        
             
                            // http://www.microsoft.com/industry/healthcare...
                u[62] = '//[\\w\\.-]+/industry/healthcare';
                p[62] = '/library/svy/SiteRecruit_PageConfiguration_2943mt54.js';
                x[62] = 0;
                                        
             
                            // http://www.microsoft.com/industry/hospitality...
                u[63] = '//[\\w\\.-]+/industry/hospitality';
                p[63] = '/library/svy/SiteRecruit_PageConfiguration_2943mt56.js';
                x[63] = 0;
                                        
             
                            // http://www.microsoft.com/industry/manufacturing...
                u[64] = '//[\\w\\.-]+/industry/manufacturing';
                p[64] = '/library/svy/SiteRecruit_PageConfiguration_2943mt52.js';
                x[64] = 0;
                                        
             
                            // http://www.microsoft.com/industry/professionalservices...
                u[65] = '//[\\w\\.-]+/industry/professionalservices';
                p[65] = '/library/svy/SiteRecruit_PageConfiguration_2943mt55.js';
                x[65] = 0;
                                        
             
                            // http://www.microsoft.com/industry/retail...
                u[66] = '//[\\w\\.-]+/industry/retail';
                p[66] = '/library/svy/SiteRecruit_PageConfiguration_2943mt53.js';
                x[66] = 0;
                                        
             
                            // http://www.microsoft.com/italy/pmi...
                u[67] = '//[\\w\\.-]+/italy/pmi(/(?!plus)|$)';
                p[67] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_Italy.js';
                x[67] = 0;
                                        
             
                            // http://www.microsoft.com/japan/msdn...
                u[68] = '//[\\w\\.-]+/japan/msdn';
                p[68] = '/library/svy/SiteRecruit_PageConfiguration_2944mt2-msdn.js';
                x[68] = 0;
                                        
             
                            // http://www.microsoft.com/japan/office...
                u[69] = '//[\\w\\.-]+/japan/office';
                p[69] = '/library/svy/SiteRecruit_PageConfiguration_2944mt2-office.js';
                x[69] = 0;
                                        
             
                            // http://www.microsoft.com/japan/smallbiz...
                u[70] = '//[\\w\\.-]+/japan/smallbiz';
                p[70] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_Japan.js';
                x[70] = 0;
                                        
             
                            // http://www.microsoft.com/japan/technet...
                u[71] = '//[\\w\\.-]+/japan/technet';
                p[71] = '/library/svy/SiteRecruit_PageConfiguration_2944mt2-technet.js';
                x[71] = 0;
                                        
             
                            // http://www.microsoft.com/japan/windowsserver/facts
                u[72] = '//[\\w\\.-]+/japan/windowsserver/facts((/)|(/((default)|(index))\\.((html?)|(aspx?)|(mspx))))?$';
                p[72] = '/library/svy/SiteRecruit_PageConfiguration_3214mt.js';
                x[72] = 0;
                                        
             
                            // http://www.microsoft.com/japan/windowsxp...
                u[73] = '//[\\w\\.-]+/japan/windowsxp';
                p[73] = '/library/svy/SiteRecruit_PageConfiguration_2944mt2-wxp.js';
                x[73] = 0;
                                        
             
                            // http://www.microsoft.com/learning
                u[74] = '//[\\w\\.-]+/learning((/)|(/default\\.asp)|(/default\\.mspx)|(/default\\.aspx))?$';
                p[74] = '/library/svy/SiteRecruit_PageConfiguration_2943mt9.js';
                x[74] = 0;
                                        
             
                            // http://www.microsoft.com/learning/MyLearning/MyLearningDetails.aspx
                u[75] = '//[\\w\\.-]+/learning/MyLearning/MyLearningDetails\\.aspx$';
                p[75] = '/library/svy/SiteRecruit_PageConfiguration_3126mt.js';
                x[75] = 0;
                                        
             
                            // http://www.microsoft.com/licensing...
                u[76] = '//[\\w\\.-]+/licensing';
                p[76] = '/library/svy/SiteRecruit_PageConfiguration_2943mt10.js';
                x[76] = 0;
                                        
             
                            // http://www.microsoft.com/midsizebusiness...
                u[77] = '//[\\w\\.-]+/midsizebusiness';
                p[77] = '/library/svy/SiteRecruit_PageConfiguration_3123mt.js';
                x[77] = 0;
                                        
             
                            // http://www.microsoft.com/mscorp
                u[78] = '//[\\w\\.-]+/mscorp((/)|(/default\\.asp)|(/default\\.mspx)|(/default\\.aspx))?$';
                p[78] = '/library/svy/SiteRecruit_PageConfiguration_2943mt1.js';
                x[78] = 0;
                                        
             
                            // http://www.microsoft.com/netherlands/ondernemers...
                u[79] = '//[\\w\\.-]+/netherlands/ondernemers(/(?!plus)|$)';
                p[79] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_Netherlands.js';
                x[79] = 0;
                                        
             
                            // http://www.microsoft.com/products/info/...
                u[80] = '//[\\w\\.-]+/products/info';
                p[80] = '/library/svy/SiteRecruit_PageConfiguration_2943mt31.js';
                x[80] = 0;
                                        
             
                            // http://www.microsoft.com/security...
                u[81] = '//[\\w\\.-]+/security';
                p[81] = '/library/svy/SiteRecruit_PageConfiguration_2943mt11-2944mt1.js';
                x[81] = 0;
                                        
             
                            // http://www.microsoft.com/smallbusiness...
                u[82] = '//[\\w\\.-]+/smallbusiness(/(?!small-business-plus)|$)';
                p[82] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_UnitedStates.js';
                x[82] = 0;
                                        
             
                            // http://www.microsoft.com/spain/empresas...
                u[83] = '//[\\w\\.-]+/spain/empresas';
                p[83] = '/library/svy/SiteRecruit_PageConfiguration_2933mt_Spain.js';
                x[83] = 0;
                                        
             
                            // http://www.microsoft.com/sql...
                u[84] = '//[\\w\\.-]+/sql';
                p[84] = '/library/svy/SiteRecruit_PageConfiguration_2943mt14.js';
                x[84] = 0;
                                        
             
                            // http://www.microsoft.com/technet...
                u[85] = '//[\\w\\.-]+/technet(/(?!mnp_utility\\.mspx/framesmenu)|$)';
                p[85] = '/library/svy/SiteRecruit_PageConfiguration_3089mt-2944mt1-2943mt33.js';
                x[85] = 0;
                                        
             
                            // http://www.microsoft.com/technet/community/chats/chatroom.aspx
                u[86] = '//[\\w\\.-]+/technet/community/chats/chatroom\\.aspx$';
                p[86] = '/library/svy/SiteRecruit_PageConfiguration_3056mt-technet-3089mt-2944mt1-2943mt33.js';
                x[86] = 1;
                                        
             
                            // http://www.microsoft.com/technet/itsolutions/branch...
                u[87] = '//[\\w\\.-]+/technet/itsolutions/branch(/|$)';
                p[87] = '/library/svy/SiteRecruit_PageConfiguration_3248mt-Branch-3089mt-2944mt1-2943mt33.js';
                x[87] = 1;
                                        
             
                            // http://www.microsoft.com/technet/itsolutions/cits...
                u[88] = '//[\\w\\.-]+/technet/itsolutions/cits(/|$)';
                p[88] = '/library/svy/SiteRecruit_PageConfiguration_3248mt-cits-3089mt-2944mt1-2943mt33.js';
                x[88] = 1;
                                        
             
                            // http://www.microsoft.com/technet/itsolutions/cits/interopmigration...
                u[89] = '//[\\w\\.-]+/technet/itsolutions/cits/interopmigration(/|$)';
                p[89] = '/library/svy/SiteRecruit_PageConfiguration_3248mt-inter-3089mt-2944mt1-2943mt33.js';
                x[89] = 2;
                                        
             
                            // http://www.microsoft.com/technet/itsolutions/cits/mo...
                u[90] = '//[\\w\\.-]+/technet/itsolutions/cits/mo(/|$)';
                p[90] = '/library/svy/SiteRecruit_PageConfiguration_3248mt-mo-3089mt-2944mt1-2943mt33.js';
                x[90] = 2;
                                        
             
                            // http://www.microsoft.com/technet/itsolutions/smbiz...
                u[91] = '//[\\w\\.-]+/technet/itsolutions/smbiz(/|$)';
                p[91] = '/library/svy/SiteRecruit_PageConfiguration_3248mt-smbiz-3089mt-2944mt1-2943mt33.js';
                x[91] = 1;
                                        
             
                            // http://www.microsoft.com/technet/itsolutions/wssra...
                u[92] = '//[\\w\\.-]+/technet/itsolutions/wssra(/|$)';
                p[92] = '/library/svy/SiteRecruit_PageConfiguration_3248mt-wssra-3089mt-2944mt1-2943mt33.js';
                x[92] = 1;
                                        
             
                            // http://www.microsoft.com/technet/security...
                u[93] = '//[\\w\\.-]+/technet/security';
                p[93] = '/library/svy/SiteRecruit_PageConfiguration_2943mt34-3089mt-2943mt33-2944mt1.js';
                x[93] = 1;
                                        
             
                            // http://www.microsoft.com/technet/security/topics...
                u[94] = '//[\\w\\.-]+/technet/security/topics(/|$)';
                p[94] = '/library/svy/SiteRecruit_PageConfiguration_3248mt-topics-3089mt-2944mt1-2943mt33-2943mt34.js';
                x[94] = 2;
                                        
             
                            // http://www.microsoft.com/windows/ie...
                u[95] = '//[\\w\\.-]+/windows/ie(/|$)';
                p[95] = '/library/svy/SiteRecruit_PageConfiguration_2943mt17.js';
                x[95] = 0;
                                        
             
                            // http://www.microsoft.com/windowsserver2003...
                u[96] = '//[\\w\\.-]+/windowsserver2003';
                p[96] = '/library/svy/SiteRecruit_PageConfiguration_2943mt15.js';
                x[96] = 0;
                                        
             
                            // http://www.microsoft.com/windowsvista....
                u[97] = '//[\\w\\.-]+/windowsvista(/|$)';
                p[97] = '/library/svy/SiteRecruit_PageConfiguration_2943mt21.js';
                x[97] = 0;
                                        
             
                            // http://www.microsoft.com/windowsxp...
                u[98] = '//[\\w\\.-]+/windowsxp';
                p[98] = '/library/svy/SiteRecruit_PageConfiguration_2943mt18.js';
                x[98] = 0;
                                        
             
                            // http://www.microsoft.com/windowsxp/expertzone/chats/chatroom.aspx
                u[99] = '//[\\w\\.-]+/windowsxp/expertzone/chats/chatroom\\.aspx$';
                p[99] = '/library/svy/SiteRecruit_PageConfiguration_3056mt-WXP-2943mt18.js';
                x[99] = 1;
                                        
             
                            // http://www.microsoft.com/windowsxp/mediacenter...
                u[100] = '//[\\w\\.-]+/windowsxp/mediacenter';
                p[100] = '/library/svy/SiteRecruit_PageConfiguration_2943mt19-2943mt18.js';
                x[100] = 1;
                                        
             
                            // http://www.microsoft.com/windowsxp/tabletpc
                u[101] = '//[\\w\\.-]+/windowsxp/tabletpc((/)|(/((default)|(index))\\.((html?)|(aspx?)|(mspx))))?$';
                p[101] = '/library/svy/SiteRecruit_PageConfiguration_2943mt20-2943mt18-default.js';
                x[101] = 2;
                                        
             
                            // http://www.microsoft.com/windowsxp/tabletpc...
                u[102] = '//[\\w\\.-]+/windowsxp/tabletpc';
                p[102] = '/library/svy/SiteRecruit_PageConfiguration_2943mt20-2943mt18.js';
                x[102] = 1;
                                        
            }
    
    // Return the appropriate configuration for the given URL, if any.
    function PageConfigurationBroker_getConfigurationForPage(url)
    {
        
        var currentScore = 0;
        var currentMatch = -1;
        
        // Iterate over each URL.
        for (var i = 0; i < this.urls.length; i++)
        {
            // Do the reg exp match.
            var r = new RegExp(this.urls[i], 'i');
            if (url.toString().search(r) != -1)
            {
                // Take the current if the score is equal or better.
                var newScore = this.priorities[i];
                if (newScore >= currentScore)
                {
                    currentMatch = i;
                    currentScore = newScore;
                }
            }
        }
        
        // If there was a match, return the appropriate page configuration.
        var page = null;
        if (currentMatch >= 0)
        {
            page = this.pages[currentMatch];
        }
        
                
        return page;
    }
    
    // Dynamically loads the associated configuration.
    function PageConfigurationBroker_loadConfiguration(configuration)
    {
        
        document.write('<script language="JavaScript" src="' + configuration + '"></script>');
    }
}

try
{
    // Only run if the browser is supported.
    if (SiteRecruit_Globals.isInternetExplorer || SiteRecruit_Globals.isMozilla)
    {
        // Create and start the Broker for the current location.
        SiteRecruit_Globals.broker = new SiteRecruit_PageConfigurationBroker();
        SiteRecruit_Globals.broker.start(window.location);
    }
}
catch (e)
{
    // Suppress any errors.
}

SiteRecruit_Globals.parseFlag = true;

// Multiple script protection.
}