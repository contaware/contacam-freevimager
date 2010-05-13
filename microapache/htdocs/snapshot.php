<?php
/* Copyright © 2005-2010 Oliver Pfister, Switzerland, Web: www.contaware.com
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
echo "<title>" . SNAPSHOTTITLE . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
if (USESERVERPUSH == 0 || getIEVersion() >= 0 || !isset($_GET['countdown']) || $_GET['countdown'] != 'no')
	$doPoll = 1;
else
	$doPoll = 0;
if (SNAPSHOT_THUMB == 1)
	$filename = "$filesdirpath/".SNAPSHOTTHUMBNAME;
else
	$filename = "$filesdirpath/".SNAPSHOTNAME;
if ($doPoll) {
	echo "<script language=\"JavaScript\" type=\"text/javascript\">\n";
	echo "//<![CDATA[\n";
	echo "var x = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "var curimage = new Image();\n";
	echo "var tmpimage = new Image();\n";
	echo "function startClock(){\n";
	echo "    x = x - 1;\n";
	echo "    document.form1.clock.value = x;\n";
	echo "    if (x < 1)\n";
	echo "        reload();\n";
	echo "    setTimeout(\"startClock()\", 1000);\n";
	echo "}\n";
	echo "function snapshotLoaded(){\n";
	echo "    tmpimage.onload = null;\n";
	echo "    tmpimage.onerror = null;\n";
	echo "    curimage = tmpimage;\n";
	echo "    document.campicture.src = curimage.src;\n";
	echo "}\n";
	echo "function snapshotErrorLoading(){\n";
	echo "    tmpimage.onload = null;\n";
	echo "    tmpimage.onerror = null;\n";
	echo "    setTimeout(\"reload()\", 100);\n";
	echo "}\n";
	echo "function reload(){\n";
	echo "    var now = new Date();\n";
	echo "    var camImg = '$filename' + '?dummy=' + now.getTime().toString(10);\n";
	echo "    tmpimage.onload = snapshotLoaded;\n";
	echo "    tmpimage.onerror = snapshotErrorLoading;\n";
	echo "    tmpimage.src = camImg;\n";
	echo "    x = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "    document.form1.clock.value = x;\n";
	echo "}\n";
	echo "//]]>\n";
	echo "</script>\n";
}
?>
</head>

<body>
<?php
if (!isset($_GET['title']) || $_GET['title'] != 'no')
	echo "<div align=\"center\"><h1>" . SNAPSHOTTITLE . "</h1></div>\n";
if (!isset($_GET['menu']) || $_GET['menu'] != 'no') {
	echo "<div class=\"menutop\">\n";
	echo "<a href=\"#\" onclick=\"window.location.reload(); return false;\">" . RELOAD . "</a> |\n";
	echo "<a href=\"" . getParentUrl() . "\" target=\"_top\">" . HOME . "</a>\n";
	if (SHOW_PRINTCOMMAND == 1)
		echo "| <a href=\"#\" onclick=\"window.focus(); window.print(); window.focus(); return false;\">" . PRINTCOMMAND . "</a>\n";
	echo "</div>\n";
}
echo "<div class=\"wrap\" id=\"jpegviewercontainer\">\n";
if ($doPoll) {
	echo "<form name=\"form1\" action=\"\">\n";
	echo "<img name=\"campicture\" src=\"" . $filename . "?dummy=" . time() . "\" alt=\"Snapshot Image\" align=\"middle\" />\n";
	if (!isset($_GET['countdown']) || $_GET['countdown'] != 'no')
		echo "<div id=\"imagereloading\">" . IMAGERELOADIN . " <input type=\"text\" readonly=\"readonly\" id=\"clock\" name=\"clock\" size=\"3\" value=\"\" /> " . SECONDS . "</div>\n";
	else
		echo "<div id=\"imagereloading\"><input type=\"hidden\" id=\"clock\" name=\"clock\" value=\"\" /></div>\n";
	echo "</form>\n";
}
else
	echo "<img name=\"campicture\" src=\"push.php\" alt=\"Snapshot Image\" align=\"middle\" />\n";
echo "</div>\n";
if ($doPoll) {
	echo "<script language=\"JavaScript\" type=\"text/javascript\">\n";
	echo "//<![CDATA[\n";
	echo "startClock();\n";
	echo "//]]>\n";
	echo "</script>\n";
}
?>
</body>
</html>