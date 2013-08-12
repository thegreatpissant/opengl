/*  exj_3.cpp
 *  James A. Feister - thegreatpissant@gmail.com
 *  Break out different model types.
 *  Add a simple render system
 *  Add an actor
 */

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <unistd.h>
using namespace std;

//  OpenGL
#include <GL/glew.h>
#include <GL/freeglut.h>

// 3rd Party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//  Engine parts
#include "common/shader_utils.h"
#include "Render.h"
#include "Model.h"
#include "Display.h"

enum class queue_events {
  STRAFE_LEFT, STRAFE_RIGHT, MOVE_FORWARD, MOVE_BACKWARD, ROTATE_LEFT, ROTATE_RIGHT, 
    COLOR_CHANGE,
    APPLICATION_QUIT
    };
queue <queue_events> gqueue;
render Renderer;
//  Constants and Vars  
//  @@TODO Should move into a variable system
Display screen0;
int screenWidth = screen0.screenWidth;
int screenHeight = screen0.screenHeight;
glm::mat4 Projection;
glm::mat4 MVP = glm::perspective(45.0f, 4.0f / 3.0f, 1.0f, 100.f);
glm::mat4 camera = glm::mat4(0.0);
GLint MVP_loc = 0;
GLint camera_loc = 0;

//  Function Declarations
void Init ();
void UpdateView ();
void PostView ();
void GlutIdle ();
void GlutReshape ( int newWidth, int newHeight );
void GlutDisplay ( void );
void GlutKeyboard ( unsigned char key, int x, int y );
void UpdatePerspective ();
void CleanupAndExit ();

//  Globalized user vars
GLfloat strafe = 1.0f, height = 0.0f, depth = -3.0f, rotate = 0.0f;
GLint color = 1;
GLint color_loc = 0;

//  Models
simple_equation_model_t ex15_1;
simple_equation_model_t ex15_2;

void GenerateModels ();

int main ( int argc, char ** argv) {
  glutInit ( &argc, argv );
  glutInitDisplayMode ( GLUT_DOUBLE | GLUT_RGBA );
  glutInitWindowSize ( screenWidth, screenHeight );

  glutCreateWindow (argv[0]);
  if ( glewInit() ) {
    std::cerr << "Unable to initialize GLEW ... exiting " << std::endl;
    exit ( EXIT_FAILURE );
  }
  
  Init ();

  glutIdleFunc ( GlutIdle );
  glutReshapeFunc ( GlutReshape );
  glutDisplayFunc ( GlutDisplay );
  glutKeyboardFunc ( GlutKeyboard );
  
  glutMainLoop ();

}

void Init () {
  //  Models
  GenerateModels ();

  //  Shaders
  ShaderInfo shaders [] = {
    { GL_VERTEX_SHADER, "./shaders/ex15_1.v.glsl" },
    { GL_FRAGMENT_SHADER, "./shaders/ex15_1.f.glsl" },
    { GL_NONE, NULL }
  };
  GLuint program = LoadShaders(shaders);

  glUseProgram(program);
  if ( (color_loc = glGetUniformLocation ( program, "color" )) == -1 ) {
    std::cout << "Did not find the color loc\n";
  }
  if ( (MVP_loc = glGetUniformLocation (program, "mMVP" )) == -1 ) {
    std:: cout << "Did not find the mMVP loc\n";
  }
  if ( (camera_loc = glGetUniformLocation (program, "mCamera" )) == -1 ) {
    std:: cout << "Did not find the mCamera loc\n";
  }
  glUniform1i ( color_loc, color );

  //  View
  glClearColor ( 0.0, 0.0, 0.0, 1.0 );

  UpdateView ();
  glutPostRedisplay ();

}

void GlutReshape ( int newWidth, int newHeight ) {
  screenWidth = newWidth;
  screenHeight = newHeight;
  UpdatePerspective ();
  glutPostRedisplay ();
}

void GlutDisplay ( void ) {
  glClear (GL_COLOR_BUFFER_BIT);
  glViewport ( 0, 0, screenWidth, screenHeight );
  //
  //  Eventually Call on rederer to render Entities
  //
  Renderer.Render ();
  
  glBindVertexArray (0);
  glFinish ();
  glutSwapBuffers ();
}

void GlutKeyboard ( unsigned char key, int x, int y ) {
  switch ( key ) {
  default:
    break;
  case 27:
  case 'q':
  case 'Q':
    gqueue.push ( queue_events::APPLICATION_QUIT );
    break;
  case '-':
  case '_':
    gqueue.push ( queue_events::ROTATE_LEFT );
    break;
  case '+':
  case '=':
    gqueue.push ( queue_events::ROTATE_RIGHT );
    break;
  case 'a':
  case 'A':
    gqueue.push ( queue_events::STRAFE_LEFT );
    break;
  case 's':
  case 'S':
    gqueue.push ( queue_events::MOVE_BACKWARD );
    break;
  case 'd':
  case 'D':
    gqueue.push ( queue_events::STRAFE_RIGHT );
    break;
  case 'w':
  case 'W':
    gqueue.push ( queue_events::MOVE_FORWARD );
    break;
  case 'c':
  case 'C':
    gqueue.push ( queue_events::COLOR_CHANGE );
    break;
  }
}

void UpdateView () {
  //  camera = glm::rotate (glm::mat4(1.0f), rotate, glm::vec3 (0.0f, 1.0f, 0.0f) );
  camera = glm::translate (glm::mat4(), glm::vec3 ( strafe, height, depth ) );
  camera = glm::rotate (camera, rotate, glm::vec3 (0.0f, 1.0f, 0.0f) );
}

void PostView() {
  glUniformMatrix4fv( MVP_loc, 1, GL_FALSE, &MVP[0][0] ); 
  glUniformMatrix4fv( camera_loc, 1, GL_FALSE, &camera[0][0] ); 
  glUniform1i ( color_loc, color );
}


void UpdatePerspective () {
  //  @@TODO Adjustable FOV and Aspect ration
  GLfloat hResolution = screenWidth;  //  640.0f;
  GLfloat vResolution = screenHeight; //  800.0f;
  //  GLfloat hScreenSize = 0.14976f;
  //  GLfloat vScreenSize = 0.0935f;
  GLfloat eyeScreenDist = 0.041f;
  GLfloat aspect = hResolution / (2.0f * vResolution);
  GLfloat fov = 2.0f*(atan(0.0935f/(2.0f*eyeScreenDist)));
  GLfloat zNear  = 0.3f;
  GLfloat zFar   = 1000.0f;
  Projection = glm::perspective( fov, aspect, zNear, zFar );
}

void GlutIdle () {
  //  Pump the events loop
  while ( !gqueue.empty () ) {
    switch ( gqueue.front() ) {
    case queue_events::MOVE_FORWARD :
      depth += 1.0f;
      break;
    case queue_events::MOVE_BACKWARD :
      depth -= 1.0f;
      break;
    case queue_events::STRAFE_RIGHT :
      strafe += 1.0f;
      break;
    case queue_events::STRAFE_LEFT :
      strafe -= 1.0f;
      break;
    case queue_events::ROTATE_RIGHT :
      rotate += 0.5f;
      break;
    case queue_events::ROTATE_LEFT :
      rotate -= 0.5f;
      break;
    case queue_events::COLOR_CHANGE :
      color = (color >= 4 ? 1 : color + 1 );
      break;
    case queue_events::APPLICATION_QUIT :
      CleanupAndExit ();
    }
    gqueue.pop ();
  }
  UpdateView ();
  PostView ();
  glutPostRedisplay ();
  //usleep ( 1000000 );
}

void CleanupAndExit () {
  exit ( EXIT_SUCCESS );
}


void GenerateModels () {
  shared_ptr <simple_equation_model_t> tmp = shared_ptr <simple_equation_model_t> {new simple_equation_model_t};
  
  tmp->numVertices = 600;
  tmp->vertices.resize(tmp->numVertices*3);
  float x = 0.0f, z = 0.0f;
  for (int i = 0; i < tmp->numVertices; i++, x+= 0.01f, z += 0.05f) {
    tmp->vertices[i*3] = x;
    tmp->vertices[i*3 + 1]= powf(x,1);
    tmp->vertices[i*3 + 2] = z;
    if ( z >= -1.0f)
      z = -10.0f; //(i % 2 ? -8.0f:-12.0f);
  }
  tmp->name = "ex15_1";
  tmp->renderPrimitive = GL_POINTS;
  tmp->SetupRenderModel ();
  //  Renderer.models.push_back(tmp);

  for (auto power_to : {1.0f, 1.2f, 1.4f, 1.6f, 1.8f} ) {
    static int ext;
    ext++;
    tmp = shared_ptr <simple_equation_model_t> { new simple_equation_model_t };
    x = -3.0f;
    z = 0.0f;
    tmp->numVertices = 600;
    tmp->vertices.resize(tmp->numVertices*3);
    for (int i = 0; i < tmp->numVertices; i++, x+= 0.01f, z+= 0.05f) {
      tmp->vertices[i*3] = x;
      tmp->vertices[i*3 + 1]= powf(x,power_to);
      tmp->vertices[i*3 + 2] = 0.0f; //z;
      if ( z >= -1.0f) 
	z = 0.0f;
    }
    tmp->name = "ex15_" + to_string(ext);
    cout << "name: " << tmp->name << endl;
    tmp->renderPrimitive = GL_POINTS;
    tmp->SetupRenderModel ();
    Renderer.models.push_back(tmp);
  }
}
