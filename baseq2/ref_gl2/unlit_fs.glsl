uniform sampler2D diffuseTex;

void main()
{
    vec4 color = texture2D(diffuseTex, gl_TexCoord[0].st);

#if defined (ALPHA_TEST_66)
    if(color.a <= 0.666f)
    {
        discard;
    }
#endif

    gl_FragColor = color;
}
