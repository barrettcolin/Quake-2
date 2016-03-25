uniform sampler2D sDiffuse;
uniform sampler2D sLightmap;

varying vec2 texCoord0;
varying vec2 texCoord1;

void main()
{
    vec4 diffuseColor = texture2D(sDiffuse, texCoord0);
    vec4 lightmapColor = texture2D(sLightmap, texCoord1);

    gl_FragColor = diffuseColor * lightmapColor;
}
