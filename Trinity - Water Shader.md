Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

This file will detail to you how the water shader can be used a-
nd how it works. It's rather simple and works like a charm, but
some standard Half-Life features like movement of the entity are
not supported.

To use this feature, just put a func_water entity in your map l-
ike you would normally. Properties of the water shader are defi-
ned in a script file located in *modfolder*/scripts. The default
settings are in water_default, but you can specify water proper-
ties for each level. To do this, simply create a new script nam-
ed water_*level name*.txt and specify the properties.