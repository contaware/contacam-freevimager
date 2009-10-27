<?php
// If we want to remove the UTF8-BOM of a file put this on top of it!
ob_start('ob');
function ob($buffer)
{
    return str_replace("\xef\xbb\xbf", '', $buffer);
}
?>