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
<title>Jpeg Live Viewer</title>
<?php echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n"; ?>
<script language="JavaScript" type="text/javascript">
//<![CDATA[
var x = <?php echo LIVEJPEGREFRESHSEC;?>;
function startPoll() {
	x = x - 1;
	if (x < 1)
		poll();
	setTimeout("startPoll()", 1000);
}
function poll() {
	var now = new Date();
	// The dummy enforces a bypass of the browser image cache
	<?php echo "var camImg = '" . LIVEJPEGURL . "&dummy=' + now.getTime().toString(10);\n";?>
	document.campicture.src = camImg;
	x = <?php echo LIVEJPEGREFRESHSEC;?>;
}
function getIEVersion() {
	if (/MSIE (\d+\.\d+);/.test(navigator.userAgent)){
		var ieversion = new Number(RegExp.$1);
		return ieversion;
	}
	else
		return -1;
}
//]]>
</script>
<!--[if IE]>
<noscript>
<?php
$serveruri = str_replace("&", "&amp;", $_SERVER['REQUEST_URI']);
echo "<meta http-equiv=\"refresh\" content=\"" . LIVEJPEGREFRESHSEC . "; URL=" . $serveruri . "\" />\n";
?>
</noscript>
<![endif]-->
</head>

<body>
<script language="JavaScript" type="text/javascript">
//<![CDATA[
<?php
echo "var livejpegurl = '" . LIVEJPEGURL . "';\n";
echo "var livemjpegurl = '" . LIVEMJPEGURL . "';\n";
echo "var width = '" . THUMBWIDTH . "';\n";
echo "var height = '" . THUMBHEIGHT . "';\n";
echo "var useserverpush = '" . USESERVERPUSH . "';\n";
?>
if (useserverpush == 0 || getIEVersion() >= 0) {
	var now = new Date();
	var htmlout = 	"<div class=\"wrap\" id=\"jpegviewercontainer\">\n" +
					"<img name=\"campicture\" width=\"" +
					width +
					"px\" height=\"" +
					height +
					"px\" src=\""  +
					livejpegurl + "&dummy=" + now.getTime().toString(10) +
					"\" alt=\"Live Image\" align=\"middle\" />\n" +
					"</div>\n";
	document.write(htmlout);
	startPoll();
}
else {
	var htmlout = 	"<div class=\"wrap\" id=\"jpegviewercontainer\">\n" +
					"<img name=\"campicture\" width=\"" +
					width +
					"px\" height=\"" +
					height +
					"px\" src=\""  +
					livemjpegurl +
					"\" alt=\"Live Image\" align=\"middle\" />\n" +
					"</div>\n";
	document.write(htmlout);
}
//]]>
</script>
<noscript>
<?php
$livejpegurl = str_replace("&", "&amp;", LIVEJPEGURL);
$livemjpegurl = str_replace("&", "&amp;", LIVEMJPEGURL);
if (USESERVERPUSH == 0 || getIEVersion() >= 0) {
	echo 	"<div class=\"wrap\" id=\"jpegviewercontainer\">\n";
	echo	"<img name=\"campicture\" width=\"" .
			THUMBWIDTH .
			"px\" height=\"" .
			THUMBHEIGHT .
			"px\" src=\"" .
			$livejpegurl . "&amp;dummy=" . time() .
			"\" alt=\"Live Image\" align=\"middle\" />\n";
	echo 	"</div>\n";
}
else {
	echo 	"<div class=\"wrap\" id=\"jpegviewercontainer\">\n";
	echo	"<img name=\"campicture\" width=\"" .
			THUMBWIDTH .
			"px\" height=\"" .
			THUMBHEIGHT .
			"px\" src=\"" .
			$livemjpegurl .
			"\" alt=\"Live Image\" align=\"middle\" />\n";
	echo 	"</div>\n";
}
?>
</noscript>
</body>
</html>