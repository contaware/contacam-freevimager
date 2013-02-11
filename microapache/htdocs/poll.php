<?php
require_once( 'configuration.php' );
session_write_close(); // end the current session and store session data otherwise other frames are not loading!
function doClientPoll($file,$type) {
	// No Cache
	header('Cache-Control: no-cache');
	header('Pragma: no-cache');
	header('Expires: Thu, 01 Dec 1994 16:00:00 GMT');
	
	// Output Content-Type
	header("Content-Type: $type");
	
	// Load content and get its size in bytes,
	// retry for a maximum of ~ 5 seconds
	$retry = 333;
	while ($retry > 0) {
		$retry--;
		$filecontent = @file_get_contents($file);
		if ($filecontent === false)
			usleep(15000); // wait 15ms
		else
			break;
	}
	$filecontentsize = strlen($filecontent);
	
	// Output Content-Length
	header("Content-Length: $filecontentsize");
	
	// Output content data
	echo $filecontent;
}
if (!isset($_GET['thumb']) || $_GET['thumb'] == 'no')
	$filename = "$filesdirpath/".SNAPSHOTNAME;
else
	$filename = "$filesdirpath/".SNAPSHOTTHUMBNAME;
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = "$doc_root/" . trim($filename,"\\/");
doClientPoll($full_path,'image/jpeg');