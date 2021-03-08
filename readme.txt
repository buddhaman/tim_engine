
My engine + game.

Now:

Pivot: Make alife creatures 2. Use physics engine and limbs with revolute joints.
       At first: Design the creatures and train each behavior. Then release them into 
       an ecosystem that you designed yourself. After this: design creatures that can hunt.
       then release into ecosystem. Tune macro parameters. Design behavior. Guided alife.

Collect info about ecosystem, try to create balance. Sandbox ecosystem game.

------Game Engine-------

- Fix nuklear.
- 2D spritebatch. With indices.
- Immediate mode GUI.
- Import physics library. Velocity raptor.
- 2D lighting.

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
- Rewrite 2d dynamic rendering with correct normals. 
- Dynamic memory allocations

BRAINSTORM:

No more asset related code for now. Clean it up and use in game.

Fix the terrain. Make it flat. Make a walking guy. Maybe look into one more technique.

Do marching cubes ? 

Paper rendering ? later.

Fix surface normals on guy. Shadows ? 

Possible pivot:

create voxel world for guys and make them build. Use behavior trees for this.

BRAINSTORM FOR PIVOT:

Making guys with behavior trees is gonna be a lot of work. Pivot to simple neural network simulation? 

Think about it.


