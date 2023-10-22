function update_transform(scale, rotation, translation, delta_time, frame_count)

	radius = 30
	frequency = 0.02

	translation.x = -1 * math.cos(frame_count * frequency) * radius
	translation.y = -1 * math.sin(frame_count * frequency) * radius

	scale.x = math.sin(frame_count * frequency)

	return scale, rotation, translation
end