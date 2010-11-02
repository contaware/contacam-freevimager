<?php
/* Copyright © 2005-2010 Oliver Pfister, Switzerland, Web: www.contaware.com
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
$filename = $_GET['file'];
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = "$doc_root/" . trim($filename,"\\/");
if (ini_get('zlib.output_compression'))
	ini_set('zlib.output_compression', 'Off');
$file_extension = strtolower(substr(strrchr($full_path,"."),1));
if ($full_path == "" || !file_exists($full_path) || $file_extension != "swf") 
{
	echo "<html><title>Download Swf</title><body>ERROR: File not found.</body></html>";
	exit;
}
$ctype = "application/x-shockwave-flash";
header("Pragma: public");
header("Expires: 0");
header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
header("Cache-Control: private",false);
header("Content-Type: $ctype");
header("Content-Disposition: attachment; filename=\"".basename($full_path)."\";" );
header("Content-Transfer-Encoding: binary");
header("Content-Length: ".filesize($full_path));
ob_clean();
flush();
readfile("$full_path");
exit();
?>