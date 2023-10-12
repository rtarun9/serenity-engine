function update_transform(scale, rotation, translation, delta_time, frame_count)

	scale.x = 1
	scale.y = 1
	scale.z = 1
	
	radius = 10
	frequency = 0.05 

	translation.x = math.cos(frame_count * frequency) * radius
	translation.y = math.sin(frame_count * frequency) * radius

	return scale, rotation, translation
end