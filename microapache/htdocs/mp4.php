<?php
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
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
	echo "<div style=\"text-align: center\"><h2>\n";
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
	echo "<div style=\"text-align: center\">\n";
	echo "<span class=\"globalbuttons\">";
	if ($prevkey >= 0) {
		$prevrequesturi = str_replace($currentmp4 . '.mp4', $_GET["$prevkey"] . '.mp4', $_SERVER['REQUEST_URI']);
		$prevrequesturi = htmlspecialchars($prevrequesturi);
		echo "<a href=\"javascript:;\" onclick=\"parent.window.name = '" . $_GET["$prevkey"] . "'; window.location.href = '" . $prevrequesturi . "'; return false;\">&lt;</a>";
	}
	if (isset($_GET['backuri']))
		echo "<a class=\"backbuttons\" href=\"" . htmlspecialchars($_GET['backuri']) . "\">&nbsp;</a>";
	else
		echo "<a class=\"backbuttons\" href=\"javascript:history.back();\">&nbsp;</a>";
	if ($nextkey <= $lastkey) {
		$nextrequesturi = str_replace($currentmp4 . '.mp4', $_GET["$nextkey"] . '.mp4', $_SERVER['REQUEST_URI']);
		$nextrequesturi = htmlspecialchars($nextrequesturi);
		echo "<a href=\"javascript:;\" onclick=\"parent.window.name = '" . $_GET["$nextkey"] . "'; window.location.href = '" . $nextrequesturi . "'; return false;\">&gt;</a>";
	}
	echo "&nbsp;&nbsp;<a class=\"savebuttons\" href=\"download.php?file=" . urlencode($filename) . "\">&nbsp;</a>";
	echo "</span>\n";
	echo "</div>\n";
}
?>

<script type="text/javascript">
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
	var windowWidth = Math.max(document.documentElement.clientWidth, window.innerWidth || 0);
	var windowHeight = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
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