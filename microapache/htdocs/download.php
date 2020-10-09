<?php
require_once( 'configuration.php' );
$filename = $_GET['file'];
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = rtrim($doc_root,"\\/")."/".trim($filename,"\\/");
@set_time_limit(0); // PHP must not terminate this script
if (@ini_get('zlib.output_compression'))
	@ini_set('zlib.output_compression', 'Off');
$file_extension = strtolower(substr(strrchr($full_path,"."),1));
if ($full_path == "" || !file_exists($full_path) || filesize($full_path) <= 0) {
	header("HTTP/1.1 404 Not Found");
	exit;
}
if ($file_extension == 'mp4')
	$ctype = 'video/mp4';
else if ($file_extension == 'gif')
	$ctype = 'image/gif';
else {
	header("HTTP/1.1 404 Not Found");
	exit;
}

// Output given file
header("Content-Type: $ctype");
if (isset($_SERVER['HTTP_RANGE'])) { // especially iOS wants byte-ranges
	rangedownload($full_path);
} else {
	// IEs on older Windows do support range downloads but they want to see
	// the "Accept-Ranges: bytes" header for the target resource before 
	// starting with the range requests (for example to perform efficient 
	// seek operations). IE11 on Win10 and all new browsers do not need that
	// header to start the range requests. They just send the range requests 
	// and see whether the server supports them. If a server supports ranges, 
	// it replies with the "Accept-Ranges: bytes" header like done below in 
	// our rangedownload() function. Emitting a "Accept-Ranges: bytes" 
	// header here will make IEs on older Windows happy without hurting the 
	// new browsers; the specs allow that: 
	// https://tools.ietf.org/html/rfc7233#section-2.3
	header('Accept-Ranges: bytes');
	header("Content-Length: " . filesize($full_path));
	@ob_flush(); // suppress notice ob_flush(): failed to flush buffer...
	flush();
	if (!connection_aborted()) {
		$fp = @fopen($full_path, 'rb');
		$buffer = 256 * 1024; // buffer still in heap and good size for bigger files
		while (!feof($fp)) {
			echo fread($fp, $buffer);
			@ob_flush(); // suppress notice ob_flush(): failed to flush buffer...
			flush();
			if (connection_aborted()) {
				fclose($fp);
				exit;
			}
		}
		fclose($fp);
    }
}

// Range download support
function rangedownload($full_path) {
	// Open file and init vars
	$fp = @fopen($full_path, 'rb');
	$filesize = filesize($full_path);
	$rangelength = $filesize;
	$lastbytepos = $filesize - 1; // filesize is checked above and must be at least 1
	
	// Tell the client that we are supporting bytes range
	header('Accept-Ranges: bytes');

	// Extract range
	list($param, $range) = explode('=', $_SERVER['HTTP_RANGE'], 2);
	$param = trim($param);
	$range = trim($range);
	
	// Validate param
	if (strtolower($param) != 'bytes') {
		header("HTTP/1.1 400 Invalid Request");
		fclose($fp);
		exit;
	}

	// Make sure the client hasn't sent us a multi-byte range, we only support single ranges!
	if (strpos($range, ',') !== false) {
		header('HTTP/1.1 416 Requested Range Not Satisfiable');
		header("Content-Range: bytes 0-$lastbytepos/$filesize");
		fclose($fp);
		exit;
	}
	
	// Parse range
	if ($range[0] == '-') {
		// Return the substr($range, 1) last bytes of the document
		$startpos = $filesize - substr($range, 1);
		if ($startpos < 0)
			$startpos = 0;
		$endpos = $lastbytepos;
	} else {
		$range = explode('-', $range);
		$startpos = $range[0];
		$endpos = (isset($range[1]) && is_numeric($range[1])) ? $range[1] : $lastbytepos;
		if ($endpos > $lastbytepos)
			$endpos = $lastbytepos;
	}

	// Check positions
	if ($startpos > $endpos) {
		header('HTTP/1.1 416 Requested Range Not Satisfiable');
		header("Content-Range: bytes 0-$lastbytepos/$filesize");
		fclose($fp);
		exit;
	}

	// Emit headers
	$rangelength = $endpos - $startpos + 1;
	header('HTTP/1.1 206 Partial Content');
	header("Content-Range: bytes $startpos-$endpos/$filesize");
	header("Content-Length: $rangelength");
	@ob_flush(); // suppress notice ob_flush(): failed to flush buffer...
	flush();
	if (connection_aborted()) {
		fclose($fp);
		exit;
	}
	
	// Start buffered download
	fseek($fp, $startpos);
	$buffer = 256 * 1024; // buffer still in heap and good size for bigger files
	while (!feof($fp) && ($p = ftell($fp)) <= $endpos) {
		if ($p + $buffer > $endpos)
			$buffer = $endpos - $p + 1;
		echo fread($fp, $buffer);
		@ob_flush(); // suppress notice ob_flush(): failed to flush buffer...
		flush();
		if (connection_aborted()) {
			fclose($fp);
			exit;
		}
	}

	// Close file
	fclose($fp);
}