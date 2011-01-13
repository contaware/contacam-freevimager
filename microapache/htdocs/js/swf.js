/* Copyright © 2005-2011 Oliver Pfister, Switzerland, Web: www.contaware.com
 *
 * This source code is provided "as is", without any express or implied 
 * warranty. In no event will the author/developer be held liable or 
 * accountable in any way for any damages arising from the use of this 
 * source code. 
 *
 * Copyright / Usage Details: 
 *
 * You are allowed to modify this source code in any way you want provided 
 * that this license statement remains with this source code and that you
 * put a comment in this source code about the changes you have done.
 *
 *-------------------------------------------------------------------------*/

var myTotalFrames = 3000; // Set some init value
var TotalFramesSet = 0;
var FirstRun = 1;
var SliderTimer = 0;

// Note: For loading progress we could poll flashMovie.PercentLoaded()
// (it returns a value from 0 up to 100)

function InitPlayer()
{
	// Give flash player time to start loading the movie
	window.setTimeout("InitPlayerInternal()", 300);
}

function GetFlashMovieObject(movieName)
{
	if (window.document[movieName])
		return window.document[movieName];
	else if (navigator.appName.indexOf("Microsoft Internet")==-1)
	{
		if (document.embeds && document.embeds[movieName])
			return document.embeds[movieName]; 
	}
	else
		return document.getElementById(movieName);
}

function InitPlayerInternal()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	
	// For IE
	if (navigator.appName.indexOf("Microsoft Internet") != -1)
	{	
		// Wait for ReadyState = 4
		// (0=Loading, 1=Uninitialized, 2=Loaded, 3=Interactive, 4=Complete)
		if (flashMovie.ReadyState == 4)
			TotalFramesSet = 1;
		// Flash player may slowly increment the TotalFrames value while loading the movie,
		// but only with a ReadyState of 4 we have the full count!
		if (flashMovie.TotalFrames > 0)
			myTotalFrames = flashMovie.TotalFrames;
	}
	else if (flashMovie.TotalFrames() > 0)
	{
		TotalFramesSet = 1;
		myTotalFrames = flashMovie.TotalFrames();
	}
	
	// Check whether Total Frames is fully set,
	// if not give flash player some more time to set it
	if (TotalFramesSet == 0)
		window.setTimeout("InitPlayerInternal()", 300);
	
	// Start to play the movie even if we do not know
	// the Total Frames count
	if (FirstRun)
	{
		PlayFlashMovie();
		FirstRun = 0;
	}
}

function UpdateSlider()
{	
	if (mySlider.dragging == false)
	{
		var flashMovie=GetFlashMovieObject("myFlashMovie");
		if (flashMovie.IsPlaying() == false)
			window.clearInterval(SliderTimer);
		var currentFrame=flashMovie.TCurrentFrame("/");
		mySlider.setValue(parseInt(currentFrame) / (myTotalFrames - 1));
	}
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
	mySlider.setValue(0);	
}

function NextFrameFastFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var nextFrame=parseInt(currentFrame) + 25;
	if (nextFrame >= myTotalFrames)
		nextFrame = myTotalFrames - 1;
	flashMovie.GotoFrame(nextFrame);
	mySlider.setValue(nextFrame / (myTotalFrames - 1));	
}

function NextFrameFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var nextFrame=parseInt(currentFrame) + 1;
	if (nextFrame >= myTotalFrames)
		nextFrame = myTotalFrames - 1;
	flashMovie.GotoFrame(nextFrame);
	mySlider.setValue(nextFrame / (myTotalFrames - 1));	
}

function PrevFrameFastFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var prevFrame=parseInt(currentFrame) - 25;
	if (prevFrame < 0)
		prevFrame = 0;
	flashMovie.GotoFrame(prevFrame);	
	mySlider.setValue(prevFrame / (myTotalFrames - 1));	
}

function PrevFrameFlashMovie()
{
	var flashMovie=GetFlashMovieObject("myFlashMovie");
	var currentFrame=flashMovie.TCurrentFrame("/");
	var prevFrame=parseInt(currentFrame) - 1;
	if (prevFrame < 0)
		prevFrame = 0;
	flashMovie.GotoFrame(prevFrame);	
	mySlider.setValue(prevFrame / (myTotalFrames - 1));	
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

var mySlider = new Control.Slider('handle1','track1', 
      {axis:'horizontal', minimum: 0, maximum:110, alignX: 2, increment: 2, sliderValue: 0});

// Called while dragging the thumb
mySlider.options.onSlide = function(value)
{
	SeekFrame(value);
}