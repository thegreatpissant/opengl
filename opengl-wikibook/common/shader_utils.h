#ifndef __SHADER_UTILS_H__
#define __SHADER_UTILS_H__
#include <GL/glew.h>
#include <GL/glut.h>
void print_log (GLuint object);
GLuint create_shader (const char *filename, GLenum type);
#endif
