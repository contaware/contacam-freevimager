<?php
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
if (USESERVERPUSH == 0 || getIEVersion() >= 0 || !isset($_GET['countdown']) || $_GET['countdown'] != 'no')
	$doPoll = 1;
else
	$doPoll = 0;
if (!isset($_GET['thumb']) || $_GET['thumb'] == 'no') {
	$pollfilename = "poll.php?dummy=";
	$pushfilename = "push.php";
	$width = WIDTH;
	$height = HEIGHT;
}
else {
	$pollfilename = "poll.php?thumb=yes&dummy=";
	$pushfilename = "push.php?thumb=yes";
	$width = THUMBWIDTH;
	$height = THUMBHEIGHT;
}
?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=<?php echo $width; ?>" />
<meta name="author" content="Oliver Pfister" />
<?php
echo "<title>" . SNAPSHOTTITLE . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
if ($doPoll) {
	echo "<script type=\"text/javascript\">\n";
	echo "//<![CDATA[\n";
	echo "var x = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "var loadingterminated = true;\n";
	echo "var tmpimage = new Image();\n";
	echo "var rate = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "var fastpollrate = " . SERVERPUSH_POLLRATE_MS . ";\n";
	echo "function doFlip(){\n"; // always flip with some delay otherwise Firefox is flickering (it shows background) and the others are displaying the old image
	echo "    var campicture0 = document.getElementById('campicture0');\n";
	echo "    var campicture1 = document.getElementById('campicture1');\n";
	echo "    if (campicture0.style.zIndex == 3){\n";
	echo "        campicture1.style.zIndex = 2;\n";		// 1. Gently 3 steps flip of previously loaded image
	echo "        campicture0.style.zIndex = 1;\n";		// 2. Flip
	echo "        campicture1.style.zIndex = 3;\n";		// 3. Restore index
	echo "    } else {\n";
	echo "        campicture0.style.zIndex = 2;\n";		// 1. Gently 3 steps flip of previously loaded image
	echo "        campicture1.style.zIndex = 1;\n";		// 2. Flip
	echo "        campicture0.style.zIndex = 3;\n";		// 3. Restore index
	echo "    }\n";
	echo "    loadingterminated = true;\n";
	echo "}\n";
	echo "function snapshotLoaded(){\n"; // this function is triggered when the image has been downloaded from network to file and not when loaded from file into memory
	echo "    var campicture0 = document.getElementById('campicture0');\n";
	echo "    var campicture1 = document.getElementById('campicture1');\n";
	echo "    if (rate < 1)\n";
	echo "        doFlip();\n";							// flip the image which started to decode in previous call
	echo "    if (campicture1.style.zIndex == 3)\n";
	echo "        campicture0.src = tmpimage.src;\n";	// start decoding file from disk to memory (that takes some time)
	echo "    else\n";
	echo "        campicture1.src = tmpimage.src;\n";	// start decoding file from disk to memory (that takes some time)
	echo "    if (rate >= 1)\n";
	echo "        setTimeout(\"doFlip()\", 500);\n";	// flip with some delay to give the decoder enough time
	echo "}\n";
	echo "function snapshotNotLoaded(){\n";
	echo "    loadingterminated = true;\n";
	echo "}\n";
	echo "tmpimage.onload = snapshotLoaded;\n";
	echo "tmpimage.onerror = snapshotNotLoaded;\n";
	echo "tmpimage.onabort = snapshotNotLoaded;\n";
	echo "function reload(){\n";
	echo "    if (loadingterminated){\n";
	echo "        loadingterminated = false;\n";
	echo "        var now = new Date();\n";
	echo "        var camImg = '$pollfilename' + now.getTime().toString(10);\n";
	echo "        tmpimage.src = camImg;\n";
	echo "        x = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "        document.form1.clock.value = x;\n";
	echo "    }\n";
	echo "}\n";
	echo "function startClock(){\n";
	echo "    if (rate < 1){\n";
	echo "        reload();\n";
	echo "        setTimeout(\"startClock()\", fastpollrate);\n";
	echo "    } else {\n";
	echo "        x = x - 1;\n";
	echo "        document.form1.clock.value = x;\n";
	echo "        if (x < 1)\n";
	echo "            reload();\n";
	echo "        setTimeout(\"startClock()\", 1000);\n";
	echo "    }\n";
	echo "}\n";
	echo "//]]>\n";
	echo "</script>\n";
}
?>
<style type="text/css">
/*<![CDATA[*/
div#imgcontainer {
	position: relative;
	margin: 0 auto;
	width: <?php echo $width; ?>px;
	height: <?php echo $height; ?>px;
}
a#pollimglink {
	cursor: pointer;
	position: relative;
	display: block;
	z-index: 4;
	width: <?php echo $width; ?>px;
	height: <?php echo $height; ?>px;
	outline: none;
}
img.campicture, a:link img.campicture, a:visited img.campicture, a:hover img.campicture, a:active img.campicture, a:focus img.campicture {
	border: 0;
	margin: 0;
	padding: 0;
}
img.campicturepoll {
	position: absolute;
	display: block;
	left: 0;
	top: 0;
}
/*]]>*/
</style>
</head>

<body>
<?php
if (!isset($_GET['title']) || $_GET['title'] != 'no')
	echo "<div style=\"text-align: center\"><h1>" . SNAPSHOTTITLE . "</h1></div>\n";
if (!isset($_GET['menu']) || $_GET['menu'] != 'no') {
	echo "<div class=\"menutop\">\n";
	echo "<span class=\"globalbuttons\">";
	echo "<a class=\"homebuttons\" href=\"" . getParentUrl() . "\" target=\"_top\">&nbsp;</a>&nbsp;";
	echo "<a class=\"reloadbuttons\" href=\"#\" onclick=\"window.location.reload(); return false;\">&nbsp;</a>\n";
	echo "</span>\n";
	echo "</div>\n";
}
if (isset($_GET['clickurl']))
	$clickurl = $_GET['clickurl'];
else
	$clickurl = "snapshotfull.php?clickurl=" . urlencode(urldecode($_SERVER['REQUEST_URI']));
if ($doPoll) {
	echo "<form name=\"form1\">\n";
	echo "<div id=\"imgcontainer\">\n";
	echo "<a id=\"pollimglink\" class=\"transparentbkg\" onclick=\"window.top.location.href = '" . htmlspecialchars($clickurl) . "'; return false;\"></a>\n";
	echo "<img class=\"campicture campicturepoll\" style=\"z-index: 3;\" id=\"campicture0\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" alt=\"Snapshot Image\" width=\"$width\" height=\"$height\" />";
	echo "<img class=\"campicture campicturepoll\" style=\"z-index: 1;\" id=\"campicture1\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" alt=\"Snapshot Image\" width=\"$width\" height=\"$height\" />";
	echo "</div>\n";
	if (!isset($_GET['countdown']) || $_GET['countdown'] != 'no')
		echo "<div id=\"imagereloading\">" . IMAGERELOADIN . " <input type=\"text\" readonly=\"readonly\" id=\"clock\" name=\"clock\" size=\"3\" value=\"\" /> " . SECONDS . "</div>\n";
	else
		echo "<div><input type=\"hidden\" id=\"clock\" name=\"clock\" value=\"\" /></div>\n";
	echo "</form>\n";
	echo "<script type=\"text/javascript\">\n";
	echo "//<![CDATA[\n";
	echo "startClock();\n";
	echo "//]]>\n";
	echo "</script>\n";
}
else {
	echo "<div id=\"imgcontainer\">\n";
	echo "<img class=\"campicture\" style=\"display: block; cursor: pointer;\" src=\"" . htmlspecialchars($pushfilename) . "\" onclick=\"window.top.location.href = '" . htmlspecialchars($clickurl) . "';\" alt=\"Snapshot Image\" width=\"$width\" height=\"$height\" />";
	echo "</div>\n";
}
?>
</body>
</html>