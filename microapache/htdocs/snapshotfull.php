<?php
require_once( 'configuration.php' );
if (!isset($_GET['thumb']) || $_GET['thumb'] == 'no')
	$pollfilename = "poll.php?httpbasicauth=no&dummy=";
else
	$pollfilename = "poll.php?httpbasicauth=no&thumb=yes&dummy=";
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
echo "<script type=\"text/javascript\">\n";
echo "//<![CDATA[\n";
echo "var x = " . SNAPSHOTREFRESHSEC . ";\n";
echo "var loadingterminated = true;\n";
echo "var tmpimage = new Image();\n";
echo "var rate = " . SNAPSHOTREFRESHSEC . ";\n";
echo "var fastpollrate = " . SERVERPUSH_POLLRATE_MS . ";\n";
echo "function snapshotLoaded(){\n";
echo "    var campicture = document.getElementById('campictureid');\n";
echo "    campicture.src = tmpimage.src;\n";
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
	cursor: pointer;
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
if ($full_stretch)
	echo "<img id=\"campictureid\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" onclick=\"window.location.href = '" . javascriptspecialchars($clickurl) . "';\" alt=\"Snapshot Image\" />\n";
else
	echo "<img style=\"object-fit: contain\" id=\"campictureid\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" onclick=\"window.location.href = '" . javascriptspecialchars($clickurl) . "';\" alt=\"Snapshot Image\" />\n";
echo "<script type=\"text/javascript\">\n";
echo "//<![CDATA[\n";
echo "startClock();\n";
echo "//]]>\n";
echo "</script>\n";
?>
</body>
</html>