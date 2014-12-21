/*
                
        Basic Spout app for Cinder

        Search for "SPOUT" to see what is required
        Uses the Spout dll

        Based on the RotatingBox CINDER example without much modification
        Nothing fancy about this, just the basics.

        Search for "SPOUT" to see what is required

    ==========================================================================
    Copyright (C) 2014 Lynn Jarvis.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    ==========================================================================

*/

#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/qtime/QuickTimeGl.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include <vector>

// spout
#include "spout.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ReymentaQuicktimePlayerApp : public AppNative {
public:
    void prepareSettings(Settings *settings);
	void setup();
	void update();
	void draw();
    void mouseDown(MouseEvent event);
	void keyDown(KeyEvent event);
    void shutdown();
	void fileDrop(FileDropEvent event);
	void loadMovieFile(const fs::path &path);

	gl::TextureRef			mFrameTexture, mInfoTexture;
	qtime::MovieGlRef		mMovie;
private:
    // -------- SPOUT -------------
    SpoutSender         spoutsender;                    // Create a Spout sender object
    bool                bSenderInitialized;             // true if a sender initializes OK
    bool                bMemoryMode;                    // tells us if texture share compatible
    unsigned int        g_Width, g_Height;              // size of the texture being sent out
    char                SenderName[256];                // sender name 
    gl::TextureRef      spoutSenderTexture;             // Local Cinder texture used for sharing
    // ----------------------------

};

// -------- SPOUT -------------
void ReymentaQuicktimePlayerApp::prepareSettings(Settings *settings)
{
        g_Width  = 640;
        g_Height = 480;
        settings->setWindowSize( g_Width, g_Height );
        settings->setFullScreen( false );
        settings->setResizable( false ); // keep the screen size constant for a sender
        settings->setFrameRate( 60.0f );
}
// ----------------------------

void ReymentaQuicktimePlayerApp::setup()
{

	// -------- SPOUT -------------
    // Set up the texture we will use to send out
    // We grab the screen so it has to be the same size
    bSenderInitialized = false;
    spoutSenderTexture =  gl::Texture::create(g_Width, g_Height);
    strcpy_s(SenderName, "Reymenta Quicktime Spout Sender"); // we have to set a sender name first
    // Optionally test for texture share compatibility
    // bMemoryMode informs us whether Spout initialized for texture share or memory share
    bMemoryMode = spoutsender.GetMemoryShareMode();
	// Initialize the sender
	bSenderInitialized = spoutsender.CreateSender(SenderName, g_Width, g_Height);
}

void ReymentaQuicktimePlayerApp::update()
{
	if (mMovie) mFrameTexture = mMovie->getTexture();
}

void ReymentaQuicktimePlayerApp::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::enableAlphaBlending();

	if (mFrameTexture) {
		Rectf centeredRect = Rectf(mFrameTexture->getBounds()).getCenteredFit(getWindowBounds(), true);
		gl::draw(mFrameTexture, centeredRect);
	}
	if (bSenderInitialized)
	{
		// Grab the screen (current read buffer) into the local spout texture
		spoutSenderTexture->bind();
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_Width, g_Height);
		spoutSenderTexture->unbind();

		// Send the texture for all receivers to use
		// NOTE : if SendTexture is called with a framebuffer object bound, that binding will be lost
		// and has to be restored afterwards because Spout uses an fbo for intermediate rendering
		spoutsender.SendTexture(spoutSenderTexture->getId(), spoutSenderTexture->getTarget(), g_Width, g_Height);
	}

	if (mInfoTexture) {
		gl::draw(mInfoTexture, vec2(20, getWindowHeight() - 20 - mInfoTexture->getHeight()));
	}
}
void ReymentaQuicktimePlayerApp::mouseDown(MouseEvent event)
{

}
void ReymentaQuicktimePlayerApp::keyDown(KeyEvent event)
{
	if (event.getChar() == 'o') {
		fs::path moviePath = getOpenFilePath();
		if (!moviePath.empty())
			loadMovieFile(moviePath);
	}
}
void ReymentaQuicktimePlayerApp::loadMovieFile(const fs::path &moviePath)
{
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGl::create(moviePath);
		mMovie->setLoop();
		mMovie->play();

		// create a texture for showing some info about the movie
		TextLayout infoText;
		infoText.clear(ColorA(0.2f, 0.2f, 0.2f, 0.5f));
		infoText.setColor(Color::white());
		infoText.addCenteredLine(moviePath.filename().string());
		infoText.addLine(toString(mMovie->getWidth()) + " x " + toString(mMovie->getHeight()) + " pixels");
		infoText.addLine(toString(mMovie->getDuration()) + " seconds");
		infoText.addLine(toString(mMovie->getNumFrames()) + " frames");
		infoText.addLine(toString(mMovie->getFramerate()) + " fps");
		infoText.setBorder(4, 2);
		mInfoTexture = gl::Texture::create(infoText.render(true));
	}
	catch (ci::Exception &exc) {
		console() << "Exception caught trying to load the movie from path: " << moviePath << ", what: " << exc.what() << std::endl;
		mMovie.reset();
		mInfoTexture.reset();
	}

	mFrameTexture.reset();
}

void ReymentaQuicktimePlayerApp::fileDrop(FileDropEvent event)
{
	loadMovieFile(event.getFile(0));
}
// -------- SPOUT -------------
void ReymentaQuicktimePlayerApp::shutdown()
{
    spoutsender.ReleaseSender();

}
// This line tells Cinder to actually create the application
CINDER_APP_NATIVE( ReymentaQuicktimePlayerApp, RendererGl )
