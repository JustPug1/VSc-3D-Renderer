#pragma once

#include <SDL3/SDL.h>
#include <numbers>
#include <vector>

constexpr float PI = 3.14159265358979323846f;
constexpr int WINDOW_WIDTH = 1500;
constexpr int WINDOW_HEIGHT = 1500;

class RenderEngine {
public:
    // Simple struct for (x,y) point representation
    struct Point_2D {
        float x;
        float y;
    };

    // Simple struct for (x,y,z) point representation
    struct Point_3D {
        float x;
        float y;
        float z;
    };

    // Simple struct for (2D Point to 2D Point) line representation
    struct Edge {
        int start;
        int end;
    };

private:
    int width; // screen width
    int height; // screen height
    SDL_Renderer* renderer; // The class owns this!

    // Store object data inside the class
    // std::vector<Point_3D> vertices;
    // std::vector<Edge> edges;
    

public:
    // Constructor
    RenderEngine(int w, int h, SDL_Renderer* r);

    // Helpers
    float to_radians(float degrees);
    float normalize_axis(Sint16 value);

    // Rendering Functions
    void point(Point_2D p);
    
    // Pass structs by const reference (&) to avoid copying
    Point_2D project(const Point_3D& p); 
    Point_2D screen(const Point_2D& p); 

    void draw_thick_line(Point_2D p1, Point_2D p2, float thickness);

    // Rotators
    Point_3D rotate_roll(const Point_3D& p, float angle);
    Point_3D rotate_pitch(const Point_3D& p, float angle);
    Point_3D rotate_yaw(const Point_3D& p, float angle);
};
