--- FPV Drone 3D Renderer ---

-- Project Overview --

  This project is a high-performance 3D Wireframe Engine developed in C++ using SDL3. 
  It is designed to simulate an FPV drone interface, 
  providing a real-time visual representation of aircraft orientation (Roll, Pitch, and Yaw).
  

-- Key Features -- 

  - Perspective Projection: Converts 3D space to 2D screen coordinates with depth simulation.
  - Rotation Matrices: Implements dedicated functions for Roll, Pitch, and Yaw.
  - GPU-Accelerated Geometry: Uses triangle-based rendering for thick, high-quality lines.
  - Modular Design: Encapsulates math and SDL logic within a standalone RenderEngine class.
  

-- Technical Specifications -- 

  - Language: C++20
  - Graphics API: SDL3 (Geometry API)
  - Coordinate System: Normalized Cartesian (-1 to 1) mapped to pixel space.
  - Math: Perspective division and trigonometric rotation matrices.


-- Controls --

  - Pitch: Nose up/down tilt.
  - Roll: Left/right banking.
  - Yaw: Horizontal rotation.
  - Z-Translation: Distance/Altitude adjustment.
    

-- Build Requirements --

  - Compiler: GCC (MinGW) or MSVC with C++20 support.
  - Library: SDL3.
  - Build System: CMake.

    
