/*  exj_4.cpp
 * James A. Feister - thegreatpissant@gmail.com
 * DONE - Break out different model types.
 * DONE - Add a simple render system, yes it is very simple
 * DONE - Add an actor a subclass of an entity
 * PROOF - Use std library to load shaders
 * PROOF - Rendering function in renderer only
 * PROOF - Independent model movement
 * PROOF - very simple scene graph of entities to render
 * TODO - Move the shaders out of here
 * TODO - Move any other OpenGL stuff out of here.
 * Proposed - Physics engine
 * Proposed - Selection
 * Proposed - Display class, Oculus and traditional
 * Proposed - Fix input system to be more fluent
 */

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <cstdlib>

using namespace std;

//  OpenGL
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

// 3rd Party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//  Engine parts
#include "common/Shader.h"
#include "common/Render.h"
#include "common/Model.h"
#include "common/Display.h"
#include "common/Actor.h"
#include "common/Camera.h"

enum class queue_events {
    STRAFE_LEFT,
    STRAFE_RIGHT,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    YAW_LEFT,
    YAW_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    PITCH_UP,
    PITCH_DOWN,
    COLOR_CHANGE,
    MODEL_CHANGE,
    APPLICATION_QUIT
};
queue<queue_events> gqueue;
Display display;
Renderer renderer;
Renderer renderer1;
shared_ptr<Camera> camera;
shared_ptr<Entity> selected;
vector<shared_ptr<Actor>> scene_graph;
//  Constants and Vars
//  @@TODO Should move into a variable system
glm::mat4 Projection;
glm::mat4 MVP;
glm::mat4 camera_matrix;
glm::mat4 model_matrix;
GLint MVP_loc = 0;
GLint camera_loc = 0;
GLint model_matrix_loc = 0;

//  Shader programs
GLuint program, program2;

//  Function Declarations
void Init( );
void UpdateView( );
void PostView( );
void GlutIdle( );
void GlutReshape( int newWidth, int newHeight );
void GlutDisplay( void );
void GlutKeyboard( unsigned char key, int x, int y );
void UpdatePerspective( );
void CleanupAndExit( );
//  Models
void GenerateModels( );
//  Entities
void GenerateEntities( );
//  View items
void InitializeView( );
//  Shaders
void GenerateShaders( );

//  Globalized user vars
GLfloat strafe{ 1.0f }, height{ 0.0f }, depth{ -25.0f }, rotate{ 0.0f };
GLint color = 1;
GLint color_loc = 0;

float dir = 1.0f;
float xpos = 2.0f;
float ypos = 0.0f;

// MAIN //
int main( int argc, char **argv ) {
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
    glutInitWindowSize( display.screen_width, display.screen_height );

    glutCreateWindow( argv[0] );
    if ( glewInit( ) ) {
        cerr << "Unable to initialize GLEW ... exiting " << endl;
        exit( EXIT_FAILURE );
    }

    //  Initialize common systems
    renderer.init( );

    //  Load our Application Items
    GenerateModels( );
    GenerateEntities( );
    GenerateShaders( );

    //  Boiler Plate
    glutIdleFunc( GlutIdle );
    glutReshapeFunc( GlutReshape );
    glutDisplayFunc( GlutDisplay );
    glutKeyboardFunc( GlutKeyboard );

    //  Go forth and loop
    glutMainLoop( );
}

void GenerateShaders( ) {
    //  Shaders
    ShaderInfo shaders[] = { { GL_VERTEX_SHADER, "../shaders/exj_4_1.v.glsl" },
                             { GL_FRAGMENT_SHADER, "../shaders/exj_4_1.f.glsl" },
                             { GL_NONE, NULL } };

    program = LoadShaders( shaders );
    glUseProgram( program );
    if ( ( color_loc = glGetUniformLocation( program, "color" ) ) == -1 ) {
        cerr << "Did not find the color loc\n";
    }

    if ( ( MVP_loc = glGetUniformLocation( program, "mMVP" ) ) == -1 ) {
        cerr << "Did not find the mMVP loc\n";
    }
    if ( ( camera_loc = glGetUniformLocation( program, "mCamera" ) ) == -1 ) {
        cerr << "Did not find the mCamera loc\n";
    }
    if ( ( model_matrix_loc =
           glGetUniformLocation( program, "model_matrix" ) ) == -1 ) {
        cerr << "Did not find the model_matrix loc\n";
    }

    glUniform1i( color_loc, color );
    glUseProgram( 0 );
}

void InitializeView( ) {
    UpdateView( );
    PostView( );
}

void GlutReshape( int newWidth, int newHeight ) {
    display.screen_width = newWidth;
    display.screen_height = newHeight;
    UpdatePerspective( );
    glutPostRedisplay( );
}

void GlutDisplay( void ) { renderer.render( scene_graph ); }

void GlutKeyboard( unsigned char key, int x, int y ) {
    switch ( key ) {
    default:
        break;
    case 27:
    case 'q':
    case 'Q':
        gqueue.push( queue_events::APPLICATION_QUIT );
        break;
    case 'h':
    case 'H':
        gqueue.push( queue_events::YAW_LEFT );
        break;
    case 'l':
    case 'L':
        gqueue.push( queue_events::YAW_RIGHT );
        break;
    case 'a':
    case 'A':
        gqueue.push( queue_events::STRAFE_LEFT );
        break;
    case 'd':
    case 'D':
        gqueue.push( queue_events::STRAFE_RIGHT );
        break;
    case 's':
    case 'S':
        gqueue.push( queue_events::MOVE_BACKWARD );
        break;
    case 'w':
    case 'W':
        gqueue.push( queue_events::MOVE_FORWARD );
        break;
    case 'k':
    case 'K':
        gqueue.push( queue_events::MOVE_UP );
        break;
    case '-':
        gqueue.push( queue_events::PITCH_UP );
        break;
    case '+':
        gqueue.push( queue_events::PITCH_DOWN );
        break;
    case 'j':
    case 'J':
        gqueue.push( queue_events::MOVE_DOWN );
        break;
    case 'c':
    case 'C':
        gqueue.push( queue_events::COLOR_CHANGE );
        break;
    case 'm':
    case 'M':
        gqueue.push( queue_events::MODEL_CHANGE );
        break;
    }
}

void GlutIdle( ) {
    //  Pump the events loop
    while ( !gqueue.empty( ) ) {
        switch ( gqueue.front( ) ) {
        case queue_events::MOVE_FORWARD:
            selected->state.position_z += 1.0f;
            break;
        case queue_events::MOVE_BACKWARD:
            selected->state.position_z -= 1.0f;
            break;
        case queue_events::STRAFE_RIGHT:
            selected->state.position_x += 1.0f;
            break;
        case queue_events::STRAFE_LEFT:
            selected->state.position_x -= 1.0f;
            break;
        case queue_events::YAW_RIGHT:
            selected->state.orientation_y += 0.5f;
            break;
        case queue_events::YAW_LEFT:
            selected->state.orientation_y -= 0.5f;
            break;
        case queue_events::MOVE_UP:
            selected->state.position_y += 0.5f;
            break;
        case queue_events::MOVE_DOWN:
            selected->state.position_y -= 0.5f;
            break;
        case queue_events::PITCH_UP:
            selected->state.orientation_x += 0.5f;
            break;
        case queue_events::PITCH_DOWN:
            selected->state.orientation_x += -0.5f;
            break;
        case queue_events::COLOR_CHANGE:
            color = ( color >= 4 ? 1 : color + 1 );
            break;
        case queue_events::MODEL_CHANGE:
            break;
        case queue_events::APPLICATION_QUIT:
            CleanupAndExit( );
        }
        gqueue.pop( );
    }
    for ( auto i = 0; i < scene_graph.size( ); i++ )
    {
        (*scene_graph[i]).state.orientation_z += ((i%2) == 0)? 0.5f: -0.5f;
    }

    ypos = xpos * dir;
    if (xpos > 400) {
        dir = -1.0f;
    } else if ( xpos < -400) {
        dir = 1.0f;
    }
    UpdateView( );
    PostView( );
    glutPostRedisplay( );
}

void CleanupAndExit( ) 
{ 
    exit( EXIT_SUCCESS );
}

void GenerateModels( ) {

    shared_ptr<Simple_equation_model_t> tmp =
            shared_ptr<Simple_equation_model_t>{ new Simple_equation_model_t };
    int ext = 0;
    tmp->numVertices = 100;
    tmp->vertices.resize( tmp->numVertices * 3 );
    float x = 0.0f, z = 0.0f;
    for ( int i = 0; i < tmp->numVertices; i++, x += 0.01f, z += 0.05f ) {
        tmp->vertices[i * 3] = x;
        tmp->vertices[i * 3 + 1] = powf( x, 1 );
        tmp->vertices[i * 3 + 2] = 0.0f; //z;
        if ( z >= -1.0f )
            z = -10.0f; //(i % 2 ? -8.0f:-12.0f);
    }
    tmp->name = "ex15_" + to_string( ext++);
    tmp->renderPrimitive = GL_POINTS;
    tmp->setup_render_model( );
    renderer.add_model( tmp );

    for ( auto power_to :
    { 1.0f, 1.2f, 1.4f, 1.6f, 1.8f, 2.1f, 2.2f, 2.3f, 3.5f, 4.0f } ) {
        tmp =
                shared_ptr<Simple_equation_model_t>{ new Simple_equation_model_t };
        x = 0.0f;
        z = 0.0f;
        tmp->numVertices = 600;
        tmp->vertices.resize( tmp->numVertices * 3 );
        for ( int i = 0; i < tmp->numVertices; i++, x += 0.1f, z += 0.05f ) {
            tmp->vertices[i * 3] = x;
            tmp->vertices[i * 3 + 1] = powf( x, power_to );
            tmp->vertices[i * 3 + 2] = 0.0f; // z;
            if ( z >= -1.0f )
                z = 0.0f;
        }
        tmp->name = "ex15_" + to_string( ext++ );
        tmp->renderPrimitive = GL_POINTS;
        tmp->setup_render_model( );
        renderer.add_model( tmp );
    }

}

void GenerateEntities( ) {

    //  Camera
    camera = shared_ptr<Camera>{ new Camera( strafe, height, depth, 0.0f, 0.0f,
                                             0.0f ) };

    //  Actors
    GLfloat a = 0.0f;
    for ( int i = 0; i < 1; i++, a += 0.2f ) {
        scene_graph.push_back( shared_ptr<Actor>{ new Actor(
                                                  -5.0f, 0.0f, a, 0.0f, 0.0f, 0.0f, 6 ) } );
    }
    a = 0.0f;
    for ( int i = 0; i < 1000; i++, a += 0.2f ) {
        scene_graph.push_back( shared_ptr<Actor>{ new Actor(
                                                  -0.0f, 0.0f, a, 0.0f, 0.0f, 0.0f, 3 ) } );
    }
    a = 0.0f;
    for ( int i = 0; i < 1000; i++, a += 0.2f ) {
        scene_graph.push_back( shared_ptr<Actor>{ new Actor(
                                                  5.0f, 0.0f, a, 0.0f, 0.0f, 0.0f, 7 ) } );
    }
    a = 0.0f;
    for ( int i = 0; i < 1000; i++, a += 0.2f ) {
        scene_graph.push_back( shared_ptr<Actor>{ new Actor(
                                                  10.0f, 0.0f, a, 0.0f, 0.0f, 0.0f, 8 ) } );
    }

    //  Selected Entity
    selected = camera;
    //    selected = scene_graph[0];
}

void UpdateView( )
{
    glm::mat4 r_matrix =
            glm::rotate( glm::mat4 (), camera->state.orientation_x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    r_matrix =
            glm::rotate( r_matrix, camera->state.orientation_y, glm::vec3( 0.0f, 1.0f, 0.0f ) );
    r_matrix =
            glm::rotate( r_matrix, camera->state.orientation_z, glm::vec3( 0.0f, 0.0f, 1.0f ) );
    glm::vec4 cr = r_matrix * glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    glm::vec3 c_pos = glm::vec3( camera->state.position_x, camera->state.position_y, camera->state.position_z );
    camera_matrix = glm::lookAt(
                c_pos,
                c_pos + glm::vec3( cr.x, cr.y, cr.z ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
}

void PostView( ) {
    glUseProgram( program );
    glUniformMatrix4fv( MVP_loc, 1, GL_FALSE, &MVP[0][0] );
    glUniformMatrix4fv( camera_loc, 1, GL_FALSE, &camera_matrix[0][0] );
    glUniform1i( color_loc, color );
    glUseProgram( 0 );
}

void UpdatePerspective( ) {
    // GLfloat hResolution = display.screen_width;  //  640.0f;
    // GLfloat vResolution = display.screen_height; //  800.0f;
    // GLfloat eyeScreenDist = 0.041f;
    // GLfloat aspect = hResolution / (2.0f * vResolution);
    // GLfloat fov = 2.0f*(atan(0.0935f/(2.0f*eyeScreenDist)));
    // GLfloat zNear  = 0.3f;
    // GLfloat zFar   = 1000.0f;
    // Projection = glm::perspective( fov, aspect, zNear, zFar );
    MVP = glm::perspective( 45.0f, 4.0f / 3.0f, 1.0f, 1000.0f );
}
