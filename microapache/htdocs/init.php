<?php
// Files directory path initialization
$filesdirpath = str_replace("\\", "/", FILESDIRPATH);
$filesdirpath = rtrim($filesdirpath,"/");
if ($filesdirpath == "") {
	$path_parts = pathinfo($_SERVER['SCRIPT_NAME']);
	$filesdirpath = str_replace("\\", "/", $path_parts['dirname']);
	$filesdirpath = rtrim($filesdirpath,"/");
}
$path_parts = pathinfo(STYLEFILEPATH);
if (!isset($path_parts['filename']))
	$path_parts['filename'] = substr($path_parts['basename'], 0, strrpos($path_parts['basename'], '.'));
$style_postfix = $path_parts['filename'];
function getIEVersion() {
	ereg('MSIE ([0-9]\.[0-9])',$_SERVER['HTTP_USER_AGENT'],$reg);
	if (isset($reg[1]))
		return floatval($reg[1]);
	else
		return -1;
}
?>