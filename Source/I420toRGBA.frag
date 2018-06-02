uniform sampler2D u_texture0;
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;
uniform float u_chroma_div_w;
uniform float u_chroma_div_h;
varying vec2 texture_coordinate;

void main()
{
    vec3 yuv;
    yuv.r = texture2D(u_texture0, texture_coordinate).r - 0.0625;
    yuv.g = texture2D(u_texture1, vec2(texture_coordinate.x / u_chroma_div_w, texture_coordinate.y / u_chroma_div_h)).r - 0.5;
    yuv.b = texture2D(u_texture2, vec2(texture_coordinate.x / u_chroma_div_w, texture_coordinate.y / u_chroma_div_h)).r - 0.5;
    gl_FragColor = clamp(vec4(mat3(1.1643,  1.16430, 1.1643,
                                    0.0,    -0.39173, 2.0170,
                                    1.5958, -0.81290, 0.0) * yuv, 1.0), 0.0, 1.0);
}