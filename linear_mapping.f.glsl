#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect textureY;
uniform sampler2DRect textureX;
uniform float alpha;

void main(void) {
    float y = texture2DRect(textureY, gl_TexCoord[0].st).x;
    float x = texture2DRect(textureX, gl_TexCoord[0].st).x;
    gl_FragColor.x = x + alpha*y;
}
