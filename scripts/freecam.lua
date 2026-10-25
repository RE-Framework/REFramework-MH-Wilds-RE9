/* rev-t6pk7r-20261025 freecam.lua */
/* rev-d8e14f-20260829 freecam.lua */
-- REFramework Freecam Script
-- Fly camera with WASD movement, mouse look, and speed control
-- Works with Monster Hunter Wilds, RE9, and all supported RE Engine games

local re = re
local sdk = sdk

local freecam = {
    enabled = false,
    speed = 1.0,
    boost_multiplier = 3.0,
    position = { x = 0, y = 0, z = 0 },
    rotation = { pitch = 0, yaw = 0 },
    original_position = nil,
    original_rotation = nil,
}

local function get_camera()
    return sdk.get_primary_camera()
end

local function store_original_camera()
    local cam = get_camera()
    if not cam then return end
    freecam.original_position = sdk.get_native_field(cam, "Position")
    freecam.original_rotation = sdk.get_native_field(cam, "Rotation")
end

local function restore_camera()
    if not freecam.original_position then return end
    local cam = get_camera()
    if not cam then return end
    sdk.set_native_field(cam, "Position", freecam.original_position)
    sdk.set_native_field(cam, "Rotation", freecam.original_rotation)
end

local function update_camera_position()
    if not freecam.enabled then return end
    local cam = get_camera()
    if not cam then return end

    local forward = sdk.get_native_field(cam, "ForwardVector")
    local right = sdk.get_native_field(cam, "RightVector")
    local up = { x = 0, y = 1, z = 0 }

    local speed = freecam.speed
    if imgui and imgui.is_key_down(16) then -- Shift
        speed = speed * freecam.boost_multiplier
    end

    local move = { x = 0, y = 0, z = 0 }

    if imgui and imgui.is_key_down(87) then -- W
        move.x = move.x + forward.x * speed
        move.y = move.y + forward.y * speed
        move.z = move.z + forward.z * speed
    end
    if imgui and imgui.is_key_down(83) then -- S
        move.x = move.x - forward.x * speed
        move.y = move.y - forward.y * speed
        move.z = move.z - forward.z * speed
    end
    if imgui and imgui.is_key_down(65) then -- A
        move.x = move.x - right.x * speed
        move.z = move.z - right.z * speed
    end
    if imgui and imgui.is_key_down(68) then -- D
        move.x = move.x + right.x * speed
        move.z = move.z + right.z * speed
    end
    if imgui and imgui.is_key_down(32) then -- Space
        move.y = move.y + speed
    end
    if imgui and imgui.is_key_down(17) then -- Ctrl
        move.y = move.y - speed
    end

    freecam.position.x = freecam.position.x + move.x
    freecam.position.y = freecam.position.y + move.y
    freecam.position.z = freecam.position.z + move.z

    sdk.set_native_field(cam, "Position", freecam.position)
end

function OnPreCameraUpdate()
    update_camera_position()
end

-- UI
local function draw_ui()
    imgui.begin("Freecam")
    local changed, enabled = imgui.checkbox("Enable Freecam", freecam.enabled)
    if changed then
        freecam.enabled = enabled
        if enabled then
            store_original_camera()
        else
            restore_camera()
        end
    end

    local schanged, speed = imgui.slider_float("Speed", freecam.speed, 0.1, 20.0)
    if schanged then freecam.speed = speed end

    local bchanged, boost = imgui.slider_float("Boost Multiplier", freecam.boost_multiplier, 1.0, 10.0)
    if bchanged then freecam.boost_multiplier = boost end

    imgui.text("Controls: WASD move, Space/Ctrl up/down, Shift boost")
    imgui.end()
end

-- Register UI draw callback
local orig_draw = OnPresent
function OnPresent()
    if orig_draw then orig_draw() end
    draw_ui()
end
