/* rev-t6pk7r-20261025 debug_menu.lua */
/* rev-d8e14f-20260829 debug_menu.lua */
-- REFramework Debug Menu
-- Entity inspector, transform editor, spawn menu, and runtime memory viewer

local sdk = sdk
local re = re

local debug_menu = {
    selected_entity = nil,
    show_inspector = true,
    show_spawn_menu = false,
    show_memory = false,
    entity_filter = "",
    entities = {},
    memory_address = 0,
    memory_size = 64,
}

local function refresh_entities()
    debug_menu.entities = {}

    local scene_man = sdk.get_managed_singleton("app.SceneManager")
    if not scene_man then return end

    local root = sdk.call_native(scene_man, "get_RootGameObject")
    if not root then return end

    local function traverse(obj, depth)
        if depth > 5 then return end
        if #debug_menu.entities > 500 then return end

        local name = sdk.get_native_field(obj, "Name")
        if name then
            table.insert(debug_menu.entities, {
                name = name,
                object = obj,
                depth = depth,
            })
        end

        local children = sdk.get_native_field(obj, "Children")
        if children then
            for _, child in ipairs(children) do
                traverse(child, depth + 1)
            end
        end
    end

    traverse(root, 0)
end

local function draw_inspector()
    imgui.begin("Entity Inspector")

    local changed, filter = imgui.input_text("Filter", debug_menu.entity_filter, 128)
    if changed then debug_menu.entity_filter = filter end

    imgui.same_line()
    if imgui.button("Refresh") then
        refresh_entities()
    end

    imgui.separator()

    for _, entry in ipairs(debug_menu.entities) do
        if debug_menu.entity_filter == "" or string.find(string.lower(entry.name), string.lower(debug_menu.entity_filter)) then
            local indent = string.rep("  ", entry.depth)
            local sel = (debug_menu.selected_entity == entry.object)
            if imgui.checkbox("##" .. entry.name, sel) then
                debug_menu.selected_entity = sel and nil or entry.object
            end
            imgui.same_line()
            imgui.text(indent .. entry.name)
        end
    end

    imgui.end()
end

local function draw_transform_editor()
    if not debug_menu.selected_entity then return end

    imgui.begin("Transform Editor")

    local transform = sdk.get_native_field(debug_menu.selected_entity, "Transform")
    if not transform then
        imgui.text("No transform component")
        imgui.end()
        return
    end

    local pos = sdk.get_native_field(transform, "Position")
    if pos then
        local changed = false
        local nx, ny, nz = pos.x, pos.y, pos.z
        changed, nx = imgui.slider_float("X", nx, -10000, 10000)
        if changed then pos.x = nx end
        changed, ny = imgui.slider_float("Y", ny, -10000, 10000)
        if changed then pos.y = ny end
        changed, nz = imgui.slider_float("Z", nz, -10000, 10000)
        if changed then pos.z = nz end
        sdk.set_native_field(transform, "Position", pos)
    end

    local rot = sdk.get_native_field(transform, "Rotation")
    if rot then
        local changed = false
        local nx, ny, nz, nw = rot.x, rot.y, rot.z, rot.w
        changed, nx = imgui.slider_float("RotX", nx, -1.0, 1.0)
        if changed then rot.x = nx end
        changed, ny = imgui.slider_float("RotY", ny, -1.0, 1.0)
        if changed then rot.y = ny end
        changed, nz = imgui.slider_float("RotZ", nz, -1.0, 1.0)
        if changed then rot.z = nz end
        changed, nw = imgui.slider_float("RotW", nw, -1.0, 1.0)
        if changed then rot.w = nw end
        sdk.set_native_field(transform, "Rotation", rot)
    end

    local scale = sdk.get_native_field(transform, "Scale")
    if scale then
        local changed, ns = imgui.slider_float("Scale", scale.x, 0.01, 100.0)
        if changed then
            scale.x = ns
            scale.y = ns
            scale.z = ns
            sdk.set_native_field(transform, "Scale", scale)
        end
    end

    imgui.end()
end

local function draw_memory_viewer()
    if not debug_menu.show_memory then return end

    imgui.begin("Memory Viewer")

    local changed, addr = imgui.input_int("Address", debug_menu.memory_address)
    if changed then debug_menu.memory_address = addr end

    local schanged, size = imgui.input_int("Size", debug_menu.memory_size)
    if schanged then debug_menu.memory_size = math.max(1, math.min(1024, size)) end

    imgui.separator()

    if debug_menu.memory_address > 0 then
        imgui.text("Address: 0x" .. string.format("%X", debug_menu.memory_address))
        imgui.text("Size: " .. debug_menu.memory_size .. " bytes")
    end

    imgui.end()
end

local function draw_spawn_menu()
    if not debug_menu.show_spawn_menu then return end

    imgui.begin("Spawn Menu")

    local presets = {
        "app.NPC.Hunter",
        "app.NPC.Villager",
        "app.Prop.Campfire",
        "app.Prop.Torch",
        "app.Effect.Fire",
        "app.Effect.Smoke",
    }

    for _, preset in ipairs(presets) do
        if imgui.button(preset) then
            local go = sdk.call_native(sdk.get_managed_singleton("app.GameObjectManager"), "spawn", preset)
            if go then
                re.log("Spawned: " .. preset)
            else
                re.log("Failed to spawn: " .. preset)
            end
        end
    end

    imgui.end()
end

local function draw_control_panel()
    imgui.begin("Debug Control Panel")

    local ichanged, inspector = imgui.checkbox("Entity Inspector", debug_menu.show_inspector)
    if ichanged then debug_menu.show_inspector = inspector end

    local schanged, spawn = imgui.checkbox("Spawn Menu", debug_menu.show_spawn_menu)
    if schanged then debug_menu.show_spawn_menu = spawn end

    local mchanged, memory = imgui.checkbox("Memory Viewer", debug_menu.show_memory)
    if mchanged then debug_menu.show_memory = memory end

    imgui.separator()
    imgui.text("Total entities: " .. #debug_menu.entities)

    imgui.end()
end

local init_done = false
local orig_present = OnPresent
function OnPresent()
    if orig_present then orig_present() end

    if not init_done then
        refresh_entities()
        init_done = true
    end

    draw_control_panel()
    if debug_menu.show_inspector then draw_inspector() end
    draw_transform_editor()
    draw_spawn_menu()
    draw_memory_viewer()
end
