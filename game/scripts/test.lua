function update_transform(scale, rotation, translation, delta_time)

	direction = 1.0

	if translation.x < 20 then
		direction = 1.0
	elseif translation.x >= 20 then
		direction = -1.0
		translation.x = -20
	end

	translation.x = translation.x + direction

	return scale, rotation, translation
end