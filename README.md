# Proposal

### Group Members: Alexis Ward, Joshua Nadel, Tabatha Hickman

## Overview:

We were very impressed by the abilities of DXR, and we realized Josh’s old Procedural Creature Generator could really benefit from it. These creatures are created from an assortment of raymarched SDF primitives. View the readme, below, for more information. It has plenty of potential for cool new features that we can now expand on by utilizing modern day GPUs.

This project will explore the possibilities of what DXR can do by experimenting with multiple shader passes, complex meshes (from imported obj files), mouse click ray tracing for user interface, and more (depending on how much time we have). 

## Features:
* Procedural characters represented by metaballs and other SDFs raytraced in DirectX
* Mesh rasterizer (performed using marching cubes) to convert procedural character into exportable mesh
* Automatic UV unwrapping
* AO and curvature baker to drive texturing
* User interface for character customization
* GLTF exporter
* Bonus feature: skeleton generation and export

## Milestones:
* 11/18: Convert original project to C++ and DirectX, identify possible optimizations (acceleration structures, reduce amount of buffers)
* 11/25: Research and begin implementation of mesh rasterization, ambient occlusion baker
* 12/2: Complete mesh rasterization, begin UV unwrapper, add curvature baker
* 12/9: User customization, GLTF exporter

## References:
https://github.com/nmagarino/Pudgy-Pals-Procedural-Creature-Generator

https://github.com/CIS565-Fall-2019/Project5-DirectX-Procedural-Raytracing

http://chrishecker.com/My_liner_notes_for_spore#Creature_Skin_Mesh
