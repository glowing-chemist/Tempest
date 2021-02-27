require "math"


local playerSize = {}
local playerDirection = {x=0,y=0,z=1}

BasicPlayer_init = function(id)
	-- Create a player and it's controler attahced to the mesh instance.
	local playerPosition = {x=0,y=0,z=0}
	TempestEngine_createPlayerInstance(id, playerPosition, playerDirection)
	TempestEngine_createControllerInstance(id, 0)

	-- Attach the main camera to the player
	TempestEngine_attachCameraToPlayer(id, "MainCamera", 1.2)
	TempestEngine_attachShadowCameraToPlayer(id, "ShadowCamera")

	playerSize = TempestEngine_getInstanceSize(id)
end

vector3_add = function(v1, v2)

	vec = {x=v1.x + v2.x, y=v1.y + v2.y, z=v1.z + v2.z}
	return vec
end

vector3_mul = function(v, s)
	vec = {x=v.x * s, y=v.y * s, z=v.z * s}
	return vec
end

vector3_normalize = function(v)
	local mag = v.x * v.x + v.y * v.y + v.z * v.z
	mag = mag^0.5

	return {x=v.x / mag, y=v.y / mag,z=v.z / mag}
end

vector3_dot = function(v1, v2)
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z
end

quat_fromAxisAngle = function(v, a)
	local sa = math.sin(a / 2.0)
	local ca = math.cos(a / 2.0)

	return {x=v.x * sa, y=v.y * sa, z=v.z * a, w=ca}
end

vector3_angle = function(v1, v2)
	return math.acos(vector3_dot(v1, v2))
end

local State = {Resting = 1, Walking = 2, Sprinting = 3, Jumping = 4 }       -- player states
local currentState = State.Resting
local timeout = 0

-- animation names
local kWalkingAnimation = "Armature|Armature|Armature|Walk|Armature|Walk"
local kSprintAnimation = "Armature|Armature|mixamo.com|Layer0"
local kJumpAnimation = "Armature|Armature|mixamo.com|Layer1"

BasicPlayer = function(id, tick)

	TempestEngine_startInstanceFrame(id)

	if timeout > 0 then
		timeout = timeout - 1
	end

	-- update the controller
	local controller = TempestEngine_updateControllerInstance(id)
	
	local sprinting = controller.LShft;
	local speedModifier = 0.01
	if sprinting then
		speedModifier = 0.05
	end
    local x = controller.Lx * speedModifier;
    local z = controller.Ly * speedModifier;
    local moving = (x ~= 0.0 or z ~= 0.0);

    if moving then
    	local camDir = TempestEngine_getCameraDirectionByName("MainCamera")
    	camDir.y = 0
    	camDir = vector3_normalize(camDir)
    	local camRight = TempestEngine_getCameraRightByName("MainCamera")

    	local trans = vector3_add(vector3_mul(camDir, -z), vector3_mul(camRight, x))
    	TempestEngine_translateInstance(id, trans)

    	local movementDirection = {x=x, y=0, z=z}
    	movementDirection = vector3_normalize(movementDirection)

	    local angle = vector3_angle(movementDirection, playerDirection)
	    local rotation = quat_fromAxisAngle({x=0,y=1,z=0}, angle)
	    TempestEngine_setInstanceRotation(id, rotation)
    end

    local colliderPosition = TempestEngine_getPhysicsBodyPosition(id)
    colliderPosition.y = colliderPosition.y - (playerSize.y / 1.1)
    TempestEngine_setGraphicsInstancePosition(id, colliderPosition)

    TempestEngine_updatePlayersAttachedCameras(id)

    -- update animation state
  	if (timeout == 0 and currentState == State.Jumping) then
        currentState = State.Resting;
    end

    if moving and sprinting and currentState == State.Resting then
        TempestEngine_startAnimation(id, kSprintAnimation, true, 2.0)
        currentState = State.Sprinting
    elseif moving and sprinting and currentState == State.Walking then
        TempestEngine_terminateAnimation(id, kWalkingAnimation)
        TempestEngine_startAnimation(id, kSprintAnimation, true, 2.0)
        currentState = State.Sprinting
    elseif ((currentState == State.Resting or currentState == State.Sprinting) and moving and not sprinting) then
        if currentState == State.Sprinting then
            TempestEngine_terminateAnimation(id, kSprintAnimation)
        end
        TempestEngine_startAnimation(id, kWalkingAnimation, true, 1.0)
        currentState = State.Walking
    elseif ((currentState == State.Walking or currentState == State.Sprinting) and not moving) then
        TempestEngine_terminateAnimation(id, kWalkingAnimation)
        TempestEngine_terminateAnimation(id, kSprintAnimation)
        currentState = State.Resting;
    end

    if (controller.X and currentState ~= State.Jumping) then
        if (currentState == Walking or currentState == State.Sprinting) then
            TempestEngine_terminateAnimation(id, kWalkingAnimation)
            TempestEngine_terminateAnimation(id, kSprintAnimation)
        end

        currentState = State.Jumping;

        TempestEngine_startAnimation(id, kJumpAnimation, false, 1.0);

        TempestEngine_applyImpulseToInstance(id, {x=0.0, y=400.0, z=0.0});

        timeout = 80;
    end

end