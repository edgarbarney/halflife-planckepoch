Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

This file contains all you need to know in order to use the par-
ticle engine in your mod. The scripting language consista of one
text file for a particle effect, wich contains all the paramete-
rs of the particle effect and settings. These define how partic-
les will behave. This file will detail all the parameters in de-
pth.

You can have animated textures on particles too, but each frame
will have to be stored in a single texture, side-by-side. These
can occupy numerous rows too, the system will calculate the coor-
dinates properly. Examples of these textures can be found in the
gfx/textures/particles folder.

Examples of nearly all effects and parameter usage can be found
in the scripts/particles folder.

Console variables:
te_particles - Toggle rendering and simulation of particles.
te_particles_debug - Shows debugging information on the particle
renderer.


-systemshape:
Defines the basic shape of the particle system.
0 - Point shaped, particles spawn right from the origin.
1 - Box shaped, particles spawn in a rectangular volume.
2 - Plane above player, reserved for weather effects.

-systemsize:
In units, the size of the system of systemshape is set to 1 or 2.

-maxheight:
The maximal height above the player the plane will be at, if the
shape is set to Plane above player.

-minvel:
Minimal starting velocity of a particle.

-maxvel:
Maximal starting velocity of a particle.

-maxofs:
Maximal directional offset of a particle.

-fadein:
Time, in seconds, it take for a particle to fade in.
A setting of zero means this won't take place.

-fadedelay:
Time before a particle begins to fade out.
A setting of zero means this won't take place.

-fadedistnear:
The distance at wich particle begin to fade out from the viewer.
A value of zero means this won't take effect.

-fadedistfar:
The distance at wich particles become invisible from the viewer.
A value of zero means this won't take effect.

-mainalpha:
Maximal opacity a particle can achieve.

-veldamp:
Determines how much velocity is damped each second.
A setting of zero means this won't take place.

-veldampdelay:
Optional delay before velocity damping takes hold.

-randomdir:
Settin this to 1 means all passed directions are ignored, and th-
e particles will be spawned in random directions.

-life:
Maximal lifetime of a particle. 
A setting of -1 will make it infinite, but you want to make sure 
other settings cancel the particle out, otherwise massive fps l-
oss will take place.

-lifevar:
The amount of variation in lifetime.

-pcolr, pcolg, pcolb:
RGB values of the primary color in the 0-255 range.

-scolr, scolg, scolb:
RGB values of the secondary color inthe 0-255 range.
Setting any of these to -255 will make it take up random values.

-ctransd:
Delay before transition from primary to the second color takes 
place.
A setting of zero means this won't take place.

-ctranst:
Time it takes for this transition to occur.
A setting of zero means this won't take place.

-ctransv:
Maximal variation in ctransd.

-scale:
Base scale of the particle.

-scalevar:
Maximal variation in the base scale.

-scaledampdelay:
Optional delay before scale damping occurs.

-scaledampfactor:
The amount of scale that's damped each second, negative values w-
ill make the particle grow in size.
A setting of zero means this won't take place.

-gravity:
The amount by wich gravity is scaled on particles. A value of 1
means normal gravity effects.

-maxparticles:
The amount of particles the system will create. A value of -1 wi-
ll make the system emit particles indefintely.

-intensity:
The amount of particles spawned per second.

-maxparticlevar:
Maximal variation in the number of spawned particles.

-systemfadedelay:
The delay before system intensity begins fading out.

-systemfadetime:
The time it takes for the system's intensity to get to zero.
A value of zero means this won't take effect.

-startparticles:
The amount of particles emitted right on spawn, this does not af-
fect the max particle count.

-lightmaps:
Determines how lighting is added to the particle.
0 - No lighting, primary color is used.
1 - The particle takes values from the lightmaps and lights.
2 - The lighting values are stored in the secondary color.

-collision:
Determines collisions of the particles. If any setting is set to
create a new effect, that must be named accordingly in the "cre-
ate" parameter documented below.
0 - No collision takes place.
1 - Particle dies immediately on impact.
2 - Particles will bounce back from the surface.
3 - The particle will die instantly, but spawn a decal.
4 - The particle will get stuck and fade out.
5 - The particle will spawn a new system on impact and death.

-create:
If the particle is set to create something when it collides, this
can be either a decal group's name or the name of the system scr-
ipt you want to be spawned.

-deathcreate:
System to create when the particle reaches the end of it's lifet-
ime.

-watercreate:
System to create when the particle collides with water.

-colwater:
Allows the particle to collide with water, this is costly so it's
best to reserve this.

-impactdamp:
Determines how much of the velocity is lost when the particle bo-
unces back from the ground.

-stuckdie:
If the lifetime is set to -1, and the collision is set to stuck,
this is the time it will take for the particle to fade out once
it gets stuck.

-texture:
The .tga texture used by the particle.

-rendermode:
Determines what rendering mode the particle uses.
0 - Additive
1 - Alphablend
2 - Intensity

-overbright:
If set to 1, particles will be two times overbrightened.

-display:
Determines how the particle is rendered. Parallel is mostly usef-
ul for effects like rain. Plane is useful for particles that are
spawned by particles impacting the ground, meant to lay on it.
0 - Tiled to screen
1 - Paralell to screen.
2 - Take up a plane.

-numframes:
The amount of frames in the animated texture.

-framesizex:
If using an animated texture, this is the width of an individual
frame in the texture

-framesizey:
If using an animated texture, this is the height of an individual
frame in the texture.

-framerate:
The amount of frames played per second.

-rotationvar:
Variation in tiled rotation of the particle.

-rotationvel:
Tiled rotational velocity of the particle.
A value of zero means the effect won't take place.

-rotationdamp:
The mount of velocity damped each second.
A value of zero means the effect won't take place.

-rotationdampdelay:
Optional delay before rotation damping takes place.

-rotxvar:
Variation in rotation of the particle on it's x axis.

-rotxvel:
Rotational velocity of the particle on it's x axis.
A value of zero means the effect won't take place.

-rotxdamp:
The mount of velocity damped each second.
A value of zero means the effect won't take place.

-rotxdampdelay:
Optional delay before rotation damping takes place.

-rotyvar:
Variation in rotation of the particle on it's y axis.

-rotyvel:
Rotational velocity of the particle on it's y axis.
A value of zero means the effect won't take place.

-rotydamp:
The mount of velocity damped each second.
A value of zero means the effect won't take place.

-rotydampdelay:
Optional delay before rotation damping takes place.

-windx:
Base wind velocity on the x axis.

-windy:
Base wind velocity on the y axis.

-windvar:
Variation in wind velocities.

-windtype:
Determines the wind algorythm used.
0 - Linear.
1 - Sine wave wind.

-windmult:
This value multiplies the sine wind.

-windmultvar:
Variation in the previous value.

-tracerdist:
If the value is non-zero, each particle will spawn a new particle
defined in the system script in the "create" parameter for each 
time it passes this distance.
