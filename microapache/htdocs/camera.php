<?php
require_once( 'configuration.php' );
if ($show_camera_commands == 0)
	exit('Not permitted');

// Source command
$dir = rtrim(str_replace("\\", "/", dirname($_SERVER['SCRIPT_FILENAME'])), '/');
$fullfilepath = "$dir/CAMERA_IS_OBSCURED.txt";
if (isset($_GET['source'])) {
	$source = ''; // no default
	
	if ($_GET['source'] == 'toggle') {
		if (is_file($fullfilepath))
			$source = 'on';
		else
			$source = 'off';
	}
	else
		$source = $_GET['source'];
	
	if ($source == 'off')
		@file_put_contents($fullfilepath, "Note: delete this file to re-enable the camera");
	else if ($source == 'on') 
		@unlink($fullfilepath);
}

// Give ContaCam time to update the camera state
sleep(1);

// Reload page
if (isset($_GET['backuri'])) {
	$host = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];
	header("Location: $scheme://$host" . $_GET['backuri']);
}