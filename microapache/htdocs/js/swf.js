// Global vars
var myTotalFrames = 3000; // Set some init value
var TotalFramesSet = 0;
var FirstRun = 1;
var PrevPercentLoaded = 0;
var CurrentPercentLoaded = 0;
var PercentLoadedStallingCount = 0;

// Constants
var INIT_PLAYER_MS = 300;
var MAX_PERCENT_LOADED_STALLING_COUNT = 30; // -> allow a max. of 9 seconds (INIT_PLAYER_MS * MAX_PERCENT_LOADED_STALLING_COUNT) of wait time between percent increases
var UPDATE_SLIDER_MS = 500;
var ZOOMIN_PERCENT = 90;
var ZOOMOUT_PERCENT = 110;
var STEPFAST_FRAMES = 20;

function InitPlayer()
{
	// Give flash player time to start loading the movie
	window.setTimeout("InitPlayerInternal()", INIT_PLAYER_MS);
}

// See: http://www.permadi.com/tutorial/flashGetObject/
function GetFlashMovieObject(movieName)
{
	if (window.document[movieName])
		return window.document[movieName];
	else if (getIEVersion() < 0 &&
			document.embeds && document.embeds[movieName])
		return document.embeds[movieName];
	else
		return document.getElementById(movieName);
}

function InitPlayerInternal()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	
	// For IE
	if (getIEVersion() >= 0)
	{	
		// Wait for ReadyState = 4
		// (0=Loading, 1=Uninitialized, 2=Loaded, 3=Interactive, 4=Complete)
		if (flashMovie.ReadyState == 4)
		{
			TotalFramesSet = 1;
			myTotalFrames = flashMovie.TotalFrames;
		}
	}
	else if (flashMovie.TotalFrames() > 0)
	{
		TotalFramesSet = 1;
		myTotalFrames = flashMovie.TotalFrames();
	}
	
	// Update and fix percent loaded variable
	// Note: the following logic is necessary because some movies, for
	// unknown reasons, are stalling in a percentage value smaller than
	// 100 even if they are correctly and completely loaded...
	CurrentPercentLoaded = flashMovie.PercentLoaded();
	if (CurrentPercentLoaded < 0) // I have sometimes seen negative values...
		CurrentPercentLoaded = 0;
	else if (CurrentPercentLoaded > 100) // we never know...
		CurrentPercentLoaded = 100;
	if (CurrentPercentLoaded == PrevPercentLoaded)
		PercentLoadedStallingCount++;
	else
		PercentLoadedStallingCount = 0;
	if (PercentLoadedStallingCount > MAX_PERCENT_LOADED_STALLING_COUNT)
		CurrentPercentLoaded = 100;
		
	// Display info
	UpdateInfoText();
	
	// Check whether fully loaded
	if (TotalFramesSet == 0 || CurrentPercentLoaded != 100)
		window.setTimeout("InitPlayerInternal()", INIT_PLAYER_MS);
	else
	{
		var currentFrame=flashMovie.TCurrentFrame("/");
		if (myTotalFrames - 1 > 0)
			mySlider.setValue(parseInt(currentFrame) / (myTotalFrames - 1));
	}
	
	// Start to play the movie even if we do not know
	// the Total Frames count, add slider and browser
	// window resize event managers
	if (FirstRun)
	{
		PlayFlashMovie();
		mySlider.options.onSlide = function(value){SeekFrame(value);}
		Event.observe(window, "resize", resizeSwf);
		FirstRun = 0;
	}
	
	// Update previous percent loaded variable
	PrevPercentLoaded = CurrentPercentLoaded;
}

function UpdateSlider()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	if (flashMovie.IsPlaying())
		window.setTimeout("UpdateSlider()", UPDATE_SLIDER_MS);
	if (mySlider.dragging == false)
	{
		var currentFrame=flashMovie.TCurrentFrame("/");
		if (myTotalFrames - 1 > 0)
			mySlider.setValue(parseInt(currentFrame) / (myTotalFrames - 1));
		UpdateInfoText();
	}
}

function UpdateInfoText()
{	
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	if (CurrentPercentLoaded == 100)
	{
		var currentFrame=flashMovie.TCurrentFrame("/");
		document.controller.infotext.value = parseInt(currentFrame) + 1;
	}
	else
		document.controller.infotext.value = CurrentPercentLoaded + '%';
}

function StopFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	if (flashMovie.IsPlaying())
		flashMovie.StopPlay();
}

function PlayFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	if (flashMovie.IsPlaying() == false)
	{
		flashMovie.Play();
		window.setTimeout("UpdateSlider()", UPDATE_SLIDER_MS);    
	}
}

function RewindFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	flashMovie.Rewind();
	mySlider.setValue(0);
	UpdateInfoText();
}

function NextFrameFastFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var nextFrame=parseInt(currentFrame) + STEPFAST_FRAMES;
	if (nextFrame >= myTotalFrames)
		nextFrame = myTotalFrames - 1;
	flashMovie.GotoFrame(nextFrame);
	if (myTotalFrames - 1 > 0)
		mySlider.setValue(nextFrame / (myTotalFrames - 1));
	UpdateInfoText();
}

function NextFrameFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var nextFrame=parseInt(currentFrame) + 1;
	if (nextFrame >= myTotalFrames)
		nextFrame = myTotalFrames - 1;
	flashMovie.GotoFrame(nextFrame);
	if (myTotalFrames - 1 > 0)
		mySlider.setValue(nextFrame / (myTotalFrames - 1));
	UpdateInfoText();
}

function PrevFrameFastFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var prevFrame=parseInt(currentFrame) - STEPFAST_FRAMES;
	if (prevFrame < 0)
		prevFrame = 0;
	flashMovie.GotoFrame(prevFrame);
	if (myTotalFrames - 1 > 0)
		mySlider.setValue(prevFrame / (myTotalFrames - 1));
	UpdateInfoText();
}

function PrevFrameFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var prevFrame=parseInt(currentFrame) - 1;
	if (prevFrame < 0)
		prevFrame = 0;
	flashMovie.GotoFrame(prevFrame);
	if (myTotalFrames - 1 > 0)
		mySlider.setValue(prevFrame / (myTotalFrames - 1));
	UpdateInfoText();
}

function ZoominFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	flashMovie.Zoom(ZOOMIN_PERCENT);
}

function ZoomoutFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	flashMovie.Zoom(ZOOMOUT_PERCENT);
}

function SeekFrame(v)
{
	var flashMovie = GetFlashMovieObject("myFlashMovie");
	var FramePos = (myTotalFrames - 1) * v;
	flashMovie.GotoFrame(FramePos);
	UpdateInfoText();
}