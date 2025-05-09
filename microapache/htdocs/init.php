<?php
// Start session
session_start();

// Image and video files directory (path relative to document root)
// Note: $filesdirpath is not ending with a slash!
if (defined('FILESDIRPATH'))
	$filesdirpath = str_replace("\\", "/", FILESDIRPATH);
else
	$filesdirpath = "";
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
if ((!empty($_SERVER['HTTPS']) && strtolower($_SERVER['HTTPS']) != 'off') ||
	(!empty($_SERVER['HTTP_X_FORWARDED_PROTO']) && $_SERVER['HTTP_X_FORWARDED_PROTO'] == 'https'))
	$scheme = 'https';
else
	$scheme = 'http';

// Full stretch
if (defined('FULL_STRETCH'))
	$full_stretch = FULL_STRETCH;
else
	$full_stretch = 0;

// Trash command
if (defined('SHOW_TRASH_COMMAND'))
	$show_trash_command = SHOW_TRASH_COMMAND;
else
	$show_trash_command = 0;

// Camera commands
if (defined('SHOW_CAMERA_COMMANDS'))
	$show_camera_commands = SHOW_CAMERA_COMMANDS;
else
	$show_camera_commands = 0;

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

// Escape javascript strings
function javascriptspecialchars($str) {
	$output = '';
	$str = str_split($str);
	for ($i = 0 ; $i < count($str) ; $i++) {
		switch ($str[$i]) {
			case "'":
			case '"':
			case "&":
			case "<":
			case ">":
				$output .= sprintf("\\u%04x", ord($str[$i]));
				break;
			default:
				$output .= $str[$i];
				break;
		}
	}
	return $output;
}

// Do authentication
if (file_exists(dirname(dirname($_SERVER['SCRIPT_FILENAME'])).'/authenticate.php'))
	require_once(dirname(dirname($_SERVER['SCRIPT_FILENAME'])).'/authenticate.php');