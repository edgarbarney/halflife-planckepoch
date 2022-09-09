Spirinity Material System Overview
===============
Before we start, I suggest using [Notepad++](https://notepad-plus-plus.org/downloads/) as the editor instead of Windows's notepad since n++ can properly display indentions but notepad can't.

We've got a total of 4 files, with 3 new ones.

- [steptypes.txt](#Step-Types)
- [materialtypes.txt](#Material-Types)
- [materialimpacts.txt](#Material-Impacts)
- [materials.txt](#Materials)

![New files](https://i.imgur.com/tJSRUU9.png)

Note: Whitespace length is unimportant so you can use tabs and spaces freely __IF__ there is supposed to be whitespace.

# Step Types
Nothing very fancy. This file has the footstep and impact sounds' data. The syntax is like this:
```CPP
//"STEP_CATEGORYNAME"
//{
//	true/false							| Should Skip Sound?
//
//	#liquid								| Optional Special Flags for Some hardcoded things.	
//
//	$walkingVolume = 0-1						| Walking Sound volume 
//	$normalVolume = 0-1						| Sound volume 
//	$walkingStepTime = MS						| Next Walking Footstep time in MS
//	$normalStepTime = 0-1						| Next Normal Footstep time in MS
//	$crouchMultiplier = FLOAT 					| Crouch Volume Multiplier (finalVolume * $crouchMultiplier)
//
//	"soundfolder/soundfile.wav"		    			| A sound file
//  	"soundfolder/anothasound.wav"					| Another sound file
//  	"ambience/gaybar.wav"						| You can add much sounds. But I suggest using reasonable amount since precache limit is a bit short 4- ugh
//}
```

Example:
```cpp
// Snow
"STEP_SNOW"
{
	false
	
	$walkingVolume = 0.2
	$normalVolume = 0.5
	$walkingStepTime = 400
	$normalStepTime = 300
	$crouchMultiplier = 0.35
	
	"player/pl_snow1.wav"
	"player/pl_snow2.wav"
	"player/pl_snow3.wav"
	"player/pl_snow4.wav"
	"player/pl_snow5.wav"
	"player/pl_snow6.wav"
}

// "Houndeye" Step sound. Just for showcase
"STEP_HOUNDEYE"
{
	false
	
	$walkingVolume = 0.2
	$normalVolume = 0.5
	$walkingStepTime = 400
	$normalStepTime = 300
	$crouchMultiplier = 0.35
	
	"houndeye/he_pain1.wav"
	"houndeye/he_pain2.wav"
	"houndeye/he_pain3.wav"
	"houndeye/he_pain4.wav"
	"houndeye/he_pain5.wav"
}
```

# Material Types
This file contains possible material types. 

Warning: Some of them are still hardcoded so I don't suggest removing or renaming default types. You can tweak values and step types of them, though `{TEXNAME}` and `{TEXTYPE}` should stay the same

The syntax is like this:
```cpp
//{TEXNAME}						{TEXTYPE}		{STEPTYPE}				{IMPACTVOLUME}		{WEAPONVOLUME}		{ATTN}
```

And here's what they mean:
```cpp
// {TEXNAME}			=	Material Name					
// {TEXTYPE}			=	Material Type Alias. Used in "materials.txt"
// {STEPTYPE}			=	Sound Type that material uses. Used for bullet/melee impact and Footsteps.
// {IMPACTVOLUME}		=	Volume of impact sound.
// {WEAPONVOLUME}		=	Volume of weapon when impact happens. Used for melee.
// {ATTN}			=	Attenuetion of impact sound. Default is 0.8 (ATTN_NORM)
```

Example:
```cpp
"CHAR_TEX_SNOW"					"SNOW"			"STEP_SNOW"			0.9					0.0					0.8
"CHAR_TEX_HOUNDEYE"				"HOUNDEYE"		"STEP_HOUNDEYE"			0.9					0.1					0.8
```

# Material Impacts
This file contains the impact data for materials. (Particles when weapon hit etc.)

Warning: The commands that start with `$` is buggy for now. So please just use the default group. I will add the group support soon. It's actually in the code but a bit buggy so I don't suggest using it.

The syntax within groups is this:
```cpp
{TEXTYPE} 	{TEXDECAL}	        	{TEXPARTICLE}
```

And heres what they mean:
```cpp
// {TEXTYPE}		=	Material Type Alias. Used in "materials.txt"
// {TEXDECAL}		=	Decal group name that will be used for that material type. 
// {TEXPARTICLE}    	=	Particle script that will be used for that material type. You can use NULL for no particle.
```

Example:
```cpp
"SNOW"		"shot_metal"		"snow_impact_cluster.txt"
"HOUNDEYE"	"yellowblood"		"alien_blood_impact_cluster.txt"
```

# Materials
It almost works identically with vanilla
But instead of a single character as a material identifier,
You can use a string.

Also, you can use `#include` to use
other materials files __(relative to hl.exe)__

The 12 character limit still applies for compatibility reasons.

Example `materials.txt` file:
```
#include "valve\sound\materials.txt"
//For Opposing Force, uncomment the line below
//#include "gearbox\sound\materials.txt"

SNOW 5HJFD_SNOW

HOUNDEYE H
```

This file will use every texture-material association from the vanilla HL because of the `#include`

It will associate textures that __start with__ `5HJFD_SNOW` to the material `CHAR_TEX_SNOW` because `CHAR_TEX_SNOW`'s material alias is `SNOW`

It will associate textures that __start with__ `H` to the material `CHAR_TEX_HOUNDEYE` because `CHAR_TEX_HOUNDEYE`'s material alias is `HOUNDEYE`
