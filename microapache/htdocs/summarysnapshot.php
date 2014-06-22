<?php
require_once( 'configuration.php' );
require_once( LANGUAGEFILEPATH ); // Must be here at the top of this file because it outputs the UTF8-BOM!
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Frameset//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<?php
echo "<title>" . SUMMARYTITLE . "</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
</head>
<?php
$summaryuri = "summary.php";
$summaryuri_noheader = $summaryuri . "?header=no";
$snapshoturi = "snapshot.php?title=no&amp;menu=no&amp;countdown=no&amp;thumb=yes&amp;clickurl=" . urlencode("snapshotfull.php?clickurl=summarysnapshot.php");
echo "<frameset id=\"rowsframeset\" rows=\"" . THUMBHEIGHT . ",12,*\" border=\"0\" frameborder=\"0\" framespacing=\"0\">\n";
	echo "<frameset cols=\"*," . THUMBWIDTH . "\" border=\"0\" frameborder=\"0\" framespacing=\"0\">\n";
		echo "<frame src=\"summarynav.php\" scrolling=\"no\" noresize=\"noresize\" />\n";
		echo "<frame src=\"" . $snapshoturi . "\" scrolling=\"no\" noresize=\"noresize\" />\n";
	echo "</frameset>\n";
	echo "<frame src=\"separator.php\" scrolling=\"no\" noresize=\"noresize\" />\n";
	echo "<frame src=\"" . $summaryuri_noheader . "\" name=\"myframe\" />\n";
	echo "<noframes>\n";
		echo "<body>\n";
			echo "<p>Click <a href=\"" . $summaryuri . "\">here</a> to see the Summary</p>\n";
		echo "</body>\n";
	echo "</noframes>\n";
echo "</frameset>\n";
?>
</html>