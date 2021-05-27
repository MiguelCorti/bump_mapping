#include "renderwidget.h"

#include <QImage>
#include <QGLWidget>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QMouseEvent>
#include <QtMath>

#include <cmath>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

RenderWidget::RenderWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , program(nullptr)
{
}


RenderWidget::~RenderWidget()
{
    //Delete OpenGL resources
}


void RenderWidget::initializeGL()
{
    //Initializa as funções OpenGL
    initializeOpenGLFunctions();

    scrollDelta = 0.0;
    //Define a cor do fundo
    glClearColor(0,0,0,1);

    //Define a viewport
    glViewport(0, 1, width(), height());

    //Compilar os shaders
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, "../src/vertexshader.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, "../src/fragmentshader.glsl");
    program.link();

    glm::vec3 eye(4,4,5);
    glm::vec3 center(0,0,0);
    glm::vec3 up(0,1,0);

    //Definir matriz view e projection
    float ratio = static_cast<float>(width())/height();
    view = glm::lookAt(eye, center, up);
    proj = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
    //Cena de cubos
//    createCube();
//    createTexture("../src/cube_texture.png");

    //Cena de esferas
    createSphere();
    createTexture("../src/sphere_texture_named.jpg");

    //Criar VBO e VAO
    createVBO();
}


void RenderWidget::paintGL()
{
    //Habilita o teste de Z
    glEnable(GL_DEPTH_TEST);

    //Limpar a tela
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //Linkar o VAO
    glBindVertexArray(VAO);

    //Linkar o programa e passar as uniformes:
    //Matriz
    //Posição da luz
    //Ambiente, difusa, especular e brilho do material
    program.bind();

    QMatrix4x4 v(glm::value_ptr(glm::transpose(view)));
    QMatrix4x4 p(glm::value_ptr(glm::transpose(proj)));

    //Passar as uniformes da luz e do material
    program.setUniformValue("light.position", v*QVector3D(5,9,-5) );
    program.setUniformValue("material.ambient", QVector3D(0.1f,0.1f,0.1f));
    program.setUniformValue("material.diffuse", QVector3D(1.0f,1.0f,1.0f));
    program.setUniformValue("material.specular", QVector3D(1.0f,1.0f,1.0f));
    program.setUniformValue("material.shininess", 24.0f);

    //Ativar e linkar a textura
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    program.setUniformValue("sampler", 0);

    //Passar as matrizes de transformação
    for( int x = -3; x <= 3; x+=3 )
    {
        for( int z = -3; z <= 3; z+=3)
        {
            QMatrix4x4 m;
            m.translate(x,0,z);
            m = rotationMat*m;
            m.scale(1-scrollDelta/10); // Tentativa de zoom (na verdade é apenas um scale)

            //Passar as matrizes mv e mvp
            QMatrix4x4 mv = v * m;
            QMatrix4x4 mvp = p * mv;

            program.setUniformValue("mv", mv);
            program.setUniformValue("mv_ti", mv.inverted().transposed());
            program.setUniformValue("mvp", mvp);
            //Desenhar
            glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);
        }
    }

}


void RenderWidget::resizeGL(int w, int h)
{
    //Atualizar a viewport

    //Atualizar a matriz de projeção
}


void RenderWidget::createCube()
{
    //Definir vértices, normais e índices
    vertices = {
        { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 },
        { +1, -1, -1 }, { +1, -1, -1 }, { +1, -1, -1 },
        { +1, -1, +1 }, { +1, -1, +1 }, { +1, -1, +1 },
        { -1, -1, +1 }, { -1, -1, +1 }, { -1, -1, +1 },
        { -1, +1, -1 }, { -1, +1, -1 }, { -1, +1, -1 },
        { +1, +1, -1 }, { +1, +1, -1 }, { +1, +1, -1 },
        { +1, +1, +1 }, { +1, +1, +1 }, { +1, +1, +1 },
        { -1, +1, +1 }, { -1, +1, +1 }, { -1, +1, +1 }
    };
    
    normals = {
        {  0, -1,  0 }, { -1,  0,  0 }, {  0,  0, -1 },
        {  0, -1,  0 }, { +1,  0,  0 }, {  0,  0, -1 },
        {  0, -1,  0 }, { +1,  0,  0 }, {  0,  0, +1 },
        {  0, -1,  0 }, { -1,  0,  0 }, {  0,  0, +1 },
        { -1,  0,  0 }, {  0,  0, -1 }, {  0, +1,  0 },
        { +1,  0,  0 }, {  0,  0, -1 }, {  0, +1,  0 },
        { +1,  0,  0 }, {  0,  0, +1 }, {  0, +1,  0 },
        { -1,  0,  0 }, {  0,  0, +1 }, {  0, +1,  0 }
    };

    texCoords = {
        {0.25, 0.50}, {0.25, 0.50}, {0.50, 0.75},
        {0.00, 0.50}, {1.00, 0.50}, {0.75, 0.75},
        {0.00, 0.25}, {1.00, 0.25}, {0.75, 0.00},
        {0.25, 0.25}, {0.25, 0.25}, {0.50, 0.00},
        {0.50, 0.50}, {0.50, 0.50}, {0.50, 0.50},
        {0.75, 0.50}, {0.75, 0.50}, {0.75, 0.50},
        {0.75, 0.25}, {0.75, 0.25}, {0.75, 0.25},
        {0.50, 0.25}, {0.50, 0.25}, {0.50, 0.25}
    };

    indices = {
        0,   3,  6, //normal: (  0, -1,  0 )
        0,   6,  9, //normal: (  0, -1,  0 )
        12,  1, 10, //normal: ( -1,  0,  0 )
        12, 10, 21, //normal: ( -1,  0,  0 )
        18,  7,  4, //normal: ( +1,  0,  0 )
        18,  4, 15, //normal: ( +1,  0,  0 )
        22, 11,  8, //normal: (  0,  0, +1 )
        22,  8, 19, //normal: (  0,  0, +1 )
        16,  5,  2, //normal: (  0,  0, -1 )
        16,  2, 13, //normal: (  0,  0, -1 )
        23, 20, 17, //normal: (  0, +1,  0 )
        23, 17, 14  //normal: (  0, +1,  0 )
    };
}


// MOUSE HANDLING METHODS //

inline QVector3D getArcballVector(const QPoint& pt, int width, int height)
{
    float R = qMin(width/2.0, height/2.0);
    float center_x = width/2.0;
    float center_y = height/2.0;
    QVector3D P = QVector3D( (1.0*pt.x()-center_x)/R,
                             (1.0*pt.y()-center_y)/R,
                            0);
    // P.setY (-P.y ());

    float OP_squared = P.lengthSquared ();
    if (OP_squared <= 1)
        P.setZ (std::sqrt(1 - OP_squared));
    else
        P.normalize ();
    return P;
}

void RenderWidget::wheelEvent(QWheelEvent *e){
    scrollDelta +=  e->delta() / 120;
    update();
}

void RenderWidget::mousePressEvent(QMouseEvent *e){
    if(e->buttons() == Qt::LeftButton)
        rotPos=e->pos ();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *e){
    if(e->buttons ()==Qt::LeftButton)
    {
        if(!rotPos.isNull () && rotPos!=e->pos())
        {
            //rotate using an arcBall for freeform rotation
            QVector3D vec1 = getArcballVector (rotPos, width (), height ());
            QVector3D vec2 = getArcballVector (e->pos (), width (), height ());

            float squared_len = vec1.lengthSquared()*vec2.lengthSquared();
            float cos = QVector3D::dotProduct(vec1, vec2)/squared_len;
            double theta = qAcos(cos);
            QVector3D rotaxis = QVector3D::crossProduct(vec1, vec2)/squared_len;

            QQuaternion q (qCos(theta/2.0),rotaxis*qSin(theta/2.0));
            QQuaternion quat_vec1(0,vec1);
            QQuaternion quat_vec2(0,vec2);

            quat_vec2 = q*quat_vec2*q.inverted();
            q = quat_vec2*quat_vec1.inverted();
            // quat.normalize ();
            //we want to left-multiply rMat with quat but that isn't available
            //so we'll have to do it the long way around
            QMatrix4x4 rot;
            rot.rotate (q);
            rotationMat=rot*rotationMat;

            update();
        }
        rotPos=e->pos ();
    }

}

// MOUSE HANDLING METHODS //

void RenderWidget::createSphere()
{
    const int n = 100;
    const int m = 100;

    const int numTriangles = 2 * n * m;

    for( unsigned int i = 0; i <= n; i++ )
    {
        for( unsigned int j = 0; j <= m; j++ )
        {
            //Atualizar as coordenadas de textura
            double s = static_cast<double>(i) / n;
            double t = static_cast<double>(j) / m;
            texCoords.push_back(glm::vec2(s,t));

            //Calcula os parâmetros
            double theta = 2 * s * M_PI;
            double phi = t * M_PI;
            double sinTheta = sin( theta );
            double cosTheta = cos( theta );
            double sinPhi = sin( phi );
            double cosPhi = cos( phi );

            //Calcula os vértices == equacao da esfera
            vertices.push_back( glm::vec3(cosTheta * sinPhi,
                                          cosPhi,
                                          sinTheta * sinPhi) );
        }
    }

    normals = vertices;

    indices.resize(numTriangles*3);

    auto getIndex = [=]( unsigned int i, unsigned int j, unsigned int s )
    {
        return j + i * ( s + 1 );
    };

    //Preenche o vetor com a triangulação
    unsigned int k = 0;
    for( unsigned int i = 0; i < n; i++ )
    {
        for( unsigned int j = 0; j < m; j++ )
        {
            indices[ k++ ] = getIndex( i, j, n );
            indices[ k++ ] = getIndex( i + 1, j + 1, n );
            indices[ k++ ] = getIndex( i + 1, j, n );

            indices[ k++ ] = getIndex( i, j, n );
            indices[ k++ ] = getIndex( i, j + 1, n );
            indices[ k++ ] = getIndex( i + 1, j + 1, n );
        }
    }
}


void RenderWidget::createVBO()
{
    //Construir vetor do vbo
    //OBS: Os dados já poderiam estar sendo armazenados assim na classe.
    struct vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    std::vector< vertex > vbo;
    vbo.reserve( vertices.size() );
    for( unsigned int i = 0; i < vertices.size(); i++ )
    {
        vbo.push_back({vertices[i], normals[i], texCoords[i]});
    }

    // Criar VBO, linkar e copiar os dados
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vbo.size()*sizeof(vertex), vbo.data(), GL_STATIC_DRAW);

    // Criar EBO, linkar e copiar os dados
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Criar VAO, linkar e definir layouts
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Habilitar, linkar e definir o layout dos buffers
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE,
                           sizeof(vertex),
                           (void*)0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE,
                           sizeof(vertex),
                           (void*)sizeof(glm::vec3) );
    glEnableVertexAttribArray( 2 );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE,
                           sizeof(vertex),
                           (void*)(2*sizeof(glm::vec3)) );

    // Linkar o EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
}


void RenderWidget::createTexture(const std::string& imagePath)
{
    //Criar a textura
    glGenTextures(1, &textureID);
    //Linkar (bind) a textura criada//Linkar (bind) a textura criada
    glBindTexture(GL_TEXTURE_2D, textureID);
    //Abrir arquivo de imagem com o Qt
    QImage texImage = QGLWidget::convertToGLFormat(QImage(imagePath.c_str()));
    //Enviar a imagem para o OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texImage.width(), texImage.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //Definir parametros de filtro e gerar mipmap
    glGenerateMipmap(GL_TEXTURE_2D);
}
