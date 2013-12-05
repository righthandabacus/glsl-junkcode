/* GLSL for general purpose computing
 * Adrian S-W Tam
 * Mon, 02 Dec 2013 11:28:27 -0500
 *
 * This code checks the texture dimension
 */

#include "glsl_utils.h"
#include <string.h>
#include <time.h>

#ifndef GL_VERSION_2_0
#error This program needs OpenGL 2.0 libraries to compile
#endif

int main(int argc, char **argv)
{	
	int maxTexSize;

	initGlut(&argc, argv);

	/* Get texture dimension, usually 4096. Note that the card may not able to
	 * handle 4096*4096*4096 texture if graphic memory is the constraint */
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	printf("GL_MAX_TEXTURE_SIZE = %d\n", maxTexSize);
}

/* vim:set noet syntax=c sw=4 ts=4 bs=indent,eol,start: */
