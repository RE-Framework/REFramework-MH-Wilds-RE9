/* rev-t6pk7r-20261025 no_clip.lua */
/* rev-d8e14f-20260829 no_clip.lua */
-- REFramework No-Clip Script
-- Disable collision detection and fly through walls
-- Supports Monster Hunter Wilds and RE9

local sdk = sdk
local re = re

local noclip = {
    enabled = false,
    speed = 5.0,
    saved_collision = nil,
}

local function get_player()
    local player_man = sdk.get_managed_singleton("app.PlayerManager")
    if not player_man then return nil end
    return sdk.call_native(player_man, "get_Player")
end

local function get_player_transform()
    local player = get_player()
    if not player then return nil end
    local transform = sdk.get_native_field(player, "Transform")
    if not transform then return nil end
    return transform
end

local function disable_collision()
    local player = get_player()
    if not player then return end

    local collider = sdk.get_native_field(player, "ColliderComponent")
    if collider then
        noclip.saved_collision = collider
        sdk.set_native_field(collider, "Enabled", false)
    end

    local characterController = sdk.get_native_field(player, "CharacterController")
    if characterController then
        sdk.set_native_field(characterController, "Enabled", false)
    end
end

local function enable_collision()
    if noclip.saved_collision then
        sdk.set_native_field(noclip.saved_collision, "Enabled", true)
        noclip.saved_collision = nil
    end

    local player = get_player()
    if not player then return end

    local characterController = sdk.get_native_field(player, "CharacterController")
    if characterController then
        sdk.set_native_field(characterController, "Enabled", true)
    end
end

local function update_noclip()
    if not noclip.enabled then return end

    local player = get_player()
    if not player then return end

    local transform = get_player_transform()
    if not transform then return end

    local pos = sdk.get_native_field(transform, "Position")
    if not pos then return end

    local cam = sdk.get_primary_camera()
    local forward = cam and sdk.get_native_field(cam, "ForwardVector") or { x = 0, y = 0, z = 1 }
    local right = cam and sdk.get_native_field(cam, "RightVector") or { x = 1, y = 0, z = 0 }

    local speed = noclip.speed
    local move = { x = 0, y = 0, z = 0 }

    if imgui.is_key_down(87) then -- W
        move.x = move.x + forward.x * speed
        move.y = move.y + forward.y * speed
        move.z = move.z + forward.z * speed
    end
    if imgui.is_key_down(83) then -- S
        move.x = move.x - forward.x * speed
        move.y = move.y - forward.y * speed
        move.z = move.z - forward.z * speed
    end
    if imgui.is_key_down(65) then -- A
        move.x = move.x - right.x * speed
        move.z = move.z - right.z * speed
    end
    if imgui.is_key_down(68) then -- D
        move.x = move.x + right.x * speed
        move.z = move.z + right.z * speed
    end
    if imgui.is_key_down(32) then -- Space
        move.y = move.y + speed
    end
    if imgui.is_key_down(17) then -- Ctrl
        move.y = move.y - speed
    end

    pos.x = pos.x + move.x
    pos.y = pos.y + move.y
    pos.z = pos.z + move.z

    sdk.set_native_field(transform, "Position", pos)
end

function OnUpdate()
    update_noclip()
end

-- UI
local orig_present = OnPresent
function OnPresent()
    if orig_present then orig_present() end

    imgui.begin("No-Clip")
    local changed, enabled = imgui.checkbox("Enable No-Clip", noclip.enabled)
    if changed then
        noclip.enabled = enabled
        if enabled then
            disable_collision()
        else
            enable_collision()
        end
    end

    local schanged, speed = imgui.slider_float("Speed", noclip.speed, 1.0, 30.0)
    if schanged then noclip.speed = speed end

    imgui.text("Controls: WASD to move, Space/Ctrl for up/down")
    imgui.separator()
    if noclip.enabled then
        imgui.text("WARNING: Collision disabled. Do not enter triggers.")
    end
    imgui.end()
end
