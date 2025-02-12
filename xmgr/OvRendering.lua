
local over_dir = path.join(os.projectdir(),"Sources","Overload")
local proj_dir =  path.join(over_dir,"OvRendering")
local dep_dir = path.join(os.projectdir(),"Dependencies")

local function add_src(...)
    for _, fn in ipairs({...}) do
        add_files(path.join(proj_dir,"src",fn))
    end
end

local function add_inc(...)
    for _, fn in ipairs({...}) do
        add_headerfiles(path.join(proj_dir,"include",fn))
    end
end


target("OvRendering")
    set_languages("c++17")
    set_kind("static")
    add_src("**.cpp")
    add_inc("**.h","**.inl")
    add_deps("OvDebug","OvMaths")
    add_includedirs(path.join(proj_dir,"include"),{public=true})
    add_includedirs(path.join(dep_dir,"glew/include"),{public=true})
    add_includedirs(path.join(dep_dir,"assimp/include"),{public=true})
    add_includedirs(path.join(dep_dir,"stb_image/include"),{public=true})
