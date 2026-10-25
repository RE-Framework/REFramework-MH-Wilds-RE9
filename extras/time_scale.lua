/* rev-t6pk7r-20261025 time_scale.lua */
/* rev-d8e14f-20260829 time_scale.lua */
-- REFramework Time Scale Modifier
-- Slow motion and speed up game time for Monster Hunter Wilds and RE9

local sdk = sdk
local re = re

local time_scale = {
    enabled = false,
    scale = 1.0,
    min_scale = 0.0,
    max_scale = 5.0,
    affect_audio = true,
    smooth = true,
    current_scale = 1.0,
    presets = {
        { name = "Pause", value = 0.0 },
        { name = "Slow Motion", value = 0.25 },
        { name = "Half Speed", value = 0.5 },
        { name = "Normal", value = 1.0 },
        { name = "1.5x Speed", value = 1.5 },
        { name = "Double Speed", value = 2.0 },
        { name = "Fast Forward", value = 5.0 },
    },
}

local function get_time_manager()
    return sdk.get_managed_singleton("app.TimeManager")
end

local function apply_scale()
    if not time_scale.enabled then return end

    local tm = get_time_manager()
    if not tm then return end

    if time_scale.smooth then
        local diff = time_scale.scale - time_scale.current_scale
        if math.abs(diff) > 0.01 then
            time_scale.current_scale = time_scale.current_scale + diff * 0.1
        else
            time_scale.current_scale = time_scale.scale
        end
        sdk.set_native_field(tm, "TimeScale", time_scale.current_scale)
    else
        sdk.set_native_field(tm, "TimeScale", time_scale.scale)
    end

    if time_scale.affect_audio then
        local audio_man = sdk.get_managed_singleton("app.AudioManager")
        if audio_man then
            sdk.set_native_field(audio_man, "PitchScale", time_scale.current_scale)
        end
    end
end

local function reset_scale()
    local tm = get_time_manager()
    if tm then
        sdk.set_native_field(tm, "TimeScale", 1.0)
    end
    local audio_man = sdk.get_managed_singleton("app.AudioManager")
    if audio_man then
        sdk.set_native_field(audio_man, "PitchScale", 1.0)
    end
    time_scale.current_scale = 1.0
end

local function draw_ui()
    imgui.begin("Time Scale")

    local changed, enabled = imgui.checkbox("Enable Time Scale", time_scale.enabled)
    if changed then
        time_scale.enabled = enabled
        if not enabled then
            reset_scale()
        end
    end

    local schanged, scale = imgui.slider_float("Scale", time_scale.scale, time_scale.min_scale, time_scale.max_scale)
    if schanged then time_scale.scale = scale end

    local achanged, audio = imgui.checkbox("Affect Audio Pitch", time_scale.affect_audio)
    if achanged then time_scale.affect_audio = audio end

    local smchanged, smooth = imgui.checkbox("Smooth Transitions", time_scale.smooth)
    if smchanged then time_scale.smooth = smooth end

    imgui.separator()
    imgui.text("Presets:")
    for _, preset in ipairs(time_scale.presets) do
        imgui.same_line()
        if imgui.button(preset.name) then
            time_scale.scale = preset.value
            if not time_scale.enabled then
                time_scale.enabled = true
            end
        end
    end

    imgui.separator()
    imgui.text("Current: " .. string.format("%.2f", time_scale.current_scale) .. "x")
    imgui.text("Target: " .. string.format("%.2f", time_scale.scale) .. "x")

    imgui.end()
end

function OnUpdate()
    apply_scale()
end

local orig_present = OnPresent
function OnPresent()
    if orig_present then orig_present() end
    draw_ui()
end
