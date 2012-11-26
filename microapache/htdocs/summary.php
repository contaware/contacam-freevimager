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
require_once( 'setusertz.php' );
$serveruri = htmlspecialchars($_SERVER['REQUEST_URI']);
echo "<meta http-equiv=\"refresh\" content=\"" . SUMMARYREFRESHSEC . "; URL=" . $serveruri . "\" />\n";
echo "<title>" . SUMMARYTITLE . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
$header = "yes";
if (isset($_GET['header']))
	$header = $_GET['header'];
if (isset($_GET['daysago'])) {	// Days Ago
	$days_elapsed = (int)$_GET['daysago'];
	$days_elapsed_time = GetDaysAgoTime($days_elapsed);
	$display_year = (int)date('Y', $days_elapsed_time);
	$display_month = (int)date('m', $days_elapsed_time);
	$lastdayof_display_month = date('d',mktime(0, 0, 0, $display_month + 1, 0, $display_year));
	$display_day = (int)date('d', $days_elapsed_time);
	$params = "?daysago=$days_elapsed";
	if (isset($_GET['header']))
		$params = $params . "&amp;header=$header";
}
else if (isset($_GET['year']) && isset($_GET['month']) && isset($_GET['day'])) {	// Exact Date
	$display_year = (int)$_GET['year'];
	$display_month = (int)$_GET['month'];
	$lastdayof_display_month = date('d',mktime(0, 0, 0, $display_month + 1, 0, $display_year));
	$display_day = min((int)$_GET['day'],$lastdayof_display_month);
	$days_elapsed = GetDaysElapsed(mktime(12,0,0,$display_month,$display_day,$display_year));
	$params = "?year=$display_year&amp;month=$display_month&amp;day=$display_day";
	if (isset($_GET['header']))
		$params = $params . "&amp;header=$header";
}
else {	// Today
	$display_year = (int)date('Y');
	$display_month = (int)date('m');
	$lastdayof_display_month = date('d',mktime(0, 0, 0, $display_month + 1, 0, $display_year));
	$display_day = (int)date('d');
	$days_elapsed = GetDaysElapsed(mktime(12,0,0,$display_month,$display_day,$display_year));
	if (isset($_GET['header']))
		$params = "?header=$header";
	else
		$params = "";
}
$max_per_page = MAX_PER_PAGE;
$page_offset = 0;
if (isset($_GET['pageoffset']))	{// Thumbs offset
	$page_offset = (int)$_GET['pageoffset'];
	if ($page_offset < 0)
		$page_offset = 0;
}
?>
<script language="JavaScript" type="text/javascript">
//<![CDATA[
// Replace all 'top' with 'parent' if we are iframed from another server (different domain).
// For security reasons in this special case we are not allowed to access the top window name.
function changeStyle(id) {
	if (top.window.name != '' && document.getElementById(top.window.name))
		document.getElementById(top.window.name).className = 'notselected';
	if (document.getElementById(id))
		document.getElementById(id).className = 'lastselected';
	top.window.name = id; // this var survives between pages!
}
//]]>
</script>
</head>

<body style="overflow-x: hidden">
<?php
            
// Functions

function GetDaysElapsed($passed_time) {
	$passed_date  = mktime(12,0,0,date('m',$passed_time),date('d',$passed_time),date('Y',$passed_time));
	$current_date = mktime(13,0,0,date('m'),date('d'),date('Y')); // Leave 13 and not 12 because of rounding problems!
	$days_elapsed = ($current_date - $passed_date) / (60*60*24);
	return ((int)floor($days_elapsed));
}
function GetDaysAgoTime($days_ago) {
	return (int)(time() - ($days_ago*60*60*24));
}
function PrintFileDate($days_elapsed) {
	$daynames = explode(",", str_replace("'", "", DAYNAMES));
	$monthnames = explode(",", str_replace("'", "", MONTHNAMES));
	$days_ago_time = GetDaysAgoTime($days_elapsed);
	$weekday_num = strftime('%w', $days_ago_time);
	$day_name = $daynames[$weekday_num];
	$day_num = strftime('%d', $days_ago_time);
	$month_num = strftime('%m', $days_ago_time) - 1;
	$month_name = $monthnames[$month_num];
	$year_num = strftime('%Y', $days_ago_time);
	if ($days_elapsed == 0) {
		echo $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " (" . TODAY . ")";
	}
	else if ($days_elapsed > 0) {
		if ($days_elapsed == 1)
			echo $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " ($days_elapsed " . DAYAGO . ")";
		else
			echo $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " ($days_elapsed " . DAYSAGO . ")";
	}
	else {
		$in_days = -$days_elapsed;
		if ($days_elapsed == -1)
			echo $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " (" . IN . " $in_days " . DAY . ")";
		else
			echo $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " (" . IN . " $in_days " . DAYS . ")";
	}
}
function PrintNoFilesDate($days_elapsed) {
	$daynames = explode(",", str_replace("'", "", DAYNAMES));
	$monthnames = explode(",", str_replace("'", "", MONTHNAMES));
	$days_ago_time = GetDaysAgoTime($days_elapsed);
	$weekday_num = strftime('%w', $days_ago_time);
	$day_name = $daynames[$weekday_num];
	$day_num = strftime('%d', $days_ago_time);
	$month_num = strftime('%m', $days_ago_time) - 1;
	$month_name = $monthnames[$month_num];
	$year_num = strftime('%Y', $days_ago_time);
	if ($days_elapsed == 0) {
		echo NOFILESFOR . ' ' . $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " (" . TODAY . ")";
	}
	else if ($days_elapsed > 0) {
		if ($days_elapsed == 1)
			echo NOFILESFOR . ' ' . $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " ($days_elapsed " . DAYAGO . ")";
		else
			echo NOFILESFOR . ' ' . $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " ($days_elapsed " . DAYSAGO . ")";
	}
	else {
		$in_days = -$days_elapsed;
		if ($days_elapsed == -1)
			echo NOFILESFOR . ' ' . $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " (" . IN . " $in_days " . DAY . ")";
		else
			echo NOFILESFOR . ' ' . $day_name . ' ' . $day_num . ' ' . $month_name . ' ' . $year_num . " (" . IN . " $in_days " . DAYS . ")";
	}
}
            
// Title + Dates navigation header
if ($header != "no") {
	echo "<div align=\"center\"><h1>" . SUMMARYTITLE . "</h1></div>\n";
	echo "<div align=\"center\">" . YEAR . " [\n";
	for ($year = date('Y') ; $year > date('Y') - MAX_YEARS ; $year--) {
		if ($year == $display_year)
			echo " <a class=\"highlight\" href=\"summary.php?year=$year&amp;month=$display_month&amp;day=$display_day\">$year</a>\n";
		else
			echo " <a href=\"summary.php?year=$year&amp;month=$display_month&amp;day=$display_day\">$year</a>\n";
	}
	echo "]</div>\n";

	echo "<div align=\"center\">" . MONTH . " [\n";
	for ($month = 1 ; $month <= 12 ; $month++) {
		$lastdayofmonth = date('d',mktime(0, 0, 0, $month + 1, 0, $display_year));
		$month_time = mktime(12,0,0,$month,min($display_day,$lastdayofmonth),$display_year);
		$shortmonthnames = explode(",", str_replace("'", "", SHORTMONTHNAMES));
		$month_name = $shortmonthnames[$month - 1];
		if ($month == $display_month)
			echo " <a class=\"highlight\" href=\"summary.php?year=$display_year&amp;month=$month&amp;day=$display_day\">$month_name</a>\n";
		else
			echo " <a href=\"summary.php?year=$display_year&amp;month=$month&amp;day=$display_day\">$month_name</a>\n";
	}
	echo "]</div>\n";

	echo "<div align=\"center\">" . DAY . " [\n";
	for ($day = 1 ; $day <= $lastdayof_display_month ; $day++) {
		if ($day == $display_day)
			echo " <a class=\"highlight\" href=\"summary.php?year=$display_year&amp;month=$display_month&amp;day=$day\">$day</a>\n";
		else
			echo " <a href=\"summary.php?year=$display_year&amp;month=$display_month&amp;day=$day\">$day</a>\n";
	}
	echo "]</div>\n";

	echo "<div align=\"center\">" . DAYSAGO . " [\n";
	if ($days_elapsed == 0)
		echo "<a class=\"highlight\" href=\"summary.php\">" . TODAY . "</a>\n";
	else
		echo "<a href=\"summary.php\">" . TODAY . "</a>\n";
	for ($daysago = 1 ; $daysago <= MAX_DAYSAGO ; $daysago++) {
		if ($daysago == $days_elapsed)
			echo " <a class=\"highlight\" href=\"summary.php?daysago=$daysago\">$daysago</a>\n";
		else
			echo " <a href=\"summary.php?daysago=$daysago\">$daysago</a>\n";
	}
	echo "]</div><br /><hr />\n";
}

// Add a leading 0 if necessary
$display_year_string = "$display_year";
if ($display_month < 10)
	$display_month_string = "0"."$display_month";
else
	$display_month_string = "$display_month";
if ($display_day < 10)
	$display_day_string = "0"."$display_day";
else
	$display_day_string = "$display_day";

// Loop through the directory's files and display them
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$dir = "$filesdirpath/".$display_year_string."/".$display_month_string."/".$display_day_string;
else
	$dir = "$doc_root/".ltrim($filesdirpath,"\\/")."/".$display_year_string."/".$display_month_string."/".$display_day_string;
if ($handle = @opendir($dir)) {
	$day_has_files = false;
	$bfirst = true;
	$pages = 1;
			
	// First catch all the wanted files
	while (false !== ($file = readdir($handle))) {
		$path_parts = pathinfo($file);
		if (is_file("$dir/$file")) {
			list($file_prefix, $file_year, $file_month, $file_day, $file_hour, $file_min, $file_sec, $file_postfix) = sscanf($file, "%[a-z,A-Z]_%d_%d_%d_%d_%d_%d_%[a-z,A-Z]");
			if ($path_parts['extension'] == 'gif'									||	// Movement detection thumbs
				(($path_parts['extension'] == 'jpg') && ($file_postfix == 'thumb'))	||	// Snapshot thumbs
				(($file_prefix == 'rec') && ($path_parts['extension'] == 'avi'))) {		// Avi recordings
				$file_time = mktime($file_hour, $file_min, $file_sec, $file_month, $file_day, $file_year); 
				$file_array[$file] = $file_time;
			}
		}
	}
			
	// Now order them and display the wanted page
	if ($file_array !== NULL) {
		// Get total images and calc. total pages
		$total_size = count($file_array);
		$pages_float = $total_size / (float)$max_per_page;
		$pages = floor($pages_float);
		if (($pages_float - $pages) > 0.0)
			$pages++;
		
		// Sort file time
		if (SORT_OLDEST_FIRST == 1)
			asort($file_array);
		else
			arsort($file_array);
		
		// Get html query string of swfs pointed to by gifs
		$pos = 0;
		$count = 0;
		$swfpos = 0;
		foreach($file_array as $file => $file_time) {
			$path_parts = pathinfo($file);
			if (!isset($path_parts['filename']))
				$path_parts['filename'] = substr($path_parts['basename'], 0, strrpos($path_parts['basename'], '.'));
			
			if ($pos >= $page_offset && $count < $max_per_page) {
				if ($path_parts['extension'] == 'gif') {
					$swffile = $dir."/".basename($file, ".gif").".swf";
					if (is_file($swffile)) {
						$swfs .= "&amp;" . $swfpos . '=' . urlencode(basename($file, ".gif"));
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
		echo "<div align=\"center\">\n";
		echo "<h2>";
		PrintFileDate($days_elapsed);
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
				// Leave a final space so that the text can wrap in the browser
				$file_timestamp = sprintf("%02d:%02d:%02d ", $file_date['hours'], $file_date['minutes'], $file_date['seconds']);
				if ($path_parts['extension'] == 'gif') {
					$swfuri = "$filesdirpath/".$display_year_string."/".$display_month_string."/".$display_day_string."/".basename($file, ".gif").".swf";
					$swfuri_get = urlencode($swfuri);
					$aviuri = "$filesdirpath/".$display_year_string."/".$display_month_string."/".$display_day_string."/".basename($file, ".gif").".avi";
					$gifuri = "$filesdirpath/".$display_year_string."/".$display_month_string."/".$display_day_string."/".$file;
					$swffile = $dir."/".basename($file, ".gif").".swf";
					$avifile = $dir."/".basename($file, ".gif").".avi";
					if (is_file($swffile) && is_file($avifile))
						echo "<span class=\"thumbcontainer\"><a href=\"swf.php?file=$swfuri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . $swfs . "\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a><a class=\"avilink\" href=\"$aviuri\">AVI</a></span>";
					else if (is_file($swffile))
						echo "<a href=\"swf.php?file=$swfuri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . $swfs . "\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
					else if (is_file($avifile))
						echo "<a href=\"$aviuri\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
					else
						echo "<a href=\"#\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$gifuri\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
				}
				else if ($path_parts['extension'] == 'jpg') {
					$jpegthumburi = "$filesdirpath/".$display_year_string."/".$display_month_string."/".$display_day_string."/".$file;
					$jpeguri = str_replace("_thumb", "", $jpegthumburi);
					$jpeguri_get = urlencode($jpeguri);
					echo "<a href=\"jpeg.php?file=$jpeguri_get&amp;backuri=" . urlencode(urldecode($_SERVER['REQUEST_URI'])) . "\" class=\"notselected\" id=\"" . $path_parts['filename'] . "\" onclick=\"changeStyle(this.id);\"><img src=\"$jpegthumburi\" alt=\"$file_timestamp\" align=\"middle\" /></a>";
				}
				else if ($path_parts['extension'] == 'avi') {
					$aviuri = "$filesdirpath/".$display_year_string."/".$display_month_string."/".$display_day_string."/".$file;
					echo "<span class=\"line1\">Rec</span><a class=\"line2\" href=\"$aviuri\" >$file_timestamp</a>";
				}
				$count++;
			}
			$pos++;
		}
		echo "</div>\n";
	}
			
	// If no files found
	if (!$day_has_files) {
		echo "<div align=\"center\"><h2>\n";
		PrintNoFilesDate($days_elapsed);
		echo "</h2></div>\n";
	}
	
	// If more than one page show the pages navigator
	if ($pages > 1) {
		echo "<hr />\n";
		echo "<div align=\"center\">" . PAGES . " [\n";
		$current_page_offset = 0;
		$file_time_array = array_values($file_array);
		for ($page=1 ; $page <= $pages ; $page++) {
			$file_date = getdate($file_time_array[$current_page_offset]);
			$file_timestamp = sprintf("%02d:%02d", $file_date['hours'], $file_date['minutes']);
			if ($page == 1) {
				if ($current_page_offset == $page_offset) {
					if ($params == "")
						echo " <a class=\"highlight\" href=\"summary.php\">$file_timestamp</a>\n";
					else
						echo " <a class=\"highlight\" href=\"summary.php" . $params . "\">$file_timestamp</a>\n";
				}
				else {
					if ($params == "")
						echo " <a href=\"summary.php\">$file_timestamp</a>\n";
					else
						echo " <a href=\"summary.php" . $params . "\">$file_timestamp</a>\n";
				}
			} else {
				if ($current_page_offset == $page_offset) {
					if ($params == "")
						echo " <a class=\"highlight\" href=\"summary.php?pageoffset=$current_page_offset\">$file_timestamp</a>\n";
					else
						echo " <a class=\"highlight\" href=\"summary.php" . $params . "&amp;pageoffset=$current_page_offset\">$file_timestamp</a>\n";
				}
				else {
					if ($params == "")
						echo " <a href=\"summary.php?pageoffset=$current_page_offset\">$file_timestamp</a>\n";
					else
						echo " <a href=\"summary.php" . $params . "&amp;pageoffset=$current_page_offset\">$file_timestamp</a>\n";
				}
			}
			$current_page_offset += $max_per_page;
		}
		echo "]</div>\n";
	}
}
// Given day doesn't exist
else {
	echo "<div align=\"center\"><h2>\n";
	PrintNoFilesDate($days_elapsed);
	echo "</h2></div>\n";
}
?>
<script language="JavaScript" type="text/javascript">
//<![CDATA[
// Replace all 'top' with 'parent' if we are iframed from another server (different domain).
// For security reasons in this special case we are not allowed to access the top window name.
if (top.window.name != '' && document.getElementById(top.window.name))
	document.getElementById(top.window.name).className = 'lastselected';
//]]>
</script>
</body>
</html>