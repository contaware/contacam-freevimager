<?php
if (file_exists(dirname($_SERVER['SCRIPT_FILENAME']).'/authenticate.php')) {
	session_start();
	require_once(dirname($_SERVER['SCRIPT_FILENAME']).'/authenticate.php');
}
?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta name="author" content="Oliver Pfister" />
<meta name="mobile-web-app-capable" content="yes" />
<meta name="apple-mobile-web-app-capable" content="yes" />
<meta name="application-name" content="ContaCam" />
<meta name="apple-mobile-web-app-title" content="ContaCam" />
<link rel="manifest" href="manifest.json" />
<link rel="icon" type="image/x-icon" href="favicon.ico" sizes="16x16 32x32" />
<link rel="icon" type="image/png" href="contaware_256x256.png" sizes="256x256" />
<link rel="apple-touch-icon" type="image/png" href="contaware_256x256.png" sizes="256x256" />
<title>ContaCam</title>
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
a:hover, a:focus {
	color: #ffffff;
	text-decoration: underline;
}
span.previewscontainer {
	display: inline-block;
	margin: 6px;
	padding: 0;
	border: 0;
}
span.globalbuttons {
	white-space: nowrap;
}
span.globalbuttons a {
	font-size: 13px;
	font-style: normal;
	font-weight: bold;
	font-family: sans-serif,Arial,Helvetica;
	display: inline-block;
	width: 38px;
	height: 28px;
	line-height: 28px;
	text-align: center;
	vertical-align: middle;
	text-decoration: none;
	color: #111111;
	background-color: #ededed;
	border: 1px solid #d0d0d0;
}
a#back2top {
	margin: 0;
	padding: 0;
	border: 0;
	display: none;
	position: fixed;
    top: 4px;
    right: 4px;
    width: 40px;
    line-height: 40px;
    overflow: hidden;
    z-index: 999;
    -moz-transform: rotate(270deg);
    -webkit-transform: rotate(270deg);
    -o-transform: rotate(270deg);
    -ms-transform: rotate(270deg);
    transform: rotate(270deg);
    cursor: pointer;
    background-color: #ddd;
    color: #555;
    text-align: center;
    font-size: 30px;
    text-decoration: none;
	font-family: Arial, Helvetica, sans-serif;
}
/*]]>*/
</style>
</head>
<body>
<?php
if (isset($_SESSION['username'])) {
	echo "<div>\n";
	echo "<span class=\"globalbuttons\">";
	echo "<a href=\"authenticate.php\">[&#x2192;</a>&nbsp;";
	echo "</span>\n";
	echo "</div>\n";
}
$rel_path = dirname($_SERVER['PHP_SELF']);
$rel_path = str_replace("\\", "/", $rel_path);
$rel_path = rtrim($rel_path, "/");
$dir = rtrim(str_replace("\\", "/", dirname($_SERVER['SCRIPT_FILENAME'])), '/');
$hasdir = false;
$iframe_width = 100;
$iframe_height = 100;
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
				$url_iframe = $url_for_html . "snapshot.php?title=no&amp;menu=no&amp;countdown=no&amp;scrolling=no&amp;thumb=yes&amp;clickurl=" . urlencode($url);
				echo "<span class=\"previewscontainer\"><iframe id=\"iframe$count\" name=\"iframe$count\" src=\"$url_iframe\" style=\"border: 0; overflow: hidden;\" width=\"" . $iframe_width . "\" height=\"" . $iframe_height . "\"></iframe><br /><a href=\"$url_for_html\">$file</a></span>";
				$count++;
			}
		}
	}
	echo "</div>\n";
}
if (!$hasdir) {
	echo "<p>No device found</p>\n";
}
?>
<a id="back2top" href="#" onclick="window.scrollTo(0, 0); return false;">&#x276f;</a>
<script type="text/javascript">
//<![CDATA[
window.addEventListener('scroll', function() {
	var back2TopButton = document.getElementById('back2top');
	var minScrollHeight = 2 * <?php echo $iframe_height; ?>;
	if (document.body.scrollTop > minScrollHeight || document.documentElement.scrollTop > minScrollHeight)
		back2TopButton.style.display = 'block';
	else
		back2TopButton.style.display = 'none';
}, false);
//]]>
</script>
</body>
</html>