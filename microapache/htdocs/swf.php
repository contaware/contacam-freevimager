<?php
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta name="author" content="Oliver Pfister" />
<?php
$doc_root = $_SERVER['DOCUMENT_ROOT'];
$filename = $_GET['file'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = rtrim($doc_root,"\\/")."/".trim($filename,"\\/");
$currentswf = basename(substr($filename, strrpos($filename, '/') + 1), '.swf');
echo "<title>$currentswf</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
<script type="text/javascript">
//<![CDATA[
// Major version of Flash required
var requiredMajorVersion = 6;
// Minor version of Flash required
var requiredMinorVersion = 0;
// Revision of Flash required
var requiredRevision = 40;
//]]>
</script>
<script type="text/vbscript">
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
<script type="text/javascript">
//<![CDATA[
function getIEVersion() {
	if (/MSIE (\d+\.\d+)/.test(navigator.userAgent)) {
		var ieversion = new Number(RegExp.$1);
		return ieversion;
	}
	else if (/Trident\/.*rv:(\d+\.\d+)/.test(navigator.userAgent)) {
		var ieversion = new Number(RegExp.$1);
		return ieversion;
	}
	else
		return -1;
}
var isWin = (navigator.appVersion.toLowerCase().indexOf("win") != -1) ? true : false;
var isOpera = (navigator.userAgent.indexOf("Opera") != -1) ? true : false;
var useVBScript = (getIEVersion() >= 0 && getIEVersion() < 11 && isWin && !isOpera);
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
		if (useVBScript) {
			versionStr = VBGetSwfVer(i);
		} else {
			versionStr = JSGetSwfVer(i);            
		}
		if (versionStr == -1 ) { 
			return false;
		} else if (versionStr != 0) {
			if (useVBScript) {
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
echo "#slidercontainer {\n";
echo "position: relative;\n";
echo "margin: 0 auto;\n";
echo "text-align: left;\n";
echo "width: " . $width . "px;\n}\n";
?>
/*]]>*/
</style>
</head>

<body>
<?php
if (!is_file("$full_path")) {
	echo "<div style=\"text-align: center\"><h2>\n";
	echo NOFILE;
	echo "</h2></div>\n";
	echo "</body>\n";
	echo "</html>\n";
	exit();
}
?>

<div class="wrap" id="playercontainer">
<script type="text/javascript">
//<![CDATA[ 
var hasRightVersion = DetectFlashVer(requiredMajorVersion, requiredMinorVersion, requiredRevision);
if (hasRightVersion)
{
	var oeTags = '<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"'
	+ 'codebase="<?php echo $scheme;?>://fpdownload.adobe.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,40,0"'
	+ 'id="myFlashMovie" width="<?php echo $width;?>" height="<?php echo $height;?>">'
	+ '<?php echo "<param name=\"movie\" value=\"$filename\">";?>'
	+ '<param name="quality" value="high" />'
	+ '<param name="play" value="false" />'
	+ '<param name="allownetworking" value="internal" />'
	+ '<param name="allowScriptAccess" value="never" />'
	+ '<param name="scale" value="exactfit" />'
	+ '<param name="menu" value="false" />'
	+ '<param name="loop" value="true" />'
	+ '<param name="swliveconnect" value="false" />'
	+ '<param name="wmode" value="opaque" />'
	+ '<?php echo "<embed src=\"$filename\"";?>'
	+ 'play="false" swliveconnect="false" name="myFlashMovie" quality="high" wmode="opaque"'  
	+ 'width="<?php echo $width;?>" height="<?php echo $height;?>" menu="false" allownetworking="internal" allowScriptAccess="never"'
	+ 'loop="true" scale="exactfit" type="application/x-shockwave-flash"'
	+ 'pluginspage="<?php echo $scheme;?>://www.adobe.com/go/getflashplayer">'
	+ '</embed>'
	+ '</object>';
	document.write(oeTags);   // embed the flash movie
}
else {
	var alternateContent = 'This content requires the Flash Player '
	+ '<a href="<?php echo $scheme;?>://www.adobe.com/go/getflashplayer" target="_blank">Get Flash</a>';
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
	echo "<input type=\"button\" value=\"Play\" name=\"Play\" class=\"playerbutton\" onclick=\"PlayFlashMovie();\" />";
	echo "<input type=\"button\" value=\"Stop\" name=\"Stop\" class=\"playerbutton\" onclick=\"StopFlashMovie();\" />";
	echo "<input type=\"button\" value=\"Rew\" name=\"Rewind\" class=\"playerbutton\" onclick=\"RewindFlashMovie();\" />";
	echo "<input type=\"button\" value=\"&lt;&lt;\" name=\"PrevFrameFast\" class=\"playerbutton\" onclick=\"PrevFrameFastFlashMovie();\" ondblclick=\"PrevFrameFastFlashMovie();\" />";
	echo "<input type=\"button\" value=\"&lt;\" name=\"PrevFrame\" class=\"playerbutton\" onclick=\"PrevFrameFlashMovie();\" ondblclick=\"PrevFrameFlashMovie();\" />";
	echo "<input type=\"text\" readonly=\"readonly\" id=\"infotext\" name=\"infotext\" class=\"playerinfotext\" value=\"0%\" size=\"5\" />";
	echo "<input type=\"button\" value=\"&gt;\" name=\"NextFrame\" class=\"playerbutton\" onclick=\"NextFrameFlashMovie();\" ondblclick=\"NextFrameFlashMovie();\" />";
	echo "<input type=\"button\" value=\"&gt;&gt;\" name=\"NextFrameFast\" class=\"playerbutton\" onclick=\"NextFrameFastFlashMovie();\" ondblclick=\"NextFrameFastFlashMovie();\" />";
	echo "<input type=\"button\" value=\"Zoom+\" name=\"Zoomin\" class=\"playerbutton\" onclick=\"ZoominFlashMovie();\" ondblclick=\"ZoominFlashMovie();\" />";
	echo "<input type=\"button\" value=\"Zoom-\" name=\"Zoomout\" class=\"playerbutton\" onclick=\"ZoomoutFlashMovie();\" ondblclick=\"ZoomoutFlashMovie();\" />";
} else {
	echo "<input type=\"button\" value=\"Play\" name=\"Play\" class=\"playerbutton\" onclick=\"PlayFlashMovie();\" />";
	echo "<input type=\"button\" value=\"Stop\" name=\"Stop\" class=\"playerbutton\" onclick=\"StopFlashMovie();\" />";
	echo "<input type=\"button\" value=\"Rew\" name=\"Rewind\" class=\"playerbutton\" onclick=\"RewindFlashMovie();\" />";
	echo "<input type=\"button\" value=\"&lt;&lt;\" name=\"PrevFrameFast\" class=\"playerbutton\" onclick=\"PrevFrameFastFlashMovie();\" />";
	echo "<input type=\"button\" value=\"&lt;\" name=\"PrevFrame\" class=\"playerbutton\" onclick=\"PrevFrameFlashMovie();\" />";
	echo "<input type=\"text\" readonly=\"readonly\" id=\"infotext\" name=\"infotext\" class=\"playerinfotext\" value=\"0%\" size=\"5\" />";
	echo "<input type=\"button\" value=\"&gt;\" name=\"NextFrame\" class=\"playerbutton\" onclick=\"NextFrameFlashMovie();\" />";
	echo "<input type=\"button\" value=\"&gt;&gt;\" name=\"NextFrameFast\" class=\"playerbutton\" onclick=\"NextFrameFastFlashMovie();\" />";
	echo "<input type=\"button\" value=\"Zoom+\" name=\"Zoomin\" class=\"playerbutton\" onclick=\"ZoominFlashMovie();\" />";
	echo "<input type=\"button\" value=\"Zoom-\" name=\"Zoomout\" class=\"playerbutton\" onclick=\"ZoomoutFlashMovie();\" />";
}
?>
</div>
</form>
</div>

<?php
echo "<div style=\"text-align: center\">\n";
echo "<div id=\"slidercontainer\">\n";
echo "<div id=\"track1\" style=\"width: " . $width . "px;\n";
echo "background-repeat: repeat-x; background-position: center left; height: 26px; padding: 0px;\">\n";
echo "<div id=\"handle1\" style=\"width: 13px; height: 26px;\"><img src=\"styles/scaler_slider_$style_postfix.gif\" alt=\"[Slider]\" /></div>\n";
echo "</div>\n";
echo "</div>\n";
echo "</div>\n";
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
echo "<div style=\"text-align: center\">\n";
echo "<span class=\"globalbuttons\">";
if ($prevkey >= 0) {
	$prevrequesturi = str_replace($currentswf . '.swf', $_GET["$prevkey"] . '.swf', $_SERVER['REQUEST_URI']);
	$prevrequesturi = htmlspecialchars($prevrequesturi);
	echo "<a href=\"javascript:;\" onclick=\"parent.window.name = '" . $_GET["$prevkey"] . "'; window.location.href = '" . $prevrequesturi . "'; return false;\">&lt;</a>&nbsp;";
}
if (isset($_GET['backuri']))
	echo "<a class=\"backbuttons\" href=\"" . htmlspecialchars($_GET['backuri']) . "\">&nbsp;</a>&nbsp;";
if ($nextkey <= $lastkey) {
	$nextrequesturi = str_replace($currentswf . '.swf', $_GET["$nextkey"] . '.swf', $_SERVER['REQUEST_URI']);
	$nextrequesturi = htmlspecialchars($nextrequesturi);
	echo "<a href=\"javascript:;\" onclick=\"parent.window.name = '" . $_GET["$nextkey"] . "'; window.location.href = '" . $nextrequesturi . "'; return false;\">&gt;</a>&nbsp;";
}
echo "<a class=\"savebuttons\" href=\"download.php?file=" . urlencode($filename) . "\">&nbsp;</a>";
echo "</span>\n";
echo "</div>\n";
?>

<script src="js/prototype.js" type="text/javascript"></script>
<script src="js/scriptaculous.js?load=slider" type="text/javascript"></script>
<script type="text/javascript">
//<![CDATA[
var mySlider = new Control.Slider('handle1','track1',{axis:'horizontal'});
function resizeSwf() {
	var width = parseInt("<?php echo $width;?>");
	var height = parseInt("<?php echo $height;?>");
	if (width <= 0)
		width = 1;
	if (height <= 0)
		height = 1;
	var windowWidth = Math.max(document.documentElement.clientWidth, window.innerWidth || 0);
	var windowHeight = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
	if (windowWidth <= 0 || windowHeight <= 0) { // in case not fully loaded
		window.setTimeout("resizeSwf()", 1000);
		return;
	}
	// Left and right margin
	windowWidth -= 50; // this must be less than 60, see snapshothistory.php!
	// Bottom player control and back button
	windowHeight -= 120;
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
		slidercontainer.style.width = fittedSlider + "px";
		mySlider.trackLength = mySlider.maximumOffset() - mySlider.minimumOffset();
		mySlider.setValue(mySlider.value);
	}
}
//]]>
</script>
<script type="text/javascript" src="js/swf.js"></script>
<script type="text/javascript">
//<![CDATA[
InitPlayer();
resizeSwf();
//]]>
</script>
</body>
</html>