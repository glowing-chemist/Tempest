

BasicPlayer_init = function(id)
	-- Create a player and it's controler attahced to the mesh instance.
	local playerPosition = {x=0,y=0,z=0}
	local playerDirection = {x=0,y=0,z=-1}
	TempestEngine_createPlayerInstance(id, playerPosition, playerDirection)
	TempestEngine_createControllerInstance(id, 0)

	-- Attach the main camera to the player
	TempestEngine_attachCameraToPlayer(id, "MainCamera", 1.2)
	TempestEngine_attachShadowCameraToPlayer(id, "ShadowCamera")
end


BasicPlayer = function(id, tick)
	-- update the controller
	TempestEngine_updateControllerInstance(id)
	-- update player based on controller
	TempestEngine_updatePlayerInstance(id)

end