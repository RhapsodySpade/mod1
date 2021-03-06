#include <GL/glew.h>
#include <InputManager.hpp>
#include "../includes/GLFWManager.hpp"
#include "../includes/GLApplication.hpp"


/*
This file is created to abstract out any environment information that will be used to create
the window and handle input.  Depending on the API used to create the window, it will have
different functions on how to create an OpenGL Context, which is used to store the OpenGL
state and handle interaction between the framebuffer, which is swapped to the front when
all objects have been drawn each frame (used for double buffering).  When the context is
destroyed, OpenGL is destroyed.

Currently we are using the third-party library GLFW to handle cross-platform compiling
so that Windows, Mac OSX and Linux users can compile the tutorials, where before GT was
just focused on Windows (Win32) development.  You can consider this our attempt to
broaden our tent to the other technologies :)  If you want to use Win32, you can just
add the WinMain and code to this file, replacing the main() function below, then just
replace the rest of the functions internals to be Win32 specific.  You can find an
implementation of this online on the tutorial page, which is a Win32Manager.
*/

//Coplien defs


GLFWManager::GLFWManager()	{  }
GLFWManager::~GLFWManager()	{ destroy(); }

GLFWManager::GLFWManager(GLFWManager const &src) {
    *this = src;
    return ;
}

GLFWManager&		GLFWManager::operator=(GLFWManager const & rhs) {

this->_window = rhs.getWindow();

return *this;
}

GLFWwindow*		GLFWManager::getWindow() const{

    return this->_window;
};





// This initializes our window and creates the OpenGL context
int GLFWManager::initialize(int width, int height, std::string strTitle, bool bFullScreen)
{
	// This tries to first init the GLFW library and make sure it is available
	if ( !glfwInit() )
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	// Now we need to use GLFW to tell OpenGL what settings we want, and most importantly, 
	// which version of OpenGL we are using.  We do this with glfwWindowHint() function calls.
	// See http://www.glfw.org/docs/latest/window.html#window_hints for all the window hints.

	// This tells OpenGL that our OpenGL context will have a multisampling value of 4, which means
	// it will use antialiasing to smooth out the jagged pixels when rendering.  So it will same
	// the final rendered image 4 times to make it smooth as silk.  Comment this out to see it jagged.
	glfwWindowHint(GLFW_SAMPLES, 4);

	// This requires that we must be using OpenGL 4 and by specifying the CORE_PROFILE we are saying
	// that we don't want to support any older OpenGL version and don't care about compatibility.
	// If you want to use older OpenGL functions, you would need to pass in GLFW_OPENGL_COMPAT_PROFILE.
	// If your computer doesn't run with 4, try changing it to 3 or download the latest drivers here:
	// https://www.opengl.org/wiki/Getting_Started
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed

	// This next function creates the window with the settings given above. It takes the window width 
	// and height, the title, and monitor information that tells GLFW if we want a fullscreen window or not.
	// To make the window fullscreen you pass in glfwGetPrimaryMonitor() to the second to last parameter.
	if( bFullScreen )
		_window = glfwCreateWindow(width, height, strTitle.c_str(), glfwGetPrimaryMonitor(), nullptr);
	else
		_window = glfwCreateWindow(width, height, strTitle.c_str(), nullptr, nullptr);

	// Make sure the window is valid, if not, throw an error.
	if ( _window == nullptr )
	{
		fprintf(stderr, "Failed to create a GLFW window, you might need to download the latest drivers or change the OpenGL version to 3\n");
		destroy();

		return -1;
	}

	// Create the OpenGL context from the window and settings specified
	glfwMakeContextCurrent(_window);

	// This turns on STICKY_KEYS for keyboard input, so that glfwgetKey() returns GLFW_PRESS the next 
	// time it's called if the key has been released before the call.  We pass in our Window object
	// as the first parameter, then the settings to turn STICKY_KEYS set to TRUE.  Keyboard input will 
	// still work without this, just not "sticky".
	glfwSetInputMode(_window, GLFW_STICKY_KEYS, GL_TRUE);

    // We want to hide the mouse since it will be used to move the camera's view around
    // and don't want to see it being pushed up into the top left corner.
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the cursor position of the hidden mouse to be in the top left of the window.
    // This way we can get a delta of the mouse position from (0, 0) and reset it again.
    glfwSetCursorPos(_window, 0, 0);

    // This turns off the vertical sync to your monitor so it renders as fast as possible
    glfwSwapInterval(0);

	// This is an important part, at least for my system I had to set this parameter to true otherwise
	// OpenGL wouldn't work and the GLEW library would fail to load.  Apparently, this is because by
	// setting it to Experimental, it goes and search for all supported OpenGL functions even if the
	// video card doesn't mark them as standard.  So be sure to set this TRUE if you have the same issue.
	glewExperimental = GL_TRUE;

	// Initialize the GLEW library so that it can go and find the correct OpenGL functions for the
	// current environment, and any new functions that were defined after OpenGL 1.1, since if you
	// are on Windows for instance, the gl.h library won't recognize anything after version 1.1.
	// That means no shader support or OpenGL 4 support.  We could define them ourselves like we
	// did in our old shader tutorials, but this just makes it easier and has become a standard.
	GLenum err = glewInit();

	// If we had an error, return -1.  Be sure to see if glewExperimental isn't true, this worked for me.
	if ( GLEW_OK != err )
	{
		fprintf(stderr, "Failed to initialize glew\n");
		return -1;
	}

	// Return success
	return 0;
}


// This swaps the backbuffer with the front buffer to display the content rendered in OpenGL
void GLFWManager::swapTheBuffers()
{
	// This takes the Window and swaps the backbuffer to the front.  This way we don't see tearing
	// as the content is drawn to the screen each frame.  This is called double buffering.  This
	// should be called at the end of the Render() function in the GLApplication, once all content
	// in done rendering.  We pass in the GLFW window object to this function.
	glfwSwapBuffers(_window);
}


// This function processes all the application's input and returns a bool to tell us if we should continue
bool GLFWManager::processInput(bool continueGame = true)
{
	// Use the GLFW function to check for the user pressing the Escape button, as well as a window close event.
	// If any of these checks return true, return false back to the caller to let them know the user has quit.
	if ( glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(_window) != 0 )
		return false;

    // Below we check if the UP, DOWN, LEFT, RIGHT or W,A,S,D keys are pressed, and send the InputCode
    // down to our InputManager so we can know when to move the camera.  Only up and down will work for now.
    // NOTE: we use if statements instead of a case statement since multiple keys could be down at the same time
    if ( glfwGetKey(_window, GLFW_KEY_UP) || glfwGetKey(_window, GLFW_KEY_W) )
        _inputManager.keyPressed(InputCodes::Up);
    if ( glfwGetKey(_window, GLFW_KEY_DOWN) || glfwGetKey(_window, GLFW_KEY_S) )
        _inputManager.keyPressed(InputCodes::Down);
    if ( glfwGetKey(_window, GLFW_KEY_LEFT) || glfwGetKey(_window, GLFW_KEY_A) )
        _inputManager.keyPressed(InputCodes::Left);
    if ( glfwGetKey(_window, GLFW_KEY_RIGHT) || glfwGetKey(_window, GLFW_KEY_D) )
        _inputManager.keyPressed(InputCodes::Right);

    // Create some variables to store the current mouse position
    double mouseX, mouseY;

    // Grab the current mouse position from our window
    glfwGetCursorPos(_window, &mouseX, &mouseY);

    // If the mouse moved then send it to our InputManager to tell the camera
    if ( mouseX != 0 && mouseY != 0 )
    {
        // Send the updated mouse position to our InputManager
        this->_inputManager.mouseMoved((float)mouseX, (float)mouseY);
    }

    // Set the window's cursor position back to 0,0 (top left corner) so we keep getting a delta
    glfwSetCursorPos(_window, 0, 0);

	// Poll the input events to see if the user quit or closed the window.  This can only be called
	// in the main thread of the application, so apparently you can't spin off another thread for this.
	glfwPollEvents();

	// Return the value passed in to tell the game loop that we should continue or not.  This would be used
	// to have an external variable tracking the state of the game, like from a menu or something.
	return continueGame;
}


// This destroys the window
void GLFWManager::destroy()
{
	// This closes the OpenGL window and terminates the application
	glfwTerminate();
}


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// This file is to abstract our window code from the rest of the application so
// that we can change or inherit from the WindowManager to do another API like 
// Win32, GLUT or SDL.  We are using the cross-platform GLFW library which should 
// allow you to compile this on Windows, Mac OSX or Linux by using the necessary 
// compilers.  We'll try include these cross-platform projects for Mac and Linux 
// so you don't have to generate them yourself.  You will also need to make sure 
// you have the library compiled for your environment instead of the Windows 
// environment in this tutorial.  Please refer to the Main.cpp file for the start 
// of this tutorial.