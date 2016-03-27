uniform mat4 mClipFromView;
uniform mat4 mViewFromWorld;
uniform mat4 mWorldFromModel;

attribute vec3 a_vPosition;
attribute vec2 a_vTexCoord0;
attribute vec2 a_vTexCoord1;

varying vec2 vTexCoord0;
varying vec2 vTexCoord1;

void main()
{
    gl_Position = mClipFromView * mViewFromWorld * mWorldFromModel * vec4(a_vPosition.xyz, 1.0);
    vTexCoord0 = a_vTexCoord0;
    vTexCoord1 = a_vTexCoord1;
}
