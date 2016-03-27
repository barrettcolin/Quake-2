uniform vec4 vDiffuseColor;

uniform sampler2D sDiffuse;

varying vec2 vTexCoord;

void main()
{
    // texture alpha only used for alpha testing
    vec4 color = texture2D(sDiffuse, vTexCoord);

#if defined (ALPHA_TEST_66)
    if(color.a <= 0.666f)
    {
        discard;
    }
#endif

    gl_FragColor = vec4(color.rgb, 1.0f) * vDiffuseColor;
}
