<?php
/* Copyright © 2005-2011 Oliver Pfister, Switzerland, Web: www.contaware.com
 *
 * This source code is provided "as is", without any express or implied 
 * warranty. In no event will the author/developer be held liable or 
 * accountable in any way for any damages arising from the use of this 
 * source code. 
 *
 * Copyright / Usage Details: 
 *
 * You are allowed to modify this source code in any way you want provided 
 * that this license statement remains with this source code and that you
 * put a comment in this source code about the changes you have done.
 *
 *-------------------------------------------------------------------------*/
require_once( 'configuration.php' );
function doServerPush($file,$type,$poll) {
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
	
	// The multipart header
	header("Content-Type: multipart/x-mixed-replace;boundary=$separator",true);
	$extrabreaks = '';

	do {
		// Send one file starting with a multipart boundary
		echo "$extrabreaks--$separator\n";
		if (!$extrabreaks) { $extrabreaks = "\n\n"; } // Safari needs exactly one blank line, no extra linebreaks on the first one
		echo "Content-Type: $type\n";
		$filecontent = file_get_contents($file);
		$filecontentsize = strlen($filecontent);
		echo "Content-Length: $filecontentsize\n\n";
		echo $filecontent;

		// Some browsers like Safari need more than 200 bytes between headers, be safe and make 260 spaces
		echo str_pad('',260); // These whitespaces are ignored because of content-length
		$lastupdate = filemtime($file);
		ob_flush();
		flush();
		$looptime = time();
		do {
			clearstatcache();	// Need updated file info
			usleep($poll);		// This function works on windows starting from php 5.0.0
			if (time() - $looptime >= 10)
			{
				// Every 10 seconds, force it to re-evaluate if the user has disconnected
				// (connection_aborted does not know until this happens)
				// most use-cases will not need this protection because they will keep trying
				// to send updates, but it is here just in case
				$looptime = time();
				echo ' '; // Whitespace is ignored at this specific point in the stream
				ob_flush();
				flush();
			}
			// Poll the filesystem until the file changes, then send the update
			// the file may not always exist for the instant it is being replaced on the filesystem
			// nested loop and filemtime check inspired by http://web.they.org/software/php-push.php
		} while (!connection_aborted() && (!file_exists($file) || ($lastupdate == filemtime($file))));
	} while (!connection_aborted()); // if aborts are ignored, exit anyway to avoid endless threads
}
if (SNAPSHOT_THUMB == 1)
	$filename = SNAPSHOTTHUMBNAME;
else
	$filename = SNAPSHOTNAME;
$filename = "$filesdirpath/".$filename;
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = "$doc_root/" . trim($filename,"\\/");
doServerPush($full_path,'image/jpeg',SERVERPUSH_POLLRATE_MS);
?>