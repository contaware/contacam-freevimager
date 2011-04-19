<?php
/* Copyright © 2005-2011 Oliver Pfister, Switzerland, Web: www.contaware.com
 *
 * This source code is provided "as is", without any express or implied 
 * warranty. In no event will the author/developer be held liable or 
 * accountable in any way for any damages arising from the use of this 
 * source code. 
 *
 * Copyright / Usage Details: 
 *
 * You are allowed to modify this source code in any way you want provided 
 * that this license statement remains with this source code and that you
 * put a comment in this source code about the changes you have done.
 *
 *-------------------------------------------------------------------------*/
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<?php
echo "<title>Flash Player</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
<script language="JavaScript" type="text/javascript">
//<![CDATA[
// Major version of Flash required
var requiredMajorVersion = 6;
// Minor version of Flash required
var requiredMinorVersion = 0;
// Revision of Flash required
var requiredRevision = 40;
//]]>
</script>
<script language="VBScript" type="text/vbscript">
'<![CDATA[
Function VBGetSwfVer(i)
on error resume next
Dim swControl, swVersion
swVersion = 0
set swControl = CreateObject("ShockwaveFlash.ShockwaveFlash." + CStr(i))
if (IsObject(swControl)) then
	swVersion = swControl.GetVariable("$version")
end if
VBGetSwfVer = swVersion
End Function
']]>
</script>
<script language="JavaScript1.1" type="text/javascript">
//<![CDATA[
var isIE  = (navigator.appVersion.indexOf("MSIE") != -1) ? true : false;
var isWin = (navigator.appVersion.toLowerCase().indexOf("win") != -1) ? true : false;
var isOpera = (navigator.userAgent.indexOf("Opera") != -1) ? true : false;
// JavaScript helper required to detect Flash Player PlugIn version information
function JSGetSwfVer(i) {
	// NS/Opera version >= 3 check for Flash plugin in plugin array
	if (navigator.plugins != null && navigator.plugins.length > 0) {
		if (navigator.plugins["Shockwave Flash 2.0"] || navigator.plugins["Shockwave Flash"]) {
			var swVer2 = navigator.plugins["Shockwave Flash 2.0"] ? " 2.0" : "";
			var flashDescription = navigator.plugins["Shockwave Flash" + swVer2].description;
			descArray = flashDescription.split(" ");
			tempArrayMajor = descArray[2].split(".");
			versionMajor = tempArrayMajor[0];
			versionMinor = tempArrayMajor[1];
			if ( descArray[3] != "" ) {
				tempArrayMinor = descArray[3].split("r");
			} else {
				tempArrayMinor = descArray[4].split("r");
			}
			versionRevision = tempArrayMinor[1] > 0 ? tempArrayMinor[1] : 0;
			flashVer = versionMajor + "." + versionMinor + "." + versionRevision;
		} else {
			flashVer = -1;
		}
	}
	// MSN/WebTV 2.6 supports Flash 4
	else if (navigator.userAgent.toLowerCase().indexOf("webtv/2.6") != -1) flashVer = 4;
	// WebTV 2.5 supports Flash 3
	else if (navigator.userAgent.toLowerCase().indexOf("webtv/2.5") != -1) flashVer = 3;
	// older WebTV supports Flash 2
	else if (navigator.userAgent.toLowerCase().indexOf("webtv") != -1) flashVer = 2;
	// Can't detect in all other cases
	else {	
		flashVer = -1;
	}
	return flashVer;
} 
// If called with no parameters this function returns a floating point value 
// which should be the version of the Flash Player or 0.0 
// ex: Flash Player 7r14 returns 7.14
// If called with reqMajorVer, reqMinorVer, reqRevision returns true if that version or greater is available
function DetectFlashVer(reqMajorVer, reqMinorVer, reqRevision) 
{
	reqVer = parseFloat(reqMajorVer + "." + reqRevision);
	// loop backwards through the versions until we find the newest version 
	for (i=25;i>0;i--) {    
		if (isIE && isWin && !isOpera) {
			versionStr = VBGetSwfVer(i);
		} else {
			versionStr = JSGetSwfVer(i);            
		}
		if (versionStr == -1 ) { 
			return false;
		} else if (versionStr != 0) {
			if(isIE && isWin && !isOpera) {
				tempArray         = versionStr.split(" ");
				tempString        = tempArray[1];
				versionArray      = tempString .split(",");                             
			} else {
				versionArray      = versionStr.split(".");
			}
			versionMajor      = versionArray[0];
			versionMinor      = versionArray[1];
			versionRevision   = versionArray[2];
			versionString     = versionMajor + "." + versionRevision;   // 7.0r24 == 7.24
			versionNum        = parseFloat(versionString);
			// is the major.revision >= requested major.revision AND the minor version >= requested minor
			if ( (versionMajor > reqMajorVer) && (versionNum >= reqVer) ) {
				return true;
			} else {
				return ((versionNum >= reqVer && versionMinor >= reqMinorVer) ? true : false ); 
			}
		}
	}       
	return (reqVer ? false : 0.0);
}
//]]>
</script>
<!-- to correct the unsightly Flash of Unstyled Content. http://www.bluerobot.com/web/css/fouc.asp -->
<script type="text/javascript"></script>
<script src="js/prototype.js" type="text/javascript"></script>
<script src="js/scriptaculous.js?load=slider" type="text/javascript"></script>
<style type="text/css">
/*<![CDATA[*/
<?php
if (!isset($_GET['width']))
	$width = WIDTH;
else
	$width = $_GET['width'];
if (!isset($_GET['height']))
	$height = HEIGHT;
else
	$height = $_GET['height'];
$stylewidth = intval($width) + 1;
echo "#slidercontainer {\n";
echo "position: relative;\n";
echo "margin: 0 auto;\n";
echo "text-align: left;\n";
echo "width: " . $stylewidth . "px;\n}\n";
?>
/*]]>*/
</style>
</head>

<body>
<?php
$doc_root = $_SERVER['DOCUMENT_ROOT'];
$filename = $_GET['file'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = "$doc_root/" . trim($filename,"\\/");
if (!is_file("$full_path")) {
	echo "<div align=\"center\"><h2>\n";
	echo NOFILE;
	echo "</h2></div>\n";
	echo "</body>\n";
	echo "</html>\n";
	exit();
}
?>

<div class="wrap" id="playercontainer">
<script language="JavaScript" type="text/javascript">
//<![CDATA[ 
var hasRightVersion = DetectFlashVer(requiredMajorVersion, requiredMinorVersion, requiredRevision);
if (hasRightVersion)
{
	var oeTags = '<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"'
	+ 'codebase="http://active.macromedia.com/flash6/cabs/swflash.cab#version=6,0,40,0"'
	+ 'id="myFlashMovie" width="<?php echo $width;?>" height="<?php echo $height;?>">'
	+ '<?php echo "<param name=\"movie\" value=\"$filename\">";?>'
	+ '<param name="quality" value="high" />'
	+ '<param name="play" value="false" />'
	+ '<param name="allownetworking" value="internal" />'
	+ '<param name="allowScriptAccess" value="never" />'
	+ '<param name="scale" value="exactfit" />'
	+ '<param name="menu" value="false" />'
	+ '<param name="loop" value="false" />'
	+ '<param name="swliveconnect" value="false" />'
	+ '<param name="wmode" value="opaque" />'
	+ '<?php echo "<embed src=\"$filename\"";?>'
	+ 'play="false" swliveconnect="false" name="myFlashMovie" quality="high" wmode="opaque"'  
	+ 'width="<?php echo $width;?>" height="<?php echo $height;?>" menu="false" allownetworking="internal" allowScriptAccess="never"'
	+ 'loop="false" scale="exactfit" type="application/x-shockwave-flash"'
	+ 'pluginspage="http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash">'
	+ '</embed>'
	+ '</object>';
	document.write(oeTags);   // embed the flash movie
}
else {
	var alternateContent = 'This content requires the Flash Player '
	+ '<a href="http://www.adobe.com/go/getflashplayer" target="_blank">Get Flash</a>';
	document.write(alternateContent);  // insert non-flash content
}
//]]>
</script>
<noscript>
<b>This player requires JavaScript!</b>
</noscript>
</div><br />

<div id="buttonscontainer">
<form name="controller" action="" method="post" id="controller">
<div class="wrap">
<?php
$IEVersion = getIEVersion();
if ($IEVersion >= 0 && $IEVersion < 9) {
	echo "<input type=\"button\" value=\"Play\" name=\"Play\" class=\"playerbutton\" onclick=\"PlayFlashMovie();\" /><input type=\"button\" value=\"Stop\" name=\"Stop\" class=\"playerbutton\" onclick=\"StopFlashMovie();\" /><input type=\"button\" value=\"Rew\" name=\"Rewind\" class=\"playerbutton\" onclick=\"RewindFlashMovie();\" /><input type=\"button\" value=\"&lt;&lt;\" name=\"PrevFrameFast\" class=\"playerbutton\" onclick=\"PrevFrameFastFlashMovie();\" ondblclick=\"PrevFrameFastFlashMovie();\" /><input type=\"button\" value=\"&lt;\" name=\"PrevFrame\" class=\"playerbutton\" onclick=\"PrevFrameFlashMovie();\" ondblclick=\"PrevFrameFlashMovie();\" /><input type=\"text\" readonly=\"readonly\" id=\"infotext\" name=\"infotext\" class=\"playerinfotext\" value=\"0%\" size=\"5\" /><input type=\"button\" value=\"&gt;\" name=\"NextFrame\" class=\"playerbutton\" onclick=\"NextFrameFlashMovie();\" ondblclick=\"NextFrameFlashMovie();\" /><input type=\"button\" value=\"&gt;&gt;\" name=\"NextFrameFast\" class=\"playerbutton\" onclick=\"NextFrameFastFlashMovie();\" ondblclick=\"NextFrameFastFlashMovie();\" /><input type=\"button\" value=\"Zoom+\" name=\"Zoomin\" class=\"playerbutton\" onclick=\"ZoominFlashMovie();\" ondblclick=\"ZoominFlashMovie();\" /><input type=\"button\" value=\"Zoom-\" name=\"Zoomout\" class=\"playerbutton\" onclick=\"ZoomoutFlashMovie();\" ondblclick=\"ZoomoutFlashMovie();\" />";
} else {
	echo "<input type=\"button\" value=\"Play\" name=\"Play\" class=\"playerbutton\" onclick=\"PlayFlashMovie();\" /><input type=\"button\" value=\"Stop\" name=\"Stop\" class=\"playerbutton\" onclick=\"StopFlashMovie();\" /><input type=\"button\" value=\"Rew\" name=\"Rewind\" class=\"playerbutton\" onclick=\"RewindFlashMovie();\" /><input type=\"button\" value=\"&lt;&lt;\" name=\"PrevFrameFast\" class=\"playerbutton\" onclick=\"PrevFrameFastFlashMovie();\" /><input type=\"button\" value=\"&lt;\" name=\"PrevFrame\" class=\"playerbutton\" onclick=\"PrevFrameFlashMovie();\" /><input type=\"text\" readonly=\"readonly\" id=\"infotext\" name=\"infotext\" class=\"playerinfotext\" value=\"0%\" size=\"5\" /><input type=\"button\" value=\"&gt;\" name=\"NextFrame\" class=\"playerbutton\" onclick=\"NextFrameFlashMovie();\" /><input type=\"button\" value=\"&gt;&gt;\" name=\"NextFrameFast\" class=\"playerbutton\" onclick=\"NextFrameFastFlashMovie();\" /><input type=\"button\" value=\"Zoom+\" name=\"Zoomin\" class=\"playerbutton\" onclick=\"ZoominFlashMovie();\" /><input type=\"button\" value=\"Zoom-\" name=\"Zoomout\" class=\"playerbutton\" onclick=\"ZoomoutFlashMovie();\" />";
}
if (SHOW_SAVECOMMAND == 1) {
	echo "<input type=\"button\" value=\"Save\" name=\"Save\" class=\"playerbutton\" onclick=\"window.location.href='download.php?file=" . urlencode($filename) . "';\" />";
}
echo "\n";
?>
</div>
</form>
</div>

<?php
echo "<div align=\"center\">\n";
echo "<div id=\"slidercontainer\">\n";
echo "<div id=\"track1\" style=\"width: " . $width . "px;\n";
echo "background-repeat: repeat-x; background-position: center left; height:26px; padding: 0px 0px 0px 1px;\">\n";
echo "<div id=\"handle1\" style=\"width: 13px; height: 26px;\"><img src=\"styles/scaler_slider_$style_postfix.gif\" alt=\"[Slider]\" /></div>\n";
echo "</div>\n";
echo "</div>\n";
echo "</div>\n";

if (!isset($_GET['back']) || $_GET['back'] != 'no') {
	$currentswf = basename(substr($filename, strrpos($filename, '/') + 1), '.swf');
	$currentkey = '0';
	$lastkey = 0;
	foreach($_GET as $key=>$val) {
		if (is_numeric($key) && intval($key) >= 0) {
			if ($currentswf == $val)
				$currentkey = $key;
			$lastkey = intval($key);
		}
	}
	$prevkey = intval($currentkey) - 1;
	$nextkey = intval($currentkey) + 1;
	echo "<br/>\n";
	echo "<div align=\"center\">\n";
	if ($prevkey >= 0) {
		$prevrequesturi = str_replace($currentswf . '.swf', $_GET["$prevkey"] . '.swf', $_SERVER['REQUEST_URI']);
		$prevrequesturi = str_replace("&", "&amp;", $prevrequesturi);
		echo "<a class=\"back\" href=\"$prevrequesturi\" onclick=\"top.window.name = '" . $_GET["$prevkey"] . "';\">&lt;</a>&nbsp;\n";
	}
	if (isset($_GET['backuri']))
		echo "<a class=\"back\" href=\"" . str_replace("&", "&amp;", $_GET['backuri']) . "\">" . BACK . "</a>\n";
	else
		echo "<a class=\"back\" href=\"javascript:history.back();\">" . BACK . "</a>\n";
	if ($nextkey <= $lastkey) {
		$nextrequesturi = str_replace($currentswf . '.swf', $_GET["$nextkey"] . '.swf', $_SERVER['REQUEST_URI']);
		$nextrequesturi = str_replace("&", "&amp;", $nextrequesturi);
		echo "&nbsp;<a class=\"back\" href=\"$nextrequesturi\" onclick=\"top.window.name = '" . $_GET["$nextkey"] . "';\">&gt;</a>\n";
	}
	echo "</div>\n";
}
?>
<script type="text/javascript" src="js/swf.js"></script>
<script language="JavaScript" type="text/javascript">
//<![CDATA[ 
function resizeSwf() {
<?php
	if (isset($_GET['back']) && $_GET['back'] == 'no')
		echo "	var back = false;\n";
	else
		echo "	var back = true;\n";
?>
	var width = parseInt("<?php echo $width;?>");
	var height = parseInt("<?php echo $height;?>");
	if (width <= 0)
		width = 1;
	if (height <= 0)
		height = 1;
	var windowWidth = 0;
	var windowHeight = 0;
	if (typeof(window.innerWidth) == 'number') {
		// Non-IE
		windowWidth = window.innerWidth;
		windowHeight = window.innerHeight;
	} else if (document.documentElement && (document.documentElement.clientWidth || document.documentElement.clientHeight)) {
		// IE 6+ in 'standards compliant mode'
		windowWidth = document.documentElement.clientWidth;
		windowHeight = document.documentElement.clientHeight;
	} else if (document.body && (document.body.clientWidth || document.body.clientHeight)) {
		// IE 4 compatible
		windowWidth = document.body.clientWidth;
		windowHeight = document.body.clientHeight;
	}
	// Left and right margin
	windowWidth -= 50; // this must be less than 60, see snapshothistory.php!
	// Bottom player control and ev. back button
	if (back)
		windowHeight -= 114;
	else
		windowHeight -= 80; // this must be less than 90, see snapshothistory.php!
	// Set a min. size
	if (windowWidth < 32)
		windowWidth = 32;
	if (windowHeight < 32)
		windowHeight = 32;
	var fittedWidth;
	var fittedHeight;
	var fittedSlider;
	if (width > windowWidth) {
		fittedWidth = windowWidth;
		fittedSlider = windowWidth;
	}
	else {
		fittedWidth = width;
		fittedSlider = width;
	}
	if (height > windowHeight)
		fittedHeight = windowHeight;
	else
		fittedHeight = height;
	if (fittedWidth / fittedHeight > width / height)
		fittedWidth = parseInt((fittedHeight * width) / height);
	else
		fittedHeight = parseInt((fittedWidth * height) / width);
	var flashMovie = GetFlashMovieObject("myFlashMovie");
	flashMovie.width = fittedWidth;
	flashMovie.height = fittedHeight;
	var slidercontainer = document.getElementById('slidercontainer');
	var track1 = document.getElementById('track1');
	if (slidercontainer && track1) {
		track1.style.width = fittedSlider + "px";
		fittedSlider++;
		slidercontainer.style.width = fittedSlider + "px";
		mySlider.trackLength = mySlider.maximumOffset() - mySlider.minimumOffset();
		mySlider.setValue(mySlider.value);
	}
} 
InitPlayer();
resizeSwf();
window.onresize=resizeSwf;
//]]>
</script>
</body>
</html>
