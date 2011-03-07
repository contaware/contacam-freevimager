<?php
require_once( 'configuration.php' );
require_once( 'language.php' ); // Must be here at the top of this file because it outputs the UTF8-BOM!
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<?php
echo "<title>Flash Player</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
<script type="text/javascript"></script>
<script src="js/prototype.js" type="text/javascript"></script>
<script src="js/scriptaculous.js?load=slider" type="text/javascript"></script>
<script src="js/swfobject.js" type="text/javascript"></script>
<script type="text/javascript">
//<![CDATA[ 
swfobject.registerObject("myFlashMovie", "6.0.40", "swf/expressInstall.swf");
//]]>
</script>
<style type="text/css">
/*<![CDATA[*/
<?php
$stylewidth = WIDTH + 1;
echo "#slidercontainer {\n";
echo "position: relative;\n";
echo "margin: 0 auto;\n";
echo "text-align: left;\n";
echo "width: " . $stylewidth . "px;\n}\n";
?>
/*]]>*/
</style>
</head>

<body>
<?php
$filename = $_GET['file'];
$doc_root = $_SERVER['DOCUMENT_ROOT'];
if ($doc_root == "")
	$full_path = trim($filename,"\\/");
else
	$full_path = "$doc_root/" . trim($filename,"\\/");
if (!is_file("$full_path")) {
	echo "<div align=\"center\"><h2>\n";
	echo NOFILE;
	echo "</h2></div>\n";
	echo "</body>\n";
	echo "</html>\n";
	exit();
}
?>

<div class="wrap" id="playercontainer">
<object id="myFlashMovie" classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" width="<?php echo WIDTH;?>" height="<?php echo HEIGHT;?>">
	<param name="movie" value="<?php echo $filename;?>" />
	<param name="quality" value="high" />
	<param name="play" value="false" />
	<param name="allownetworking" value="internal" />
	<param name="allowScriptAccess" value="never" />
	<param name="scale" value="exactfit" />
	<param name="menu" value="false" />
	<param name="loop" value="false" />
	<param name="swliveconnect" value="false" />
	<param name="wmode" value="transparent" />
	<!--[if !IE]>-->
	<object type="application/x-shockwave-flash" data="<?php echo $filename;?>" width="<?php echo WIDTH;?>" height="<?php echo HEIGHT;?>">
	<param name="quality" value="high" />
	<param name="play" value="false" />
	<param name="allownetworking" value="internal" />
	<param name="allowScriptAccess" value="never" />
	<param name="scale" value="exactfit" />
	<param name="menu" value="false" />
	<param name="loop" value="false" />
	<param name="swliveconnect" value="false" />
	<param name="wmode" value="transparent" />
	<!--<![endif]-->
	<div>
		<b>Please download Flash:</b>
		<p><a href="http://www.adobe.com/go/getflashplayer"><img src="http://www.adobe.com/images/shared/download_buttons/get_flash_player.gif" alt="Get Adobe Flash player" /></a></p>
	</div>
	<!--[if !IE]>-->
	</object>
	<!--<![endif]-->
</object>
<br />

<div id="buttonscontainer">
<form name="controller" action="" method="post" id="controller">
<div class="wrap">
<input type="button" value="Play" name="Play" onclick="PlayFlashMovie();" /><input type="button" value="Stop" name="Stop" onclick="StopFlashMovie();" /><input type="button" value="Rew" name="Rewind" onclick="RewindFlashMovie();" /><input type="button" value="&lt;&lt;" name="PrevFrameFast" onclick="PrevFrameFastFlashMovie();" /><input type="button" value="&lt;" name="PrevFrame" onclick="PrevFrameFlashMovie();" /><input type="button" value="&gt;" name="NextFrame" onclick="NextFrameFlashMovie();" /><input type="button" value="&gt;&gt;" name="NextFrameFast" onclick="NextFrameFastFlashMovie();" /><input type="button" value="Zoom+" name="Zoomin" onclick="ZoominFlashMovie();" /><input type="button" value="Zoom-" name="Zoomout" onclick="ZoomoutFlashMovie();" />
</div>
</form>
</div>

<?php
echo "<div id=\"slidercontainer\">\n";
echo "<div id=\"track1\" style=\"width: " . WIDTH . "px; background-image: url('images/scaler_slider_track.gif'); \n";
echo "background-repeat: repeat-x; background-position: center left; height:26px; padding: 0px 0px 0px 1px;\">\n";
echo "<div id=\"handle1\" style=\"width: 13px; height: 26px;\"><img src=\"images/scaler_slider.gif\" alt=\"Slider\" /></div>\n";
echo "</div>\n";
echo "</div>\n";

if (!isset($_GET['back']) || $_GET['back'] != 'no') {
	echo "<br/>\n";
	echo "<div align=\"center\">\n";
	echo "<a class=\"back\" href=\"javascript:history.back();\">" . BACK . "</a>\n";
	echo "</div>\n";
}
?>
<script type="text/javascript" src="js/swf.js"></script>
<script language="JavaScript" type="text/javascript">
//<![CDATA[ 
InitPlayer();
//]]>
</script>
</body>
</html>
