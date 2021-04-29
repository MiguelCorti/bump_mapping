#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QMatrix4x4>

#include <vector>

#include "glm/glm.hpp"

class RenderWidget
        : public QOpenGLWidget
        , protected QOpenGLExtraFunctions
{
public:
    RenderWidget(QWidget* parent);
    virtual ~RenderWidget();

private:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int w, int h);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    //virtual void mouseReleaseEvent(QMouseEvent *e);
    //virtual void updateMouse();
    virtual void wheelEvent(QWheelEvent *e);

    void createQuad();
    void createSphere();
    void createVBO();
    void createTexture(const std::string& imagePath);
    void createNormalTexture(const std::string& imagePath);

    QOpenGLShaderProgram program;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    float scrollDelta;
    std::vector< glm::vec3 > vertices;
    std::vector< glm::vec3 > normals;
    std::vector< glm::vec2 > texCoords;
    std::vector< glm::vec3 > tangents;
    std::vector< glm::vec3 > bitangents;
    std::vector< unsigned int > indices;

    glm::mat4x4 view;
    glm::mat4x4 proj;
    QMatrix4x4 rotationMat;
    QPoint rotPos;

    unsigned int normalTextureID;
    unsigned int textureID;
};

#endif // RENDERWIDGET_H
