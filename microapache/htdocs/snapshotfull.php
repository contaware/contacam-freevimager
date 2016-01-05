<?php
require_once( 'configuration.php' );
?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
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
	cursor: pointer;
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
	echo "<a id=\"pollimglink\" class=\"transparentbkg\" onclick=\"window.top.location.href = '" . htmlspecialchars($clickurl) . "'; return false;\"></a>\n";
	echo "<img class=\"campicture\" style=\"width: 100%; height: 100%; z-index: 3;\" id=\"campicture0\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" alt=\"Snapshot Image\" />";
	echo "<img class=\"campicture\" style=\"width: 100%; height: 100%; z-index: 1;\" id=\"campicture1\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" alt=\"Snapshot Image\" />";
	echo "<script type=\"text/javascript\">\n";
	echo "//<![CDATA[\n";
	echo "startClock();\n";
	echo "//]]>\n";
	echo "</script>\n";
}
else
	echo "<img class=\"campicture\" style=\"width: 100%; height: 100%; cursor: pointer;\" src=\"" . htmlspecialchars($pushfilename) . "\" onclick=\"window.top.location.href = '" . htmlspecialchars($clickurl) . "';\" alt=\"Snapshot Image\" />";
?>
</body>
</html>