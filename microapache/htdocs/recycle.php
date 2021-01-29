<?php
require_once( 'configuration.php' );
if ($show_trash_command == 0)
	exit('Not permitted');

function check_safety($name) {
	if (strpos($name, '/') !== false)
		exit('/ not allowed!');
	else if (strpos($name, '\\') !== false)
		exit('\\ not allowed!');
	else if (strpos($name, '.') !== false)
		exit('. not allowed!');
	else if (strpos($name, ':') !== false)
		exit(': not allowed!');
	else if (strpos($name, chr(0)) !== false)
		exit('null byte not allowed!');
}

function strong_delete($fullfilepath) {
	// Try deleting for a maximum of ~ 5 seconds
	$retry = 333;
	while ($retry > 0) {
		$retry--;
		if (@unlink($fullfilepath) === false)
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
			
			// Delete file(s)
			if (is_file("$filenoext.mp4"))
				strong_delete("$filenoext.mp4");
			if (is_file("$filenoext.gif"))
				strong_delete("$filenoext.gif");
		}
	}
}

// Reload page
if (isset($_GET['backuri'])) {
	$host = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];
	header("Location: $scheme://$host" . $_GET['backuri']);
}