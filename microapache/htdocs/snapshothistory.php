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
echo "<title>" . SNAPSHOTTITLE . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
<script language="JavaScript" src="js/calendarpopup.js" type="text/javascript"></script>
<script language="JavaScript" type="text/javascript">
//<![CDATA[
document.write(getCalendarStyles());
//]]>
</script>
</head>

<body>
<?php echo "<div align=\"center\"><h1>" . SNAPSHOTTITLE . "</h1></div>"; ?>
<div class="menutop">
<a href="#" onclick="myiframe.location.reload(); return false;"><?php echo RELOAD;?></a> |
<a href="<?php echo getParentUrl();?>" target="_top"><?php echo HOME;?></a>
<?php
if (SHOW_PRINTCOMMAND == 1)
	echo "| <a href=\"#\" onclick=\"myiframe.focus(); myiframe.print(); myiframe.focus(); return false;\">" . PRINTCOMMAND . "</a>\n";
?>
</div>
<div class="wrap">
<form action="">
<input type="hidden" name="dateall" value="" />
<div id="navcontrols">
<span class="inlinenowrap">
<?php echo LOCALTIME;?> <input type="text" readonly="readonly" id="clock" name="clock" value="" size="44" />
</span>
&nbsp;
<span class="inlinenowrap">
<?php echo VIEWSNAPSHOTSFOR;?> <input type="button" value="&lt;" class="navbutton" name="prev" id="prev" onclick="onPrev();" /><input id="datetext" type="text" readonly="readonly" name="datetext" value="" size="32" /><input type="button" value="&gt;" class="navbutton" name="next" id="next" onclick="onNext();" /> <a href="#" onclick="positionCalendar(); var dateselected = LZ(sel.getDate()) + '/' + LZ(sel.getMonth()+1) + '/' + sel.getFullYear(); cal.select(document.forms[0].dateall,'anchor2','dd/MM/yyyy',dateselected); return false;" title="<?php echo SHOWCALENDAR;?>" name="anchor2" id="anchor2"><?php echo SELECT;?></a>
</span>
</div>
</form>
</div>
<div id="calendar"></div>
<script language="JavaScript" id="jscal" type="text/javascript">
//<![CDATA[
var now = new Date();
var sel = new Date();
var todayselected = true;
var cal = new CalendarPopup("calendar");
cal.setCssPrefix("my");
cal.showNavigationDropdowns();
cal.setYearSelectStartOffset(<?php echo STARTOFFSET_YEARS;?>);
cal.setMonthNames(<?php	echo MONTHNAMES;?>);
cal.setDayHeaders(<?php echo DAYHEADERS;?>);
var daynames = new Array(<?php echo DAYNAMES;?>);
cal.setWeekStartDay(<?php echo MONDAYSTARTSWEEK;?>);
var todaytext = '<?php echo str_replace('\'','\\\'',TODAY);?>';
cal.setTodayText(todaytext);
cal.setReturnFunction('loadIFrame');
cal.offsetX = 0;
cal.offsetY = 0;
document.forms[0].datetext.value = todaytext;
document.forms[0].dateall.value = LZ(now.getDate()) + '/' + LZ(now.getMonth()+1) + '/' + now.getFullYear();
updateClock();
function loadIFrame(y,m,d) {
	document.forms[0].dateall.value = LZ(d) + '/' + LZ(m) + '/' + y;
	if (window.frames && window.frames.myiframe) {
		now = new Date();
		sel.setFullYear(y,m-1,d);
		if (sel.getDate() == now.getDate() &&
			sel.getMonth() == now.getMonth() &&
			sel.getFullYear() == now.getFullYear()) {
			todayselected = true;
			document.forms[0].datetext.value = todaytext;
			<?php
			$srcuri = "snapshot.php?title=no&menu=no&clickurl=" . urlencode("snapshotfull.php?clickurl=snapshothistory.php");
			echo "var srcuri = '$srcuri';\n";
			?>
		}
		else {
			todayselected = false;
			document.forms[0].datetext.value = daynames[sel.getDay()] + ' , ' + LZ(d) + ' ' + cal.monthNames[m-1] + ' ' + y;
			<?php
			if (SNAPSHOTHISTORY_THUMB == 1)
			{
				$swf_width = THUMBWIDTH;
				$swf_height = THUMBHEIGHT;
			}
			else
			{
				$swf_width = WIDTH;
				$swf_height = HEIGHT;
			}
			$srcuri = "swf.php?width=" . $swf_width . "&height=" . $swf_height . "&file=";
			echo "var filesdirpath = '$filesdirpath';\n";
			echo "var srcuri = '$srcuri';\n";
			echo "var usethumb = " . SNAPSHOTHISTORY_THUMB . ";\n";
			?>
			var filename;
			if (usethumb == 1)
				filename = 'shot' + '_' + y + '_' + LZ(m) + '_' + LZ(d) + '_thumb.swf';
			else
				filename = 'shot' + '_' + y + '_' + LZ(m) + '_' + LZ(d) + '.swf';
			var swfuri = filesdirpath + '/' + y + '/' + LZ(m) + '/' + LZ(d) + '/' + filename;
			var swfuri_get = new String(swfuri);
			swfuri_get = swfuri_get.replace(/\//g, "%2F");
			srcuri += swfuri_get + '&back=no';
		}
		window.frames.myiframe.location.href = srcuri;
	}
}
function updateClock() {
	// Update the now variable
	now = new Date();
	
	// If the selected day was in the future and now we got there update the display!
	if (!todayselected						&&
		sel.getDate() == now.getDate()		&&
		sel.getMonth() == now.getMonth()	&&
		sel.getFullYear() == now.getFullYear()) {
		todayselected = true;
		loadIFrame(now.getFullYear(),now.getMonth()+1,now.getDate());
	}
	// Update the sel variable for today in case we passed midnight
	else if (todayselected)
		sel = now;
	
	// Set the clock
	document.forms[0].clock.value =	daynames[now.getDay()] + ' , ' +
									LZ(now.getDate()) + ' ' +
									cal.monthNames[now.getMonth()] + ' ' +
									now.getFullYear() + ' , ' +
									LZ(now.getHours()) + ':' +
									LZ(now.getMinutes()) + ':' +
									LZ(now.getSeconds());
	window.setTimeout('updateClock()', 1000);
}
function onPrev() {
	var tmpdate = new Date();
	tmpdate = sel;
	tmpdate.setDate(tmpdate.getDate()-1);
	loadIFrame(tmpdate.getFullYear(),tmpdate.getMonth()+1,tmpdate.getDate());
}
function onNext() {
	var tmpdate = new Date();
	tmpdate = sel;
	tmpdate.setDate(tmpdate.getDate()+1);
	loadIFrame(tmpdate.getFullYear(),tmpdate.getMonth()+1,tmpdate.getDate());
}
function positionCalendar() {
	var CALENDARWIDTH = 154;
	var pageW = document.documentElement.clientWidth;
	var anchor2elem = document.getElementById('anchor2');
	var anchor2OffsetX = anchor2elem.offsetLeft;
	var offsetParent = anchor2elem.offsetParent;
	while (offsetParent) {
		anchor2OffsetX += offsetParent.offsetLeft;
		offsetParent = offsetParent.offsetParent;
	}
	if (anchor2OffsetX + CALENDARWIDTH > pageW) {
		var x = pageW - CALENDARWIDTH;
		if (x < 0) x = 0;
		cal.offsetX = x - anchor2OffsetX;
	}
	else
		cal.offsetX = 0;
}
//]]>
</script>
<?php
$srcuri = "snapshot.php?title=no&amp;menu=no&amp;clickurl=" . urlencode("snapshotfull.php?clickurl=snapshothistory.php");
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
echo "<div align=\"center\">\n";
echo "<iframe name=\"myiframe\" src=\"$srcuri\" width=\"" . $iframe_width . "\" height=\"" . $iframe_height . "\" frameborder=\"0\">\n";
echo "<p>Click <a href=\"snapshot.php\">here</a> to see today's sequence of snapshots</p>\n";
echo "</iframe>\n";
echo "</div>\n";
?>
</body>
</html>