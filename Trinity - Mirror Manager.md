Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

This is a very basic entity with little to really discuss. In order to use this entity, just put a func_mirror in your level and texture every face with a null texture except the one you want to be the mirror, then fit the non-null texture to the brush face so it doesn't tile. Otherwise it will just print out an error message and won't work.

Console Variables:
te_mirrors - Toggle rendering of mirrors.
te_mirror_players - Set to 1, players will appear in mirrors.
