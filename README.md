# softlight

Generative visuals toolkit in **C++ + GLSL** for live audio visual album instillation, "spinning through softlight" in the AlloSphere.
https://youtu.be/iq5s6EcoII4?si=OcqIv_SzAm7NA1MY

## Features
- **Shader library**: modular GLSL snippets for materials, post, and FX.
- **Mesh effects**: vertex/geometry manipulations for organic motion.
- **Scenes & examples**: ready-to-run references you can adapt quickly.
- **Asset pack**: textures and utilities for iteration & testing.

## Repo layout
assets/ # images, LUTs, reference media
eoys-mesh-fx/ # mesh effect studies (vertex/geometry experiments)
eoys-shader/ # shader studies (materials / post / compositing)
examples/ # minimal example apps / sketches
scenes/ # assembled scene presets
shaders/ # reusable GLSL includes/snippets
utility/ # small helpers (headers, math, loaders)

markdown
Copy code

## Getting started
This repository is intended to be **consumed from a host app**. You can:
1. Copy the relevant folders (`shaders/`, `utility/`, any `examples/…`) into your app, or add this repo as a submodule.
2. Include the GLSL snippets from `shaders/` and compile them in your engine/framework of choice.
3. Start by opening an item in `examples/` and substituting your windowing/graphics boilerplate.

> Tip: keep shader hot-reload on while iterating.

## Roadmap
- Audio-reactive parameters (envelope/spectral features)
- More mesh FX presets and scene timelines
- Packaging as a header-only “fx” library
