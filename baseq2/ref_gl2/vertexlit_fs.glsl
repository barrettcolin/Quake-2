#if defined (GL_ES)
precision mediump float;
#endif

uniform sampler2D sDiffuse;

varying vec4 vColor;
varying vec2 vTexCoord;

void main()
{
    gl_FragColor = texture2D(sDiffuse, vTexCoord) * vColor;
}
