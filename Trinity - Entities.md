Trinity Entities
===============
This is a list of new/replaced entities of Trinity. They are present in the provided FGD of the repo, so you don't have to add them yourself manually.

env_elight
===============
```
@PointClass base(Targetname) iconsprite("sprites/lightbulb.spr") size(-16 -16 -16, 16 16 16) = env_elight : "Model light"
[
  renderamt(integer) : "Radius ( 1-25)" : 15
  rendercolor(color255) : "Render Color" : "255 255 255"
  spawnflags(flags) =
  [
    1 : "Start On" : 1
  ]
]
```

Description:
Spawns a light that only affects models, it gives per-vertex calculated lighting on models for extra detail. This is the only way you'll get per-vertex lighting on models.

env_spotlight
===============
```
@PointClass base(Targetname, Angles) size(-16 -16 -16, 16 16 16) = env_spotlight : "SpotLight"
[
  renderamt(integer) : "Radius" : 15
  rendercolor(color255) : "Render Color" : "255 255 255"
  scale(string) : "Cone size" : "20"

  body(integer) : "Texture Index" : 0

  sequence(Choices) : "Disable Shadowmaps" : 0 =
  [ 
      0 : "No"
      1 : "Yes"
  ]

  spawnflags(flags) =
  [
    1 : "Start On" : 1
  ]
]
```

Description:
Spawns a spotlight in your level not unlike the flashlight, however be aware that it is not shadowed.

env_decal
===============
```
@PointClass base(Targetname) size(-8 -8 -8, 8 8 8) color(230 230 255) = env_decal: "Decal Entity" 
[
  message(string) : "Decal Texture"

  spawnflags(flags) =
  [
    1 : "Wait for Trigger" : 1
  ]
]
```
Description:
The env_decal entity will spawn a decal on the wall it's been pasted onto, specify the texture regardless of grouping.

Texture key is based on Trinity's decals folder, not based on WADs.
For example: 
- Decal Path: gfx\textures\decals\bloodbigsplat.DDS
- In the entity: Decal Texture = "bloodbigsplat"

env_fog_trinity
===============
```
@PointClass base(Targetname) size(-16 -16 -16, 16 16 16) = env_fog_trinity : "Client Fog" 
[
  startdist(string) : "Start Distance" : "1"
  enddist(integer) : "End Distance" : 1500
  rendercolor(color255) : "Fog Color (R G B)" : "0 0 0"

  affectsky(Choices) : "Don't Fog Sky" : 0 =
  [ 
      0 : "No"
      1 : "Yes"
  ]

  spawnflags(flags) =
  [
    1 : "Start On" : 1
  ]
]
```
Description:
Your standard fog entity, but here you can specify wether the fog will fog the skybox also. This will cut out fog distance optimization though, so I would only recommend using it in less stressing maps.

env_particle_system
===============
```
@PointClass base(Targetname, Angles) = env_particle_system: "Particle System"
[
	spawnflags(flags) =
	[
		1: "Start On" : 0
		2: "Remove on Fire" : 0
	]

	message(string) : "Definition File" : "your_particle_script_here.txt"
  	frags(Choices) : "Script Type" : 0 =
  	[
  	    0 : "System Script"
  	    1 : "Cluster Script"
  	]
]
```
Description:
All particle scripts are loaded from *modfolder*/scripts/particles/. Here you simply need to specify the script name. You can also specify wether this is a single script, or a cluster file containing the names of multiple particle scripts.

3D Sky
===============
```
@PointClass base(Targetname) size(-4 -4 -4, 4 4 4) color(200 100 50) = envpos_sky : "Sky Marker" 
[
  startdist(string) : "Start Distance" : "1"
  enddist(integer) : "End Distance" : 1500
  rendercolor(color255) : "Fog Color (R G B)" : "0 0 0"
  affectsky(Choices) : "Don't Fog Skybox" : 0 =
  [ 
      0 : "No"
      1 : "Yes"
  ]
]
@PointClass base(Targetname) size(-4 -4 -4, 4 4 4) color(200 100 50) = envpos_world : "Sky World Marker" 
[
 health(string) : "Size" : "16"
]
```
Description:
Only solid entities and client-side managed item_generics will be shown in a 3D sky, and only if their Render FX is set to 70. 

The envpos_sky entity should be put in the skybox you want to use for the map, while the envpos_world should be located inside the world where the player interacts.

info_light_origin
===============
```
@PointClass base(Targetname) size(-4 -4 -4, 4 4 4) color(200 100 50) = info_light_origin : "Entity Light Origin Marker" []
```

item_generic
===============
```
@PointClass base(Angles, Targetname, RenderFields) size(-16 -16 0, 16 16 36) studio() = item_generic : "Generic scene item" 
[
  model(studio) : "Model"
  body(Integer) : "Body" : 0
  skin(Integer) : "Skin" : 0
  scale(Integer) : "Scale" : 0
  sequence(string) : "Sequence" : "idle"
  lightorigin(string) : "Light Origin"
]
```
Description:
If you want this to animate and/or you want to be able to trigger it, just give it a targetname. It'll render slower, though. If you want the entity to pick up light from a different position than it is at now, just put an info_light_origin in the level and specify it in the entity. This will only work on entities managed on the client side, so if you want to use this don't give it a targetname.

func_mirror
===============
```
@SolidClass base(Targetname, Global, Appearflags, RenderFields, ZHLT) = func_mirror : "Mirror" []
```

Description is in the SDK - Mirror Manager file.
