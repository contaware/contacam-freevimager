// JScript source code
window.onload = onLoad;

function onLoad()
{
	if(typeof(IsPrinterFriendly) == "undefined")
	{
		showExpandAll();
		collapseHiddenSections();
	}
	else
	{
		var sa = document.getElementById("chkViewAll");

		if (sa != null)
		{
			sa.checked = false;
		}
	}
}

function showExpandAll()
{
	var elem = document.getElementById("selectAll");
	if (elem != null)
	{
	    elem.style.display ='';
	}
}

function collapseHiddenSections()
{
	var sections = document.getElementsByTagName("tr");
	for (var i = 0; i < sections.length; i++)
	{
		if (sections[i].className=="collapse")
		{
			showPieceFindIcon( false, sections[i]);
		}
	}
}

function toggleAllCollapsedSections()
{
    var sa = document.getElementById("chkViewAll");
    var ch = sa.checked;
	var sections = document.getElementsByTagName("tr");
	var visibleClass;
	
	for (var i = 0; i < sections.length; i++)
	{
		if ((sections[i].className=="expand") ||
		    (sections[i].className=="collapse"))
		{
			showPieceFindIcon( ch, sections[i]);
		}
	}
}

function toggleV(id)
{
	var elem = document.getElementById("eC"+id);
	var icon = document.getElementById("ico"+id);
	var show = (elem.style.display != '');

	showPiece(show, elem, icon);
}

function showPieceFindIcon( show, elem )
{
	var iconId = "ico"+elem.id.slice(2);
	var icon = document.getElementById(iconId);
	
	showPiece(show, elem, icon);
}

function showPiece( show, elem, icon )
{
	if ( elem == null || icon == null)
	{
		return;
	}
	
    if (show)
    {
	    elem.style.display ='';
   		icon.src=GetApPath()+"/templates/common/img/minus.gif";
    }
    else
    {
	    elem.style.display = 'none';
		icon.src=GetApPath()+"/templates/common/img/plus.gif";
	    var sa = document.getElementById("chkViewAll");
	    
	    if (sa != null)
	    {
		    sa.checked = false;
		}
		
    }

}

function closep(id)
{
	var a = document.getElementById("ch_"+id);
	if (a != null)
	{
		a.checked = true;
	}
	
	var t = document.getElementById(id);
	if (t!=null)
	{
		t.style.display = "none";
	}
	
}

function p(id, num)
{
	var a = document.getElementById(num+"_"+id);
	var elem = a;
	
	var defPop = document.getElementById(id);
	var x = 0;
	var y = elem.offsetHeight * 2;
	


	if(defPop.style.display=="none")
	{
		closeOtherGlossaryDefs();		
		defPop.style.display="block";
		if (defPop.clientHeight > 200)
		{
			defPop.style.height = 200;
		}
		
		while (elem.tagName != "BODY")
		{
			y += elem.offsetTop;
			x += elem.offsetLeft;
			
			elem = elem.offsetParent;
		}
		
		if (x + defPop.clientWidth > document.body.clientWidth - 8 )
		{
			var temp = document.body.clientWidth - defPop.clientWidth - 8;
			if (temp < 0)
			{
				temp = 0
			}
			
			x = temp;
		}

		defPop.style.top = y
		defPop.style.left = x;
		a.setCapture();
		event.returnValue = false;
	}
	else
	{
		defPop.style.display="none";
		a.releaseCapture();
		event.returnValue = false;
	}
}

function closeOtherGlossaryDefs()
{
	var defs = document.getElementsByTagName("div");
	for (var i = 0; i < defs.length; i++)
	{
		if (defs[i].className=="glossaryItem")
		{
			defs[i].style.display = "none";
		}
	}
}
