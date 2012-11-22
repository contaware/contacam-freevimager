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
$filename = "$filesdirpath/pollthumb.php";
if ($doPoll) {
	echo "<script language=\"JavaScript\" type=\"text/javascript\">\n";
	echo "//<![CDATA[\n";
	echo "var x = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "var curimage = new Image();\n";
	echo "var tmpimage = new Image();\n";
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
	height: 100%;
}
img#campictureid {
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
if ($doPoll)
	echo "<img id=\"campictureid\" name=\"campicture\" src=\"" . $filename . "?dummy=" . time() . "\" alt=\"Snapshot Image\" width=\"100%\" height=\"100%\" align=\"middle\" />\n";
else
	echo "<img id=\"campictureid\" name=\"campicture\" src=\"pushthumb.php\" alt=\"Snapshot Image\" width=\"100%\" height=\"100%\" align=\"middle\" />\n";
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