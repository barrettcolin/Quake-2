#if defined (GL_ES)
precision mediump float;
#endif

#if defined DIFFUSE_COLOR
// inline bmodels with RF_TRANSLUCENT use 0.25 alpha
uniform vec4 vDiffuseColor;
#endif

uniform sampler2D sDiffuse;
uniform sampler2D sLightmap;

varying vec2 vTexCoord0;
varying vec2 vTexCoord1;

void main()
{
    vec4 diffuseColor = texture2D(sDiffuse, vTexCoord0);
    vec4 lightmapColor = vec4(texture2D(sLightmap, vTexCoord1).rgb, 1.0);
#if defined DIFFUSE_COLOR
    gl_FragColor = diffuseColor * lightmapColor * vDiffuseColor;
#else
    gl_FragColor = diffuseColor * lightmapColor;
#endif
}
