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

unsigned int planeVAO;

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMapFBO;
unsigned int depthMap;
glm::vec3 lightPos;

QOpenGLShaderProgram shadowProgram(nullptr);
QOpenGLShaderProgram debugProgram(nullptr);

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
    // Initializa as funções OpenGL
    initializeOpenGLFunctions();

    scrollDelta = 45.0f;
    // Define a cor do fundo
    glClearColor(0,0,0,1);

//    // Define a viewport
//    glViewport(0, 1, width(), height());

    // Compila os shaders do programa normal
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, "../src/vertexshader.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, "../src/fragmentshader.glsl");
    program.link();

    // Compila os shaders do programa para debug (usa o depth map como cor dos objetos)
    debugProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "../src/debug_vertexshader.glsl");
    debugProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "../src/debug_fragmentshader.glsl");
    debugProgram.link();

    // Compila os shaders do programa que calcula o shadow map
    shadowProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "../src/shadowmap_vertexshader.glsl");
    shadowProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "../src/shadowmap_fragmentshader.glsl");
    shadowProgram.link();

    eye = glm::vec3(-3.0,3.0,4.0);
    glm::vec3 center(0,0,0);
    glm::vec3 up(0,1,0);

    // Define matriz view e projection
    float ratio = static_cast<float>(width())/height();
    view = glm::lookAt(eye, center, up);
    proj = glm::perspective(glm::radians(scrollDelta), ratio, 0.1f, 100.0f);

    lightPos = glm::vec3(-1.0f, 2.0f, 2.0f);

    //Habilita o teste de Z
    glEnable(GL_DEPTH_TEST);
    //Cena de cubos
//    createCube();
//    createTexture("../src/cube_texture.png");

    // CRIANDO A ESFERA
    createSphere();
    createTexture("../src/wood_texture02.jpeg");
    // Cria VBO e VAO da Esfera
//    createVBO();

    // CRIANDO O PLANO
    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 10.0f
    };

    // Cria VAO do Plano
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // CRIANDO FBO PARA O DEPTH MAP
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    // Criando textura do depth map
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // Linkando textura do depth map com o FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    const GLenum buffers[]{ GL_NONE, GL_NONE, GL_NONE, GL_NONE };
    glDrawBuffers(4, buffers);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    program.bind();
    program.setUniformValue("sampler", 0);
    program.setUniformValue("shadowMap", 1);
    debugProgram.bind();
    debugProgram.setUniformValue("depthMap", 0);
}


void RenderWidget::paintGL()
{
    // 1) RENDERIZA A CENA NA POSIÇÃO DA LUZ E GUARDA O DEPTH MAP
    glm::mat4 lightProjection, lightView, lightModel(1.0f);
    glm::mat4 lightSpaceMatrix;
    float near_plane = -1.0f, far_plane = 10.0f;
//    lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
//    lightModel = glm::rotate(lightModel, glm::radians(-180.0f), glm::normalize(glm::vec3(0.0, 1.0, 1.0)));
//    lightModel = glm::rotate(lightModel, glm::radians(-180.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
    lightSpaceMatrix = lightProjection * lightView * lightModel;
    // Renderiza a cena da posição da luz
    shadowProgram.bind();
    QMatrix4x4 qLightSpaceMatrix(glm::value_ptr(lightSpaceMatrix));
    shadowProgram.setUniformValue("lightSpaceMatrix", qLightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        renderScene(shadowProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2) RENDERIZA A CENA NORMAL UTILIZANDO O DEPTH MAP COMO SHADOW MAP
    // Reseta o viewport
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.bind();
    program.setUniformValue("lightSpaceMatrix", qLightSpaceMatrix);
    program.setUniformValue("viewPos", QVector3D(eye.x, eye.y, eye.z));
    program.setUniformValue("lightPosition", QVector3D(lightPos.x,lightPos.y,lightPos.z) );
    program.setUniformValue("material.ambient", QVector3D(0.1f,0.1f,0.1f));
    program.setUniformValue("material.diffuse", QVector3D(1.0f,1.0f,1.0f));
    program.setUniformValue("material.specular", QVector3D(1.0f,1.0f,1.0f));
    program.setUniformValue("material.shininess", 100.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    renderScene(program);


    // DEBUG) Renderiza o depth map como textura para debug
//    debugProgram.bind();
//    debugProgram.setUniformValue("near_plane", near_plane);
//    debugProgram.setUniformValue("far_plane", far_plane);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, depthMap);
//    renderQuad();
}

void RenderWidget::renderScene(QOpenGLShaderProgram &shader) {
    QMatrix4x4 v(glm::value_ptr(glm::transpose(view)));
    QMatrix4x4 p(glm::value_ptr(glm::transpose(proj)));
    QMatrix4x4 m, mv, mvp;

    shader.setUniformValue("view", v);
    shader.setUniformValue("proj", p);
    // DESENHANDO O PLANO
    m = QMatrix4x4(glm::value_ptr(glm::mat4(1.0f)));
    m = rotationMat*m;
    shader.setUniformValue("model", m);
    mv = v * m;
    mvp = p * mv;
    shader.setUniformValue("mv", mv);
    shader.setUniformValue("mv_ti", mv.inverted().transposed());
    shader.setUniformValue("mvp", mvp);
    // Desenha
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // DESENHANDO UMA ESFERA
    m = QMatrix4x4(glm::value_ptr(glm::mat4(1.0f)));
    m.translate(0.0f,1.0f,2.0f);
    m.scale(0.5f);
    m = rotationMat*m;
    mv = v * m;
    mvp = p * mv;
    shader.setUniformValue("model", m);
    shader.setUniformValue("mv", mv);
    shader.setUniformValue("mv_ti", mv.inverted().transposed());
    shader.setUniformValue("mvp", mvp);
    createVBO();
    glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);

    // DESENHANDO OS CUBOS
    // Cubo 01
    m = QMatrix4x4(glm::value_ptr(glm::mat4(1.0f)));
    m.translate(0.0f,1.5f,0.0f);
    m.scale(0.5f);
    m = rotationMat*m;
    mv = v * m;
    mvp = p * mv;
    shader.setUniformValue("model", m);
    shader.setUniformValue("mv", mv);
    shader.setUniformValue("mv_ti", mv.inverted().transposed());
    shader.setUniformValue("mvp", mvp);
    renderCube(); // Desenha

    // Cubo 02
    m = QMatrix4x4(glm::value_ptr(glm::mat4(1.0f)));
    m.translate(2.0f,0.0f,1.0f);
    m.scale(0.5f);
    m = rotationMat*m;
    mv = v * m;
    mvp = p * mv;
    shader.setUniformValue("model", m);
    shader.setUniformValue("mv", mv);
    shader.setUniformValue("mv_ti", mv.inverted().transposed());
    shader.setUniformValue("mvp", mvp);
    renderCube(); // Desenha

    // Cubo 03
    m = QMatrix4x4(glm::value_ptr(glm::mat4(1.0f)));
    m.translate(-1.0f,0.0f,2.0f);
    m.rotate(60.0f, QVector3D(1.0, 0.0, 0.0));
    m = rotationMat*m;
    m.scale(0.25f);
    mv = v * m;
    mvp = p * mv;
    shader.setUniformValue("model", m);
    shader.setUniformValue("mv", mv);
    shader.setUniformValue("mv_ti", mv.inverted().transposed());
    shader.setUniformValue("mvp", mvp);
    renderCube(); // Desenha
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

unsigned int quadVAO = 0;
unsigned int quadVBO;
void RenderWidget::renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void RenderWidget::renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
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

// MOUSE HANDLING METHODS //
