# TODO

- [x] Finalize the build process for the `MilkdropConverter`.
- [x] Implement the core translation logic in `MilkdropConverter.cpp`.
- [x/o] Create a comprehensive mapping from Milkdrop built-in variables to GLSL uniforms.
- [x/o] Implement a robust system for translating Milkdrop expression syntax to GLSL.
- [x/o] Handle the `q` and `t` variables correctly, ensuring data flow from per-frame to per-pixel logic.
- [x] Add support for generating UI controls (JSON annotations) for the converted shaders.
- [ ] (Stretch Goal) Investigate and implement translation for `warp` and `comp` HLSL shaders.
- [ ] (Stretch Goal) Add support for custom shapes and waves.

## Plan for Full Preset Conversion (Hybrid Approach)

**Problem:** The current converter only translates the `per_frame` and `per_pixel` logic. It completely ignores the wave and shape code, which is essential for rendering the majority of MilkDrop presets. This is why the converted shaders are only rendering a solid color instead of the complex patterns and shapes from the original presets.

**Solution:** A new hybrid approach is needed that combines the strengths of the existing converter with a manual translation of the wave and shape code.

**The Plan:**

1.  **Enhance the `Milk-Converter` to be a two-stage converter.** The converter will be modified to output two separate files for each preset:
    *   **A `.cpp` file for the per-frame logic.** The converter will translate the `per_frame` code into a C++ function. This function will take the current state (q-vars, t-vars, audio data, etc.) as input and return the updated state.
    *   **A `.frag` file for the per-pixel logic.** The converter will translate the `per_pixel` code into a standard GLSL fragment shader, as it does now. This shader will receive the `q` and `t` variables as uniforms.

2.  **Create a generic `MilkdropPresetEffect` class in RaymarchVibe.** This class will be responsible for:
    *   Loading a preset by name.
    *   Looking up the corresponding `per_frame` C++ function.
    *   Executing the `per_frame` function each frame.
    *   Passing the updated state as uniforms to the `per_pixel` shader.

3.  **Manually translate the wave and shape code.** This is the most difficult part of the plan. The wave and shape code is often the most complex part of a MilkDrop preset. It will need to be manually translated to GLSL and added to the `per_pixel` shader.

**Implementation Details:**

*   **Wave and Shape Code:** The wave and shape code is usually in sections labeled `[wave]` and `[shape]` in the `.milk` file. This code will need to be carefully analyzed and translated to GLSL. This will likely involve creating new functions in the shader to calculate the wave and shape geometry.
*   **`per_pixel` Shader:** The `per_pixel` shader will need to be modified to call the new wave and shape functions and use the results to draw the final image. This will likely involve modifying the `transformed_uv` calculation and the final color composition.
*   **`MilkdropPresetEffect` Class:** The `MilkdropPresetEffect` class will need to be designed to be as generic as possible, so that it can be used to render any MilkDrop preset. This will likely involve using a map or a similar data structure to associate preset names with `per_frame` function pointers.

**Next Steps:**

1.  **Re-read the `3dragonz.milk` file to identify the wave and shape code.**
2.  **Begin the process of manually translating this code to GLSL.**
3.  **Modify the `per_pixel` shader to incorporate the new wave and shape logic.**
4.  **Modify the `MilkdropPresetEffect` class to be a generic preset renderer.**
5.  **Enhance the `Milk-Converter` to be a two-stage converter.**