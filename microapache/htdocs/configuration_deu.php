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
define ("SERVERPUSH_POLLRATE_MS","150");
// Summary will auto-reload each given seconds
define ("SUMMARYREFRESHSEC","300");
// Javascript date picker years start offset
define ("STARTOFFSET_YEARS","4");
// Linear date picker maximum number of shown years
define ("MAX_YEARS","11");
// Linear date picker maximum number of shown days ago
define ("MAX_DAYSAGO","20");
// mjpeg(=server push) is not supported by Internet Explorer.
// If this flag is set to 1 Internet Explorer will continue to work
// in client poll mode, all the other browsers will switch to server
// push mode: Firefox, Chrome, Safari and iPhone support that well,
// Opera and Android have problems.
define ("USESERVERPUSH","0");


/******************************
* Change the following values *
******************************/

// Titles
define("SUMMARYTITLE","Summary");
define("SNAPSHOTTITLE","Snapshot");

// Default page (index.php redirects to this page)
// Choose between: summary.php, summarysnapshot.php
// summaryiframe.php, snapshot.php, snapshotfull.php, snapshothistory.php
define ("DEFAULTPAGE","summarysnapshot.php");

// Set the Language
define ("LANGUAGEFILEPATH","languages/german.php");

// Set the Style
define ("STYLEFILEPATH","styles/darkgray.css");

// Show / hide the print command
define ("SHOW_PRINTCOMMAND","0");     // 0 = Hide , 1 = Show

// Show / hide the save command
define ("SHOW_SAVECOMMAND","0");      // 0 = Hide , 1 = Show

// Use normal or thumb version for the snapshot history viewer
define ("SNAPSHOTHISTORY_THUMB","0"); // 0 = Fullsize version , 1 = Thumb version

// Snapshots will refresh each given seconds
define ("SNAPSHOTREFRESHSEC","1");

// Top-right preview size for summarysnapshot.php
// and snapshot history thumb size
define ("THUMBWIDTH","228");
define ("THUMBHEIGHT","172");

// The with and height of the fullsize snapshots and the fullsize swf movies
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
?>