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

if (isset($_GET['year']) && isset($_GET['month']) && isset($_GET['day'])) {
	// Check safety
	check_safety($_GET['year']);
	check_safety($_GET['month']);
	check_safety($_GET['day']);
	
	// Loop passed files
	$doc_root = $_SERVER['DOCUMENT_ROOT'];
	foreach($_GET as $key=>$filenamenoext) {
		if (is_numeric($key) && intval($key) >= 0) {
			// Check safety
			check_safety($filenamenoext);
			
			// Full path without extension
			$filenoext = $_GET['year']."/".$_GET['month']."/".$_GET['day']."/".$filenamenoext;
			if ($doc_root == "")
				$filenoext = "$filesdirpath/$filenoext";
			else
				$filenoext = rtrim($doc_root,"\\/")."/".ltrim($filesdirpath,"\\/")."/$filenoext";
			
			// Rename filename(s) to .recycled
			if (is_file("$filenoext.mp4"))
				strong_rename("$filenoext.mp4","$filenoext.mp4.recycled");
			if (is_file("$filenoext.gif"))
				strong_rename("$filenoext.gif","$filenoext.gif.recycled");
		}
	}
}

// Reload page
if (isset($_GET['backuri'])) {
	$host = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];
	header("Location: $scheme://$host" . $_GET['backuri']);
}