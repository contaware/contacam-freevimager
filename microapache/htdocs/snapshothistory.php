<?php
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
if (intval(WIDTH) > intval(THUMBWIDTH))
	$iframe_width = intval(WIDTH) + 60;
else
	$iframe_width = intval(THUMBWIDTH) + 60;
if (intval(HEIGHT) > intval(THUMBHEIGHT))
	$iframe_height = intval(HEIGHT) + 90;
else
	$iframe_height = intval(THUMBHEIGHT) + 90;
// Min. size 320x240:
if ($iframe_width < 380)
	$iframe_width = 380;
if ($iframe_height < 330)
	$iframe_height = 330;
?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="viewport" content="width=<?php echo $iframe_width; ?>" />
<meta name="author" content="Oliver Pfister" />
<?php
echo "<title>" . SNAPSHOTTITLE . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
</head>
<body>
<div>
<span class="globalbuttons">
<a class="homebuttons" href="<?php echo getParentUrl();?>" target="_top">&nbsp;</a>
<?php
if (isset($_SESSION['username'])) {
	echo "<a href=\"" . getParentUrl() . "authenticate.php\">[&#x2192;</a>\n";
}
?>
</span>
</div>
<?php echo "<div style=\"text-align: center\"><h1>" . SNAPSHOTTITLE . "</h1></div>"; ?>
<div class="wrap">
<form>
<span class="globalbuttons">
<a href="javascript:;" id="prevLink">&lt;</a>
<script type="text/javascript">
//<![CDATA[
var now = new Date();
var sel = new Date();
var todayselected = true;
function LZ(x){return(x<0||x>9?"":"0")+x}
document.write('<input id="DatePicker" type="date" value="' + now.getFullYear() + '-' + LZ(now.getMonth()+1) + '-' + LZ(now.getDate()) + '" />');
//]]>
</script>
<a href="javascript:;" id="nextLink">&gt;</a>
</span>
<a href="javascript:;" id="todayLink"><?php echo TODAY;?></a>
</form>
</div>
<br />
<?php
$srcuri = "snapshot.php?title=no&amp;menu=no&amp;clickurl=" . urlencode("snapshotfull.php?clickurl=snapshothistory.php");
echo "<div style=\"text-align: center\">\n";
echo "<iframe id=\"myiframe\" name=\"myiframe\" src=\"$srcuri\" style=\"border: 0; overflow: hidden;\" width=\"" . $iframe_width . "\" height=\"" . $iframe_height . "\" >\n";
echo "<p>Click <a href=\"snapshot.php\">here</a> to see today's sequence of snapshots</p>\n";
echo "</iframe>\n";
echo "</div>\n";
?>
<script type="text/javascript">
//<![CDATA[
function urlExists(url) {
	var ajax = new XMLHttpRequest(); 
	ajax.open('HEAD', url, false); // sync. call
	ajax.send(null);
	return ajax.status != 404;
}
function loadIFrame(y,m,d) {
	// Update the now variable
	now = new Date();
	
	// Update the sel variable
	sel.setFullYear(y,m-1,d);
	
	// Today selected?
	if (sel.getDate() == now.getDate() &&
		sel.getMonth() == now.getMonth() &&
		sel.getFullYear() == now.getFullYear()) {
		todayselected = true;
		<?php
		$srcuri = "snapshot.php?title=no&menu=no&clickurl=" . urlencode("snapshotfull.php?clickurl=snapshothistory.php");
		echo "var srcuri = '$srcuri';\n";
		?>
	}
	else {
		todayselected = false;
		<?php
		if (SNAPSHOTHISTORY_THUMB == 1)
		{
			$video_width = THUMBWIDTH;
			$video_height = THUMBHEIGHT;
		}
		else
		{
			$video_width = WIDTH;
			$video_height = HEIGHT;
		}
		$srcuri = "?width=" . $video_width . "&height=" . $video_height . "&file=";
		echo "var filesdirpath = '$filesdirpath';\n";
		echo "var srcuri = '$srcuri';\n";
		echo "var usethumb = " . SNAPSHOTHISTORY_THUMB . ";\n";
		?>
		var filename;
		if (usethumb == 1)
			filename = 'shot' + '_' + y + '_' + LZ(m) + '_' + LZ(d) + '_thumb';
		else
			filename = 'shot' + '_' + y + '_' + LZ(m) + '_' + LZ(d);
		var videouri = filesdirpath + '/' + y + '/' + LZ(m) + '/' + LZ(d) + '/' + filename;
		var videouri_get = new String(videouri);
		if (urlExists(videouri_get + '.mp4')) {
			videouri_get += '.mp4';
			videouri_get = videouri_get.replace(/\//g, "%2F");
			srcuri = 'mp4.php' + srcuri + videouri_get;
		}
		else {
			videouri_get += '.avi';
			videouri_get = videouri_get.replace(/\//g, "%2F");
			srcuri = 'avi.php' + srcuri + videouri_get;
		}
	}
	document.getElementById('myiframe').src = srcuri;
}
function updateDatePicker() {
	document.getElementById('DatePicker').value = sel.getFullYear() + '-' + LZ(sel.getMonth()+1) + '-' + LZ(sel.getDate());
}
function updateDates() {
	// Update the now variable
	now = new Date();
	
	// If the selected day was in the future and now we got there -> update!
	if (!todayselected						&&
		sel.getDate() == now.getDate()		&&
		sel.getMonth() == now.getMonth()	&&
		sel.getFullYear() == now.getFullYear()) {
		loadIFrame(now.getFullYear(),now.getMonth()+1,now.getDate());
		updateDatePicker();
	}
	// If today selected and we passed midnight -> update!
	else if (	todayselected						&&
				!(sel.getDate() == now.getDate()	&&
				sel.getMonth() == now.getMonth()	&&
				sel.getFullYear() == now.getFullYear())) {
		sel = now;
		updateDatePicker();
	}
	
	// Call us again
	window.setTimeout('updateDates()', 1000);
}
document.getElementById('prevLink').addEventListener("click", function(ev) {
	ev.preventDefault();
	var tmpdate = new Date();
	tmpdate = sel;
	tmpdate.setDate(tmpdate.getDate()-1);
	loadIFrame(tmpdate.getFullYear(),tmpdate.getMonth()+1,tmpdate.getDate());
	updateDatePicker();
}, false);
document.getElementById('nextLink').addEventListener("click", function(ev) {
	ev.preventDefault();
	var tmpdate = new Date();
	tmpdate = sel;
	tmpdate.setDate(tmpdate.getDate()+1);
	loadIFrame(tmpdate.getFullYear(),tmpdate.getMonth()+1,tmpdate.getDate());
	updateDatePicker();
}, false);
document.getElementById('todayLink').addEventListener("click", function(ev) {
	ev.preventDefault();
	loadIFrame(now.getFullYear(),now.getMonth()+1,now.getDate());
	updateDatePicker();
}, false);
document.getElementById('DatePicker').addEventListener("input", function(ev) {
	if (ev.target.value == '') {// if pressing X in date picker
		loadIFrame(now.getFullYear(),now.getMonth()+1,now.getDate());
	}
	else {
		var parts = ev.target.value.split('-'); // parse date in yyyy-mm-dd format
		parts[0] = parseInt(parts[0], 10);
		parts[1] = parseInt(parts[1], 10);
		parts[2] = parseInt(parts[2], 10);
		if (parts[0] > 0 && parts[1] > 0 && parts[2] > 0)
			loadIFrame(parts[0],parts[1],parts[2]);
	}
}, false);
updateDates();
//]]>
</script>
</body>
</html>