// inline bmodels with RF_TRANSLUCENT use 0.25 alpha
uniform vec4 vDiffuseColor;

uniform sampler2D sDiffuse;
uniform sampler2D sLightmap;

varying vec2 vTexCoord0;
varying vec2 vTexCoord1;

void main()
{
    vec4 diffuseColor = texture2D(sDiffuse, vTexCoord0);
    vec4 lightmapColor = texture2D(sLightmap, vTexCoord1);

    gl_FragColor = diffuseColor * lightmapColor * vDiffuseColor;
}
