
My engine + game.

Evolutionary battlefield simulation.

Cool.

Now:

- Draw guy direction and collision circle.
- Make walking guy.
- Ai
- Video
- Make 2d alpha.

- Give guy sword.
- Offscreen rendering.
- Mouse picking.
- Shadow mapping.
- In game text.
- Automatic texture generation.
- Combine font and other textures into one atlas

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

BRAINSTORM:

No more asset related code for now. Clean it up and use in game.

Fix the terrain. Make it flat. Make a walking guy. Maybe look into one more technique.

Do marching cubes ? 

Paper rendering ? later.

Fix surface normals on guy. Shadows ? 

But first: look into masks and draw circle.


