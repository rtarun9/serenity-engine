function update_transform(scale, rotation, translation, delta_time, frame_count)

	radius = 8
	frequency = 0.1

	translation.x = math.cos(frame_count * frequency) * radius

	return scale, rotation, translation
end