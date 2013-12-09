/* Test script of OpenGL Shader Language for General Purpose Computing
 *
 * Code adopted from the tutorial:
 *   http://www.mathematik.tu-dortmund.de/~goeddeke/gpgpu/tutorial2.html
 *
 * This code allocates 2^k x 2^k floats of random value and find the maximum of
 * them using GPGPU parallelization.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "glsl_utils.h"

extern int usePBO;

int main(int argc, char **argv) {
    /* command line parameters */
    int k;                          // Exponent
    int size;                       // 2^k
    /* application variables */
    float*data;                     // data
    GLuint fb, prog;                // FBO handle and program handle
    GLuint tex[2];                  // textures handle
    int writePos=0;                 // ping-pong variable
    GLint texParam, deltaParam;     // connection to params in GLSL

    usePBO = 1;

    /* parse command line ***********/
    if (argc < 2) {
        printf("Command line parameters:\n");
        printf("Param 1: exponent k\n");
        exit(0);
    } else {
        k = atoi(argv[1]);
        size = 0x01 << k;
        printf("k=%d, 2^k=%d\n", k, size);
    }

    /* setup parameters *************/
    data = (float*)malloc(size*size*sizeof(float));
    srand(0);
    for (int i=0; i<size*size; i++) {
        data[i] = rand() / ((float)rand()+1.0); // tons of floats
    }

    /* print out data ***************/
    float expected = data[0];
    for (int i=0; i<size*size; i++) {
        printf("%.3f",data[i]);
        printf(((1+i) % size)?"\t":"\n");
        if (data[i] > expected) expected = data[i];
    };

    /* initialize system ************/
    GLuint hwnd = initGlut(&argc, argv);
    // First two textures for ping-pong, third texture for input
    float* dataWrap[] = {NULL, data};
    GLuint fbo = setupFBO(size, size, dataWrap, 2, &fb, tex); // input texture
    assert(fbo == fb);
    prog = createProgram(NULL, "max_reduce.f.glsl");
    texParam   = glGetUniformLocation(prog, "texture");
    deltaParam = glGetUniformLocation(prog, "delta");
    glUseProgram(prog);

    /* perform calculation in loop ***/
    int outSize = size >> 1;
    while (outSize) {
        glUniform1f(deltaParam, outSize);           // bind use outSize as offset, outSize >= 1
        glActiveTexture(GL_TEXTURE1);               // select texture1
        glBindTexture(texTarget, tex[1-writePos]);  // apply data into texture 1
        glUniform1i(texParam, 1);                   // use texture 1 as uniform sampler texture
        glDrawBuffer(ATTACHMENTPOINT[writePos]);    // set render destination
        render(outSize, outSize);                   // run GLSL program
        outSize >>= 1;                              // Set output size to half for next iteration
        writePos = 1-writePos;                      // swap the role of two textures for next iteration
    };
    glFinish();

    float result;
    readFBO(ATTACHMENTPOINT[1-writePos], 1, 1, &result);
    printf("Maximum  = %f\n", result);
    printf("Expected = %f\n", expected);

    /* clean up **********************/
    glDeleteProgram(prog);
    cleanupFBO(&fb, tex, 2);
    glutDestroyWindow (hwnd);
    free(data);
    // exit
    return 0;
}
