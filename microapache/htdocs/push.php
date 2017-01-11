<?php
$httpbasicauth = 1;
require_once( 'configuration.php' );
session_write_close(); // end the current session and store session data otherwise other frames are not loading!
function doServerPush($file,$type,$poll) {
	// With HTTP/1.1 the output will always be chunked if you don't specify
	// a Content-Length header and a flush occurs. To avoid that we force
	// HTTP/1.0 which is not supporting chunking.
	@apache_setenv('downgrade-1.0','true');
	@apache_setenv('force-response-1.0','true');
	@set_time_limit(0);			// PHP must not terminate this script
	@ignore_user_abort(false);	// Do not ignore user aborts (may not be allowed, so status is checked in the loop)
	@ini_set('zlib.output_compression','off');	// Disable compressed output
	$poll *= 1000;				// Poll in micro seconds

	// Frames separator
	$separator = 'frame_boundary_';
	for ($i = 0, $randChars = Array('A','B'); $i < 16; $i++)
		$separator .= $randChars[rand(0,1)];

	// No Cache
	header('Cache-Control: no-cache');
	header('Pragma: no-cache');
	header('Expires: Thu, 01 Dec 1994 16:00:00 GMT');
	
	// The multipart header
	header("Content-Type: multipart/x-mixed-replace; boundary=$separator");
	$extrabreaks = '';

	do {
		// Multipart boundary
		echo "$extrabreaks--$separator\r\n";
		if ($extrabreaks == '') // Safari needs exactly one blank line, no extra linebreaks on the first one
			$extrabreaks = "\r\n\r\n";
		
		// Output Content-Type
		echo "Content-Type: $type\r\n";
		
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
		
		// Output Content-Length and the content data
		echo "Content-Length: $filecontentsize\r\n\r\n";
		echo $filecontent;

		// Some browsers like Safari need more than 200 bytes between headers, be safe and make 260 spaces
		echo str_pad('',260); // these whitespaces are ignored because of Content-Length
		
		// Store send time and send data
		$lastupdate = @filemtime($file);
		ob_flush();
		flush();
		
		// Wait loop
		$looptime = time();
		do {
			// Update file info
			clearstatcache();
			
			// Sleep
			usleep($poll); // this function works on windows starting from php 5.0.0
			
			// Every 10 seconds, force it to re-evaluate if the user has disconnected
			// (connection_aborted does not know until this happens)
			if (time() - $looptime >= 10)
			{
				$looptime = time();
				echo ' '; // whitespace is ignored at this specific point in the stream
				ob_flush();
				flush();
			}
			
			// Poll the filesystem until the file changes, then send the update.
			// The file time resolution is 1 second, so in case of sub-second
			// refresh do not wait and send with the given $poll rate
			if (SNAPSHOTREFRESHSEC >= 1)
				$wait = !connection_aborted() && $lastupdate == @filemtime($file);
			else
				$wait = false;
		} while ($wait);
	} while (!connection_aborted()); // if aborts are ignored, exit anyway to avoid endless threads
}
if (!isset($_GET['thumb']) || $_GET['thumb'] == 'no')
	$filename = "$filesdirpath/".SNAPSHOTNAME;
else
	$filename = "$filesdirpath/".SNAPSHOTTHUMBNAME;
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = rtrim($doc_root,"\\/")."/".trim($filename,"\\/");
doServerPush($full_path, 'image/jpeg', SERVERPUSH_POLLRATE_MS);