#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <array>
#include <iostream>

// Define a Color struct to hold the RGB values of a pixel
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

// Define the Fragment struct here
struct Fragment {
    glm::ivec2 position; // X and Y coordinates of the pixel (in screen space)
    // Other interpolated attributes (e.g., color, texture coordinates, normals) can be added here

    Fragment() : position(glm::ivec2(0, 0)) {}
    Fragment(int x, int y) : position(glm::ivec2(x, y)) {}
    Fragment(const glm::ivec2& pos) : position(pos) {}
};

struct Face {
    std::vector<std::array<int, 3>> vertexIndices;
};

struct Uniforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct Camera {
    glm::vec3 cameraPosition;
    glm::vec3 targetPosition;
    glm::vec3 upVector;
};

std::string getCurrentPath();

std::string getParentDirectory(const std::string& path);

void setColor(const Color& color);

void clear();

void point(int x, int y);

void line(glm::vec3 start, glm::vec3 end);

int min3(int a, int b, int c);

int max3(int a, int b, int c);

std::vector<Fragment> triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C);

std::vector<std::vector<glm::vec3>> primitiveAssembly(const std::vector<glm::vec3>& transformedVertices);

std::vector<Fragment> rasterize(const std::vector<std::vector<glm::vec3>>& assembledVertices);

Color fragmentShader(const Fragment& fragment);

glm::vec3 vertexShader(const glm::vec3& vertex, const Uniforms& uniforms);

bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<Face>& out_faces);

std::vector<glm::vec3> setupVertexArray(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces);
