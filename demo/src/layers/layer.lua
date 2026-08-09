local camera = nil
local tallBox = nil
local time = 0
local displayedFps = 0
local fpsTimer = 0

function onAttach()
    print("[LUA DEBUG] Lua Cornell Box layer attached")

    camera = scene:addFlyingCamera()
    camera:setPosition(Vector3.new(0, 0, 5.5))
    camera:setRotation(Vector3.new(0, -90, 0))

    local cubeMesh = MeshFactory.cube()

    -- Cornell Box Walls
    -- Floor 
    local floor = scene:addEntity()
    floor:setMesh(cubeMesh)
    floor:setPosition(Vector3.new(0, -2, 0))
    floor:setScale(Vector3.new(4, 0.1, 4))
    floor:setColor(Vector4.new(0.8, 0.8, 0.8, 1))

    -- Ceiling 
    local ceiling = scene:addEntity()
    ceiling:setMesh(cubeMesh)
    ceiling:setPosition(Vector3.new(0, 2, 0))
    ceiling:setScale(Vector3.new(4, 0.1, 4))
    ceiling:setColor(Vector4.new(0.8, 0.8, 0.8, 1))

    -- Back Wall
    local backWall = scene:addEntity()
    backWall:setMesh(cubeMesh)
    backWall:setPosition(Vector3.new(0, 0, -2))
    backWall:setScale(Vector3.new(4, 4, 0.1))
    backWall:setColor(Vector4.new(0.8, 0.8, 0.8, 1))

    -- Left Wall 
    local leftWall = scene:addEntity()
    leftWall:setMesh(cubeMesh)
    leftWall:setPosition(Vector3.new(-2, 0, 0))
    leftWall:setScale(Vector3.new(0.1, 4, 4))
    leftWall:setColor(Vector4.new(0.8, 0.1, 0.1, 1))

    -- Right Wall
    local rightWall = scene:addEntity()
    rightWall:setMesh(cubeMesh)
    rightWall:setPosition(Vector3.new(2, 0, 0))
    rightWall:setScale(Vector3.new(0.1, 4, 4))
    rightWall:setColor(Vector4.new(0.1, 0.8, 0.1, 1))

    -- Light on Ceiling
    local light = scene:addEntity()
    light:setMesh(cubeMesh)
    light:setPosition(Vector3.new(0, 1.95, 0))
    light:setScale(Vector3.new(1, 0.02, 1))
    light:setColor(Vector4.new(1, 1, 0.9, 1))

    -- Short Box
    local shortBox = scene:addEntity()
    shortBox:setMesh(cubeMesh)
    shortBox:setPosition(Vector3.new(0.5, -1.4, 0.5))
    shortBox:setScale(Vector3.new(1.2, 1.2, 1.2))
    shortBox:setRotation(Vector3.new(0, -18, 0))
    shortBox:setColor(Vector4.new(0.75, 0.75, 0.75, 1))

    -- Tall Box
    tallBox = scene:addEntity()
    tallBox:setMesh(cubeMesh)
    tallBox:setPosition(Vector3.new(-0.5, -0.8, -0.3))
    tallBox:setScale(Vector3.new(1.2, 2.0, 1.2))
    tallBox:setRotation(Vector3.new(0, 0, 0))
    tallBox:setColor(Vector4.new(0.0, 0.75, 0.75, 1))
end

function onUpdate(dt)
    time = time + dt

    if tallBox then
        tallBox:setRotation(Vector3.new(0, time * 20, 0))
    end
end

function onRender(renderer)
    renderer:render(scene, app:getWidth(), app:getHeight())

    ImGui.SetNextWindowCollapsed(true)
    ImGui.Begin("Lua Script Controller")
    ImGui.Text("Hello from Antigravity Live!")
    ImGui.Separator()
    if camera then
        local pos = camera:getPosition()
        local rot = camera:getRotation()
        ImGui.Text("Camera Pos: X:" .. string.format("%.2f", pos.x) .. ", Y:" .. string.format("%.2f", pos.y) .. ", Z:" .. string.format("%.2f", pos.z))
        ImGui.Text("Camera Rot: Yaw:" .. string.format("%.1f", rot.y) .. ", Pitch:" .. string.format("%.1f", rot.x))
    end
    ImGui.End()
end
