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


const bool isQuadStones = true;

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

    scrollDelta = 45.0f;
    //Define a cor do fundo
    glClearColor(0,0,0,1);

    //Define a viewport
    glViewport(0, 1, width(), height());

    //Compilar os shaders
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, "../src/vertexshader.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, "../src/fragmentshader.glsl");
    program.link();

    glm::vec3 eye(0,0,5);
    glm::vec3 center(0,0,0);
    glm::vec3 up(0,1,0);

    //Definir matriz view e projection
    float ratio = static_cast<float>(width())/height();
    view = glm::lookAt(eye, center, up);
    proj = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);

    if (isQuadStones) {
        // Cena quadrado
        createQuad();
        createNormalTexture("../src/stones_norm.jpg");
        createTexture("../src/stones.jpg");
    } else {
        // Cena de esfera
        createSphere();
        createNormalTexture("../src/golfball.png");
    }

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

    // Ativa e linka a textura
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, normalTextureID);
    program.setUniformValue("normalSampler", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureID);
    program.setUniformValue("sampler", 1);

    //Passar as matrizes de transformação
    QMatrix4x4 m;
    m.translate(0,0,0);
    m = rotationMat*m;

    //Passar as matrizes mv e mvp
    QMatrix4x4 mv = v * m;
    QMatrix4x4 mvp = p * mv;

    program.setUniformValue("m", m);
    program.setUniformValue("mv", mv);
    program.setUniformValue("mv_ti", mv.inverted().transposed());
    program.setUniformValue("mvp", mvp);

    //Passar as uniformes da luz e do material
    // v*QVector3D(0,0,5)
    program.setUniformValue("light.position", QVector3D(0,0,5) );
    if (isQuadStones) {
        program.setUniformValue("material.ambient", QVector3D(0.1f,0.1f,0.1f));
        program.setUniformValue("material.diffuse", QVector3D(0.6f,0.6f,0.6f));
        program.setUniformValue("material.specular", QVector3D(0.3f,0.3f,0.3f));
        program.setUniformValue("material.shininess", 100.0f);
    } else {
        program.setUniformValue("material.ambient", QVector3D(0.2f,0.2f,0.2f));
        program.setUniformValue("material.diffuse", QVector3D(0.8f,0.8f,0.8f));
        program.setUniformValue("material.specular", QVector3D(1.0f,1.0f,1.0f));
        program.setUniformValue("material.shininess", 100.0f);
    }
    //Desenhar
    glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);
}


void RenderWidget::resizeGL(int w, int h)
{
    //Atualizar a viewport

    //Atualizar a matriz de projeção
}


void RenderWidget::createQuad()
{
    //Definir vértices, normais e índices
    vertices = {
        { -1, +1, 0 },
        { -1, -1, 0 },
        { +1, -1, 0 },

        { +1, -1, 0 },
        { +1, +1, 0 },
        { -1, +1, 0 }

    };
    
    normals = {
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 },
        { 0, 0, 1 }
    };

    texCoords = {
        {0.0, 1.0},
        {0.0, 0.0},
        {1.0, 0.0},

        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, 1.0}

    };

    indices = {
        0,   1,  2,
        3,   4,  5
    };

    for (int i = 0; i < vertices.size(); i+=3) {
        // Shortcuts for vertices
        glm::vec3 v0 = vertices[i+0];
        glm::vec3 v1 = vertices[i+1];
        glm::vec3 v2 = vertices[i+2];

        // Shortcuts for UVs
        glm::vec2 uv0 = texCoords[i+0];
        glm::vec2 uv1 = texCoords[i+1];
        glm::vec2 uv2 = texCoords[i+2];

        // Edges of the triangle : postion delta
        glm::vec3 deltaPos1 = v1-v0;
        glm::vec3 deltaPos2 = v2-v0;

        // UV delta
        glm::vec2 deltaUV1 = uv1-uv0;
        glm::vec2 deltaUV2 = uv2-uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
        //glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

        tangents.push_back(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);
    }
}

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

            //Calcula os parametros
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

    for ( int i=0; i<vertices.size(); i+=3){
        // Shortcuts for vertices
        glm::vec3 v0 = vertices[i+0];
        glm::vec3 v1 = vertices[i+1];
        glm::vec3 v2 = vertices[i+2];

        // Shortcuts for UVs
        glm::vec2 uv0 = texCoords[i+0];
        glm::vec2 uv1 = texCoords[i+1];
        glm::vec2 uv2 = texCoords[i+2];

        // Edges of the triangle : postion delta
        glm::vec3 deltaPos1 = v1-v0;
        glm::vec3 deltaPos2 = v2-v0;

        // UV delta
        glm::vec2 deltaUV1 = uv1-uv0;
        glm::vec2 deltaUV2 = uv2-uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
        //glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

        tangents.push_back(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);
    }



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
        glm::vec3 tangent;
    };

    std::vector< vertex > vbo;
    vbo.reserve( vertices.size() );
    for( unsigned int i = 0; i < vertices.size(); i++ )
    {
        vbo.push_back({vertices[i], normals[i], texCoords[i], tangents[i]});
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
    glEnableVertexAttribArray( 3 );
    glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE,
                           sizeof(vertex),
                           (void*)sizeof(glm::vec3) );

    // Linkar o EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
}


void RenderWidget::createNormalTexture(const std::string& imagePath)
{
    // Cria a textura
    glGenTextures(1, &normalTextureID);
    // Linka (bind) a textura criada
    glBindTexture(GL_TEXTURE_2D, normalTextureID);
    // Abre arquivo de imagem com o Qt
    QImage texImage = QGLWidget::convertToGLFormat(QImage(imagePath.c_str()));
    // Envia a imagem para o OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texImage.width(), texImage.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // Define parametros de filtro e gera mipmap
    glGenerateMipmap(GL_TEXTURE_2D);
}

void RenderWidget::createTexture(const std::string& imagePath)
{
    // Cria a textura
    glGenTextures(1, &textureID);
    // Linka (bind) a textura criada
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Abre arquivo de imagem com o Qt
    QImage texImage = QGLWidget::convertToGLFormat(QImage(imagePath.c_str()));
    // Envia a imagem para o OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texImage.width(), texImage.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // Define parametros de filtro e gera mipmap
    glGenerateMipmap(GL_TEXTURE_2D);
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
    P.setY (-P.y ());

    float OP_squared = P.lengthSquared ();
    if (OP_squared <= 1)
        P.setZ (std::sqrt(1 - OP_squared));
    else
        P.normalize ();
    return P;
}

void RenderWidget::wheelEvent(QWheelEvent *e){
    scrollDelta +=  e->delta()/120;
    if(scrollDelta < 1.0f)
        scrollDelta = 1.0f;
    if(scrollDelta > 45.0f)
        scrollDelta = 45.0f;
    proj = glm::perspective(glm::radians(scrollDelta), static_cast<float>(width())/height(), 0.1f, 100.0f);
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

// END OF MOUSE HANDLING METHODS //
