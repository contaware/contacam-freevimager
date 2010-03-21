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
<title>Jpeg Viewer</title>
<?php echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n"; ?>
</head>

<body>
<?php
$filename = $_GET['file'];
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = "$doc_root/" . trim($filename,"\\/");
if (!is_file("$full_path")) {
	echo "<div align=\"center\"><h2>\n";
	echo NOFILE;
	echo "</h2></div>\n";
	echo "</body>\n";
	echo "</html>\n";
	exit();
}
echo "<div class=\"wrap\" id=\"jpegviewercontainer\">\n";
echo "<img src=\"$filename\" alt=\"Image File\" align=\"middle\" />\n";
echo "</div>\n";
if (!isset($_GET['back']) || $_GET['back'] != 'no') {
	echo "<br/>\n";
	echo "<div align=\"center\">\n";
	if (isset($_GET['backuri']))
		echo "<a class=\"back\" href=\"" . str_replace("&", "&amp;", $_GET['backuri']) . "\">" . BACK . "</a>\n";
	else
		echo "<a class=\"back\" href=\"javascript:history.back();\">" . BACK . "</a>\n";
	echo "</div>\n";
}
?>
</body>
</html>
