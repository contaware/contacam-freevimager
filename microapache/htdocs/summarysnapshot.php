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
// Init page refresh, title and style sheet
require_once( 'setusertz.php' );
$serveruri = htmlspecialchars($_SERVER['REQUEST_URI']);
echo "<meta http-equiv=\"refresh\" content=\"" . SUMMARYREFRESHSEC . "; URL=" . $serveruri . "\" />\n";
echo "<title>" . SUMMARYTITLE . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
$scriptname = basename($_SERVER['SCRIPT_FILENAME']);

// Selected date
$today_year = (int)date('Y');
$today_month = (int)date('m');
$today_day = (int)date('d');
if (isset($_GET['year']) && isset($_GET['month']) && isset($_GET['day'])) {	// Exact Date
	$selected_year = (int)$_GET['year'];
	$selected_month = (int)$_GET['month'];
	$lastdayof_selected_month = date('d',mktime(0, 0, 0, $selected_month + 1, 0, $selected_year));
	$selected_day = min((int)$_GET['day'],$lastdayof_selected_month);
	$params = "?year=$selected_year&amp;month=$selected_month&amp;day=$selected_day";
}
else {	// Today
	$selected_year = $today_year;
	$selected_month = $today_month;
	$selected_day = $today_day;
	$params = "";
}
$selected_time = mktime(12,0,0,$selected_month,$selected_day,$selected_year);
$selected_weekday_num = date('w', $selected_time);
$selected_days_elapsed = GetDaysElapsed($selected_time);
$selected_year_string = "$selected_year";
if ($selected_month < 10)
	$selected_month_string = "0"."$selected_month";
else
	$selected_month_string = "$selected_month";
if ($selected_day < 10)
	$selected_day_string = "0"."$selected_day";
else
	$selected_day_string = "$selected_day";

// Max thumbs per page
$max_per_page = MAX_PER_PAGE;
$page_offset = 0;
if (isset($_GET['pageoffset']))	{// Thumbs offset
	$page_offset = (int)$_GET['pageoffset'];
	if ($page_offset < 0)
		$page_offset = 0;
}
?>
<script type="text/javascript">
//<![CDATA[
function changeStyle(id) {
	if (parent.window.name != '' && document.getElementById(parent.window.name))
		document.getElementById(parent.window.name).className = 'notselected';
	if (document.getElementById(id))
		document.getElementById(id).className = 'lastselected';
	parent.window.name = id; // this var survives between pages!
}
//]]>
</script>
</head>

<body>
<?php
            
// Functions
function GetDaysElapsed($passed_time) {
	$passed_date  = mktime(12,0,0,date('m',$passed_time),date('d',$passed_time),date('Y',$passed_time));
	$current_date = mktime(13,0,0,date('m'),date('d'),date('Y')); // Leave 13 and not 12 because of rounding problems!
	$days_elapsed = ($current_date - $passed_date) / (60*60*24);
	return ((int)floor($days_elapsed));
}
function GetDeltaUrl($delta_year, $delta_month, $delta_day) {
	global $selected_day;
	global $selected_month;
	global $selected_year;
	global $scriptname;
	$new_time = mktime(12,0,0,$selected_month+$delta_month,$selected_day+$delta_day,$selected_year+$delta_year);
	$new_year = (int)date('Y', $new_time);
	$new_month = (int)date('m', $new_time);
	$new_day = (int)date('d', $new_time);
	return "$scriptname?year=$new_year&amp;month=$new_month&amp;day=$new_day";
}
function PrintFileDate() {
	global $selected_days_elapsed;
	global $selected_weekday_num;
	$daynames = explode(",", str_replace("'", "", DAYNAMES));
	$day_name = $daynames[$selected_weekday_num];
	if ($selected_days_elapsed == 0) {
		echo "$day_name (" . TODAY . ")";
	}
	else if ($selected_days_elapsed > 0) {
		if ($selected_days_elapsed == 1)
			echo "$day_name ($selected_days_elapsed " . DAYAGO . ")";
		else
			echo "$day_name ($selected_days_elapsed " . DAYSAGO . ")";
	}
	else {
		$in_days = -$selected_days_elapsed;
		if ($selected_days_elapsed == -1)
			echo "$day_name (" . IN . " $in_days " . DAY . ")";
		else
			echo "$day_name (" . IN . " $in_days " . DAYS . ")";
	}
}
function PrintNoFilesDate() {
	global $selected_days_elapsed;
	global $selected_weekday_num;
	$daynames = explode(",", str_replace("'", "", DAYNAMES));
	$day_name = $daynames[$selected_weekday_num];
	if ($selected_days_elapsed == 0) {
		echo NOFILESFOR . " $day_name (" . TODAY . ")";
	}
	else if ($selected_days_elapsed > 0) {
		if ($selected_days_elapsed == 1)
			echo NOFILESFOR . " $day_name ($selected_days_elapsed " . DAYAGO . ")";
		else
			echo NOFILESFOR . " $day_name ($selected_days_elapsed " . DAYSAGO . ")";
	}
	else {
		$in_days = -$selected_days_elapsed;
		if ($selected_days_elapsed == -1)
			echo NOFILESFOR . " $day_name (" . IN . " $in_days " . DAY . ")";
		else
			echo NOFILESFOR . " $day_name (" . IN . " $in_days " . DAYS . ")";
	}
}
            
// Header
echo "<div style=\"overflow: auto; width: 100%\">\n";

// Left Header
echo "<div style=\"float: left;\">\n";

// Top Menu
echo "<div>\n";
echo "<a href=\"#\" onclick=\"window.location.reload(); return false;\">" . RELOAD . "</a> |\n";
echo "<a href=\"" . getParentUrl() . "\" target=\"_top\">" . HOME . "</a>\n";
echo "</div>\n";

// Title
echo "<h1>" . SUMMARYTITLE . "</h1>\n";

// Year selector
echo "<span class=\"calbuttons\">";
echo "<a href=\"" . GetDeltaUrl(-1,0,0) . "\">&lt;</a>";
if ($selected_year == $today_year)
	echo "<a class=\"highlight\" href=\"$scriptname\">$selected_year</a>";
else
	echo "<a href=\"$scriptname\">$selected_year</a>";
echo "<a href=\"" . GetDeltaUrl(1,0,0) . "\">&gt;</a>";
echo "</span>\n";

// Month selector
echo "<span class=\"calbuttons\">";
$shortmonthnames = explode(",", str_replace("'", "", SHORTMONTHNAMES));
$month_name = $shortmonthnames[$selected_month - 1];
echo "<a href=\"" . GetDeltaUrl(0,-1,0) . "\">&lt;</a>";
if ($selected_month == $today_month)
	echo "<a class=\"highlight\" href=\"$scriptname\">$month_name</a>";
else
	echo "<a href=\"$scriptname\">$month_name</a>";
echo "<a href=\"" . GetDeltaUrl(0,1,0) . "\">&gt;</a>";
echo "</span>\n";

// Day selector
echo "<span class=\"calbuttons\">";
echo "<a href=\"" . GetDeltaUrl(0,0,-7) . "\">&lt;&lt;</a>";
echo "<a href=\"" . GetDeltaUrl(0,0,-1) . "\">&lt;</a>";
if ($selected_day == $today_day)
	echo "<a class=\"highlight\" href=\"$scriptname\">$selected_day</a>";
else
	echo "<a href=\"$scriptname\">$selected_day</a>";
echo "<a href=\"" . GetDeltaUrl(0,0,1) . "\">&gt;</a>";
echo "<a href=\"" . GetDeltaUrl(0,0,7) . "\">&gt;&gt;</a>";
echo "</span>\n";

// End Left Header
echo "</div>\n";

// Live Preview
$clickbackurl = urlencode(urldecode($_SERVER['REQUEST_URI']));
$clickurl = urlencode("snapshotfull.php?clickurl=$clickbackurl");
$url_iframe = "snapshot.php?title=no&amp;menu=no&amp;countdown=no&amp;thumb=yes&amp;clickurl=$clickurl";
echo "<div style=\"float: right;\"><iframe id=\"iframe$count\" name=\"iframe$count\" src=\"$url_iframe\" width=\"" . THUMBWIDTH . "\" height=\"" . THUMBHEIGHT . "\" frameborder=\"0\" scrolling=\"no\"></iframe></div>";

// End Header
echo "</div>\n";

// Header - Content Separator
echo "<hr />\n";

// Loop through the directory's files and display them
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$dir = "$filesdirpath/".$selected_year_string."/".$selected_month_string."/".$selected_day_string;
else
	$dir = "$doc_root/".ltrim($filesdirpath,"\\/")."/".$selected_year_string."/".$selected_month_string."/".$selected_day_string;
if ($handle = @opendir($dir)) {
	$day_has_files = false;
	$bfirst = true;
	$pages = 1;
			
	// First catch all the wanted files
	$file_array = array();
	while (false !== ($file = readdir($handle))) {
		$path_parts = pathinfo($file);
		if (is_file("$dir/$file")) {
			list($file_prefix, $file_year, $file_month, $file_day, $file_hour, $file_min, $file_sec, $file_postfix) = sscanf($file, "%[a-z,A-Z]_%d_%d_%d_%d_%d_%d_%[a-z,A-Z]");
			$hasgif = is_file($dir."/".basename($file, ".".$path_parts['extension']).".gif");
			if ($path_parts['extension'] == 'gif'				||					// Gif thumb
				($path_parts['extension'] == 'mp4' && !$hasgif)	||					// Mp4 without Gif thumb
				($path_parts['extension'] == 'swf' && !$hasgif)	||					// Swf without Gif thumb
				($path_parts['extension'] == 'avi' && !$hasgif)	||					// Avi without Gif thumb
				($path_parts['extension'] == 'jpg' && $file_postfix == 'thumb')) {	// Snapshot thumb
				$file_time = mktime($file_hour, $file_min, $file_sec, $file_month, $file_day, $file_year); 
				$file_array[$file] = $file_time;
			}
		}
	}
			
	// Now order them and display the wanted page
	if (!empty($file_array)) {
		// Get total images and calc. total pages
		$total_size = count($file_array);
		$pages_float = $total_size / (float)$max_per_page;
		$pages = floor($pages_float);
		if (($pages_float - $pages) > 0.0)
			$pages++;
		
		// Sort by file time
		if (SORT_OLDEST_FIRST == 1)
			asort($file_array);
		else
			arsort($file_array);
		
		// Get html query string of mp4s and swfs pointed by gifs
		$pos = 0;
		$count = 0;
		$mp4pos = 0;
		$mp4s = "";
		$swfpos = 0;
		$swfs = "";
		foreach($file_array as $file => $file_time) {
			$path_parts = pathinfo($file);
			if (!isset($path_parts['filename']))
				$path_parts['filename'] = substr($path_parts['basename'], 0, strrpos($path_parts['basename'], '.'));
			
			if ($pos >= $page_offset && $count < $max_per_page) {
				$filenamenoext = basename($file, ".".$path_parts['extension']);
				if ($path_parts['extension'] == 'gif') {		// Gif thumb
					$mp4file = "$dir/$filenamenoext.mp4";
					$swffile = "$dir/$filenamenoext.swf";
					if (is_file($mp4file)) {
						$mp4s .= "&amp;" . $mp4pos . '=' . urlencode($filenamenoext);
						$mp4pos++;
					}
					if (is_file($swffile)) {
						$swfs .= "&amp;" . $swfpos . '=' . urlencode($filenamenoext);
						$swfpos++;
					}
				}
				$count++;
			}
			$pos++;
		}
		
		// Display
		$pos = 0;
		$count = 0;
		echo "<div style=\"text-align: center\">\n";
		echo "<h2>";
		PrintFileDate();
		echo "</h2>\n";
		foreach($file_array as $file => $file_time) {
			$path_parts = pathinfo($file);
			if (!isset($path_parts['filename']))
				$path_parts['filename'] = substr($path_parts['basename'], 0, strrpos($path_parts['basename'], '.'));
		  
			if ($bfirst) {
				$bfirst = false;
				$day_has_files = true;
			}
			
			if ($pos >= $page_offset && $count < $max_per_page) {
				$file_date = getdate($file_time);
				$file_timestamp = sprintf("%02d:%02d:%02d ", $file_date['hours'], $file_date['minutes'], $file_date['seconds']); // leave a final space so that the text can wrap in the browser!
				$filenamenoext = basename($file, ".".$path_parts['extension']);
				list($file_prefix, $file_year, $file_month, $file_day, $file_hour, $file_min, $file_sec, $file_postfix) = sscanf($filenamenoext, "%[a-z,A-Z]_%d_%d_%d_%d_%d_%d_%[a-z,A-Z]");
				$uribasenoext = "$filesdirpath/$selected_year_string/$selected_month_string/$selected_day_string/$filenamenoext";
				$mp4uri = "$uribasenoext.mp4"; $mp4uri_get = urlencode($mp4uri); $mp4file = "$dir/$filenamenoext.mp4";
				$swfuri = "$uribasenoext.swf"; $swfuri_get = urlencode($swfuri); $swffile = "$dir/$filenamenoext.swf";
				$aviuri = "$uribasenoext.avi"; $avifile = "$dir/$filenamenoext.avi";
				$gifuri = "$uribasenoext.gif";
				if ($path_parts['extension'] == 'gif') {
					if (is_file($mp4file))
						echo "<a href=\"mp4.php?file=$mp4uri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . $mp4s . "\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
					else if (is_file($swffile) && is_file($avifile))
						echo "<span class=\"thumbcontainer\"><a href=\"swf.php?file=$swfuri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . $swfs . "\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a><a class=\"avilink\" href=\"$aviuri\">AVI</a></span>";
					else if (is_file($swffile))
						echo "<a href=\"swf.php?file=$swfuri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . $swfs . "\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
					else if (is_file($avifile))
						echo "<a href=\"$aviuri\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
					else
						echo "<a href=\"#\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
				}
				else if ($path_parts['extension'] == 'jpg') {
					$jpegthumburi = "$uribasenoext.jpg";
					$jpeguri = str_replace("_thumb", "", $jpegthumburi);
					$jpeguri_get = urlencode($jpeguri);
					echo "<a href=\"jpeg.php?file=$jpeguri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . "\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$jpegthumburi\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
				}
				else if ($path_parts['extension'] == 'avi')
					echo "<span class=\"line1\">" . strtoupper($file_prefix) . "</span><a class=\"line2\" href=\"$aviuri\" >$file_timestamp</a>";
				else if ($path_parts['extension'] == 'mp4')
					echo "<span class=\"line1\">" . strtoupper($file_prefix) . "</span><a class=\"line2\" href=\"mp4.php?file=$mp4uri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . "\" >$file_timestamp</a>";
				else if ($path_parts['extension'] == 'swf')
					echo "<span class=\"line1\">" . strtoupper($file_prefix) . "</span><a class=\"line2\" href=\"swf.php?file=$swfuri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . "\" >$file_timestamp</a>";
				$count++;
			}
			$pos++;
		}
		echo "</div>\n";
	}
			
	// If no files found
	if (!$day_has_files) {
		echo "<div style=\"text-align: center\"><h2>\n";
		PrintNoFilesDate();
		echo "</h2></div>\n";
	}
	
	// If more than one page show the pages navigator
	if ($pages > 1) {
		echo "<hr />\n";
		echo "<div style=\"text-align: center\">" . PAGES . " [\n";
		$current_page_offset = 0;
		$file_time_array = array_values($file_array);
		for ($page=1 ; $page <= $pages ; $page++) {
			$file_date = getdate($file_time_array[$current_page_offset]);
			$file_timestamp = sprintf("%02d:%02d", $file_date['hours'], $file_date['minutes']);
			if ($page == 1) {
				if ($current_page_offset == $page_offset) {
					if ($params == "")
						echo " <a class=\"highlight\" href=\"$scriptname\">$file_timestamp</a>\n";
					else
						echo " <a class=\"highlight\" href=\"$scriptname" . $params . "\">$file_timestamp</a>\n";
				}
				else {
					if ($params == "")
						echo " <a href=\"$scriptname\">$file_timestamp</a>\n";
					else
						echo " <a href=\"$scriptname" . $params . "\">$file_timestamp</a>\n";
				}
			} else {
				if ($current_page_offset == $page_offset) {
					if ($params == "")
						echo " <a class=\"highlight\" href=\"$scriptname?pageoffset=$current_page_offset\">$file_timestamp</a>\n";
					else
						echo " <a class=\"highlight\" href=\"$scriptname" . $params . "&amp;pageoffset=$current_page_offset\">$file_timestamp</a>\n";
				}
				else {
					if ($params == "")
						echo " <a href=\"$scriptname?pageoffset=$current_page_offset\">$file_timestamp</a>\n";
					else
						echo " <a href=\"$scriptname" . $params . "&amp;pageoffset=$current_page_offset\">$file_timestamp</a>\n";
				}
			}
			$current_page_offset += $max_per_page;
		}
		echo "]</div>\n";
	}
}
// Given day doesn't exist
else {
	echo "<div style=\"text-align: center\"><h2>\n";
	PrintNoFilesDate();
	echo "</h2></div>\n";
}
?>
<script type="text/javascript">
//<![CDATA[
if (parent.window.name != '' && document.getElementById(parent.window.name))
	document.getElementById(parent.window.name).className = 'lastselected';
//]]>
</script>
</body>
</html>