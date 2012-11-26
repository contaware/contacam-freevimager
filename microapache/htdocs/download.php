<?php
$filename = $_GET['file'];
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = "$doc_root/" . trim($filename,"\\/");
@set_time_limit(0); // PHP must not terminate this script
if (@ini_get('zlib.output_compression'))
	@ini_set('zlib.output_compression', 'Off');
$file_extension = strtolower(substr(strrchr($full_path,"."),1));
if ($full_path == "" || !file_exists($full_path) || $file_extension != "swf") 
{
	echo "<html><title>Download Swf</title><body>ERROR: File not found.</body></html>";
	exit;
}
$ctype = "application/x-shockwave-flash";
header("Pragma: public");
header("Expires: Thu, 01 Dec 1994 16:00:00 GMT");
header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
header("Cache-Control: private",false);
header("Content-Type: $ctype");
header("Content-Disposition: attachment; filename=\"".basename($full_path)."\";" );
header("Content-Transfer-Encoding: binary");
header("Content-Length: ".filesize($full_path));
ob_clean();
flush();
readfile("$full_path");
exit;