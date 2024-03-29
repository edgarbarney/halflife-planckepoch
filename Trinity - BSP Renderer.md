Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

This file details usage and the function of the bsp renderer th-
at renders the world. It explains basic scripting of decals and
detail textures.

The bsp renderer uses Vertex Buffer Objects for enhanced perfor-
mance, but fallback is possible in case this feature is not sup-
ported by your hardware, though performance will suffer. The en-
gine uses very simplified per-pixel lighting that does not suff-
er from lightmap artifacts, and is faster than the previous app-
roach. Also supported are projective lights, TGA decalling and
detail texturing. Also is supported the ability to port out data
from func_detail_ext entities into external files, wich can get
loaded regardless of the engine and used to render these brushes
without any real limitation. However be sure that the texture t-
hese brushes use are within the map, or an error will occur. Al-
so these brushes will not be collidable with, nor will any obje-
cts or bullets impact them.

Variables:
te_world - Toggle world rendering.
te_speeds - Displays global rendering information.
te_detail - Toggle detail textures.
te_dynlights - Toggle dynamic lights.
te_radialfog - Toggle radial fog on nVidia cards.
te_world_shaders - Toggle shader use.
te_wireframe - Toggles wireframe mode.
te_shadows - Toggle projective light shadows.
te_shadows_filter - Toggle PCF filtering on shadows.

Decals:
Decal related files are located in gfx/textures/decals. The dec-
al script is in decalinfo.txt, the format divides into Decal Gr-
oups wich are used to randomly address decals stored in these g-
roups for the engine. World decals access decals by the texture
name, regardless of grouping. Here's an example of a basic group
in the text file:

scorch
{
	scorch1		96	96
	scorch2		88	88
	scorch3		84	84
}

The group name is at the top, the first part of the line is the
tga texture itself, the other two denote the X and Y size in un-
its.

Detail Textures:
Detail texturing is mostly like how it was back in Steam HL, but
a bit changed. All textures are accessed at gfx/textures/details
and I mostly not advise doing it by hand, there is an implement-
ed function that can associate world textures with their detail
textures specified in maps/detailtextures.txt. This will calcul-
ate detail texture coordinates manually for each map using the
te_detail_auto command, wich should make it a lot less troubles-
ome to add detail textures to your map, and will also calculate
the detail texture scales based on the relative texture sizes.

In detailtextures.txt, you only must specify the texture in the
wad, and the detail texture tied to it:
*wad texture name* *detail texture name*

Extra Detail Data:
This is rather simple. You can run this feature at any time, the
function will allow you to have more polygons in your levels th-
an what Half-Life would permit. In order to use this, you must
first assign all the entities you want to be externally loaded
into func_detail_ext entities and compile them normally. Afterw-
ards you have to load the map, and once in, run the te_exportde-
tail command in the console. A file will be dumped in your maps
folder, with the map name on it using the .edd extension. After
this compile your map without the func_detail_ext entities. What
this does is export all func_detail_ext data to an external file
and load it from then later on. It's useful for reducing geomet-
ry that won't be touched by the player anyway, and save on reso-
urces.

Using external textures:
External textures are defined in gfx/textures/texture_flags.txt.

The script goes like this:
world *texturename* alternate

The texture will be looked for in 
gfx/textures/world as .tga files.

HD Sky:
This lets you use high resolution tga images for the skybox, but it's not as simple as just putting them in your env folder. Name your HD skybox textures as you would normally, but append "_large" at the end of each file, so that if you specified "desert" as your skybox, the files should be named "desertlf_large", "desertup_large", and the like. These should be put in your env folder, and it's not necessary to have a normal file, the hd texture will be loaded regardless.

Shadow Mapping:
With version v1.14 I've added projective shadow mapping to proj-
ective lights in the game, so your flashlight will cast shadows
off of characters, world objects and props. This implementation
uses Percentage Closer Filtering, but this tends to be slow, the
filtering can be toggled but quality will suffer.

Also, right now I don't advise for this to be used as an entity
on it's own, I'd have to implement VIS for each of these, and r-
ight now that's off the table. If you do it however, don't be s-
urprised to see faults in rendering.

I don't plan to extend on the system much. I might put in HL2-l-
ike shadows for character entities, but nothing big, the codeba-
se isn't very fast for this. This was more of a test to see how
it adds to a horror mod's atmosphere.

Alternate spotlight textures:
To add an alternate spotlight texture into the game, add the fo-
llowing into your texture_flags.txt file:
flashlight *yourtexturename* none

The texture will be loaded from gfx/textures, it can only be  a 
.tga file. In your env_spotlight entity, be sure to add the pro-
per number to "texture index", which should be +1 the number of 
the texture you have defined in the file. The texture 
