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
    // Draws a 25x25 pixel square at the provided screen coordinates.
    // Maps the logic from a centered Cartesian system (-1 to 1) 
    // to the actual SDL rendering window.
    // Uses SDL_FRect for floating-point precision when positioning the point.

    const float scale = 25.0f;

    SDL_SetRenderDrawColor(renderer, 70, 255, 70, 255); 
    SDL_FRect rect = {  p.x - scale/2, p.y - scale/2, scale, scale}; 
    SDL_RenderFillRect(renderer, &rect);
}

RenderEngine::Point_2D RenderEngine::project(const Point_3D& p) {
    // Perspective Projection: Maps a 3D point (x, y, z) to a 2D plane.
    // We divide X and Y by Z to simulate how objects appear smaller 
    // as they get further away from the camera.

    Point_2D new_point;
    new_point.x = p.x / p.z;
    new_point.y = p.y / p.z;

    return new_point;
}

RenderEngine::Point_2D RenderEngine::screen(const Point_2D& p) {
    // Maps normalized coordinates (-1 to 1) to actual screen pixels (0 to width/height).
    // It also flips the Y-axis because screen pixels start at the top (0) and go down, 
    // whereas mathematical 3D space usually goes from bottom up.

    Point_2D sp;
    sp.x = ((p.x + 1)/2) * width;
    sp.y = (1 - (p.y + 1)/2) * height;

    return sp;
} 

void RenderEngine::draw_thick_line(Point_2D p1, Point_2D p2, float thickness) {
    /**
    * Renders a line with custom thickness using GPU geometry.
    * 
    * Process:
    * 1. Calculates the line direction (dx, dy) and its length.
    * 2. Finds the 'Normal' vector (perpendicular) to the line.
    * 3. Offsets the start and end points by this normal to create 4 corners.
    * 4. Uses SDL_RenderGeometry to draw two triangles forming a thick rectangle.
    */

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
    // Rotates the point around the Z-axis (tilting the drone left or right).
    // Affects X and Y coordinates while Z remains unchanged.

    float c = std::cos(angle);
    float s = std::sin(angle);
    return { p.x * c - p.y * s, p.x * s + p.y * c, p.z };
}

RenderEngine::Point_3D RenderEngine::rotate_pitch(const Point_3D& p, float angle) {
    // Rotates the point around the X-axis (tilting the nose up or down).
    // Affects Y and Z coordinates while X remains unchanged.

    float c = std::cos(angle);
    float s = std::sin(angle);
    return { p.x, p.y * c - p.z * s, p.y * s + p.z * c };
}

RenderEngine::Point_3D RenderEngine::rotate_yaw(const Point_3D& p, float angle) {
    // Rotates the point around the Y-axis (turning the nose left or right).
    // Affects X and Z coordinates while Y remains unchanged.

    float c = std::cos(angle);
    float s = std::sin(angle);
    return { p.x * c + p.z * s, p.y, -p.x * s + p.z * c };
}
