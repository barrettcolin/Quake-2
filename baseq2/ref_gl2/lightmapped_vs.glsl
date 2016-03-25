varying vec2 texCoord0;
varying vec2 texCoord1;

void main()
{
    gl_Position = ftransform();
    texCoord0 = gl_MultiTexCoord0.st;
    texCoord1 = gl_MultiTexCoord1.st;
}
