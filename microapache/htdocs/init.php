<?php
$filesdirpath = str_replace("\\", "/", FILESDIRPATH);
$filesdirpath = rtrim($filesdirpath,"/");
if ($filesdirpath == "") {
	$filesdirpath = dirname($_SERVER['PHP_SELF']);
	$filesdirpath = str_replace("\\", "/", $filesdirpath);
	$filesdirpath = rtrim($filesdirpath, "/");
}
$path_parts = pathinfo(STYLEFILEPATH);
if (!isset($path_parts['filename']))
	$path_parts['filename'] = substr($path_parts['basename'], 0, strrpos($path_parts['basename'], '.'));
$style_postfix = $path_parts['filename'];
function getIEVersion() {
	$match = preg_match('/MSIE ([0-9]+\.[0-9]+)/', $_SERVER['HTTP_USER_AGENT'], $reg);
	if ($match == 0)
		return -1;
	else
		return floatval($reg[1]);
}
function getParentUrl() {
	$parent_path = dirname($_SERVER['PHP_SELF']);
	$parent_path = str_replace("\\", "/", $parent_path);
	$parent_path = rtrim($parent_path, "/");
	$parent_path = dirname($parent_path);
	$parent_path = str_replace("\\", "/", $parent_path);
	$parent_path = rtrim($parent_path, "/");
	$host = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];
	return "http://$host$parent_path/";
}
?>