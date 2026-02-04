#include <SDL3/SDL.h>
#include <iostream> 
#include <cmath>
#include <numbers>

const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 1500;
const int FPS = 60;
const float PI = std::numbers::pi_v<float>;

struct Point_2D {
    float x;
    float y;
};

struct Point_3D {
    float x;
    float y;
    float z;
};

struct Edge {
    int start;
    int end;
};

Point_3D drone_verts[] = {
    // --- Central Stack (Tiny box) ---
    { 0.05f,  0.03f,  0.1f}, { 0.05f, -0.03f,  0.1f}, // Front-Right (0, 1)
    {-0.05f,  0.03f,  0.1f}, {-0.05f, -0.03f,  0.1f}, // Front-Left  (2, 3)
    { 0.05f,  0.03f, -0.1f}, { 0.05f, -0.03f, -0.1f}, // Back-Right   (4, 5)
    {-0.05f,  0.03f, -0.1f}, {-0.05f, -0.03f, -0.1f}, // Back-Left    (6, 7)

    // --- Motor 1 (Front-Right) ---
    { 0.40f,  0.06f,  0.30f}, { 0.44f,  0.06f,  0.34f}, // Top circle points (approx)
    { 0.40f,  0.00f,  0.30f}, { 0.44f,  0.00f,  0.34f}, // Bottom circle points

    // ... you can repeat this for all 4 motors, but to keep the code 
    // clean, let's use a "Motor" generator logic in the loop.
};

// Helper to convert Degrees (from drone) to Radians (for C++)
float to_radians(float degrees) {
    return degrees * (std::numbers::pi_v<float> / 180.0f);
}

//Helper to normalize axis input to -1.0 to 1.0 range
float normalize_axis(Sint16 value) {
    return static_cast<float>(value) / 32768.0f;
}

// Rotates around X-axis (PITCH) - Affects Y and Z
Point_3D rotate_pitch(Point_3D p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    Point_3D rotated = p;
    rotated.y = p.y * c - p.z * s;
    rotated.z = p.y * s + p.z * c;
    return rotated;
}

// Rotates around Y-axis (YAW) - Affects X and Z (This was your rotate_xz)
Point_3D rotate_yaw(Point_3D p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    Point_3D rotated = p;
    rotated.x = p.x * c - p.z * s;
    rotated.z = p.x * s + p.z * c;
    return rotated;
}

// Rotates around Z-axis (ROLL) - Affects X and Y
Point_3D rotate_roll(Point_3D p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    Point_3D rotated = p;
    rotated.x = p.x * c - p.y * s;
    rotated.y = p.x * s + p.y * c;
    return rotated;
}

void point(Point_2D p, SDL_Renderer* renderer) {
    const float scale = 25.0f;

    SDL_SetRenderDrawColor(renderer, 70, 255, 70, 255); 
    SDL_FRect rect = {  p.x - scale/2, p.y - scale/2, scale, scale}; 
    SDL_RenderFillRect(renderer, &rect);
}

Point_2D screen(Point_2D p) {
    p.x = ((p.x + 1)/2) * WINDOW_WIDTH;
    p.y = (1 - (p.y + 1)/2) * WINDOW_HEIGHT;

    return p;
}

Point_2D project(Point_3D p) {
    Point_2D new_point;
    new_point.x = p.x / p.z;
    new_point.y = p.y / p.z;

    return new_point;
}

Point_3D rotate_xz(Point_3D p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    Point_3D rotated;
    rotated.x = p.x * c - p.z * s;
    rotated.z = p.x * s + p.z * c;
    rotated.y = p.y;

    return rotated;
}

void draw_thick_line(SDL_Renderer* renderer, Point_2D p1, Point_2D p2, float thickness) {
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

void frame(float delta_z, float delta_angle, SDL_Renderer* renderer) {
    Point_3D vertices[8] = {
        { 0.2f,  0.2f,  0.2f}, { 0.2f, -0.2f,  0.2f}, // 0, 1
        {-0.2f,  0.2f,  0.2f}, {-0.2f, -0.2f,  0.2f}, // 2, 3
        { 0.2f,  0.2f, -0.2f}, { 0.2f, -0.2f, -0.2f}, // 4, 5
        {-0.2f,  0.2f, -0.2f}, {-0.2f, -0.2f, -0.2f}  // 6, 7
    };

    Point_2D projected_points[8];

    // 1. Transform and Project all points
    for (int i = 0; i < 8; i++) {
        Point_3D rotated = rotate_xz(vertices[i], delta_angle);
        rotated.z += delta_z;
        projected_points[i] = screen(project(rotated));
        
        // Optional: Keep drawing the points/vertices
        //point(projected_points[i], renderer);
    }

    // 2. Connect them with lines
    SDL_SetRenderDrawColor(renderer, 70, 255, 70, 255); // White lines
    
    Edge edges[12] = {
        {0, 1}, {1, 3}, {3, 2}, {2, 0},
        {4, 5}, {5, 7}, {7, 6}, {6, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (int i = 0; i < 12; i++) {
        Point_2D p1 = projected_points[edges[i].start];
        Point_2D p2 = projected_points[edges[i].end];
        
        // SDL3 uses SDL_RenderLine(renderer, x1, y1, x2, y2)
        draw_thick_line(renderer, p1, p2, 4.0f); // 4.0f is the thickness in pixels
    }
}

void fpv_drone_optimized(float delta_z, float pitch_deg, float yaw_deg, float roll_deg, SDL_Renderer* renderer) {
    // --- 1. CONVERT INPUTS TO RADIANS ---
    float rad_pitch = to_radians(pitch_deg);
    float rad_yaw   = to_radians(yaw_deg);
    float rad_roll  = to_radians(roll_deg);

    // --- 2. STATIC DATA (Same as before) ---
    static const int segments = 16;
    static float circle_sin[segments];
    static float circle_cos[segments];
    static bool initialized = false;

    if (!initialized) {
        for (int i = 0; i < segments; i++) {
            float angle = (float)i * 2.0f * std::numbers::pi_v<float> / segments;
            circle_sin[i] = std::sin(angle);
            circle_cos[i] = std::cos(angle);
        }
        initialized = true;
    }

    static const float bw = 0.05f, bh = 0.03f, bl = 0.12f;
    static const Point_3D body_local[8] = {
        {bw, bh, bl}, {bw, -bh, bl}, {-bw, bh, bl}, {-bw, -bh, bl},
        {bw, bh, -bl}, {bw, -bh, -bl}, {-bw, bh, -bl}, {-bw, -bh, -bl}
    };
    static const Point_3D motor_offsets[4] = {
        { 0.4f, 0.0f,  0.3f}, {-0.4f, 0.0f,  0.3f},
        { 0.4f, 0.0f, -0.3f}, {-0.4f, 0.0f, -0.3f}
    };

    // --- 3. UPDATED TRANSFORM LAMBDA ---
    auto transform = [&](Point_3D p) {
        // Apply rotations in sequence
        Point_3D r = p;
        
        // 1. Roll (Tilt left/right)
        r = rotate_roll(r, rad_roll);
        
        // 2. Pitch (Tilt nose up/down)
        r = rotate_pitch(r, rad_pitch);
        
        // 3. Yaw (Spin left/right)
        r = rotate_yaw(r, rad_yaw);

        // 4. Move into distance
        r.z += delta_z;

        return screen(project(r));
    };

    // --- 4. DRAWING (Same as before) ---
    Point_2D body_screen[8];
    for(int i=0; i<8; i++) body_screen[i] = transform(body_local[i]);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    static const int b_edges[12][2] = {{0,1},{2,3},{4,5},{6,7},{0,2},{2,6},{6,4},{4,0},{1,3},{3,7},{7,5},{5,1}};
    for(const auto& e : b_edges) 
        draw_thick_line(renderer, body_screen[e[0]], body_screen[e[1]], 2.0f);

    Point_2D center_screen = transform({0.0f, 0.0f, 0.0f});
    float mr = 0.045f, mh = 0.03f;  

    for(int i=0; i<4; i++) {
        Point_2D motor_center_screen = transform(motor_offsets[i]);
        draw_thick_line(renderer, center_screen, motor_center_screen, 10.0f); 

        Point_2D top_ring[segments], bot_ring[segments];
        for(int s=0; s<segments; s++) {
            float dx = circle_cos[s] * mr;
            float dz = circle_sin[s] * mr;
            Point_3D top = {motor_offsets[i].x + dx, motor_offsets[i].y + mh, motor_offsets[i].z + dz};
            Point_3D bot = {motor_offsets[i].x + dx, motor_offsets[i].y - mh, motor_offsets[i].z + dz};
            top_ring[s] = transform(top);
            bot_ring[s] = transform(bot);
        }

        for(int s=0; s<segments; s++) {
            int next = (s + 1) % segments; 
            draw_thick_line(renderer, top_ring[s], top_ring[next], 1.5f);
            draw_thick_line(renderer, bot_ring[s], bot_ring[next], 1.5f);
            draw_thick_line(renderer, top_ring[s], bot_ring[s], 1.5f);
        }
    }
}


SDL_Gamepad* ConnectFirstAvailableGamepad() {
    int count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
    
    SDL_Gamepad* handle = nullptr;

    if (count > 0) {
        handle = SDL_OpenGamepad(gamepads[0]);
        if (handle) {
            SDL_Log("Successfully connected to: %s", SDL_GetGamepadName(handle));
        } else {
            SDL_Log("Failed to open gamepad: %s", SDL_GetError());
        }
    } else {
        SDL_Log("No gamepads detected.");
    }
    SDL_free(gamepads);
    
    return handle;
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }
    std::cout << "SDL Initialized Successfully!" << std::endl;

    SDL_Window *window = SDL_CreateWindow("SDL3 Window", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);


    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Gamepad *controller = ConnectFirstAvailableGamepad();

    float delta_z = 2.0f; // Keep it further back so we can see it
    
    // Variables to hold "Received Data" in Degrees
    float pitch_cmd = 0.0f;
    float roll_cmd = 0.0f;
    float yaw_cmd = 0.0f;
    
    float pitch_rate = 0.0f;
    float roll_rate = 0.0f;
    float yaw_rate = 10.0f;

    float time = 0.0f;

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
             if (e.type == SDL_EVENT_QUIT) {
                return 0; 
            }

            if (e.type == SDL_EVENT_GAMEPAD_REMOVED) {
             SDL_CloseGamepad(controller);
             controller = nullptr;
            }
        }
        if (!controller) {
            yaw_cmd += normalize_axis(SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFTX)) * yaw_rate;  // some bugs still
            //normalize_axis(SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFTY));
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // --- SIMULATE RECEIVED DATA ---
        time += 0.01f;
        // Pitch: Nod up and down
        pitch_cmd = std::sin(time) * 20.0f; 
        // Yaw: Slowly spin around
        //yaw_cmd += 0.5f; 
        // Roll: Wobble left and right
        roll_cmd = std::cos(time * 2.0f) * 15.0f;

        // Pass the angles to the function
        fpv_drone_optimized(delta_z, pitch_cmd, yaw_cmd, roll_cmd, renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


