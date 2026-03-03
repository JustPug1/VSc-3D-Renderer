#include <SDL3/SDL.h>
#include <iostream> 
#include <cmath>
#include <numbers>
#include "render_engine.h"
#include "sdl_engine.h"

const int FPS = 120;

void draw_drone_with_engine(RenderEngine& engine, float delta_x, float delta_y, float delta_z, float pitch_deg, float yaw_deg, float roll_deg) {
    float width = (float)engine.get_width();
    float height = (float)engine.get_height();
    float aspect = width / height;
    float screen_scale = height / 1000.0f; 
    
    float rad_pitch = engine.to_radians(pitch_deg);
    float rad_yaw   = engine.to_radians(yaw_deg);
    float rad_roll  = engine.to_radians(roll_deg);

    auto transform = [&](RenderEngine::Point_3D p) {
        // 1. Rotation (Local Space)
        RenderEngine::Point_3D r = engine.rotate_roll(p, rad_roll);
        r = engine.rotate_pitch(r, rad_pitch);
        r = engine.rotate_yaw(r, rad_yaw);
        
        // 2. Translation (World Space)
        // We add the offsets to move the object in the 3D world
        r.x += delta_x;
        r.y += delta_y;
        r.z += delta_z; 

        // 3. Projection & Aspect Correction
        RenderEngine::Point_2D projected = engine.project(r);
        projected.x /= aspect; 

        return engine.screen(projected);
    };

    // --- REST OF THE DRAWING LOGIC REMAINS THE SAME ---
    static const float base = 0.15f; 
    static const float bw = base * 0.5f, bh = base * 0.3f, bl = base * 1.2f;
    
    // Body
    RenderEngine::Point_3D body_local[8] = {
        {bw,bh,bl}, {bw,-bh,bl}, {-bw,bh,bl}, {-bw,-bh,bl},
        {bw,bh,-bl}, {bw,-bh,-bl}, {-bw,bh,-bl}, {-bw,-bh,-bl}
    };
    
    RenderEngine::Point_2D body_screen[8];
    for(int i=0; i<8; i++) body_screen[i] = transform(body_local[i]);

    float body_thickness = 2.0f * screen_scale;
    float arm_thickness = 15.0f * screen_scale;
    float arrow_thickness = 6.0f * screen_scale; 
    float ring_thickness = 1.5f * screen_scale;

    static const int b_edges[12][2] = {{0,1},{2,3},{4,5},{6,7},{0,2},{2,6},{6,4},{4,0},{1,3},{3,7},{7,5},{5,1}};
    for(const auto& e : b_edges) 
        engine.draw_thick_line(body_screen[e[0]], body_screen[e[1]], body_thickness);

    // Front Arrow (Center of front face)
    RenderEngine::Point_2D arrow_base = transform({0.0f, 0.0f, bl});
    RenderEngine::Point_2D arrow_tip  = transform({0.0f, 0.0f, bl + (base * 1.5f)});
    RenderEngine::Point_2D arrow_left  = transform({-base * 0.4f, 0.0f, bl + (base * 0.8f)});
    RenderEngine::Point_2D arrow_right = transform({base * 0.4f, 0.0f, bl + (base * 0.8f)});

    engine.draw_thick_line(arrow_base, arrow_tip, arrow_thickness);
    engine.draw_thick_line(arrow_tip, arrow_left, arrow_thickness);
    engine.draw_thick_line(arrow_tip, arrow_right, arrow_thickness);

    // Arms and Motors
    RenderEngine::Point_2D center_screen = transform({0.0f, 0.0f, 0.0f});
    static const RenderEngine::Point_3D motors[4] = {
        {base*5, 0, base*3.5}, {-base*5, 0, base*3.5}, 
        {base*5, 0, -base*3.5}, {-base*5, 0, -base*3.5}
    };

    for(int i=0; i<4; i++) {
        RenderEngine::Point_2D m_pos = transform(motors[i]);
        engine.draw_thick_line(center_screen, m_pos, arm_thickness);

        for(int s=0; s<16; s++) {
            float a1 = (float)s * (2.0f * M_PI / 16.0f);
            float a2 = (float)(s+1) * (2.0f * M_PI / 16.0f);
            auto get_ring_pt = [&](float angle, float h) {
                return transform({
                    motors[i].x + std::cos(angle) * (base * 0.5f), 
                    motors[i].y + h * base, 
                    motors[i].z + std::sin(angle) * (base * 0.5f)
                });
            };
            engine.draw_thick_line(get_ring_pt(a1, 0.2f), get_ring_pt(a2, 0.2f), ring_thickness); 
            engine.draw_thick_line(get_ring_pt(a1, -0.2f), get_ring_pt(a2, -0.2f), ring_thickness); 
            engine.draw_thick_line(get_ring_pt(a1, 0.2f), get_ring_pt(a1, -0.2f), ring_thickness); 
        }
    }
}

int main() {
    SDL_Engine sdl_obj("window", 1800, 1300, SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD, SDL_WINDOW_RESIZABLE);
    RenderEngine engine(1800, 1300, sdl_obj.renderer);
    SDL_Gamepad* controller = sdl_obj.Connect_First_Controller();

    float delta_x = 0.2f;
    float delta_y = 0.2f;
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
            yaw_cmd += engine.normalize_axis(SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFTX)) * yaw_rate;  // some bugs still
            //normalize_axis(SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFTY));
        }

        SDL_SetRenderDrawColor(sdl_obj.renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl_obj.renderer);

        // --- SIMULATE RECEIVED DATA ---
        time += 0.01f;
        // Delta X

        // Delta Y 
        //delta_y += 0.01f;

        // Pitch: Nod up and down
        pitch_cmd = std::sin(time) * 100.0f; 
        // Yaw: Slowly spin around
        yaw_cmd += 1.0f; 
        // Roll: Wobble left and right
        roll_cmd = std::cos(time * 2.0f) * 100.0f;

        // Pass the angles to the function
        draw_drone_with_engine(engine, delta_x, delta_y, delta_z, pitch_cmd, yaw_cmd, roll_cmd);

        SDL_RenderPresent(sdl_obj.renderer);
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(sdl_obj.renderer);
    SDL_DestroyWindow(sdl_obj.window);
    SDL_Quit();
    return 0;
}
