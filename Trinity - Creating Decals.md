How to create new decals for Trinity/Spirinity?
===============

For creating decals (or any texture) we need an image editor. You can use gimp, paint or something else. If it does have a TGA exporter OR if you can convert to TGA in some way, you can use the tool that you're comfortable with.

I'll use photoshop, but I'll show you some steps to convert the image to a format that the renderer can read (TGA) for various software.

Also; we need an image with a transparent background, obviously.

Our final texture should be  **24-Bit Uncompressed TARGA** with **no alpha** and with a **grey background**, typical for dx decals. So, if you know what you've gotta do, just do it! Don't let your drea- ok just kidding.

## 1. Create a "Texture" with background colour RGB(127,127,127)

We don't index the alpha channel in the renderer. So we use the image itself as transparency.

Firstly, create an image with desired (should be a power of 2) size. Then, cover the whole background with grey colour RGB(127,127,127). It should look like this.

![Create The Image](https://i.imgur.com/O2WvudX.png)

## 2. Find your decal image with transparent background

OK, since we've got our decal background ready, we need a transparent image, preferably PNG.
Find or create your desired decal with transparent background, then just drag and drop onto the background that we've created earlier.

![Paste the real decal](https://i.imgur.com/Dno10cd.png)

## 2.5. (Optional) Do some editing!

I don't want to use just a bloody bush! I want to show everyone that this is a bush! So I'll add a text and some effects to it. Also, I'll add a random image that contains a bit of grey. This is not a bloody Photoshop tutorial so I won't show you how I did it but for getting a grip of what we can and can't do with decals I've put some things to see in the end result.

![Paste the real decal](https://i.imgur.com/VJNwqlB.png)

## 3. Let's export!

Now, we've got a decal! But the engine can read it neither as a photoshop document, nor as a PNG, JPG or other formats that we use in our daily internet life. So we need to convert it into something fancy, called "Targa raster graphic format" or "TARGA" (abbreviated as TGA).

[Photoshop](#3---1-Exporting-as-TGA-in-Photoshop)
[GIMP (Soon!)](#3---1-Exporting-as-TGA-in-Photoshop)
[Convert to TGA (Soon!)](#3---1-Exporting-as-TGA-in-Photoshop)

#### 3 - 1. Exporting as TGA in Photoshop

For older versions of Photoshop, you can use File>Save a Copy... (Highlighted Green)

It was also used in older versions within the "Save As" dialogue but they've moved it. So if you've got older software, you can still use File>Save As...

![Choose "Save As" or "Save a Copy"](https://i.imgur.com/NHNlcmu.png)

Then, you should click on the dropdown menu, and choose TARGA.

![Dropdown > TARGA](https://i.imgur.com/bPPqSqK.png)
![Dropdown > TARGA](https://i.imgur.com/3LSRFHs.png)

After that, click "Save".

![Save](https://i.imgur.com/mRYvonw.png)

The file  **MUST** be a  **24-Bit Uncompressed TARGA**. So export it that way like so:

![24-Bit Uncompressed TARGA](https://i.imgur.com/NYyXoV4.png)

## 4. Putting it into the game files.

Yay! We've got ourselves a neat decal now! Let's put it into the game, shall we?

![Final Texture](https://i.imgur.com/ychlf2o.png)

Move out texture into your mod directory. I'll use the default Spirinity location,
So the full path is: `F:\SteamLibrary\steamapps\common\Half-Life\SpiritTrinity\gfx\textures\decals`
Relative path to Half-Life is: `SpiritTrinity\gfx\textures\decals`

![Moved into directory](https://i.imgur.com/IEXBYiG.png)

Now open the decalinfo.txt file that is also in the same folder

![Moved into directory](https://i.imgur.com/7WiSsBq.png)

Then, add these lines to the end of the file.
```CPP
mybushdecals
{
	me_awsum_decal	128	128
}
```
![Moved into directory](https://i.imgur.com/igynZ33.png)

The syntax is like this:
>```CPP
>my_decal_group_name
>{
>       decalfilename1      128 196
>       decalfilename2      128 196
>       anotherrandomdecal  32 32
>}
>```
>
>`my_decal_group_name` is the groupname used in code. 
>We'll use `UTIL_CustomDecal(&tr, "my_decal_group_name");` for creating a random decal from this group.
>
>`decalfilename1`,  `decalfilename2` and `anotherrandomdecal` are the filenames. For example: `anotherrandomdecal.tga` was the original filename. We remove the extension `tga` and use it like that.
>
>The numbers after the filenames are `width` and `height` of the decal in the world. Resolution of the decal, texture size on the brush or texture scaling doesn't matter.

After that, save the file.

Congratulations! We've got ourselves a decal!

## 5. Putting it into the maps.

Since we've configured our file, we can now use it in maps.
Open your favourite map editor (which shouldn't be Trenchbroom).

I use JACK, but the steps are identical for the default Hammer, too. Make sure you're using Spirinity's FGD

Firsly open up the map you want to put the decal into.

Then create an "env_decal" point entity.

![Moved into directory](https://i.imgur.com/UACKsUU.png)

Bring up the properties window, change the `Decal Texture` property to the filename of the decal without the extension. For example, for the decal we've just created: The filename is `me_awsum_decal.tga` but we will use `me_awsum_decal` as the property.

(For Experts: key name for `Decal Texture` is `message`)

![Moved into directory](https://i.imgur.com/wBEACIP.png)

Then save and run the map.

That's it! You've got your decal texture on the map!

![Moved into directory](https://i.imgur.com/Zc0WtSv.png)

We've got some small flaws, though.

1: Texture is flipped (Can be fixed before exporting)
2: Greyish  areas are semi-transparent or fully transparent.

But in the end, we've got our decal in-game! Yay!
