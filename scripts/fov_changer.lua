/* rev-t6pk7r-20261025 fov_changer.lua */
/* rev-d8e14f-20260829 fov_changer.lua */
-- REFramework FOV Changer
-- Adjustable field of view for Monster Hunter Wilds and RE9

local sdk = sdk

local fov_config = {
    enabled = false,
    target_fov = 75.0,
    min_fov = 30.0,
    max_fov = 120.0,
    affect_cutscenes = true,
    smooth_transition = true,
    current_fov = 75.0,
}

local function get_camera()
    return sdk.get_primary_camera()
end

local function apply_fov()
    if not fov_config.enabled then return end
    local cam = get_camera()
    if not cam then return end

    local fov_field = "FieldOfView"
    local current = sdk.get_native_field(cam, fov_field)
    if current == nil then return end

    if fov_config.smooth_transition then
        local diff = fov_config.target_fov - fov_config.current_fov
        if math.abs(diff) > 0.1 then
            fov_config.current_fov = fov_config.current_fov + diff * 0.1
        else
            fov_config.current_fov = fov_config.target_fov
        end
        sdk.set_native_field(cam, fov_field, fov_config.current_fov)
    else
        sdk.set_native_field(cam, fov_field, fov_config.target_fov)
    end
end

local function on_pre_camera_update()
    apply_fov()
end

-- UI
local function draw_ui()
    imgui.begin("FOV Changer")

    local changed, enabled = imgui.checkbox("Enable Custom FOV", fov_config.enabled)
    if changed then
        fov_config.enabled = enabled
        if enabled then
            local cam = get_camera()
            if cam then
                fov_config.current_fov = sdk.get_native_field(cam, "FieldOfView") or 75.0
            end
        end
    end

    local fchanged, fov = imgui.slider_float("FOV", fov_config.target_fov, fov_config.min_fov, fov_config.max_fov)
    if fchanged then fov_config.target_fov = fov end

    local schanged, smooth = imgui.checkbox("Smooth Transition", fov_config.smooth_transition)
    if schanged then fov_config.smooth_transition = smooth end

    local cchanged, cutscenes = imgui.checkbox("Affect Cutscenes", fov_config.affect_cutscenes)
    if cchanged then fov_config.affect_cutscenes = cutscenes end

    imgui.separator()
    imgui.text("Current FOV: " .. string.format("%.1f", fov_config.current_fov))
    imgui.text("Target FOV: " .. string.format("%.1f", fov_config.target_fov))

    imgui.end()
end

-- Hooks
if OnPreCameraUpdate then
    local orig = OnPreCameraUpdate
    OnPreCameraUpdate = function()
        orig()
        on_pre_camera_update()
    end
else
    OnPreCameraUpdate = on_pre_camera_update
end

local orig_present = OnPresent
function OnPresent()
    if orig_present then orig_present() end
    draw_ui()
end
