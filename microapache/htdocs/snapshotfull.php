<?php
require_once( 'configuration.php' );
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
if (USESERVERPUSH == 0 || getIEVersion() >= 0)
	$doPoll = 1;
else
	$doPoll = 0;
if (!isset($_GET['thumb']) || $_GET['thumb'] == 'no') {
	$pollfilename = "poll.php?dummy=";
	$pushfilename = "push.php";
}
else {
	$pollfilename = "poll.php?thumb=yes&dummy=";
	$pushfilename = "push.php?thumb=yes";
}
if ($doPoll) {
	echo "<script language=\"JavaScript\" type=\"text/javascript\">\n";
	echo "//<![CDATA[\n";
	echo "var x = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "var loadingterminated = true;\n";
	echo "var tmpimage = new Image();\n";
	echo "var rate = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "var fastpollrate = " . SERVERPUSH_POLLRATE_MS . ";\n";
	echo "function doFlip(){\n"; // always flip with some delay otherwise Firefox is flickering (it shows background) and the others are displaying the old image
	echo "    if (document.campicture0.style.zIndex == 3){\n";
	echo "        document.campicture1.style.zIndex = 2;\n";	// 1. Gently 3 steps flip of previously loaded image
	echo "        document.campicture0.style.zIndex = 1;\n";	// 2. Flip
	echo "        document.campicture1.style.zIndex = 3;\n";	// 3. Restore index
	echo "    } else {\n";
	echo "        document.campicture0.style.zIndex = 2;\n";	// 1. Gently 3 steps flip of previously loaded image
	echo "        document.campicture1.style.zIndex = 1;\n";	// 2. Flip
	echo "        document.campicture0.style.zIndex = 3;\n";	// 3. Restore index
	echo "    }\n";
	echo "    loadingterminated = true;\n";
	echo "}\n";
	echo "function snapshotLoaded(){\n"; // this function is triggered when the image has been downloaded from network to file and not when loaded from file into memory
	echo "    if (rate < 1)\n";
	echo "        doFlip();\n";									// flip the image which started to decode in previous call
	echo "    if (document.campicture1.style.zIndex == 3)\n";
	echo "        document.campicture0.src = tmpimage.src;\n";	// start decoding file from disk to memory (that takes some time)
	echo "    else\n";
	echo "        document.campicture1.src = tmpimage.src;\n";	// start decoding file from disk to memory (that takes some time)
	echo "    if (rate >= 1)\n";
	echo "        setTimeout(\"doFlip()\", 500);\n";			// flip with some delay to give the decoder enough time
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
	echo "    }\n";
	echo "}\n";
	echo "function startClock(){\n";
	echo "    if (rate < 1){\n";
	echo "        reload();\n";
	echo "        setTimeout(\"startClock()\", fastpollrate);\n";
	echo "    } else {\n";
	echo "        x = x - 1;\n";
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
* {
	margin: 0;
	padding: 0;
}
html, body {
	overflow: hidden;
	height: 100%;
}
a#pollimglink {
	position: relative;
	display: block;
	z-index: 4;
	width: 100%;
	height: 100%;
	outline: none;
}
img.campicture, a:link img.campicture, a:visited img.campicture, a:hover img.campicture, a:active img.campicture, a:focus img.campicture {
	border: 0;
	margin: 0;
	padding: 0;
	position: absolute;
	display: block;
	width: 100%;
	height: 100%;
	left: 0;
	top: 0;
}
/*]]>*/
</style>
</head>

<body>
<?php
if (isset($_GET['clickurl']))
	$clickurl = $_GET['clickurl'];
else
	$clickurl = getParentUrl();
if ($doPoll) {
	echo "<a id=\"pollimglink\" class=\"transparentbkg\" href=\"" . htmlspecialchars($clickurl) . "\" target=\"_top\"></a>\n";
	echo "<img class=\"campicture\" style=\"z-index: 3;\" name=\"campicture0\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" alt=\"Snapshot Image\" width=\"100%\" height=\"100%\" />";
	echo "<img class=\"campicture\" style=\"z-index: 1;\" name=\"campicture1\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" alt=\"Snapshot Image\" width=\"100%\" height=\"100%\" />";
}
else {
	echo "<a href=\"" . htmlspecialchars($clickurl) . "\" target=\"_top\">";
	echo "<img class=\"campicture\" src=\"" . htmlspecialchars($pushfilename) . "\" alt=\"Snapshot Image\" width=\"100%\" height=\"100%\" align=\"middle\" />";
	echo "</a>\n";
}
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