#pragma once

#include "OpenGL/OpenGLProgramVariablesBase.h"
#include "OpenGL/OpenGLTexture2D.h"

class WebMPlayerOpenGLProgramVariables : public OpenGLProgramVariablesBase<WebMPlayerOpenGLProgramVariables>
{
public:
    BEGIN_OPENGL_VARIABLES_MAP
        OPENGL_ATTRIBUTE_VARIABLE_LOCATION(position_location_, "a_position");
        OPENGL_ATTRIBUTE_VARIABLE_LOCATION(texture_coordinates_location_, "a_textureCoords");
        OPENGL_UNIFORM_VARIABLE(projection_matrix_, "u_projectionMatrix");
        OPENGL_UNIFORM_VARIABLE(model_view_matrix_, "u_modelViewMatrix");
        OPENGL_UNIFORM_VARIABLE(y_texture_, "u_texture0");
        OPENGL_UNIFORM_VARIABLE(u_texture_, "u_texture1");
        OPENGL_UNIFORM_VARIABLE(v_texture_, "u_texture2");
        OPENGL_UNIFORM_VARIABLE(chroma_div_w_, "u_chroma_div_w");
        OPENGL_UNIFORM_VARIABLE(chroma_div_h_, "u_chroma_div_h");
    END_OPENGL_VARIABLES_MAP
    
public:
    GLint position_location_;
    GLint texture_coordinates_location_;
    Variable<GLfloat[16]> projection_matrix_;
    Variable<GLfloat[16]> model_view_matrix_;
    Variable<SingleChannelOpenGLTexture2D> y_texture_;
    Variable<SingleChannelOpenGLTexture2D> u_texture_;
    Variable<SingleChannelOpenGLTexture2D> v_texture_;
    Variable<GLfloat> chroma_div_w_;
    Variable<GLfloat> chroma_div_h_;
};
