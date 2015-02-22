<?php
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<?php
$doc_root = $_SERVER['DOCUMENT_ROOT'];
$filename = $_GET['file'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = "$doc_root/" . trim($filename,"\\/");
$currentmp4 = basename(substr($filename, strrpos($filename, '/') + 1), '.mp4');
echo "<title>$currentmp4</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
</head>

<body>
<?php
if (!is_file("$full_path")) {
	echo "<div align=\"center\"><h2>\n";
	echo NOFILE;
	echo "</h2></div>\n";
	echo "</body>\n";
	echo "</html>\n";
	exit();
}
?>

<div class="wrap" id="playercontainer">
<?php
if (!isset($_GET['width']))
	$width = WIDTH;
else
	$width = $_GET['width'];
if (!isset($_GET['height']))
	$height = HEIGHT;
else
	$height = $_GET['height'];
echo "<video id=\"myMp4Movie\"  width=\"$width\" height=\"$height\" autoplay controls>\n";
echo "<source src=\"$filename\" type=\"video/mp4\">\n";
echo "<p>Try this page in a modern browser or <a href=\"$filename\">download the video</a> instead.</p>\n";
echo "</video>\n";
?>
</div>

<?php
if (!isset($_GET['back']) || $_GET['back'] != 'no') {
	$currentkey = '0';
	$lastkey = 0;
	foreach($_GET as $key=>$val) {
		if (is_numeric($key) && intval($key) >= 0) {
			if ($currentmp4 == $val)
				$currentkey = $key;
			$lastkey = intval($key);
		}
	}
	$prevkey = intval($currentkey) - 1;
	$nextkey = intval($currentkey) + 1;
	echo "<br/>\n";
	echo "<div align=\"center\">\n";
	echo "<form name=\"videonav\" action=\"\" method=\"post\" id=\"videonav\">";
	if ($prevkey >= 0) {
		$prevrequesturi = str_replace($currentmp4 . '.mp4', $_GET["$prevkey"] . '.mp4', $_SERVER['REQUEST_URI']);
		$prevrequesturi = htmlspecialchars($prevrequesturi);
		echo "<input type=\"button\" value=\"&lt;&lt;&lt;\" name=\"PrevVideo\" class=\"playerbutton\" onclick=\"parent.window.name = '" . $_GET["$prevkey"] . "'; window.location.href = '" . $prevrequesturi . "';\" />";
	}
	if (isset($_GET['backuri']))
		echo "<input type=\"button\" value=\"" . BACK . "\" name=\"BackButton\" class=\"playerbutton\" onclick=\"window.location.href = '" . htmlspecialchars($_GET['backuri']) . "';\" />";
	else
		echo "<input type=\"button\" value=\"" . BACK . "\" name=\"BackButton\" class=\"playerbutton\" onclick=\"window.history.back();\" />";
	echo "<input type=\"button\" value=\"Save\" name=\"Save\" class=\"playerbutton\" onclick=\"window.location.href = 'download.php?file=" . urlencode($filename) . "';\" />";
	if ($nextkey <= $lastkey) {
		$nextrequesturi = str_replace($currentmp4 . '.mp4', $_GET["$nextkey"] . '.mp4', $_SERVER['REQUEST_URI']);
		$nextrequesturi = htmlspecialchars($nextrequesturi);
		echo "<input type=\"button\" value=\"&gt;&gt;&gt;\" name=\"NextVideo\" class=\"playerbutton\" onclick=\"parent.window.name = '" . $_GET["$nextkey"] . "'; window.location.href = '" . $nextrequesturi . "';\" />";
	}
	echo "</form>\n";
	echo "</div>\n";
}
?>

<script language="JavaScript" type="text/javascript">
//<![CDATA[
function resizeMp4() {
<?php
	if (isset($_GET['back']) && $_GET['back'] == 'no')
		echo "	var back = false;\n";
	else
		echo "	var back = true;\n";
?>
	var width = parseInt("<?php echo $width;?>");
	var height = parseInt("<?php echo $height;?>");
	if (width <= 0)
		width = 1;
	if (height <= 0)
		height = 1;
	var windowWidth = 0;
	var windowHeight = 0;
	if (typeof(window.innerWidth) == 'number') {
		// Non-IE
		windowWidth = window.innerWidth;
		windowHeight = window.innerHeight;
	} else if (document.documentElement && (document.documentElement.clientWidth || document.documentElement.clientHeight)) {
		// IE 6+ in 'standards compliant mode'
		windowWidth = document.documentElement.clientWidth;
		windowHeight = document.documentElement.clientHeight;
	} else if (document.body && (document.body.clientWidth || document.body.clientHeight)) {
		// IE 4 compatible
		windowWidth = document.body.clientWidth;
		windowHeight = document.body.clientHeight;
	}
	if (windowWidth <= 0 || windowHeight <= 0) { // in case not fully loaded
		window.setTimeout("resizeMp4()", 1000);
		return;
	}
	// Left and right margin
	windowWidth -= 50; // this must be less than 60, see snapshothistory.php!
	// Back button
	if (back)
		windowHeight -= 50;
	else
		windowHeight -= 10; // this must be less than 90, see snapshothistory.php!
	// Set a min. size
	if (windowWidth < 32)
		windowWidth = 32;
	if (windowHeight < 32)
		windowHeight = 32;
	var fittedWidth;
	var fittedHeight;
	var fittedSlider;
	if (width > windowWidth) {
		fittedWidth = windowWidth;
		fittedSlider = windowWidth;
	}
	else {
		fittedWidth = width;
		fittedSlider = width;
	}
	if (height > windowHeight)
		fittedHeight = windowHeight;
	else
		fittedHeight = height;
	if (fittedWidth / fittedHeight > width / height)
		fittedWidth = parseInt((fittedHeight * width) / height);
	else
		fittedHeight = parseInt((fittedWidth * height) / width);
	var mp4Movie = document.getElementById("myMp4Movie");
	mp4Movie.width = fittedWidth;
	mp4Movie.height = fittedHeight;
}
window.addEventListener('resize', function(event){
	resizeMp4();
});
resizeMp4();
//]]>
</script>
</body>
</html>