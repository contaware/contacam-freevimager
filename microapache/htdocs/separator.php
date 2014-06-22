<?php
require_once( 'configuration.php' );
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<?php
echo "<title>Separator</title>\n";
echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n";
?>
<script language="JavaScript" type="text/javascript">
//<![CDATA[
var origRows = null;
function toggleNavAndThumb() {
    if (origRows)
        showNavAndThumb();
	else
        hideNavAndThumb();
}
function hideNavAndThumb() {
	var frameset = parent.document.getElementById("rowsframeset");
	if (frameset) {
		document.showhideimage.src = '<?php echo "styles/show_$style_postfix.gif"; ?>';
		origRows = frameset.rows;
		frameset.rows = "0,12,*";
	}
}
function showNavAndThumb() {
	var frameset = parent.document.getElementById("rowsframeset");
	if (frameset) {
		document.showhideimage.src = '<?php echo "styles/hide_$style_postfix.gif"; ?>';
		frameset.rows = origRows;
		origRows = null;
	}
}
//]]>
</script>
</head>
<body>
<table style="position:relative;width:100%;height:12px" border="0" cellspacing="0" cellpadding="0">
<tr><td style="text-align:center;vertical-align:top"><hr style="display:block;position:absolute;left:0;top:1px;padding:0;margin:0;width:100%;font-size:1px;line-height:0;overflow:visible" /><a style="display:block;position:absolute;left:0;bottom:0;border:0;padding:0;margin:0;outline:none" href="#" onclick="toggleNavAndThumb(); return false;"><img style="border:0;padding:0;margin:0" name="showhideimage" src="<?php echo "styles/hide_$style_postfix.gif"; ?>" alt="show/hide" width="28" height="10" /></a></td></tr>
</table>
</body>
</html>