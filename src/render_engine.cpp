#include "render_engine.h"
#include <iostream>
#include <numbers> // For std::numbers::pi_v
#include <cmath>

RenderEngine::RenderEngine(int w, int h, SDL_Renderer* r) {
    // Constructor - Sets variables to given values

    this->width = w;
    this->height = h; 
    this->renderer = r;
}

float RenderEngine::to_radians(float degrees) {
    // Simple converter from degress to radians

    return degrees * (std::numbers::pi_v<float> / 180.0f);
}

float RenderEngine::normalize_axis(Sint16 value) {
    // A function which normalizes the given xbox 
    // controller axis values and normalizes them to (from 0 to 1) 0.x
    // returns a float value

    return static_cast<float>(value) / 32768.0f;
}

void RenderEngine::point(Point_2D p) {
    // this function draws a pre-set size point 
    // onto the screen - BUT the coordinates are centered
    // meaning the center point of the screen is the origin
    // of the screen - and it spreads as a floating value
    // from -1 to 1 on each axis

    // basically a normalizing function to Cartesian coordinate system
    // with the origin at the center of the screen

    const float scale = 25.0f;

    SDL_SetRenderDrawColor(renderer, 70, 255, 70, 255); 
    SDL_FRect rect = {  p.x - scale/2, p.y - scale/2, scale, scale}; 
    SDL_RenderFillRect(renderer, &rect);
}

RenderEngine::Point_2D RenderEngine::project(const Point_3D& p) {
    Point_2D new_point;
    new_point.x = p.x / p.z;
    new_point.y = p.y / p.z;

    return new_point;
}

RenderEngine::Point_2D RenderEngine::screen(const Point_2D& p) {
    Point_2D sp;
    sp.x = ((p.x + 1)/2) * width;
    sp.y = (1 - (p.y + 1)/2) * height;

    return sp;
} 

void RenderEngine::draw_thick_line(Point_2D p1, Point_2D p2, float thickness) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length <= 0.0f) return;

    // Normal vector for thickness
    float nx = -dy / length * (thickness / 2.0f);
    float ny = dx / length * (thickness / 2.0f);

    SDL_Vertex verts[4];
    SDL_FColor col = { 0.07f, 1.0f, 0.07f, 1.0f }; // SDL3 uses 0.0-1.0 for FColor

    // Vertex 0
    verts[0].position.x = p1.x + nx;
    verts[0].position.y = p1.y + ny;
    verts[0].color = col;

    // Vertex 1
    verts[1].position.x = p1.x - nx;
    verts[1].position.y = p1.y - ny;
    verts[1].color = col;

    // Vertex 2
    verts[2].position.x = p2.x + nx;
    verts[2].position.y = p2.y + ny;
    verts[2].color = col;

    // Vertex 3
    verts[3].position.x = p2.x - nx;
    verts[3].position.y = p2.y - ny;
    verts[3].color = col;

    int indices[6] = { 0, 1, 2, 2, 1, 3 };

    SDL_RenderGeometry(renderer, nullptr, verts, 4, indices, 6);
}

RenderEngine::Point_3D RenderEngine::rotate_roll(const Point_3D& p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return { p.x * c - p.y * s, p.x * s + p.y * c, p.z };
}

RenderEngine::Point_3D RenderEngine::rotate_pitch(const Point_3D& p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return { p.x, p.y * c - p.z * s, p.y * s + p.z * c };
}

RenderEngine::Point_3D RenderEngine::rotate_yaw(const Point_3D& p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return { p.x * c + p.z * s, p.y, -p.x * s + p.z * c };
}

