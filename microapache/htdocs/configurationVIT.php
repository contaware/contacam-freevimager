<?php
/************************************
* Usually do not edit the following *
************************************/
// Version
define ("VERSION","3.0");
// Image and video files directory (path relative to document root)
// If FILESDIRPATH is set to "" configuration.php's dir will be used
define ("FILESDIRPATH","");
// The snapshot file names
// (these files must reside in the above set FILESDIRPATH directory)
define ("SNAPSHOTNAME","snapshot.jpg");
define ("SNAPSHOTTHUMBNAME","snapshot_thumb.jpg");
// In mjpeg(=server push) mode snapshots will be checked each
// given milliseconds to see if they have changed
define ("SERVERPUSH_POLLRATE_MS","200");
// Summary will auto-reload each given seconds
define ("SUMMARYREFRESHSEC","300");


/******************************
* Change the following values *
******************************/

// Titles
define("SUMMARYTITLE","Summary");
define("SNAPSHOTTITLE","Snapshot");

// Default page (index.php redirects to this page)
// Choose between: summarysnapshot.php, summaryiframe.php,
// snapshot.php, snapshotfull.php, snapshothistory.php
define ("DEFAULTPAGE","summarysnapshot.php");

// Set the Language
define ("LANGUAGEFILEPATH","languages/vietnamese.php");

// Set the Style
define ("STYLEFILEPATH","styles/darkgray.css");

// Show / hide the camera commands
define ("SHOW_CAMERA_COMMANDS","0");  // 0 = Hide , 1 = Show

// Snapshots will refresh each given seconds
define ("SNAPSHOTREFRESHSEC","1");

// Top-right preview size for summarysnapshot.php
define ("THUMBWIDTH","228");
define ("THUMBHEIGHT","172");

// The with and height of the fullsize snapshots and the fullsize movies
define ("WIDTH","640");
define ("HEIGHT","480");

// Maximum number of summary thumbnails per page
// Note: Internet explorer has a limitation of the maximum
// number of shown animated gifs per page (not more than 36 is safe)
define ("MAX_PER_PAGE","27");

// Sort order of summary thumbnails
define ("SORT_OLDEST_FIRST","1"); // 0 = newest first , 1 = oldest first


/**************************************
* Initialization, do not remove that! *
**************************************/
require_once( 'init.php' );