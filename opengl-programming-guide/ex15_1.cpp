//////////////////////////////////////////////////////////////////////
//
//  ex15_1.cpp
//
//////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <math.h>
//using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "common/Shader.h"

//  Models
struct model_t {
  long   numVertices;
  std::vector <float> vertices;
  std::vector <GLuint> vaos;
  std::vector <GLuint> buffers;
  int    renderPrimitive;
  void Render () {
    glBindVertexArray(vaos[0]);
    glBindBuffer( GL_ARRAY_BUFFER, buffers[0]);
    glDrawArrays(renderPrimitive, 0, numVertices);
  } 
};

struct model_t ex15_1;
struct model_t ex15_2;
struct model_t ex15_3;
struct model_t ex15_4;
struct model_t ex15_5;

enum Attrib_IDs { vPosition = 0 };

GLfloat rotate = 0.0f;
GLint color = 1;
GLint color_loc = 0;
GLfloat hResolution = 640.0f;
GLfloat vResolution = 800.0f;
GLfloat hScreenSize = 0.14976f;
GLfloat vScreenSize = 0.0935f;
GLfloat eyeScreenDist = 0.041f;
GLfloat aspect = hResolution / (2.0f * vResolution);
GLfloat fov = 2.0f*(atan(0.0935f/(2.0f*eyeScreenDist)));
GLfloat zNear  = 0.3f;
GLfloat zFar   = 1000.0f;
//glm::mat4 MVP = glm::perspective( fov, aspect, zNear, zFar );
glm::mat4 MVP = glm::perspective(45.0f, 4.0f / 3.0f, 1.0f, 100.f);
glm::mat4 camera = glm::mat4(0.0);
GLint MVP_loc = 0;
GLint camera_loc = 0;

GLfloat depth = -3.0f;
GLfloat height = 0.0f;
GLfloat strafe = 0.0f;
GLsizei deviceWidth = 1280;
GLsizei deviceHeight = 800;
GLsizei screenWidth = 1280;
GLsizei screenHeight = 800;

void ExitOnGLError ( const char * );
#define BUFFER_OFFSET(offset)  ((void *)(offset))

void Init ();
void UpdateView ();
void DrawGrid ();
void GenerateModels ();
void IdleFunction ();
void MouseFunction (int, int, int, int);
void GlutKeyboardFunc (unsigned char key, int x, int y )
{
  switch (key) {
  case 27:
  case 'q':
  case 'Q':
    exit (0);
  case 'w':
  case 'W':
    depth += 0.10f;
    std::cout << "depth= " << depth << std::endl;
    break;
  case 's':
  case 'S':
    depth -= 0.10f;
    std::cout << "depth= " << depth << std::endl;
    break;
  case 'a':
  case 'A':
    strafe -= 0.10f;
  break;
  case 'd':
  case 'D':
    strafe += 0.10f;
  break;
  case ' ':
    height += 0.10f;
  break;
  case 'c':
    height -= 0.10f;
  break;
  case 'r':
  case 'R':
    color = 1;
    break;
  case 'g':
  case 'G':
    color = 2;
    break;
  case 'b':
  case 'B':
    color = 3;
    break;
  case 'n':
  case 'N':
    color = -1;
    break;
  case 'f':
  case 'F':
    glutFullScreenToggle ();
    break;
  case '+':
  case '=':
    rotate += 0.5f;
    break;
  case '-':
    rotate -= 0.5f;
    break;
  }
  glUniform1i ( color_loc, color );
  UpdateView();
  glutPostRedisplay ();
}
/*
  Display
*/
void UpdateView () {
  //  camera = glm::rotate (glm::mat4(1.0f), rotate, glm::vec3 (0.0f, 1.0f, 0.0f) );
  camera = glm::translate (glm::mat4(), glm::vec3 ( strafe, height, depth ) );
  camera = glm::rotate (camera, rotate, glm::vec3 (0.0f, 1.0f, 0.0f) );
}
void PostView() {
  glUniformMatrix4fv( MVP_loc, 1, GL_FALSE, &MVP[0][0] ); 
  glUniformMatrix4fv( camera_loc, 1, GL_FALSE, &camera[0][0] ); 
}

void Display(void)
{
  glClear(GL_COLOR_BUFFER_BIT);

//  Left Side
  glViewport (0,0, screenWidth, screenHeight);  
  PostView();
  ex15_1.Render ();
  ex15_2.Render ();	

  glBindVertexArray (0);
  glFinish ();
  glutSwapBuffers ();
}

void Reshape (int newWidth, int newHeight) {
  screenWidth = newWidth;
  screenHeight = newHeight;
  UpdateView ();
  glutPostRedisplay ();
}

/*
  Main
*/
int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(screenWidth,screenHeight);
  //  glutInitContextVersion(4, 3);
  //  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutCreateWindow(argv[0]);
  if (glewInit()) {
    std::cerr << "Unable to initialize GLEW ... exiting" << std::endl;
    exit(EXIT_FAILURE);
  }
  Init();
  //  glutIdleFunc     (IdleFunction);
  glutMouseFunc    (MouseFunction);
  glutDisplayFunc  (Display);
  glutReshapeFunc  (Reshape);
  glutKeyboardFunc (GlutKeyboardFunc);
  glutMainLoop();
}

void Init(void)
{
  //  Models
  GenerateModels ();

  //  Shaders
  ShaderInfo  shaders[] = {
    { GL_VERTEX_SHADER, "../shaders/ex15_1.v.glsl" },
    { GL_FRAGMENT_SHADER, "../shaders/ex15_1.f.glsl" },
    { GL_NONE, NULL }
  };
  GLuint program = LoadShaders(shaders);
  glUseProgram(program);
  if ( (color_loc = glGetUniformLocation ( program, "color" )) == -1 ) {
    std::cout << "Did not find the color loc\n";
  }
  if ( (MVP_loc = glGetUniformLocation (program, "mMVP" )) == -1 ) {
    std::cout << "Did not find the mMVP loc\n";
  }
  if ( (camera_loc = glGetUniformLocation (program, "mCamera" )) == -1 ) {
    std::cout << "Did not find the mCamera loc\n";
  }
  glUniform1i ( color_loc, color );

  //  View
  glClearColor ( 0.0, 0.0, 0.0, 1.0 );

  UpdateView ();
  glutPostRedisplay ();

}

void GenerateModels () {
  float x = 0.0f, z = 0.0f;;
  ex15_1.numVertices = 600;
  ex15_1.vertices.resize(ex15_1.numVertices*3);
  for (int i = 0; i < ex15_1.numVertices; i++, x+= 0.01f, z += 0.05f) {
    ex15_1.vertices[i*3] = x;
    ex15_1.vertices[i*3 + 1]= powf(x,1);
    ex15_1.vertices[i*3 + 2] = z;
    if ( z >= -1.0f)
      z = -10.0f; //(i % 2 ? -8.0f:-12.0f);
  }

  x = -3.0f;
  z = 0.0f;
  ex15_2.numVertices = 600;
  ex15_2.vertices.resize(ex15_2.numVertices*3);
  for (int i = 0; i < ex15_2.numVertices; i++, x+= 0.01f, z+= 0.05f) {
    ex15_2.vertices[i*3] = x;
    ex15_2.vertices[i*3 + 1]= powf(x,3);
    ex15_2.vertices[i*3 + 2] = 0.0f; //z;
    if ( z >= -1.0f) 
       z = 0.0f;
  }

  ex15_1.vaos.resize(1);
  glGenVertexArrays( ex15_1.vaos.size(), &ex15_1.vaos[0] );
  if ( ex15_1.vaos[0] == 0 ) {
    std::cerr << "ex15_1: Did not get a valid Vertex Attribute Object" << std::endl;
  } 
  glBindVertexArray( ex15_1.vaos[0] );
  ex15_1.buffers.resize(1);
  glGenBuffers(ex15_1.buffers.size(), &ex15_1.buffers[0]);
  glBindBuffer(GL_ARRAY_BUFFER, ex15_1.buffers[0]);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*ex15_1.vertices.size(), &ex15_1.vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition);
  glBindVertexArray (0);
  ex15_1.renderPrimitive = GL_POINTS;

  ex15_2.vaos.resize(1);
  glGenVertexArrays( ex15_2.vaos.size(), &ex15_2.vaos[0] );
  if ( ex15_2.vaos[0] == 0 ) {
    std::cerr << "ex15_2: Did not get a valid Vertex Attribute Object" << std::endl;
  } 
  glBindVertexArray( ex15_2.vaos[0] );
  ex15_2.buffers.resize(1);
  glGenBuffers(ex15_2.buffers.size(), &ex15_2.buffers[0]);
  glBindBuffer(GL_ARRAY_BUFFER, ex15_2.buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*ex15_2.vertices.size(), &ex15_2.vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition);
  glBindVertexArray (0);
  ex15_2.renderPrimitive = GL_POINTS;
}

void DrawGrid () {
}

void ExitOnGLError ( const char * error_message ) {
  std::cout << error_message << std::endl;
}

void IdleFunction () {
  //  glutPostRedisplay ();
}
void MouseFunction (int x, int y, int j, int k) {
  //  glutPostRedisplay ();
}
