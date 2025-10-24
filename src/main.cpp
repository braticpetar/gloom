/*
  Compilation on Linux
  g++ -std=c++17 ./src/* -o prog -I ./include/ -I ./thirdparty/glm-master/ -lSDL2 -ldl
*/

// Third Party Libraries
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// #################### vvv Globals vvv ####################
// Globals are prefixed with 'g'

// Screen dimensions
int gScreenWidth = 640;
int gScreenHeight = 480;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

// Main loop flag
bool gQuit = false; // If this is true then the program terminates

// shader
// The following stores the unique id for the graphics pipeline
// program object that will be used for our OpenGL draw calls.
GLuint gGraphicsPipelineShaderProgram = 0;

// OpenGL Objects
// Vertex Array Object (VAO)
// Vertex array objects encapsulate all of the items needed to render an object
// For example, we may have multiple vertex buffer objects (VBO) related to rendering one object.
// The VAO allows us to setup the OpenGL state to render that object using the correct
// layout and correct buffers with one call after being setup
GLuint gVertexArrayObject = 0;
//Vertex Buffer Object (VBO)
// Vertex Buffer Objects store information relating to vertices (e.g. positions, normals, textures)
// VBOs are our mechanisim for arranging geometry on the GPU.
GLuint gVertexBufferObject = 0;
// Index Buffer Object (IBO)
// This is used to store the array of indices that we want
// to draw from, when we do indexed drawing.
GLuint gIndexBufferObject = 0;
// #################### ^^^ Globals ^^^ ####################



// #################### vvv Error handling routines vvv ####################
static void GLClearAllErrors()
{
  while(glGetError() != GL_NO_ERROR)
  {
    
  }
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line)
{
  while (GLenum error = glGetError())
  {
    std::cout << "OpenGL Error:" << error
	      << "\tLine: " << line
	      << "\tfunction: " << function << std::endl;
    return true;
  }

  return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);
// #################### ^^^ Error handling routines ^^^ ####################

/*
  Helper function to get OpenGL Version Information
*/
void GetOpenGLVersionInfo()
{
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

/*
  LoadShaderAsString takes a filepath as an argument and will read line by line a file and return a string that is meant to be compiled at runtime for a vertex, fragment, geometry, tesselation, or compute shader.
  e.g.
    LoadShaderAsString("./shaders/filepath");
  @param filename Path to the shader file
  @return Entire file stored as a single string
*/
std::string LoadShaderAsString(const std::string& filename)
{
  // Resulting shader program loaded as a single string
  std::string result = "";

  std::string line = "";
  std::ifstream myFile(filename.c_str());

  if(myFile.is_open())
  {
    while(std::getline(myFile, line))
    {
      result += line + '\n';
    }
    myFile.close();
  }
  return result;
}

/*
  CompileShader will compile any valid vertex, fragment, geometry, tesselation or compute shader.
  e.g.
    Compile a vertex shader: CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    Compile a fragment shader: CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  @param type We use the 'type' field to determine which shader we are going to compile
  @param source : The shader source code.
  @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source)
{
  // Compile our shaders
  GLuint shaderObject;

  // Based on the type passed in, we create a shader object specifically for that
  if (type == GL_VERTEX_SHADER)
  {
    shaderObject = glCreateShader(GL_VERTEX_SHADER);
  
  } 
  else if (type == GL_FRAGMENT_SHADER)
  {
    shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
  }

  const char* src = source.c_str();
  // The source of our shader
  glShaderSource(shaderObject, 1, &src, nullptr);
  // Now compile our shader
  glCompileShader(shaderObject);

  // Retrieve the result of our compilation
  int result;
  // Our goal with glGetShaderiv is to retrieve the compilation status
  glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

  if (result == GL_FALSE)
  {
    int length;
    glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
    char * errorMessages = new char[length]; // Could also use alloc here
    glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

    if (type == GL_VERTEX_SHADER)
    {
      std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
    }
    else if (type == GL_FRAGMENT_SHADER)
    {
      std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
    }

    // Reclaim our memory
    delete[] errorMessages;

    // Delete our broken shader
    glDeleteShader(shaderObject);

    return 0;
  }

  return shaderObject;
}

/*
  Creates a graphics program object (i.e. graphics pipeline) with a Vertex Shader and a Fragment Shader
  
  @param vertexShaderSource Vertex source code as a string
  @param fragmentShaderSource Fragment shader source code as a string
  @return id of the program Object
*/
GLuint CreateShaderProgram(const std::string& vertexShaderSource,
		           const std::string& fragmentShaderSource)
{
  // Create a new program object
  GLuint programObject = glCreateProgram();

  // Compile our shaders
  GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  // Link our two shader programs together
  // Consider this the equivalent of taking two .cpp files, and linking them into one executable file
  glAttachShader(programObject, myVertexShader);
  glAttachShader(programObject, myFragmentShader);
  glLinkProgram(programObject);

  // Validate our program
  glValidateProgram(programObject);

  // Once our final program Object has been created, we can detach and then delete our individual shaders
  glDetachShader(programObject,myVertexShader);
  glDetachShader(programObject, myFragmentShader);
  // Delete individual shaders once we are done
  glDeleteShader(myVertexShader);
  glDeleteShader(myFragmentShader);

  return programObject;
}

/*
  Create the graphics pipeline

  @return void
*/
void CreateGraphicsPipeline()
{
  std::string vertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
  std::string fragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");
  gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

/*
  @return void
*/
void VertexSpecification()
{
  // Geometry Data
  // Here we are going to store x, y and z position attributes within vertexPositions for the data.
  // For now, this information is just stored on the CPU, and we are going to store this data on the GPU shortly,
  // in  a call to glBufferData which will store this information into a vertex buffer object.
  // Note: The data has been segregated from the OpenGL calls which follow in this function
  // It is not strictly necessary, but the code is cleaner if OpenGL (GPU) related functions
  // are packed closer together versus CPU operations
  const std::vector<GLfloat> vertexData{
    // 0 - Vertex 
    -0.5f, -0.5f, 0.0f,  // bottom left left vertex position
     1.0f,  0.0f, 0.0f,  // color
    // 1 - Vertex
     0.5f, -0.5f, 0.0f,  // bottom right vertex position
     0.0f,  1.0f, 0.0f,  // color 
    // 2 - Vertex
    -0.5f,  0.5f, 0.0f,  // top left vertex position
     0.0f,  0.0f, 1.0f,   // color 
    // 3 - Vertex
     0.5f,  0.5f, 0.0f,  // top right
     0.0f,  1.0f, 0.0f,  // color 
  };

  // Vertex Arrays Object (VAO) Setup
  // Note: We can think of the VAO as a 'wrapper around' all of the Vertex Buffer Objects
  // in the sense that it encapsulates all VBO state that we are setting up. 
  // Thus, it is also important that we glBindVertexArray (i.e. select the VAO we want to use)
  // before our vertex buffer object operations.
  glGenVertexArrays(1, &gVertexArrayObject);
  // We bind (i.e. select) to the Vertex Array Object (VAO) that we want to work within.
  glBindVertexArray(gVertexArrayObject);

  // Start generating VBO
  // Vertex Buffer Object (VBO) creation
  // Create a new vertex buffer object
  // Note; We'll see this pattern of code often in OpenGL of creating and binding to a buffer
  glGenBuffers(1, &gVertexBufferObject);
  // Next we'll do glBindBuffer.
  // Bind is equivalent to 'selecting the active buffer object' that we want to work with
  glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
  // Now in our currently binded buffer, we populate the data from our 'vertexPositions' which is on the CPU
  // onto a buffer that will live on the GPU.

  glBufferData(GL_ARRAY_BUFFER, // Kind of buffer we are working with e.g. GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER
	       vertexData.size() * sizeof(GLfloat), // Size of data in bytes
	       vertexData.data(), // Raw array of data
	       GL_STATIC_DRAW // How we intend to use the data
	       );

  const std::vector<GLuint> indexBufferData {2, 0, 1, 3, 2, 1};

  // Setup the Index Buffer Object (IBO i.e. EBO)
  glGenBuffers(1, &gIndexBufferObject);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       indexBufferData.size() * sizeof(GLuint),
	       indexBufferData.data(),
	       GL_STATIC_DRAW
	       );

  // For our Given Vertex Array Object, we need to tell OpenGL 'how' the information in our buffer will be used.
  glEnableVertexAttribArray(0);
  // For the specific attribute in our vertex specification, we use
  // 'glVertexAttribPointer' to figure out how we are going to move through the data
  glVertexAttribPointer(0, // Attribute 0 corresponds to the enabled glEnableVertexAttribArray.
			   // In the future, we will also see that in our vertex shader this also correcpond to (layout=0) which selects these attributes
			3, // The number of components (e.g. x, y, z = 3 components)
			GL_FLOAT, // Type
			GL_FALSE, // Is the data normalized
			sizeof(GLfloat) * 6, // Stride
			(void*)0 // Offset
  );
  
  // VAO setup for color information
  // We follow the same pattern except we use a stride for colors
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
			3,
			GL_FLOAT,
			GL_FALSE,
			sizeof(GLfloat) * 6,
			(GLvoid*)(sizeof(GLfloat)*3)
  );

  // Unbind our currently bound Vertex Array Object
  glBindVertexArray(0);
  // Disable any attributes we opened in our Vertex Attribute Array,
  // as we do not want to leave them open
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

/*
  Initialization of the graphics application. Typically this will involve setting up a window
  and the OpenGL Context (with the appropriate version)
  
  @return void
*/
void InitializeProgram()
{
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    std::cout << "SDL2 could not initialize video subsystem" << std::endl;
    exit(1);
  }
  // Setup the OpenGL Context
  // Use OpenGL 4.1 core or greater
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  
  // We want to request a double buffer for smooth updating
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 2);

  // Create an application window using OpenGL that supports SDL
  gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL Window", 0, 0, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);

  // Check if Window did not create
  if (gGraphicsApplicationWindow == nullptr)
  {
    std::cout << "SDL Window was not able to be created" << std::endl;
    exit(1);
  }

  // Create an OpenGL Graphics Context
  gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);

  if (gOpenGLContext == nullptr)
  {
    std::cout << "OpenGL context could not be created" << std::endl;
    exit(1);
  }

  // Initialize the Glad Library
  if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
  {
    std::cout << "glad was not initialized" << std::endl;
    exit(1);
  }

  GetOpenGLVersionInfo();
}

/*
  // Function called in the main application loop to handle user input
  
  @return void
*/
void Input()
{
  // Event handler that handles various events in SDL
  // that are related to input and output
  SDL_Event e;
  // Handle events on queue
  while(SDL_PollEvent(&e) != 0)
  {
    // If user posts an event to quit
    // An example is hitting the "x" in the corner of the window
    if (e.type == SDL_QUIT)
    {
      std::cout << "Goodbye!" << std::endl;
      gQuit = true;
    }
  }
  
}

/*
  PreDraw
  Typically we will use this for setting some sort of 'state'
  Note: some of the calls may take place at different stages (post-processing) of the pipeline
  @return void
*/
void PreDraw()
{
  // Disable depth test and face culling.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Initialize clear color
  // This is the background of the screen
  glViewport(0, 0, gScreenWidth, gScreenHeight);
  glClearColor(.03f, .05f, 0.27f, 1.f);

  // Clear color buffer and depth buffer
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  
  // Use our shader
  glUseProgram(gGraphicsPipelineShaderProgram);
  
}

/*
  Draw
  The render function gets called once per loop
  Typically this includes glDraw related calls, and the relevant setup of buffers for those calls

  @return void
*/
void Draw()
{
  // Enable our attributes
  glBindVertexArray(gVertexArrayObject);

  // Select the vertex buffer object we want to enable
  glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

  // Render data
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  // Stop using our current graphics pipeline
  // Note: This is not necessary if we only have one graphics pipeline.
  glUseProgram(0);
  
}

/*
  Main Application Loop
  This is an infinite loop
  
  @return void
*/
void MainLoop()
{
  // While application is running
  while (!gQuit)
  {
    // Handle input 
    Input();
    // Setup anything that needs to take place before draw calls 
    PreDraw();
    // Draw calls in OpenGL
    Draw();
    // Update screen of our specified window
    SDL_GL_SwapWindow(gGraphicsApplicationWindow);
  }
}

void CleanUp()
{
  SDL_DestroyWindow(gGraphicsApplicationWindow);
  SDL_Quit();
}

/*
  The entry point into a program
  @return program status
*/
int main( int argc, char* args[] )
{

  // 1. Setup the graphics program
  InitializeProgram();

  // 2. Setup the geometry
  VertexSpecification();

  // 3. Create our graphics pipeline
  // - At a minimum, this means the vertex and fragment shader
  CreateGraphicsPipeline();

  // 4. Call the main application loop
  MainLoop();

  // 5. Call the cleanup funcion when our program terminates
  CleanUp();

  return 0;
  
}
