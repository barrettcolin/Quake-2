uniform sampler2D diffuseTex;

void main()
{
    vec4 color = texture2D(diffuseTex, gl_TexCoord[0].st);

    if(color.a <= 0.666f)
    {
        discard;
    }

    gl_FragColor = color;
}
