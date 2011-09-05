<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<title>Select Camera</title>
<style type="text/css">
/*<![CDATA[*/
body {
	font-family: Helvetica,Arial,sans-serif;
	font-size: 12px;
	background-color: #ffffff;
	color: #666666;
}
a {
	font-size: 14px;
	text-decoration: none;
}
a:link {
	color: #1188bb;
}
a:visited {
	color: #1188bb;
}
a:active {
	color: #1188bb;
}
a:hover {
	color: #116699;
	text-decoration: underline;
}
/*]]>*/
</style>
</head>
<body>
<?php
$rel_path = dirname($_SERVER['PHP_SELF']);
$rel_path = str_replace("\\", "/", $rel_path);
$rel_path = rtrim($rel_path, "/");
$dir = dirname($_SERVER['SCRIPT_FILENAME']);
$hasdir = false;
if ($handle = @opendir($dir)) {
	while (false !== ($file = readdir($handle))) {
		$path_parts = pathinfo($file);
		$basename = $path_parts['basename'];
		if (is_dir("$file") && $basename != "." && $basename != "..") {
			$hasdir = true;
			$url = $rel_path . "/" . $basename . "/";
			echo "<a href=\"$url\">$basename</a><br/>\n";
		}
	}
}
if (!$hasdir) {
	echo "No device found";
}
?>
</body>
</html>
