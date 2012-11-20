<?php
require_once( 'configuration.php' );
$host     = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];
$rel_path = dirname($_SERVER['PHP_SELF']);
$rel_path = str_replace("\\", "/", $rel_path);
$rel_path = rtrim($rel_path, "/");
$file  = DEFAULTPAGE;
if (!empty($_SERVER['HTTPS']) && (strtolower($_SERVER['HTTPS']) != 'off'))
	header("Location: https://$host$rel_path/$file");
else
	header("Location: http://$host$rel_path/$file");