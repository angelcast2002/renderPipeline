#include <fstream>
#include <vector>
#include "glm/glm.hpp"
#include <array>
#include <SDL.h>
#include <sstream>
#include <filesystem>
#include <iostream>
#pragma once
#include "shaders.hpp"
#include "glm/gtc/matrix_transform.hpp"

std::string getCurrentPath() {
    return std::filesystem::current_path().string();
}

std::string getParentDirectory(const std::string& path) {
    std::filesystem::path filePath(path);
    return filePath.parent_path().string();
}

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 480;

Color currentColor = {255, 255, 255, 255}; // Initially set to white
Color clearColor = {0, 0, 0, 255}; // Initially set to black
std::vector<glm::vec3> vertices;
std::vector<Face> faces;

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Software Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void setColor(const Color& color) {
    currentColor = color;
}
// Function to clear the framebuffer with the clearColor
void clear() {
    SDL_SetRenderDrawColor(renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    SDL_RenderClear(renderer);
}
// Function to set a specific pixel in the framebuffer to the currentColor
void point(int x, int y) {
    SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
    SDL_RenderDrawPoint(renderer, x, y);
}

void line(glm::vec3 start, glm::vec3 end) {
    int x1 = round(start.x), y1 = round(start.y);
    int x2 = round(end.x), y2 = round(end.y);

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        point(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Función para encontrar el mínimo de tres valores enteros
int min3(int a, int b, int c) {
    int minAB = a < b ? a : b;
    return minAB < c ? minAB : c;
}

// Función para encontrar el máximo de tres valores enteros
int max3(int a, int b, int c) {
    int maxAB = a > b ? a : b;
    return maxAB > c ? maxAB : c;
}

std::vector<Fragment> triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    std::vector<Fragment> fragments;

    // Calculate the minimum and maximum y-coordinates of the triangle
    int minY = min3(static_cast<int>(A.y), static_cast<int>(B.y), static_cast<int>(C.y));
    int maxY = max3(static_cast<int>(A.y), static_cast<int>(B.y), static_cast<int>(C.y));

    // Calculate the minimum and maximum x-coordinates of the triangle
    int minX = min3(static_cast<int>(A.x), static_cast<int>(B.x), static_cast<int>(C.x));
    int maxX = max3(static_cast<int>(A.x), static_cast<int>(B.x), static_cast<int>(C.x));

    // Rasterization algorithm (scanline)
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            glm::vec3 P(x + 0.5f, y + 0.5f, 0.0f);

            // Calculate barycentric coordinates
            float alpha = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) /
                          ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));
            float beta = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) /
                         ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));
            float gamma = 1.0f - alpha - beta;

            if (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f) {
                fragments.push_back(Fragment(x, y));
            }
        }
    }

    return fragments;
}

Color fragmentShader(const Fragment& fragment) {
    // Example: Assign a constant color to each fragment
    Color fragColor(255, 0, 0, 255); // Red color with full opacity

    // You can modify this function to implement more complex shading
    // based on the fragment's attributes (e.g., depth, interpolated normals, texture coordinates, etc.)

    return fragColor;
}


void render(const std::vector<glm::vec3>& vertexArray, const Uniforms& uniforms) {
    // Limpiamos el framebuffer con el color de fondo
    clear();

    // 1. Vertex Shader
    std::vector<glm::vec3> transformedVertices;
    for (const auto& vertex : vertexArray) {
        // Aplicamos el vertex shader a cada vértice
        glm::vec3 transformedVertex = vertexShader(vertex, uniforms);
        transformedVertices.push_back(transformedVertex);
    }

    // 2. Primitive Assembly
    std::vector<std::vector<glm::vec3>> triangles = primitiveAssembly(transformedVertices);

    // 3. Rasterization
    std::vector<Fragment> fragments = rasterize(triangles);

    // 4. Fragment Shader
    for (const auto& fragment : fragments) {
        // En este caso, el fragment shader simplemente asigna un color constante a cada fragmento
        Color fragColor = fragmentShader(fragment);
        setColor(fragColor);

        // Dibujamos el píxel en la pantalla
        point(fragment.position.x, fragment.position.y);
    }

    // Mostramos los cambios en pantalla
    SDL_RenderPresent(renderer);
}

glm::mat4 createModelMatrix(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Aplicar traslación
    modelMatrix = glm::translate(modelMatrix, translation);

    // Aplicar rotación
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Aplicar escala
    modelMatrix = glm::scale(modelMatrix, scale);

    return modelMatrix;
}

// Función para leer el archivo .obj y cargar los vértices y caras
bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<Face>& out_faces) {
    out_vertices.clear();
    out_faces.clear();

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            out_vertices.push_back(vertex);
        } else if (type == "f") {
            std::string lineHeader;
            Face face;
            while (iss >> lineHeader)
            {
                std::istringstream tokenstream(lineHeader);
                std::string token;
                std::array<int, 3> vertexIndices;

                // Read all three values separated by '/'
                for (int i = 0; i < 3; ++i) {
                    std::getline(tokenstream, token, '/');
                    vertexIndices[i] = std::stoi(token) - 1;
                }

                face.vertexIndices.push_back(vertexIndices);
            }
            out_faces.push_back(face);
        }
    }

    file.close();
    return true;
}

glm::mat4 createViewMatrix(const Camera& camera) {
    return glm::lookAt(camera.cameraPosition, camera.targetPosition, camera.upVector);
}

glm::mat4 createProjectionMatrix() {
    float fovInDegrees = 120.0f;
    float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
    float nearClip = 0.1f;
    float farClip = 100.0f;

    return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale to adjust the aspect ratio
    float scaleX = 2.0f / static_cast<float>(SCREEN_WIDTH);
    float scaleY = 2.0f / static_cast<float>(SCREEN_HEIGHT);
    viewport = glm::scale(viewport, glm::vec3(scaleX, scaleY, 1.0f));

    // Translate to adjust the origin
    viewport = glm::translate(viewport, glm::vec3(-1.0f, -1.0f, 0.0f));

    return viewport;
}

std::vector<glm::vec3> setupVertexArray(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces) {
    std::vector<glm::vec3> vertexArray;

    // For each face
    for (const auto& face : faces) {
        // For each vertex in the face
        for (const auto& vertexIndices : face.vertexIndices) {
            // Get the vertex position from the input array using the indices from the face
            glm::vec3 vertexPosition = vertices[vertexIndices[0]];

            // Add the vertex position to the vertex array
            vertexArray.push_back(vertexPosition);
        }
    }

    return vertexArray;
}

int main(int argc, char* argv[]) {
    init();

    std::string currentPath = getCurrentPath();
    std::string fileName = "naveLab3.obj";
    std::string filePath = getParentDirectory(currentPath) + "\\" + fileName;

    bool success = loadOBJ(filePath, vertices, faces);

    if (!success) {
        std::cerr << "Error: Unable to load OBJ file " << filePath << std::endl;
        return -1;
    }

    std::vector<glm::vec3> vertexArray = setupVertexArray(vertices, faces);

    /* vertices = {
            {300.0f, 200.0f, 0.0f},
            {400.0f, 400.0f, 0.0f},
            {500.0f, 200.0f, 0.0f}
    };*/

    // Definir la cámara en el mundo
    Camera camera;
    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f); // Posición de la cámara
    camera.targetPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Punto al que la cámara apunta
    camera.upVector = glm::vec3(0.0f, 1.0f, 0.0f);       // Vector que apunta hacia arriba

    // Crear las matrices de modelo, vista, proyección y viewport
    float multiplicador = 10000.0f;
    float translationX = 170.0f;
    float translationY = 40.0f;
    float translationZ = 0.0f;

    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;

    float scaleX = 10.0f;
    float scaleY = 10.0f;
    float scaleZ = 0.0f;

    translationX = translationX * multiplicador;
    translationY = translationY * multiplicador;
    translationZ = translationZ * multiplicador;
    /*
    rotationX = rotationX * multiplicador;
    rotationY = rotationY * multiplicador;
    rotationZ = rotationZ * multiplicador;
     */
    scaleX = scaleX * multiplicador;
    scaleY = scaleY * multiplicador;
    scaleZ = scaleZ * multiplicador;

    glm::vec3 translation(translationX, translationY, translationZ); // No se aplica traslación
    glm::vec3 rotation(rotationX, rotationY, rotationZ);    // No se aplica rotación
    glm::vec3 scale(scaleX, scaleY, scaleZ);       // No se aplica escala

    glm::mat4 modelMatrix = createModelMatrix(translation, rotation, scale);
    glm::mat4 viewMatrix = createViewMatrix(camera);
    glm::mat4 projectionMatrix = createProjectionMatrix();
    glm::mat4 viewportMatrix = createViewportMatrix();

    // Crear la estructura de uniformes y asignar las matrices
    Uniforms uniforms;
    uniforms.model = modelMatrix;
    uniforms.view = viewMatrix;
    uniforms.projection = projectionMatrix;
    uniforms.viewport = viewportMatrix;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render(vertexArray, uniforms); // Renderizar el triángulo con las matrices de transformación

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}