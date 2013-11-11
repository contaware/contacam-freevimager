<?php
// Start session
session_start();

// Image and video files directory (path relative to document root)
$filesdirpath = str_replace("\\", "/", FILESDIRPATH);
$filesdirpath = rtrim($filesdirpath,"/");
if ($filesdirpath == "") {
	$filesdirpath = dirname($_SERVER['PHP_SELF']);
	$filesdirpath = str_replace("\\", "/", $filesdirpath);
	$filesdirpath = rtrim($filesdirpath, "/");
}

// Current style name
$path_parts = pathinfo(STYLEFILEPATH);
if (!isset($path_parts['filename']))
	$path_parts['filename'] = substr($path_parts['basename'], 0, strrpos($path_parts['basename'], '.'));
$style_postfix = $path_parts['filename'];

// Current protocol
if (!empty($_SERVER['HTTPS']) && (strtolower($_SERVER['HTTPS']) != 'off'))
	$scheme = 'https';
else
	$scheme = 'http';

// Get Internet Explorer version
function getIEVersion() {
	$match = preg_match('/MSIE ([0-9]+\.[0-9]+)/', $_SERVER['HTTP_USER_AGENT'], $reg);
	if ($match == 0) {
		$match = preg_match('/Trident\/.*rv:([0-9]+\.[0-9]+)/', $_SERVER['HTTP_USER_AGENT'], $reg);
		if ($match == 0)
			return -1;
	}
	return floatval($reg[1]);
}

// Get parent URL
function getParentUrl() {
	global $scheme;
	$parent_path = dirname($_SERVER['PHP_SELF']);
	$parent_path = str_replace("\\", "/", $parent_path);
	$parent_path = rtrim($parent_path, "/");
	$parent_path = dirname($parent_path);
	$parent_path = str_replace("\\", "/", $parent_path);
	$parent_path = rtrim($parent_path, "/");
	$host = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];
	return "$scheme://$host$parent_path/";
}