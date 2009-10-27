<?php
require_once( 'configuration.php' );
require_once( 'language.php' ); // Must be here at the top of this file because it outputs the UTF8-BOM!
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<?php
if (USESERVERPUSH == 0 || getIEVersion() >= 0 || !isset($_GET['countdown']) || $_GET['countdown'] != 'no')
	$doPoll = 1;
else
	$doPoll = 0;
if (SNAPSHOT_THUMB == 1)
	$filename = "$filesdirpath/".SNAPSHOTTHUMBNAME;
else
	$filename = "$filesdirpath/".SNAPSHOTNAME;
echo "<title>Jpeg Snapshots</title>\n";
if ($doPoll) {
	echo "<script language=\"JavaScript\" type=\"text/javascript\">\n";
	echo "//<![CDATA[\n";
	echo "var x = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "var myimage = new Image();\n";
	echo "var myimage0 = new Image();\n";
	echo "var myimage1 = new Image();\n";
	echo "var imagenum = 0;\n";
	echo "function startClock(){\n";
	echo "    x = x - 1;\n";
	echo "    document.form1.clock.value = x;\n";
	echo "    if (x < 1)\n";
	echo "        reload();\n";
	echo "    setTimeout(\"startClock()\", 1000);\n";
	echo "}\n";
	echo "function snapshot0Loaded(){\n";
	echo "    myimage0.onload = null;\n";
	echo "    myimage0.onerror = null;\n";
	echo "    if (myimage0.complete){\n";
	echo "        myimage = myimage0;\n";
	echo "        document.campicture.src = myimage.src;\n";
	echo "        document.form1.debug1.value = '0 complete ' + (Math.floor(Math.random() * 100000)).toString(6);\n";
	echo "    }\n";
	echo "    else\n";
	echo "        document.form1.debug1.value = '0 not complete ' + (Math.floor(Math.random() * 100000)).toString(6);\n";
	echo "}\n";
	echo "function snapshot1Loaded(){\n";
	echo "    myimage1.onload = null;\n";
	echo "    myimage1.onerror = null;\n";
	echo "    if (myimage1.complete){\n";
	echo "        myimage = myimage1;\n";
	echo "        document.campicture.src = myimage.src;\n";
	echo "        document.form1.debug2.value = '1 complete ' + (Math.floor(Math.random() * 100000)).toString(6);\n";
	echo "    }\n";
	echo "    else\n";
	echo "        document.form1.debug2.value = '1 not complete ' + (Math.floor(Math.random() * 100000)).toString(6);\n";
	echo "}\n";
	echo "function snapshot0ErrorLoading(){\n";
	echo "    myimage0.onload = null;\n";
	echo "    myimage0.onerror = null;\n";
	echo "    setTimeout(\"reload()\", 100);\n";
	echo "    document.form1.debug3.value = '0 onError ' + (Math.floor(Math.random() * 100000)).toString(6);\n";
	echo "}\n";
	echo "function snapshot1ErrorLoading(){\n";
	echo "    myimage1.onload = null;\n";
	echo "    myimage1.onerror = null;\n";
	echo "    setTimeout(\"reload()\", 100);\n";
	echo "    document.form1.debug4.value = '1 onError ' + (Math.floor(Math.random() * 100000)).toString(6);\n";
	echo "}\n";
	echo "function reload(){\n";
	echo "    var camImg = '$filename' + '?dummy=' + (Math.floor(Math.random() * 10000000)).toString(10);\n";
	echo "    if (imagenum == 0){\n";
	echo "        myimage0.onload = snapshot0Loaded;\n";
	echo "        myimage0.onerror = snapshot0ErrorLoading;\n";
	echo "        myimage0.src = camImg;\n";
	echo "        imagenum = 1;\n";
	echo "    }\n";
	echo "    else{\n";
	echo "        myimage1.onload = snapshot1Loaded;\n";
	echo "        myimage1.onerror = snapshot1ErrorLoading;\n";
	echo "        myimage1.src = camImg;\n";
	echo "        imagenum = 0;\n";
	echo "    }\n";
	echo "    x = " . SNAPSHOTREFRESHSEC . ";\n";
	echo "    document.form1.clock.value = x;\n";
	echo "}\n";
	echo "//]]>\n";
	echo "</script>\n";
}
echo "<link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\" />\n";
?>
</head>

<body>
<?php
echo "<div class=\"wrap\" id=\"jpegviewercontainer\">\n";
if ($doPoll) {
	echo "<form name=\"form1\" action=\"\">\n";
	echo "<img name=\"campicture\" src=\"" . $filename . "?dummy=" . time() . "\" alt=\"Snapshot Image\" align=\"middle\" />\n";
	if (!isset($_GET['countdown']) || $_GET['countdown'] != 'no') {
		echo "<div id=\"reload\">" . IMAGERELOADIN . " <input type=\"text\" readonly=\"readonly\" id=\"clock\" name=\"clock\" size=\"3\" value=\"\" /> " . SECONDS . "</div>\n";
		echo "<div><input type=\"text\" readonly=\"readonly\" id=\"debug1\" name=\"debug1\" size=\"60\" value=\"\" /></div>\n";
		echo "<div><input type=\"text\" readonly=\"readonly\" id=\"debug2\" name=\"debug2\" size=\"60\" value=\"\" /></div>\n";
		echo "<div><input type=\"text\" readonly=\"readonly\" id=\"debug3\" name=\"debug3\" size=\"60\" value=\"\" /></div>\n";
		echo "<div><input type=\"text\" readonly=\"readonly\" id=\"debug4\" name=\"debug4\" size=\"60\" value=\"\" /></div>\n";
	}
	else {
		echo "<div id=\"reload\"><input type=\"hidden\" id=\"clock\" name=\"clock\" value=\"\" /></div>\n";
	}
	echo "</form>\n";
}
else
	echo "<img name=\"campicture\" src=\"" . JPEGPUSHFILEPATH . "\" alt=\"Snapshot Image\" align=\"middle\" />\n";
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