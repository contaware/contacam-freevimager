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
echo "<title>" . SUMMARYTITLE . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
<style type="text/css">
/*<![CDATA[*/
<?php
echo "table#navcontrols {\n";
echo "height: " . THUMBHEIGHT . "px;\n}\n";
?>
/*]]>*/
</style>
<script language="JavaScript" src="js/calendarpopup.js" type="text/javascript"></script>
<script language="JavaScript" type="text/javascript">
//<![CDATA[
document.write(getCalendarStyles());
//]]>
</script>
</head>

<body>
<div class="menubottom">
<a href="#" onclick="parent.myframe.location.reload(); return false;"><?php	echo RELOAD;?></a> |
<a href="<?php echo getParentUrl();?>" target="_top"><?php echo HOME;?></a>
<?php
if (SHOW_PRINTCOMMAND == 1)
	echo "| <a href=\"#\" onclick=\"parent.myframe.focus(); parent.myframe.print(); parent.myframe.focus(); return false;\">" . PRINTCOMMAND . "</a>\n";
?>
</div>
<div class="wrap">
<form action="">
<input type="hidden" name="dateall" value="" />
<table id="navcontrols">
<tr>
<td height="24px" align="left"><?php echo VIEWFILESFOR;?></td>
<td height="24px" align="left"><input type="button" value="&lt;" class="navbutton" name="prev" id="prev" onclick="onPrev();" /><input id="datetext" type="text" readonly="readonly" name="datetext" value="" size="32" /><input type="button" value="&gt;" class="navbutton" name="next" id="next" onclick="onNext();" />
<a href="#" onclick="positionCalendar(); var dateselected = LZ(sel.getDate()) + '/' + LZ(sel.getMonth()+1) + '/' + sel.getFullYear(); cal.select(document.forms[0].dateall,'anchor2','dd/MM/yyyy',dateselected); return false;" title="<?php echo SHOWCALENDAR;?>" name="anchor2" id="anchor2"><?php echo SELECT;?></a>
</td>
</tr>
<tr>
<td height="24px" align="left"><?php echo LOCALTIME;?></td>
<td height="24px" align="left"><input type="text" readonly="readonly" id="clock" name="clock" value="" size="44" /></td>
</tr>
<tr align="center">
<?php echo "<td colspan=\"2\" align=\"center\"><h1>" . SUMMARYTITLE . "</h1></td>\n";?>
</tr>
</table>
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
cal.setReturnFunction('loadFrame');
cal.offsetX = 0;
cal.offsetY = 0;
document.forms[0].datetext.value = todaytext;
document.forms[0].dateall.value = LZ(now.getDate()) + '/' + LZ(now.getMonth()+1) + '/' + now.getFullYear();
updateClock();
function loadFrame(y,m,d) {
	document.forms[0].dateall.value = LZ(d) + '/' + LZ(m) + '/' + y;
	if (parent.myframe) {
		now = new Date();
		sel.setFullYear(y,m-1,d);
		if (sel.getDate() == now.getDate() &&
			sel.getMonth() == now.getMonth() &&
			sel.getFullYear() == now.getFullYear()) {
			todayselected = true;
			document.forms[0].datetext.value = todaytext;
			<?php
			$srcuri = "summary.php?header=no";
			echo "var srcuri = '$srcuri';\n";
			?>
		}
		else {
			todayselected = false;
			document.forms[0].datetext.value = daynames[sel.getDay()] + ' , ' + LZ(d) + ' ' + cal.monthNames[m-1] + ' ' + y;
			<?php
			$srcuri = "summary.php?header=no";
			echo "var srcuri = '$srcuri';\n";
			?>
			srcuri += '&year=' + y + '&month=' + m + '&day=' + d;
		}
		parent.myframe.location.href = srcuri;
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
		loadFrame(now.getFullYear(),now.getMonth()+1,now.getDate());
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
	loadFrame(tmpdate.getFullYear(),tmpdate.getMonth()+1,tmpdate.getDate());
}
function onNext() {
	var tmpdate = new Date();
	tmpdate = sel;
	tmpdate.setDate(tmpdate.getDate()+1);
	loadFrame(tmpdate.getFullYear(),tmpdate.getMonth()+1,tmpdate.getDate());
}
function positionCalendar() {
	var CALENDARWIDTH = 154;
	var CALENDARHEIGHT = 144;
	var frameW = document.documentElement.clientWidth;
<?php echo "	var frameH = " . THUMBHEIGHT . ";\n";?>
	var anchor2elem = document.getElementById('anchor2');
	var anchor2OffsetX = anchor2elem.offsetLeft;
	var anchor2OffsetY = anchor2elem.offsetTop;
	var offsetParent = anchor2elem.offsetParent;
	while (offsetParent) {
		anchor2OffsetX += offsetParent.offsetLeft;
		anchor2OffsetY += offsetParent.offsetTop;
		offsetParent = offsetParent.offsetParent;
	}
	if (anchor2OffsetX + CALENDARWIDTH > frameW) {
		var x = frameW - CALENDARWIDTH;
		if (x < 0) x = 0;
		cal.offsetX = x - anchor2OffsetX;
	}
	else
		cal.offsetX = 0;
	if (anchor2OffsetY + CALENDARHEIGHT > frameH) {
		var y = frameH - CALENDARHEIGHT;
		if (y < 0) y = 0;
		cal.offsetY = y - anchor2OffsetY;
	}
	else
		cal.offsetY = 0;
}
//]]>
</script>
</body>
</html>
