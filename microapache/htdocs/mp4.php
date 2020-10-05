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
	$full_path = rtrim($doc_root,"\\/")."/".trim($filename,"\\/");
$currentmp4 = basename(substr($filename, strrpos($filename, '/') + 1), '.mp4');
echo "<title>" . htmlspecialchars($currentmp4) . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
<script type="text/javascript">
//<![CDATA[
var previousTime = 0; 
function playRate(rate) {
	var v = document.getElementById("myMp4Movie");
	if (!isNaN(v.duration))	// make sure currentTime is valid
		previousTime = v.currentTime;
	v.defaultPlaybackRate = rate;
	v.playbackRate = rate;
	v.load();
}
function restoreTime(v) {
	v.currentTime = previousTime; // currentTime can now be set
}
function playRateInc() {
	var v = document.getElementById("myMp4Movie");
	if (v.playbackRate < 0.125)
		playRate(0.125);
	else if (v.playbackRate < 0.25)
		playRate(0.25);
	else if (v.playbackRate < 0.5)
		playRate(0.5);
	else if (v.playbackRate < 1.0)
		playRate(1.0);
	else if (v.playbackRate < 2.0)
		playRate(2.0);
	else if (v.playbackRate < 4.0)
		playRate(4.0);
	else if (v.playbackRate < 8.0)
		playRate(8.0);
}
function playRateDec() {
	var v = document.getElementById("myMp4Movie");
	if (v.playbackRate > 8.0)
		playRate(8.0);
	else if (v.playbackRate > 4.0)
		playRate(4.0);
	else if (v.playbackRate > 2.0)
		playRate(2.0);
	else if (v.playbackRate > 1.0)
		playRate(1.0);
	else if (v.playbackRate > 0.5)
		playRate(0.5);
	else if (v.playbackRate > 0.25)
		playRate(0.25);
	else if (v.playbackRate > 0.125)
		playRate(0.125);
}
function stepFrame() {
	var v = document.getElementById("myMp4Movie");
	if (!v.paused)
		v.pause(); // first pause, then with the next clicks step the frames
	else {
		if (typeof v.seekToNextFrame === "function")
			v.seekToNextFrame(); // only Firefox supports that (in year 2020)
		else
			v.currentTime += 0.03333; // for other Browsers use 30 fps (daily summary video has that framerate)
	}
}
function unloadVideo() {
	/* Chrome and all browsers based on it will defer loading another php script 
	as long as the video player buffers from download.php (it's not a problem 
	when the video src attribute is a direct .mp4 file). Note that Chrome only
	buffers	bigger files; a file with a few MBs is directly downloaded and does
	not	manifest the problem. So before switching to any other php script always 
	interrupt the buffering with this javascript function */
	var v = document.getElementById("myMp4Movie");
	if (!v.paused)
		v.pause();
	v.src = "";
    v.load();
}
//]]>
</script>
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
$enlarge = !isset($_GET['width']) && !isset($_GET['height']);
if (!isset($_GET['width']))
	$width = intval(WIDTH);
else
	$width = intval($_GET['width']);
if (!isset($_GET['height']))
	$height = intval(HEIGHT);
else
	$height = intval($_GET['height']);
echo "<video onloadedmetadata=\"restoreTime(this);\" id=\"myMp4Movie\" width=\"$width\" height=\"$height\" autoplay controls>\n";
echo "<source src=\"download.php?file=" . urlencode($filename) . "\" type=\"video/mp4\">\n";
echo "<p>Try this page in a modern browser or <a download=\"$currentmp4.mp4\" href=\"download.php?file=" . urlencode($filename) . "\" target=\"_top\">download the video</a> instead.</p>\n";
echo "</video>\n";
?>
</div>

<?php
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
echo "<br />\n";
echo "<div style=\"text-align: center\">\n";
echo "<span class=\"globalbuttons\">";
if ($prevkey >= 0) {
	$prevrequesturi = str_replace($currentmp4 . '.mp4', $_GET["$prevkey"] . '.mp4', $_SERVER['REQUEST_URI']);
	$prevrequesturi = javascriptspecialchars($prevrequesturi);
	$prevval = javascriptspecialchars($_GET["$prevkey"]);
	echo "<a href=\"javascript:;\" onclick=\"unloadVideo(); parent.window.name = '" . $prevval . "'; window.location.href = '" . $prevrequesturi . "'; return false;\">&lt;</a>&nbsp;";
}
else {
	echo "<a style=\"color: #c0c0c0;\" href=\"javascript:;\" onclick=\"return false;\">&lt;</a>&nbsp;";
}
if ($nextkey <= $lastkey) {
	$nextrequesturi = str_replace($currentmp4 . '.mp4', $_GET["$nextkey"] . '.mp4', $_SERVER['REQUEST_URI']);
	$nextrequesturi = javascriptspecialchars($nextrequesturi);
	$nextval = javascriptspecialchars($_GET["$nextkey"]);
	echo "<a href=\"javascript:;\" onclick=\"unloadVideo(); parent.window.name = '" . $nextval . "'; window.location.href = '" . $nextrequesturi . "'; return false;\">&gt;</a>&nbsp;";
}
else {
	echo "<a style=\"color: #c0c0c0;\" href=\"javascript:;\" onclick=\"return false;\">&gt;</a>&nbsp;";
}
if (isset($_GET['backuri']))
	echo "<a style=\"font-size: 18px;\" href=\"" . htmlspecialchars($_GET['backuri']) . "\" onclick=\"unloadVideo(); return true;\">&#x2191;</a>&nbsp;";
echo "<a href=\"javascript:;\" onclick=\"playRateDec();\">&#x231a;-</a>&nbsp;";
echo "<a href=\"javascript:;\" id=\"myStepFrameButton\" style=\"font-size: 10px; line-height: 14px;\" onclick=\"stepFrame();\">0.00<br />1x</a>&nbsp;";
echo "<a href=\"javascript:;\" onclick=\"playRateInc();\">&#x231a;+</a>&nbsp;";
echo "<a style=\"font-size: 16px;\" download=\"$currentmp4.mp4\" href=\"download.php?file=" . urlencode($filename) . "\" target=\"_top\">&#x1f4be;</a>";
echo "</span>\n";
echo "</div>\n";
?>

<script type="text/javascript">
//<![CDATA[
function resizeMp4() {
<?php if ($enlarge): ?>
	var enlarge = true;
<?php else: ?>
	var enlarge = false;
<?php endif; ?>
	var width = parseInt("<?php echo $width;?>");
	var height = parseInt("<?php echo $height;?>");
	if (width <= 0)
		width = 1;
	if (height <= 0)
		height = 1;
	var windowWidth = document.documentElement.clientWidth;
	var windowHeight = document.documentElement.clientHeight;
	if (windowWidth <= 0 || windowHeight <= 0) { // in case not fully loaded
		window.setTimeout("resizeMp4()", 1000);
		return;
	}
	// Left and right margin
	windowWidth -= 20;
	// Buttons
	windowHeight -= 60;
	// Set a min. size
	if (windowWidth < 32)
		windowWidth = 32;
	if (windowHeight < 32)
		windowHeight = 32;
	var fittedWidth = windowWidth;
	var fittedHeight = windowHeight;
	if (!enlarge) {
		if (width <= windowWidth)
			fittedWidth = width;
		if (height <= windowHeight)
			fittedHeight = height;
	}
	if (fittedWidth / fittedHeight > width / height)
		fittedWidth = parseInt((fittedHeight * width) / height);
	else
		fittedHeight = parseInt((fittedWidth * height) / width);
	var mp4Movie = document.getElementById("myMp4Movie");
	mp4Movie.width = fittedWidth;
	mp4Movie.height = fittedHeight;
}
window.addEventListener("resize", resizeMp4);
resizeMp4();
function videoPoll() {
	var v = document.getElementById("myMp4Movie");
	var btn = document.getElementById("myStepFrameButton");
	if (!isNaN(v.duration)) // make sure currentTime is valid
		btn.innerHTML = v.currentTime.toFixed(2) + "<br />" + v.playbackRate + "x";
	// Do not use the 'timeupdate' event as it is not fast enough
	window.setTimeout("videoPoll()", 100);
}
videoPoll();
//]]>
</script>
</body>
</html>