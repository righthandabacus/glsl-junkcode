#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect texture;
uniform float delta;

void main(void)
{
    float v1 = texture2DRect(texture, gl_TexCoord[0].st).x;
    float v2 = texture2DRect(texture, gl_TexCoord[0].st+vec2(delta,0)).x;
    float v3 = texture2DRect(texture, gl_TexCoord[0].st+vec2(delta,delta)).x;
    float v4 = texture2DRect(texture, gl_TexCoord[0].st+vec2(0,delta)).x;
    gl_FragColor.x = max(v1, max(v2, max(v3, v4)));
}

/* vim:set syntax=glsl sw=4 ts=4 bs=indent: */
