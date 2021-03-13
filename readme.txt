
My engine + game.

Now:

Pivot: Make alife creatures 2. Use physics engine and limbs with revolute joints.
       At first: Design the creatures and train each behavior. Then release them into 
       an ecosystem that you designed yourself. After this: design creatures that can hunt.
       then release into ecosystem. Tune macro parameters. Design behavior. Guided alife.

Collect info about ecosystem, try to create balance. Sandbox ecosystem game.

------Game Engine-------

- Create creature muscles + random movements.
- Make GRU using linalg library.
- Visualize GRU.
- Do physics engine memory management.
- 2D lighting.

DONE:

- Create creature in world.
- Create creature struct.
- Determine scale.
- Solid oriented rectangle rendering.
- Get mouse position from camera.
- Controll camera.
- Easy rotary limit joint creation.
- Chipmunk: connect bodies with rotary limit joint + pivot joint?
- Encapsulate physics in world.
- Create scene.
- Chipmunk: make rectangle bodies.
- Do Chipmunk tutorial.
- Draw oriented rectangles. Draw geometry.
- Invert y axis on command in camera.
- Import physics library. Chipmunk 2D.
- Make 2d camera, 2 stages: 1) draw custom sprites 2) render text.
- Fix font rendering with spritebatch.
- Fix textures with spritebatch.
- Fix nuklear.
- 2D spritebatch. With indices.
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

- Train creatures for individual task. Manage high level behavior with behavior tree. 
  
Low level behavior:
    Move forward
    Turn left 
    Turn right
    Move grabber to relative position. (independent of attachment)
    Attack to relative position.
    Grab and put down (building nests)

BIG QUESTIONS: 
- How to orient creature. Determine a body that contains the brain/heart?

COOL THINGS:
    Infinite generated zoom. Generate cells, dna, atoms.
    Fancy textures + normals and lighting.
    Generate textures, based on dna.

Some lay eggs, some dont.
complicated long term behavior. 
Carnivore herbivore ofcourse.
 
