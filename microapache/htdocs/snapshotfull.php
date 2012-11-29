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
	echo "function snapshotLoaded(){\n";
	echo "    document.campicture.src = tmpimage.src;\n";
	echo "    loadingterminated = true;\n";
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
	echo "    var rate = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "    if (rate < 1){\n";
	echo "        var fastpollrate = " . SERVERPUSH_POLLRATE_MS . ";\n";
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
img#campictureid {
	border: 0;
	margin: 0;
	padding: 0;
	position: absolute;
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
	echo "<a href=\"" . htmlspecialchars($clickurl) . "\" target=\"_top\">";
	echo "<img name=\"campicture\" id=\"campictureid\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" alt=\"Snapshot Image\" width=\"100%\" height=\"100%\" align=\"middle\" />";
	echo "</a>\n";
}
else {
	echo "<a href=\"" . htmlspecialchars($clickurl) . "\" target=\"_top\">";
	echo "<img name=\"campicture\" id=\"campictureid\" src=\"" . htmlspecialchars($pushfilename) . "\" alt=\"Snapshot Image\" width=\"100%\" height=\"100%\" align=\"middle\" />";
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