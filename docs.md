
# Platform Engine RPG documentation
## Get started
### Installation
Go to <a href="http://vladtopor.dev/perpg">official website </a> and download latest runtime and editor
Than, put runtime and editor in a blank directory
Type in cmd
```bash
perpg_editor --setup
```
Open an editor, now you need to learn some hotkeys
<table>
<tr>
<th>Hotkey</th>
<th>Action</th>
</tr>
<tr>
<td>S</td>
<td>Save level</td>
</tr>
<tr>
<td>L</td>
<td>Load level</td>
</tr>
<tr>
<td>T</td>
<td>Tilemode toggle</td>
</tr>
<tr>
<td>O</td>
<td>Previous brush</td>
</tr>
<tr>
<td>P</td>
<td>Next brush</td>
</tr>
<tr>
<td>Mouse click</td>
<td>Place object</td>
</tr>
<tr>
<td>Shift+mouse click</td>
<td>Edit object</td>
</tr>
<tr>
<td>Ctrl+mouse click</td>
<td>Move object</td>
</tr>
<tr>
<td>Alt+mouse click</td>
<td>Delete object</td>
</tr>
<tr>
<td>B</td>
<td>Brush editor</td>
</tr>
<tr>
<td>C</td>
<td>Clear all</td>
</tr>
<tr>
<td>N</td>
<td>Place back</td>
</tr>
<tr>
<td>M</td>
<td>Place forward</td>
</tr>
<tr>
<td>K</td>
<td>Test level</td>
</tr>
<tr>
<td>J</td>
<td>UI Editor</td>
</tr>
<tr>
<td>I</td>
<td>Toggle view id</td>
</tr>
</table>

### Adding brushes
Press B, to open brush editor, than press n to create new brush, now you can select texture, hit boxes and name, after creating it, press S to save brush and close Brush editor window
### Special brushes
By default, in editor are available some brushes:
Trigger - Run script on collide
Sound - Sound source
Barrier - Invisible wall
### Engine folders
**ALL** assets are stored at assets folder
In assets folder many folders:
textures - All textures, images
levels - All level layouts
sounds - Sounds and music
scripts - All Lua scripts
configs - Some configs, like translations
ui_layouts - UI Layouts
user - Save files and cache
anims - All animations
brushes - Brushes for mod makers, or in game editor

You can store assets in different folders, but it's standard
## Scripting
To make a script you need to create .lua file in assets/scripts
All communication with engine are using module 'engine'
To load level from Lua, use
```lua
engine.loadLevel(levelName, fastMode)
```
`fastMode`  is a option that tell engine to not remove existing level from the memory. If `fastMode`  equals `true` than can be visual issues.
This code can load UI layout
```lua
engine.loadUi(ui_layout)
```
Sometimes, you need to play a music
Platform Engine have up to 64 audio lines, that can be played runtime. For example, you can put in line 1 music, in line 2 put sound effects. To play sound, you need to use
```lua
engine.playSound(path, line, loop)
```
Sometimes, you need to modify or get properties of your objects
```lua
engine.getProperty(id, property)
engine.modifyObject(id, property, value)
```
To launch console command, use
```lua
engine.runCommand(command)
```
Also, sometimes you need to make a custom control
First, remove all data from default `player_controller.lua` and put code like this
```lua
engine.setCallback("keybind", "w", "assets/scripts/movePlayerUp.lua");
```
movePlayerUp.lua
```lua
engine.setPlayerMovementVector(0, 1);
```
## Multiplayer
At perpg SDK page you can download dedicated server software. After downloading it, put it in different folder, and copy assets folder from game.
First, setup your server using
```bash
perpg_server --setup
```
Than, launch server using
```bash
perpg_server --port=1234
```
## Console
Console is integral part of perpg
To use console your need to press ` button
### Commands
First command is `connect <ip>:[port]`
Example:
```bash
connect 192.168.0.1:1234
```
_Coming soon_
## Advanced
### Change tickrate
To change tickrate (ticks per seconds) open `config.json` in configs folder and change tickrate property
> Recommendation: set tickrate in a range 10-60
