Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

This file contains a list of all functionalities relevant to mo-
del rendering, as well as explanations on how they are to be us-
ed properly.

Variables:
te_models - Toggles rendering of studio models.
te_model_decals - Toggles rendering of decals.

-Texture Flags and External Texture Loading:
External textures can be loaded only for models that do not use
T models to store textures. They are defined in gfx/textures/te-
xture_flags.txt.

The script goes like this:
*modelname* *texturename* *flags*

In this case the field in *flags* should be only "replace"

The texture will be looked for in 
gfx/textures/models/*modelname*/ as a .tga file.

Three other flags are available. "nomipmap" disables mip mapping
on the replaced texture, this is advised to use on pictures you
want to appear sharp at all times. Mip mapping can really reduce
quality on things like v_ model textures or photo images.
"fullbright" can be used to disable lighting on the mesh that u-
ses this texture, and recieve a generic light value. Good for s-
tuff like lightbulbs.
"eraseflags" is more of a debug feature, but it essentially rem-
oves all basic flags from the texture that were originally assi-
gned.
"nomipmap" only works with textures that already make use of the
"alternate" flag, and textures of client-side loaded models.


-Decals:
Decal creation is mainly code-handled, the sdk base has it used
as part of the monster damaging code, but a base was added ins-
ide ev_hldm.cpp that allows for client-side models to be decal-
led too. This is identified simply by having a "collision" ent-
ity wrapped around these models with a texture that has a text-
ure type set in materials.txt. If you set RenderMode to Color
and Render Amount to 0, the engine code will try and decal the
model inside that brush entity volume.


-Shaders:
The shaders are all written in ARB format for maximal compatib-
ility on all systems, you won't be able to edit them much, but
that wasn't the goal anyway. They allow for advanced effects on
models via env_elight entities and dynamic lights, but there is
a fallback to non-shader lighting in case it isn't supported by
your system or in case you want to turn it off for some reason,
but quality will suffer.

You might have the game exit with an error message, stating th-
at a certain model was modified since the game was launched and
that you should run a system diagnostic. This is not a problem
at all, it occurs because normally the model compiler won't set
a flag which the engine needs, and for this reason the it's ma-
nually added and the model is re-exported. This shouldn't be c-
ommon, and once all models are modified, it will be gone.