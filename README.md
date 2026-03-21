# MyEngine

A custom 3D game engine built with **C++** and **OpenGL**.

## 🛠️ Tech Stack
* **Language:** C++
* **Graphics API:** OpenGL
* **Windowing/Input:** GLFW
* **Loading:** GLAD
* **UI:** Dear ImGui

---

## 🏗️ Core Architecture
The engine is built with a data-oriented mindset and a custom **Entity-Component-System (ECS)** to separate data from logic, maximize performance, and remain flexible for experimentation and iteration.

### 1. Entity Component System (ECS)
The **Coordinator** acts as the central manager, orchestrating the entity lifecycle, component storage, and system execution.

* **Entities**
  * Lightweight unique identifiers (`uint32_t`).
  * Contain no data themselves; they serve as IDs for attaching components.
* **Components**
  * Plain-old-data (POD) structs holding entity data.
  * *Examples:* `TransformComponent`, `MeshComponent`, `LightComponent`, `MaterialComponent`.
* **Systems**
  * Contain the logic and operate on entities matching a specific component signature.
  * *Examples:* `RenderSystem`, `PhysicsSystem`, `CameraSystem`.
* **Coordinator Responsibilities**
  * Manages entity creation and destruction.
  * Handles component attachment (stored in cache-friendly densely packed arrays).
  * Manages deterministic system registration and execution.

### 2. Asset Management & Serialization
To optimize performance, the engine implements a custom binary caching system.

* **Mesh Import:** Raw `.obj` files are parsed for vertices, normals, UVs, and calculated tangents for normal mapping.
* **Binary Caching (`.memesh`):** Processed mesh data is serialized into a custom binary format to bypass expensive parsing on subsequent loads.
* **Texture Management:** Utilizes `stb_image` for loading with support for runtime mipmap generation and anisotropic filtering.

### 3. Rendering Pipeline
Uses **Modern OpenGL (3.3+)** with a forward rendering path.

* **Pass 1 — Depth Pass (Shadow Mapping):** Renders the scene from the light's perspective into an FBO to generate a depth map.
* **Pass 2 — Lighting Pass:** Renders the final scene using information from the depth pass.
* **Lighting & Effects:** Implements **Blinn–Phong** lighting.
  * Uses **TBN matrices** for high-fidelity normal mapping.

### 4. Physics & Collisions
A custom rigid body physics system built from scratch.
* **Rigid Body Simulation:** Supports gravity, velocity, acceleration, and configurable damping. Bodies can be marked as static, kinematic, or fully dynamic with individual mass and gravity scale.
* **Collision Detection:**
  * **Broadphase:** AABB overlap test as a fast-reject pass before narrowphase.
  * **Narrowphase:** SAT (Separating Axis Theorem) for Box-Box, distance check for Sphere-Sphere, and closest-point clamping for Sphere-Box.
* **Collision Resolution:** MTV (Minimum Translation Vector) positional correction with mass-weighted push-out and restitution-based velocity reflection.
* **Terrain Collision:** Dynamic height sampling against the terrain mesh with per-collider ground offset.
* **Trigger Volumes:** Colliders can be marked as triggers — they fire callbacks without applying physical resolution.

### 5. UI System
A custom UI system built with OpenGL and FreeType, separate from the editor UI (Dear ImGui).
* **Text Rendering:** FreeType-based glyph rasterization with per-character texture atlas, bearing/advance metrics, and newline support. Batches glyphs by texture ID to minimize draw calls.
* **UI Components:** ECS-driven — entities have `UIBaseComponent` (position, z-order, visibility), `UITextComponent`, and `UIButtonComponent`.
* **Button Interaction:** Per-frame hit-testing against mouse position with three visual states — normal, hovered, and pressed — each with configurable colors.
* **Lua Scripting:** Button click events call `OnButtonClicked` function into the attached Lua script via `sol2`, keeping gameplay UI logic fully scriptable.

### 6. Scripting System
* Lua scripting via `sol2` with per-entity environment and hot-reloading at runtime. Scripts expose `OnCreate` and `OnUpdate` and full access to components of entity.
---

## 📂 Project Structure
* **EngineAssets/**: Core engine resources, including icons and basic models.
* **External/**: Third-party dependencies (GLFW, GLAD, ImGui).
* **Header Files/**: Engine and ECS architecture headers.
* **Shaders/**: GLSL source files for rendering and shadow mapping.
* **Source Files/**: Implementation of ECS logic, systems, and UI panels.
---

## Build
### Prerequisites
* CMake 3.15+
* C++17 compiler (Clang / GCC / MSVC)

### Steps
```bash
git clone https://github.com/yourusername/MyEngine.git
cd MyEngine
mkdir build && cd build
cmake ..
cmake --build . --config Debug
```
