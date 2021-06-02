
My engine + game.

Now:

Pivot: Make alife creatures 2. Use physics engine and limbs with revolute joints.
       At first: Design the creatures and train each behavior. Then release them into 
       an ecosystem that you designed yourself. After this: design creatures that can hunt.
       then release into ecosystem. Tune macro parameters. Design behavior. Guided alife.

Collect info about ecosystem, try to create balance. Sandbox ecosystem game.

Nice: Do competition. 150 dollars for best creature. 

TODO BEFORE DEMO LAUNCH:

- Draw on single bodypart :     
    - Implement right click.
    - Radial menu ? no probobly options.
    - Select draw on bodypart. Show outline of texture reach.

- Add tutorial.
- Serialization.
- Set training target and add multiple.
- Make at least one devlog.
- Setup website, newsletter or written devlog.

STRETCH GOALS: 
- Add ingame tutorial.
- Try to compile to windows and send some versions to people to test. 

- Figure out way to create graphs. Probably create own graphing. Base on own ui library.
- Combobox for training method and setup training method. 
- Edit video. Publish to youtube.
- Figure out one more way to promote game.

------ Evolution Sim -------

- Fix mouse. Make nice animated cursor for every tool.
- Add tutorial.
- Draw on single bodypart.

- Radial menu on right click to draw on single bodypart, delete, or other features. (Idea)

- Give very clear feedback for current tool.
- When selecting bodypart switch to selection. No need to cancel inbetween.
- Ui needs to be waayyyy better. 

- Add multiple views. See edges, see actual result, same in fakeworld.
- Generate random texture.
- Make videos: Create joker.
- Give clear feedback about the current tool
- Choose base color for every part.
- Add googly eyes.

- Gray out buttons when action is not available.

- Visualize fitness in world for first creature.
- Calculate fitness per second.
- Set fitness function before starting the simulation.
- Make nuklear graph.

- Anti aliasing.
- Idea: Make texture pill shaped ? 

- Serialization: load the creature defs and play.
- Save entire trained brains.
- Make googly eyes, or other decorations.
- Generate textures with noise.
- Add googly eyes.
- Serialization: Pull in json? library and save the creature defs.
- Show all inputs clearly and highlight in the world when you hover over it in the brain and vice versa
  (when in brain vis mode).

- Make mouse tooltip.
- Show matrix name on hover in mouse tooltip.
- Graph in nuklear gui? or in own gui
- Make function for visualizing constraint without gui to reuse.

- 2D lighting.

- Training mode: Get body close to ball. Implement with tournaments just like football simulation.
- Training mode: Walk with gravity.

- Add icons for all edit buttons.

- Write memory manager and give block to physics engine. Makes it easier to just throw away. 
- Write single make file with options for compiling external, internal, windows, linux.

- Make website that explains everything and has a devlog.

DONE:

- Keep tools in single tab with deselect button. Current situation is confusing.
- Mouse controll camera.
- Create some overhang on the edges of a bodypart.
- Add images.
- Toolbar on the side. Very small.
- Color picker on creature to select base color.
- Create tool select buttons.
- Create separate tool settings window.
- Solve bug: Minimum size during bodypart placement.
- Another option: Show activation as overlay.
- Set number of hidden neurons in editor.
- Make better order of drawing textures. Top-Bottom : Central bodypart-Outward. Also implement in simulation.
- Set default background and make it deletable. Set default texture to 0x00000000.
- Solve texture switching bug.
- Make attachment point visible again. Its hidden behind the texture now.
- Fix texture bleeding.
- Create a bit of overhang on each bodypart for the texture.
- Make sure you only draw in the correct range when drawing on texture.
- Add eraser.
- Move creature drawing to rendercontext and reuse in simulation.
- Visualization options: show lines, show texture, show ghosts.
- Smooth lines instead of circles per tick by interpolating.
- Draw on mousedown. 
- Adjust brush size in editor and show with tooltip.
- Add edit mode for drawing.
- Choose color in editor.
- Store scale in definition to always get the right brush size.
- Draw on bodyparts.
- Implement bodypart drawing without rectpacking. Just asign square and make best fit.
- Generate random circles.
- Generate random texture and draw creature with random textures.
- Make difference between sensors and actuators more clear.
- Set internal clocks in editor.
- Only show angles in tooltip. Make interface less messy.
- Fix bug that draws ghost limb when clicking in ui.
- Visualize GRU.
- Draw vector with colors.
- Draw matrix with colors.
- Show internal clock as actual clock + value.
- set parameters before training.
- Edit time in fake world screen.
- Keep focus on bodypart after editing.
- Tooltips with explanation of settings.
- Increase training time in fakeworldscreen.
- Solve snap bug when its smaller than the minimum.
- Put everything into appstate instead of on the stack in main.
- Save absolute index into brain input and output.
- Add inputs to only some bodyparts.
- Add more inputs for coordination.
- Adjust brainsize, base on definition. 
- Copy input and output data to the bodypart or reference definition.
- Use output definition in creature to set brain input vector values.
- Show input and output data in editor gui.
- Make input and output data editable.
- Collect all input/output info and assign to indices in brain input/output.
- Fix bug where limb turns 180 degrees on startup.
- Go back to editor and restart simulation.
- Edit learningrate and dev inside simulation.
- Start simulation from custom definition.
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
    Draw on textures.

Make everything look good. Its ugly now.

Some lay eggs, some dont.
complicated long term behavior. 
Carnivore herbivore ofcourse.

STUDY BIOLOGY:

And implement specific behavior and ecosystems.

MORE AI:
    Make Ai library that only uses bytes instead of floats.

RESEARCH:
    It would be nice to know the effect of an input on the output to
    see if an input is actually being used. Think of a meassure for this.
    Can also be used in robotics. Learning how neural networks actually work
    and generating feedback based on this.

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

    Make videos and make the interface SUUPER clear. Put a lot of effort into it.
    Make it understandable. Polish is important.
    
    Also make a competition and give a reward of 150 dollars to someone who makes the
    most creative creature.

    IDEA: Export creature texture as image. Tag bodyparts (manualy in editor). put your own body on it in gimp.
    IDEA: @ Lex Fridman in mijn volgende tweets. En @openai over hun ES paper. Denk groots
 
