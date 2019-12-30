/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "openglwindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include <QDebug>
#include <QtCore/qmath.h>
#include <QFile>

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

//! [1]
class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();

    void initialize() override;
    void render() override;

private:
    GLsizei pixel_w = 1456;
    GLsizei pixel_h = 1456;
    GLint linked;

    //TODO
    GLuint id_y, id_u, id_v; // Texture id
    char Y[1456*1456],  U[1456/2*1456/2],  V[1456/2*1456/2];

    GLuint m_textureUniformY;
    GLuint m_textureUniformU;
    GLuint m_textureUniformV;

    QOpenGLShaderProgram *m_program;
    int m_frame;
};

TriangleWindow::TriangleWindow()
    : m_program(0)
    , m_frame(0)
{
}
//! [1]

//! [2]
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    TriangleWindow window;
    window.setFormat(format);
    window.resize(1000, 1000);
    window.show();

    window.setAnimating(true);

    return app.exec();
}
//! [2]


//! [3]
static const char *vertexShaderSource ="attribute vec4 vertexIn; \n"
                                       "attribute vec2 textureIn;\n"
                                       "varying vec2 textureOut;\n"
                                       "void main(void)\n"
                                       "{\n"
                                       "    gl_Position = vertexIn; \n"
                                       "    textureOut = textureIn;\n"
                                       "}";

static const char *fragmentShaderSource ="varying highp vec2 textureOut;"
                                         "uniform highp sampler2D tex_y;"
                                         "uniform highp sampler2D tex_u;"
                                         "uniform highp sampler2D tex_v;"
                                         "void main(void){"
                                         "highp vec3 yuv;"
                                         "highp vec3 rgb;"
                                         "yuv.x = texture2D(tex_y, textureOut).r;"
                                         "yuv.y = texture2D(tex_u, textureOut).r - 0.5;"
                                         "yuv.z = texture2D(tex_v, textureOut).r - 0.5;"
                                         "rgb = mat3( 1,       1,         1,"
                                         "0,       -0.39465,  2.03211,"
                                         "1.13983, -0.58060,  0) * yuv;"
                                         "gl_FragColor = vec4(rgb, 1);}";

//! [4]
void TriangleWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->bindAttributeLocation("vertexIn",ATTRIB_VERTEX);
    m_program->bindAttributeLocation("textureIn",ATTRIB_TEXTURE);
    m_program->link();
    glGetProgramiv(m_program->programId(),GL_LINK_STATUS,&linked);
    qDebug()<<"Program连接情况"<<linked;
    m_textureUniformY = m_program->uniformLocation("tex_y");
    m_textureUniformU = m_program->uniformLocation("tex_u");
    m_textureUniformV = m_program->uniformLocation("tex_v");
}
//! [4]

//! [5]
void TriangleWindow::render()
{
    //TODO
    QString path = QString("/home/duan/Desktop/play.yuv");
    QFile file(path);
    bool isOk = file.open(QIODevice::ReadOnly);
    if(isOk){
        // 创建数据流,和file文件关联
        QDataStream stream(&file);
        int len;
        len = stream.readRawData(Y,pixel_w*pixel_h);
        qDebug()<<"读到数据的大小"<<len;

        len = stream.readRawData(U,pixel_w/2*pixel_h/2);
        qDebug()<<"读到数据的大小"<<len;

        len = stream.readRawData(V,pixel_w/2*pixel_h/2);
        qDebug()<<"读到数据的大小"<<len;
    }

    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0,255,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
    };

    GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };

    // 设置物理坐标
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    // 设置纹理坐标
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);

    glGenTextures(1, &id_y);
    glBindTexture(GL_TEXTURE_2D, id_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenTextures(1, &id_u);
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenTextures(1, &id_v);
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id_y);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED_NV, pixel_w, pixel_h, 0, GL_RED_NV, GL_UNSIGNED_BYTE, &Y);
    glUniform1i(m_textureUniformY, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED_NV, pixel_w / 2, pixel_h / 2, 0, GL_RED_NV, GL_UNSIGNED_BYTE, &U);
    glUniform1i(m_textureUniformU, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED_NV, pixel_w / 2, pixel_h / 2, 0, GL_RED_NV, GL_UNSIGNED_BYTE, &V);
    glUniform1i(m_textureUniformV, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(ATTRIB_VERTEX);
    glDisableVertexAttribArray(ATTRIB_TEXTURE);

    m_program->release();

    qDebug()<<"数据已经刷新";

    ++m_frame;
}
//! [5]
