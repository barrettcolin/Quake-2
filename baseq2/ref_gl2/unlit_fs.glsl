uniform sampler2D sDiffuse;

varying vec2 texCoord;

void main()
{
    vec4 color = texture2D(sDiffuse, texCoord);

#if defined (ALPHA_TEST_66)
    if(color.a <= 0.666f)
    {
        discard;
    }
#endif

    gl_FragColor = color;
}
