<?php
// Set the $httpbasicauth variable used in authenticate.php
// file included by the below configuration.php
if (!isset($_GET['httpbasicauth']) || $_GET['httpbasicauth'] == 'yes')
	$httpbasicauth = 1;
else
	$httpbasicauth = 0;
require_once( 'configuration.php' );

// End the current session and store session data otherwise other frames are not loading!
session_write_close();

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
		clearstatcache();
		$handle = @fopen($file, 'rb');
		if ($handle === false)
		{
			usleep(15000); // wait 15ms
			continue;
		}
		$filecontentsize = @filesize($file);
		if ($filecontentsize === false)
		{
			fclose($handle);
			usleep(15000); // wait 15ms
			continue;
		}
		$filecontent = fread($handle, 2 * $filecontentsize); // bigger size in case image has been updated between size read and data read
		if ($filecontent === false)
		{
			fclose($handle);
			usleep(15000); // wait 15ms
			continue;
		}
		else
		{
			$filecontentsize = ftell($handle); // get read bytes
			if ($filecontentsize === false)
			{
				fclose($handle);
				usleep(15000); // wait 15ms
				continue;
			}
			else
			{
				fclose($handle);
				break;
			}
		}
	}
	
	// Output Content-Length
	header("Content-Length: $filecontentsize");
	
	// Output content data
	echo $filecontent;
}

// Do a single client poll
if (!isset($_GET['thumb']) || $_GET['thumb'] == 'no')
	$filename = "$filesdirpath/".SNAPSHOTNAME;
else
	$filename = "$filesdirpath/".SNAPSHOTTHUMBNAME;
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = rtrim($doc_root,"\\/")."/".trim($filename,"\\/");
doClientPoll($full_path, 'image/jpeg');