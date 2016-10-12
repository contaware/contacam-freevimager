<?php
if (file_exists(dirname(__FILE__).'/authenticate.php')) {
	session_start();
	require_once(dirname(__FILE__).'/authenticate.php');
}
?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
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
	font-size: 16px;
	background-color: #333333;
	color: #bbbbbb;
}
a {
	font-size: 16px;
	text-decoration: none;
}
a:link {
	color: #33ccff;
}
a:visited {
	color: #33ccff;
}
a:active {
	color: #33ccff;
}
a:hover {
	color: #ffffff;
	text-decoration: underline;
}
span.previewscontainer {
	display: inline-block;
	margin: 6px;
	padding: 0;
	border: 0;
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
	echo "<div style=\"text-align: center\">";
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
				$url_for_html = htmlspecialchars(str_replace("%2F", "/", rawurlencode($url)));
				$url_iframe = $url_for_html . "snapshot.php?title=no&amp;menu=no&amp;countdown=no&amp;thumb=yes&amp;clickurl=" . urlencode($url);
				echo "<span class=\"previewscontainer\"><iframe id=\"iframe$count\" name=\"iframe$count\" src=\"$url_iframe\" style=\"border: 0; overflow: hidden;\" width=\"" . $iframe_width . "\" height=\"" . $iframe_height . "\"></iframe><br /><a href=\"$url_for_html\">$file</a></span>";
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