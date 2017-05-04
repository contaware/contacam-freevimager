<?php
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
if (!isset($_GET['thumb']) || $_GET['thumb'] == 'no') {
	$pollfilename = "poll.php?dummy=";
	$width = WIDTH;
	$height = HEIGHT;
}
else {
	$pollfilename = "poll.php?thumb=yes&dummy=";
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
?>
<style type="text/css">
/*<![CDATA[*/
div#imgcontainer {
	position: relative;
	margin: 0 auto;
	width: <?php echo $width; ?>px;
	height: <?php echo $height; ?>px;
}
img#campictureid {
	cursor: pointer;
	border: 0;
	margin: 0;
	padding: 0;
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
if (!isset($_GET['menu']) || $_GET['menu'] != 'no') {
	echo "<div>\n";
	echo "<span class=\"globalbuttons\">";
	echo "<a class=\"homebuttons\" href=\"" . getParentUrl() . "\" target=\"_top\">&nbsp;</a>&nbsp;";
	echo "<a class=\"reloadbuttons\" href=\"#\" onclick=\"window.location.reload(); return false;\">&nbsp;</a>\n";
	echo "</span>\n";
	echo "</div>\n";
}
if (!isset($_GET['title']) || $_GET['title'] != 'no')
	echo "<div style=\"text-align: center\"><h1>" . SNAPSHOTTITLE . "</h1></div>\n";
if (isset($_GET['clickurl']))
	$clickurl = $_GET['clickurl'];
else
	$clickurl = "snapshotfull.php?clickurl=" . urlencode(urldecode($_SERVER['REQUEST_URI']));
echo "<form name=\"form1\">\n";
echo "<div id=\"imgcontainer\">\n";
echo "<img id=\"campictureid\" src=\"" . htmlspecialchars($pollfilename . time()) . "\" onclick=\"window.top.location.href = '" . htmlspecialchars($clickurl) . "';\" alt=\"Snapshot Image\" width=\"$width\" height=\"$height\" />\n";
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
?>
</body>
</html>