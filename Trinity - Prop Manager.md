Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

The prop manager is basically a client-side entity manager that
runs multiple functions, not just model rendering, but that fun-
ctionality is it's main asset. It loads models, sets them up on
their base parameters and uploads them to the video card. Thanks
to this there's a marked speed increase at the sacrifice of ani-
matability. It only recognizes item_generic entities with no ta-
rgetnames as entities it should manage, wich are removed from t-
he GoldSrc engine and are not handled. So if you want your item-
_generic entities to be interact with others, just give them a
targetname.

The prop manager uses Vertex Buffer Objects to make model rende-
ring faster, that is why client-side rendered models are not an-
imated. This makes it great for foliage, as it's really fast and
allows for a lot of model polygons to be rendered.

The manager also supports an entity that could be useful for ca-
bles on street poles, but it was never really finished so there-
's no texturing on it, it's drawn as a black beam.

Variables:
te_client_ents - Toggle the rendering of client-side props.