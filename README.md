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

---

## 📂 Project Structure
* **EngineAssets/**: Core engine resources, including icons and basic models.
* **External/**: Third-party dependencies (GLFW, GLAD, ImGui).
* **Header Files/**: Engine and ECS architecture headers.
* **Shaders/**: GLSL source files for rendering and shadow mapping.
* **Source Files/**: Implementation of ECS logic, systems, and UI panels.

---
