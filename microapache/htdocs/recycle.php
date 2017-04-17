<?php
require_once( 'configuration.php' );
if ($show_trash_command == 0)
	exit('Direct access not permitted');

function check_safety($name) {
	if (strpos($name, '/') !== false)
		exit('/ not allowed!');
	else if (strpos($name, '\\') !== false)
		exit('\\ not allowed!');
	else if (strpos($name, '.') !== false)
		exit('. not allowed!');
	else if (strpos($name, ':') !== false)
		exit(': not allowed!');
}

function strong_rename($old, $new) {
	// Try renaming for a maximum of ~ 5 seconds
	$retry = 333;
	while ($retry > 0) {
		$retry--;
		if (@rename($old, $new) === false)
			usleep(15000); // wait 15ms
		else
			return;
	}
	$last_error = error_get_last();
	exit($last_error['message']);
}

if (isset($_GET['year']) && isset($_GET['month']) && isset($_GET['day']) && isset($_GET['filenamenoext'])) {
	// Check safety
	check_safety($_GET['year']);
	check_safety($_GET['month']);
	check_safety($_GET['day']);
	check_safety($_GET['filenamenoext']);
	
	// Full path without extension
	$doc_root = $_SERVER['DOCUMENT_ROOT'];
	$filenoext = $_GET['year']."/".$_GET['month']."/".$_GET['day']."/".$_GET['filenamenoext'];
	if (strpos($_GET['filenamenoext'], "_thumb") === false)
		$filenoext_other = $_GET['year']."/".$_GET['month']."/".$_GET['day']."/".$_GET['filenamenoext']."_thumb";
	else
		$filenoext_other = $_GET['year']."/".$_GET['month']."/".$_GET['day']."/".str_replace("_thumb", "", $_GET['filenamenoext']);
	if ($doc_root == "") {
		$filenoext = "$filesdirpath/$filenoext";
		$filenoext_other = "$filesdirpath/$filenoext_other";
	}
	else {
		$filenoext = rtrim($doc_root,"\\/")."/".ltrim($filesdirpath,"\\/")."/$filenoext";
		$filenoext_other = rtrim($doc_root,"\\/")."/".ltrim($filesdirpath,"\\/")."/$filenoext_other";
	}
	
	// Rename filename to .recycled
	if (is_file("$filenoext.mp4"))
		strong_rename("$filenoext.mp4","$filenoext.mp4.recycled");
	if (is_file("$filenoext.avi"))
		strong_rename("$filenoext.avi","$filenoext.avi.recycled");
	if (is_file("$filenoext.swf"))
		strong_rename("$filenoext.swf","$filenoext.swf.recycled");
	if (is_file("$filenoext.gif"))
		strong_rename("$filenoext.gif","$filenoext.gif.recycled");
	if (is_file("$filenoext.jpg"))
		strong_rename("$filenoext.jpg","$filenoext.jpg.recycled");
	
	// Rename other filename to .recycled
	if (is_file("$filenoext_other.mp4"))
		strong_rename("$filenoext_other.mp4","$filenoext_other.mp4.recycled");
	if (is_file("$filenoext_other.avi"))
		strong_rename("$filenoext_other.avi","$filenoext_other.avi.recycled");
	if (is_file("$filenoext_other.swf"))
		strong_rename("$filenoext_other.swf","$filenoext_other.swf.recycled");
	if (is_file("$filenoext_other.gif"))
		strong_rename("$filenoext_other.gif","$filenoext_other.gif.recycled");
	if (is_file("$filenoext_other.jpg"))
		strong_rename("$filenoext_other.jpg","$filenoext_other.jpg.recycled");
	
	// Reload page
	if (isset($_GET['backuri'])) {
		$host = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];
		header("Location: $scheme://$host" . $_GET['backuri']);
	}
}