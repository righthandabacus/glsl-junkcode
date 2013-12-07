/* Test script of OpenGL Shader Language for General Purpose Computing
 *
 * Code adopted from the tutorial:
 *   http://www.mathematik.tu-dortmund.de/~goeddeke/gpgpu/tutorial2.html
 *
 * This code allocates 2^k x 2^k floats of random value and find the maximum of
 * them using GPGPU parallelization.
 */

#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "glsl_utils.h"

int main(int argc, char **argv) {
    /* command line parameters */
    int k;                          // Exponent
    int size;                       // 2^k
    /* application variables */
    float*data;                     // data
    GLuint fb, prog;                // FBO handle and program handle
    GLuint tex[3];                  // textures handle
    int writePos=0;                 // ping-pong variable
    GLint texParam, deltaParam;     // connection to params in GLSL
    int texSize;

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

    /* initialize system ************/
    GLuint hwnd = initGlut(&argc, argv);
    // First two textures for ping-pong, third texture for input
    float* dataWrap[] = {NULL, NULL, data};
    GLuint fbo = setupFBO(size, size, dataWrap, 3, &fb, tex); // input texture
    assert(fbo == fb);
    prog = createProgram(NULL, "max_reduce.f.glsl");

    /* perform calculation part 1 ****/
    texParam   = glGetUniformLocation(prog, "texture");
    deltaParam = glGetUniformLocation(prog, "delta");
    glUseProgram(prog);
    glUniform1f(deltaParam, size/2);    // use size/2 as uniform float delta
    glActiveTexture(GL_TEXTURE1);       // select texture1
    glBindTexture(texTarget, tex[2]);   // apply data into texture 1
    glUniform1i(texParam, 1);           // use texture 1 as uniform sampler texture
    glFinish();                         // flush GPU for more accurate timing

    glDrawBuffer(ATTACHMENTPOINT[writePos]);    // set render destination
    render(size/2, size/2);             // run GLSL program
    glFinish();

    glReadBuffer(ATTACHMENTPOINT[writePos]);
    float* result = (float*)malloc(sizeof(float)*size*size/4);    // malloc and copy result from GPU
    glReadPixels(0, 0, size/2, size/2, texFmt, GL_FLOAT, result);
    for (int i=0; i<size*size; i++) {
        printf("%f",data[i]);
        printf(((1+i) % size)?"\t":"\n");
    };
    printf("output\n");
    for (int i=0; i<size*size/4; i++) {
        printf("%f",result[i]);
        printf(((1+i) % (size/2))?"\t":"\n");
    };
    // and clean up
    glDeleteProgram(prog);
    cleanupFBO(&fb, tex, 3);
    glutDestroyWindow (hwnd);
    free(data);
    free(result);
    // exit
    return 0;
}
