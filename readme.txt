
My engine + game.

Evolutionary battlefield simulation.

Cool.

Now:

- Click and move.
- Ray picking.

-------------------- -------------------- -------------------- --------------------

- Highest prio that i don't feel like doing : texture atlasses. Need to make some kind of
  texture packer and a parser which can read text files to turn them into an atlas.
- Offscreen rendering using FBO's.

- Shadow mapping.
- In game text.
- Paper rendering with pencil textures.
- Automatic texture generation.
- Combine font and other textures into one atlas.
- Second texture for drawing effect.
- Structure rendering/input files and make reuseable as libraries.
- In game ui with tweens. 
- Research: what to use to controll guys

- Click guy and click ground. Show route. Show list of actions ? 
- Make them attack eachother.
- Implement all basic features before implementing a controll strategy.

- Select and click somewhere.
- In game text. 3D text mesh.
- Actions: 
    - Walk towards.
    - Swing sword.
    - Pickup sword.

------Game Engine-------

- Create memory structures,
- Create sound.
- Make new game from same base. Use what you learned to refactor the game engine layer.
- Make 2D texture packer. Standalone + use ingame to generate texture atlasses.
- 2D tower defense. With lots of assets. 
- Compile to windows.

DONE:

- Make 3d shader render (2D) textures by default.
- Draw guys with circular head.
- Texturing
- Quick shader reloading
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
- Neural network rendering.
- Put all 2d rendering in one file.
- Mouse input
- Disable unused arrays in mesh.
- Raycasting.
- Mouse picking.
- Select guy.
- Give guy sword.
- Rewrite 2d dynamic rendering with correct normals. V
- Dynamic memory allocations

BRAINSTORM:

No more asset related code for now. Clean it up and use in game.

Fix the terrain. Make it flat. Make a walking guy. Maybe look into one more technique.

Do marching cubes ? 

Paper rendering ? later.

Fix surface normals on guy. Shadows ? 

For paper rendering: upload one huge texture with pencil scratches. Use parts of this for texturing.
GL_REPEAT it and use x, y in clip space? 

Possible pivot:

create voxel world for guys and make them build. Use behavior trees for this.

BRAINSTORM FOR PIVOT:

Making guys with behavior trees is gonna be a lot of work. Pivot to simple neural network simulation? 

Think about it.


