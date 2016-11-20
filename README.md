# PracticeRender
Rendering, animation system, supporting systems, using minimal external libraries and built from scratch.

# Rendering core engine
* Basic abstraction layer between opengl and render engine.

# Simple Skeletal Animation
* 4 bones per vertex using a weighted average
* Hierarchical skeletal structure used to generate per-bone skinning matrices 
* Basic support for per-animation
* Support for loading Valve's SMD format

# Text rendering
* stb_truetype for ttf files that are rendered from a custom texture packing system.

# Demo

Here's a super cheesy demo showing a single bone animation over a textured mesh loaded from an SMD file (see cubeanim.smd) with some basic font rendering.
More coming soon..

https://www.youtube.com/watch?v=pWQDgH5KnAA
