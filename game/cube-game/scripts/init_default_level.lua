-- Setup the scene game objects and params that will be parsed and used to setup the scene on the C++ side.

-- The key in table (i.e lua's map) is the name of gameobject, and the value is of the form:
-- File Path, Scale, Rotation, Translation, Script (a table that can optionally have name and path (either both or none)).
game_objects = {
	player = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 1.0, y = 1.0, z = 1.0},
		script = {
		}
	},
	
	floor = {
		model_path= "data/Cube/glTF/Cube.gltf",
		scale = {x = 1.0, y = 1.0, z = 1.0},
		rotation = {x = 0.0, y = 0.0, z = 0.0},
		translation = {x = 1.0, y = 1.0, z = 1.0},
		script = {
		}
	},
}