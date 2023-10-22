function update_transform(scale, rotation, translation, delta_time, frame_count)

	radius = 30
	frequency = 0.03

	translation.x = math.cos(frame_count * frequency) * radius
	translation.y = math.sin(frame_count * frequency) * radius

	return scale, rotation, translation
end