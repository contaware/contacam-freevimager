<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<title>Select Camera</title>
<style type="text/css">
/*<![CDATA[*/
* {
	margin: 0;
	padding: 0;
}
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
span.previewscontainer {
	display: inline-block;
	margin: 6px;
	padding: 0;
	border: none;
}
/*]]>*/
</style>
</head>
<body>
<?php
$rel_path = dirname($_SERVER['PHP_SELF']);
$rel_path = str_replace("\\", "/", $rel_path);
$rel_path = rtrim($rel_path, "/");
$dir = rtrim(str_replace("\\", "/", dirname($_SERVER['SCRIPT_FILENAME'])), '/');
$hasdir = false;
if ($handle = @opendir($dir)) {
	$count = 0;
	echo "<div align=\"center\">";
	while (false !== ($file = readdir($handle))) {
		if (is_dir("$dir/$file") && $file != "." && $file != "..") {
			$configfile = "$dir/$file/configuration.php";
			if (is_file($configfile)) {
				$hasdir = true;
				$iframe_width = 100;
				$iframe_height = 100;
				$configfilecontent = file_get_contents($configfile);
				$match = preg_match("/define\s*\(\s*['\"]THUMBWIDTH['\"]\s*,\s*['\"](\d*)['\"]/i", $configfilecontent, $matches);
				if ($match != 0)
					$iframe_width = $matches[1];
				$match = preg_match("/define\s*\(\s*['\"]THUMBHEIGHT['\"]\s*,\s*['\"](\d*)['\"]/i", $configfilecontent, $matches);
				if ($match != 0)
					$iframe_height = $matches[1];
				$url = "$rel_path/$file/";
				if (is_file("$dir/$file/snapshotthumb.php"))
					$url_iframe = $url . "snapshotthumb.php?title=no&amp;menu=no&amp;countdown=no";
				else
					$url_iframe = $url . "snapshot.php?title=no&amp;menu=no&amp;countdown=no";
				echo "<span class=\"previewscontainer\"><iframe id=\"iframe$count\" name=\"iframe$count\" src=\"$url_iframe\" width=\"" . $iframe_width . "px\" height=\"" . $iframe_height . "px\" frameborder=\"0\" scrolling=\"no\" onload=\"this.contentWindow.document.body.style.cursor = 'pointer'; this.contentWindow.document.body.onclick = function(){parent.location.href='$url'}\"></iframe><br /><a href=\"$url\">$file</a></span>";
				$count++;
			}
		}
	}
	echo "</div>\n";
}
if (!$hasdir) {
	echo "No device found";
}
?>
</body>
</html>
