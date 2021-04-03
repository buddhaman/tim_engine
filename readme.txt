
My engine + game.

Now:

Pivot: Make alife creatures 2. Use physics engine and limbs with revolute joints.
       At first: Design the creatures and train each behavior. Then release them into 
       an ecosystem that you designed yourself. After this: design creatures that can hunt.
       then release into ecosystem. Tune macro parameters. Design behavior. Guided alife.

Collect info about ecosystem, try to create balance. Sandbox ecosystem game.

------Game Engine-------

- Start simulation from custom definition.
- Save by dumping creaturedef data.

- Serialization: load the creature defs and play.
- Save entire trained brains.
- Put everything into appstate instead of on the stack in main.
- Make googly eyes, or other decorations.
- Generate textures with noise.
- Add googly eyes.
- Serialization: Pull in json library and save the creature defs.

- Visualize GRU.
- Graph in nuklear gui? or in own gui
- Make function for visualizing constraint without gui to reuse.

- Mouse input to move camera.
- set parameters before training.
- 2D lighting.
- Only recalculate body positions when dirty.

- Add icons for all edit buttons.
- Solve snap bug when its smaller than the minimum.

- Write memory manager and give block to physics engine. Makes it easier to just throw away. 

DONE:

- Put fakeworldscreen into separate arena, throw away and rebuild when training starts.
- Set max bodyparts.
- Show size of proto brain.
- Simulation settings and start button somewhere...
- Fix moving buttons.
- Grid snap / creature snap / rotation snap.
- Creature definition editor.
- Delete bodypart and all children.
- Add ui for editing all these values in nuklear.
- Visualize constraint when selecting bodypart.
- Nicer visualization of angle.
- Top bar with tools. Or sidebar? no topbar. Sidebar.
- Constraint system, build recursively from center body.
- Button for moving and stretching limb.
- Show body center.
- Make ui with button for adding bodypart.
- Calculate everything recursively in creature_definition. 
- Return full BoxEdgePosition.
- Define new bodypart+constraint.
- Remove RotoryMuscleDef and store inside bodydef
- Delete from dynamic array.
- Move creature definition to creature editor or separate file.
- Make world buttons for scaling and rotating.
- Make tim_ui basics for pressing buttons and axis button.
- Make second screen for creature definition editor.
- Define screens.
- Show all basic info in gui.
- Make 2 digit bug.
- Define rectangles under angle.
- Bottom bar with all evolution info.
- Grid 
- Calculate creature fitness.
- Next generation.
- Speed up/down.
- Move ES outside of world.
- Make subarena for world.
- Function to throw away world and clear world subarena.
- Keep track of al physics constraints and bodies for throwing away.
- Creature definition -> fake world + es
- Build entire world based on Evolution Strategies.
- Oh fuck.. memory management for minimal gated unit.
- Minimal gated unit creation in evolution system.
- Implement evolution strategies.
- Make evolution system. Requires fixed amount of memory. Thats good. 
- Uniform random generation.
- Generate random normal numbers.
- Create brain from single vector of parameters.
- Count parameters.
- Move muscles using GRU.
- Make GRU using linalg library.
- Top down ground friction for all dynamic bodies.
- Create creature muscles + random movements.
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

Brain needs to be broken up into persistent and transient part. Dynamic part is required in creature.
But while training multiple persistent parts are required. 

Gene -> persistent brain.

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

STUDY BIOLOGY:

And implement specific behavior and ecosystems.

FUN:
    Visualize everything!!!!
    especially training: show each epsilon at the same time . 
    Show combining and averaging and going to next generation. Show forming of brains.
    This helps with debugging and its fun to look at. 
    Show graph, numbers, everything.

    Make brain location, show connections. 
    Make googly eyes, mouth, claws.

    Make everything wiggly. Including mouse cursor. Pay attention to details.

    Make gun creature for video.
 
