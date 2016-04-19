<?php
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<?php
$doc_root = $_SERVER['DOCUMENT_ROOT'];
$filename = $_GET['file'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = rtrim($doc_root,"\\/")."/".trim($filename,"\\/");
$currentavi = basename(substr($filename, strrpos($filename, '/') + 1), '.avi');
echo "<title>$currentavi</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
</head>

<body>
<?php
if (!is_file("$full_path")) {
	echo "<div style=\"text-align: center\"><h2>\n";
	echo NOFILE;
	echo "</h2></div>\n";
	echo "</body>\n";
	echo "</html>\n";
	exit();
}
echo "<div class=\"wrap\" id=\"playercontainer\">\n";
echo "<a href=\"$filename\">" . basename($full_path) . "</a>\n";
echo "</div>\n";
if (!isset($_GET['back']) || $_GET['back'] != 'no') {
	echo "<br/>\n";
	echo "<div style=\"text-align: center\">\n";
	echo "<span class=\"globalbuttons\">";
	if (isset($_GET['backuri']))
		echo "<a class=\"backbuttons\" href=\"" . htmlspecialchars($_GET['backuri']) . "\">&nbsp;</a>";
	else
		echo "<a class=\"backbuttons\" href=\"javascript:history.back();\">&nbsp;</a>";
	echo "</span>\n";
	echo "</div>\n";
}
?>
</body>
</html>