var myTotalFrames = 3000;
var TotalFramesSet = 0;
var FirstRun = 1;
var SliderTimer = 0;

// Opera returns the right number of TotalFrames but is not starting if we are not waiting a bit...
function InitPlayer()
{
	window.setTimeout("InitPlayerInternal()", 200); // Start in 200 ms
}

function GetFlashMovieObject(movieName)
{
	return swfobject.getObjectById(movieName);
}

function InitPlayerInternal()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	
	// For IE
	if (navigator.appName.indexOf("Microsoft Internet") != -1)
	{	
		if (flashMovie.ReadyState == 4) // 0=Loading, 1=Uninitialized, 2=Loaded, 3=Interactive, 4=Complete
		{
			// TotalFrames is not available until the movie has loaded. Wait for ReadyState = 4
			TotalFramesSet = 1;
		}
		if (flashMovie.TotalFrames > 0)
			myTotalFrames = flashMovie.TotalFrames;
	}
	else
	{
		myTotalFrames = flashMovie.TotalFrames();
		TotalFramesSet = 1;
	}
	
	// Check whether Total Frames is set
	if (TotalFramesSet == 0)
		window.setTimeout("InitPlayerInternal()", 200); // try again in 200 ms
	
	// Start to play the movie
	if (FirstRun)
	{
		PlayFlashMovie();
		FirstRun = 0;
	}
}

function UpdateSlider()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	if (flashMovie.IsPlaying() == false)
		window.clearInterval(SliderTimer);
	var currentFrame=flashMovie.TCurrentFrame("/");
	demoSlider.setValue(parseInt(currentFrame) / (myTotalFrames - 1));
}

function StopFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	flashMovie.StopPlay();
}

function PlayFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	flashMovie.Play();
	SliderTimer = window.setInterval("UpdateSlider()", 500);    
}

function RewindFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	flashMovie.Rewind();
	demoSlider.setValue(0);	
}

function NextFrameFastFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var nextFrame=parseInt(currentFrame) + 25;
	if (nextFrame >= myTotalFrames)
		nextFrame = myTotalFrames - 1;
	flashMovie.GotoFrame(nextFrame);
	demoSlider.setValue(nextFrame / (myTotalFrames - 1));	
}

function NextFrameFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var nextFrame=parseInt(currentFrame) + 1;
	if (nextFrame >= myTotalFrames)
		nextFrame = myTotalFrames - 1;
	flashMovie.GotoFrame(nextFrame);
	demoSlider.setValue(nextFrame / (myTotalFrames - 1));	
}

function PrevFrameFastFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var prevFrame=parseInt(currentFrame) - 25;
	if (prevFrame < 0)
		prevFrame = 0;
	flashMovie.GotoFrame(prevFrame);	
	demoSlider.setValue(prevFrame / (myTotalFrames - 1));	
}

function PrevFrameFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var prevFrame=parseInt(currentFrame) - 1;
	if (prevFrame < 0)
		prevFrame = 0;
	flashMovie.GotoFrame(prevFrame);	
	demoSlider.setValue(prevFrame / (myTotalFrames - 1));	
}

function ZoominFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	flashMovie.Zoom(90);
}

function ZoomoutFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	flashMovie.Zoom(110);
}

function SeekFrame(v)
{
	var flashMovie = GetFlashMovieObject("myFlashMovie");
	var FramePos = (myTotalFrames - 1) * v;
	flashMovie.GotoFrame(FramePos);
}

var demoSlider = new Control.Slider('handle1','track1', 
      {axis:'horizontal', minimum: 0, maximum:110, alignX: 2, increment: 2, sliderValue: 0});

// Called while dragging the thumb
demoSlider.options.onSlide = function(value)
{
	SeekFrame(value);
}