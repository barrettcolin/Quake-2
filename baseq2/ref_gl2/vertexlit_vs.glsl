uniform mat4 mClipFromView;
uniform mat4 mViewFromWorld;
uniform mat4 mWorldFromModel;

attribute vec3 a_vPosition;
attribute vec4 a_vColor;
attribute vec2 a_vTexCoord;

varying vec4 vColor;
varying vec2 vTexCoord;

void main()
{
    gl_Position = mClipFromView * mViewFromWorld * mWorldFromModel * vec4(a_vPosition.xyz, 1.0);
    vTexCoord = a_vTexCoord;
    vColor = a_vColor;
}
