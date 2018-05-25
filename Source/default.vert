uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
attribute vec4 a_position;
attribute vec2 a_textureCoords;
varying vec2 texture_coordinate;

void main()
{
    gl_Position = u_projectionMatrix * (a_position * u_modelViewMatrix);
    texture_coordinate = a_textureCoords;
}