
My engine + game.

Evolutionary battlefield simulation.

Cool.

Now:

- Make walking guy.
- Ai
- Video

- Give guy sword.
- Offscreen rendering.
- Mouse picking.
- Shadow mapping.
- In game text.
- Paper rendering with pencil textures.
- Automatic texture generation.
- Combine font and other textures into one atlas
- Second texture for drawing effect.
- Structure rendering/input files and make reuseable as libraries.

- In game ui with tweens. Including neural network rendering.

DONE:

- Make 3d shader render (2D) textures by default.
- Draw guys with circular head.
- Texturing
-texture(texture0, textureCoords).xyz; Quick shader reloading
- Write shader struct
- Fix memory leak with shader reloading
- Make 2d spritebatch.
- Make font renderer.
- Make font spritesheet.
- Turn font bitmap into 4bytes per pixel rgba map.
- Use stb font texture atlas.
- Put all font data in own struct.
- Use font data to draw accurate sentences.
- 2d spritebatch.
- Efficient circle drawing (using masks & textures).
- Efficient textured line drawing from same texture.
- Draw guy direction and collision circle.
- Make 2d alpha.
- Make 3d alpha.
- Matrix library

BRAINSTORM:

No more asset related code for now. Clean it up and use in game.

Fix the terrain. Make it flat. Make a walking guy. Maybe look into one more technique.

Do marching cubes ? 

Paper rendering ? later.

Fix surface normals on guy. Shadows ? 

For paper rendering: upload one huge texture with pencil scratches. Use parts of this for texturing.
GL_REPEAT it and use x, y in clip space? 


