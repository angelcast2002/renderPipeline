#include <fstream>
#include <vector>
#include "glm/glm.hpp"
#include <array>
#include <SDL.h>
#include <sstream>
#include <filesystem>
#include <iostream>
#pragma once

// Define a Color struct to hold the RGB values of a pixel
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct Face {
    std::vector<std::array<int, 3>> vertexIndices;
};

struct Fragment {
    glm::ivec2 position; // X and Y coordinates of the pixel (in screen space)
    // Other interpolated attributes (e.g., color, texture coordinates, normals) can be added here

    Fragment() : position(glm::ivec2(0, 0)) {}
    Fragment(int x, int y) : position(glm::ivec2(x, y)) {}
    Fragment(const glm::ivec2& pos) : position(pos) {}
};

struct Uniforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

std::string getCurrentPath() {
    return std::filesystem::current_path().string();
}

std::string getParentDirectory(const std::string& path) {
    std::filesystem::path filePath(path);
    return filePath.parent_path().string();
}

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
const int SCREEN_WIDTH = 640;
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

void triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    line(A, B);
    line(B, C);
    line(C, A);
}

void render(const std::vector<glm::vec3>& vertices, const Uniforms& uniforms) {
    // 1. Vertex Shader
    std::vector<Fragment> fragments;
    for (const auto& vertex : vertices) {
        // Aquí aplicamos la transformación del vértice utilizando los uniforms
        // Por ahora, simplemente agregamos el vértice a los fragmentos para su posterior procesamiento
        fragments.push_back(Fragment(vertex));
    }

    // 2. Primitive Assembly
    // Por ahora, como estamos trabajando con triángulos, no es necesario hacer nada aquí,
    // ya que los triángulos están formados por tres vértices y ya tenemos los fragmentos correspondientes.

    // 3. Rasterization
    // En este paso, normalmente se convierten los triángulos a píxeles en la pantalla,
    // pero como ya tenemos los fragmentos, no es necesario hacer nada aquí por ahora.

    // 4. Fragment Shader
    // Aquí aplicamos el fragment shader para procesar los fragmentos y determinar el color de cada píxel.
    // Como por ahora solo tenemos la posición del fragmento, simplemente pintamos los píxeles en el framebuffer.

    for (const auto& fragment : fragments) {
        point(fragment.position.x, fragment.position.y);
    }
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

    std::vector<glm::vec3> vertices = {
            {300.0f, 200.0f, 0.0f},
            {400.0f, 400.0f, 0.0f},
            {500.0f, 200.0f, 0.0f}
    };

    Uniforms uniforms;

    glm::mat4 model = glm::mat4(1);
    glm::mat4 view = glm::mat4(1);
    glm::mat4 projection = glm::mat4(1);

    uniforms.model = model;
    uniforms.view = view;
    uniforms.projection = projection;

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

        render(vertices, uniforms);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
