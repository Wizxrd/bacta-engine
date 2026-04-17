const vscode = require("vscode");
const path = require("path");
const fs = require("fs");
const os = require("os");

const PROJECT_PATH_KEY = "taskbar.projectPath";
const WORKSPACE_ROOT_KEY = "taskbar.workspaceRoot";
const TOOLCHAIN_KEY = "taskbar.toolchain";
const COMPILER_PATH_KEY = "taskbar.compilerPath";
const IMGUI_ENABLED_KEY = "taskbar.imguiEnabled";
/** Persisted via workspaceState so path picks work even if Settings schema is stale. */
const OPENGL_INCLUDE_KEY = "taskbar.openglIncludePath";
const OPENGL_LIB_KEY = "taskbar.openglLibPath";
const GLFW_LINK_MODE_KEY = "taskbar.glfwLinkMode";
const IMGUI_INCLUDE_KEY = "taskbar.imguiIncludePath";
const IMGUI_SOURCE_DIR_KEY = "taskbar.imguiSourceDir";

const TOOLCHAIN_PRESETS = {
    mingw64: { label: "MinGW 64-bit", key: "mingw64" },
    mingw32: { label: "MinGW 32-bit", key: "mingw32" },
    custom: { label: "Custom (browse for compiler)", key: "custom" }
};

const DEFAULT_WORKSPACE_ROOT = "";
const DEFAULT_PROJECT_PATH = "";
const NEW_PROJECT_FOLDERS = ["assets", "bin", "include", "lib", "src"];

/** MinGW runtime DLLs often needed next to the exe (adjust paths if your install differs). */
const NEW_PROJECT_MINGW_DLL_SOURCES = [
    "c:\\mingw64\\libexec\\gcc\\x86_64-w64-mingw32\\14.2.0\\libgcc_s_seh-1.dll",
    "c:\\mingw64\\libexec\\gcc\\x86_64-w64-mingw32\\14.2.0\\libwinpthread-1.dll",
    "c:\\mingw64\\bin\\libstdc++-6.dll"
];

const BUILD_TASK_LABEL = "Build current project (ImGui + OpenGL)";

var BUILD_WIDGET = vscode.window.createStatusBarItem();
var RUN_WIDGET = vscode.window.createStatusBarItem();
var BUILD_RUN_WIDGET = vscode.window.createStatusBarItem();
var BUILD_DEBUG_WIDGET = vscode.window.createStatusBarItem();
var PROJECT_WIDGET = vscode.window.createStatusBarItem();
var WORKSPACE_WIDGET = vscode.window.createStatusBarItem();
var TOOLCHAIN_WIDGET = vscode.window.createStatusBarItem();
var COMPILER_WIDGET = vscode.window.createStatusBarItem();
var OPENGL_INC_WIDGET = vscode.window.createStatusBarItem();
var OPENGL_LIB_WIDGET = vscode.window.createStatusBarItem();
var GLFW_LINK_WIDGET = vscode.window.createStatusBarItem();
var IMGUI_WIDGET = vscode.window.createStatusBarItem();
var PROBLEMS_WIDGET = vscode.window.createStatusBarItem();
var CLEAN_WIDGET = vscode.window.createStatusBarItem();
var NEW_PROJECT_WIDGET = vscode.window.createStatusBarItem();
var DOCS_WIDGET = vscode.window.createStatusBarItem();
var buildAndDebugListener = null;

function CreateWidget(widget, text, tooltip, command, context, color = "white") {
    widget.command = command;
    widget.text = text;
    widget.tooltip = tooltip;
    if (color) widget.color = color;
    context.subscriptions.push(widget);
    widget.show();
}

function normalizeSlashes(value) {
    return String(value || "").replace(/\\/g, "/");
}

function ensureDir(dirPath) {
    fs.mkdirSync(dirPath, { recursive: true });
}

function safeRelative(basePath, targetPath) {
    const rel = path.relative(basePath, targetPath);
    if (!rel || rel === "") return "";
    if (rel.startsWith("..") || path.isAbsolute(rel)) return null;
    return rel;
}

function getOpenWorkspaceFolder() {
    return vscode.workspace.workspaceFolders?.[0] || null;
}

function getStoredWorkspaceRoot(context) {
    return context.workspaceState.get(WORKSPACE_ROOT_KEY, DEFAULT_WORKSPACE_ROOT);
}

function getWorkspaceRoot(context) {
    const workspaceFolder = getOpenWorkspaceFolder();
    const stored = getStoredWorkspaceRoot(context);

    if (stored && path.isAbsolute(stored)) return stored;
    if (workspaceFolder) return workspaceFolder.uri.fsPath;
    return "";
}

async function setWorkspaceRoot(context, folderPath) {
    await context.workspaceState.update(WORKSPACE_ROOT_KEY, folderPath || "");

    const projectFullPath = getProjectFullPath(context);
    if (!projectFullPath) return;

    const rel = safeRelative(folderPath, projectFullPath);
    if (rel !== null) {
        await context.workspaceState.update(PROJECT_PATH_KEY, rel);
    }
}

function getProjectPath(context) {
    return context.workspaceState.get(PROJECT_PATH_KEY, DEFAULT_PROJECT_PATH);
}

function getProjectFullPath(context) {
    const stored = getProjectPath(context);
    const root = getWorkspaceRoot(context);

    if (!stored) return null;
    if (path.isAbsolute(stored)) return stored;
    if (!root) return null;

    return path.join(root, stored);
}

async function setProjectPath(context, fullPath) {
    const root = getWorkspaceRoot(context);
    const rel = root ? safeRelative(root, fullPath) : null;
    await context.workspaceState.update(PROJECT_PATH_KEY, rel !== null ? rel : fullPath);
}

function getProjectDisplayName(context) {
    const stored = getProjectPath(context);
    if (stored) return stored;

    const full = getProjectFullPath(context);
    if (full) return path.basename(full);

    return "None";
}

function updateWorkspaceWidget(context) {
    const root = getWorkspaceRoot(context);
    const label = root ? path.basename(root) || root : "No Root";
    WORKSPACE_WIDGET.text = `🗂️ Workspace: ${label}`;
    WORKSPACE_WIDGET.tooltip = root
        ? `Workspace: ${root}. Click to change.`
        : "No workspace is set. Click to choose one.";
}

function updateProjectWidget(context) {
    const projectDisplay = getProjectDisplayName(context);
    const projectFullPath = getProjectFullPath(context);
    PROJECT_WIDGET.text = `📁 Project: ${projectDisplay}`;
    PROJECT_WIDGET.tooltip = projectFullPath
        ? `Project folder: ${projectFullPath}. Click to change.`
        : "No project folder selected. Click to choose one.";
}

function getOpenglConfig(context) {
    const config = vscode.workspace.getConfiguration("taskbar");
    const incWs = context.workspaceState.get(OPENGL_INCLUDE_KEY);
    const libWs = context.workspaceState.get(OPENGL_LIB_KEY);
    const modeWs = context.workspaceState.get(GLFW_LINK_MODE_KEY);
    return {
        include: incWs !== undefined ? incWs : config.get("opengl.include", ""),
        lib: libWs !== undefined ? libWs : config.get("opengl.lib", ""),
        glfwLinkMode: modeWs !== undefined ? modeWs : config.get("opengl.glfwLinkMode", "dynamic")
    };
}

function getSdkIncludePaths(context) {
    const config = vscode.workspace.getConfiguration("taskbar");
    const arr = config.get("sdkIncludePaths");
    if (!Array.isArray(arr)) return [];
    return arr.filter((p) => p && String(p).trim()).map((p) => normalizeSlashes(String(p).trim()));
}

function getSdkLibPaths(context) {
    const config = vscode.workspace.getConfiguration("taskbar");
    const arr = config.get("sdkLibPaths");
    if (!Array.isArray(arr)) return [];
    return arr.filter((p) => p && String(p).trim()).map((p) => normalizeSlashes(String(p).trim()));
}

/**
 * MinGW rarely has libglew32 in the official GLEW zip (MSVC-only). Prefer compiling glew.c
 * from the GLEW source tree with -DGLEW_STATIC when glewSourceFile exists.
 */
function resolveGlewBuild(context, projectRoot) {
    if (mainCppUsesGlad(projectRoot)) return { mode: "none" };
    if (!mainCppUsesGlew(projectRoot)) return { mode: "none" };
    const config = vscode.workspace.getConfiguration("taskbar");
    const libFile = (config.get("glewLibraryFile") || "").trim();
    if (libFile && fs.existsSync(libFile)) {
        return { mode: "link", libPath: libFile };
    }
    const srcDefault = "c:/glew-2.3.1/src/glew.c";
    const src = (config.get("glewSourceFile") || "").trim() || srcDefault;
    if (fs.existsSync(src)) {
        return { mode: "static", src };
    }
    return { mode: "link", libPath: null };
}

function getImGuiConfig(context) {
    const config = vscode.workspace.getConfiguration("taskbar");
    const enabledStored = context.workspaceState.get(IMGUI_ENABLED_KEY);
    const enabled = enabledStored !== undefined ? enabledStored : config.get("imgui.enabled", true);
    const incWs = context.workspaceState.get(IMGUI_INCLUDE_KEY);
    const srcWs = context.workspaceState.get(IMGUI_SOURCE_DIR_KEY);
    return {
        enabled,
        include: incWs !== undefined ? incWs : config.get("imgui.include", ""),
        sourceDir: srcWs !== undefined ? srcWs : config.get("imgui.sourceDir", "")
    };
}

function updateCppPropertiesIncludePath(context) {
    const workspaceFolder = getOpenWorkspaceFolder();
    if (!workspaceFolder) return;

    const projectPath = getProjectPath(context);
    const projectFullPath = getProjectFullPath(context);
    const propsPath = path.join(workspaceFolder.uri.fsPath, ".vscode", "c_cpp_properties.json");
    if (!fs.existsSync(propsPath)) return;

    const gl = getOpenglConfig(context);
    const imgui = getImGuiConfig(context);
    const projectInclude = projectFullPath
        ? normalizeSlashes(path.join(projectFullPath, "include"))
        : normalizeSlashes(path.join(projectPath || "", "include"));

    const includePath = [projectInclude];
    if (gl.include) includePath.push(normalizeSlashes(gl.include));
    for (const p of getSdkIncludePaths(context)) includePath.push(p);
    if (imgui.enabled) {
        const imRoot = imgui.include || imgui.sourceDir;
        if (imRoot) includePath.push(normalizeSlashes(imRoot));
    }

    try {
        const raw = fs.readFileSync(propsPath, "utf8");
        const props = JSON.parse(raw);
        if (!props.configurations || !Array.isArray(props.configurations)) return;

        for (const config of props.configurations) {
            config.includePath = includePath;
            config.browse = config.browse || {};
            config.browse.path = includePath;
        }

        fs.writeFileSync(propsPath, JSON.stringify(props, null, 4), "utf8");
    } catch (err) {
        console.warn("taskbar: could not update c_cpp_properties.json", err);
    }
}

/**
 * When the selected project folder is inside the workspace, use ${workspaceFolder}/...
 * so tasks.json matches the C/C++ extension style. Otherwise use absolute paths.
 */
function getProjectPathVarsForTasks(context) {
    const workspaceFolder = getOpenWorkspaceFolder();
    if (!workspaceFolder) return null;
    const workspaceRoot = workspaceFolder.uri.fsPath;
    const projectRoot = getProjectFullPath(context) || workspaceRoot;
    const rel = path.relative(workspaceRoot, projectRoot);
    const insideWorkspace = !rel.startsWith("..") && !path.isAbsolute(rel);

    if (!insideWorkspace) {
        const root = projectRoot.replace(/\\/g, "/");
        return {
            cwd: root,
            p: (subPath) => path.join(projectRoot, subPath).replace(/\\/g, "/")
        };
    }

    const base =
        !rel || rel === "."
            ? "${workspaceFolder}"
            : "${workspaceFolder}/" + rel.replace(/\\/g, "/");
    return {
        cwd: base,
        p: (subPath) => `${base}/${subPath.replace(/\\/g, "/")}`
    };
}

function isCppSourceFile(name) {
    const lower = name.toLowerCase();
    return (
        lower.endsWith(".cpp") ||
        lower.endsWith(".cc") ||
        lower.endsWith(".cxx") ||
        lower.endsWith(".pp")
    );
}

/** Ordered source list: main.cpp (or main.*) first, other C++, then glad.c, then other .c */
function collectExplicitSrcFiles(projectRoot) {
    const srcDir = path.join(projectRoot, "src");
    const out = { cpp: [], c: [] };
    if (!fs.existsSync(srcDir)) return out;

    for (const name of fs.readdirSync(srcDir)) {
        const full = path.join(srcDir, name);
        if (!fs.statSync(full).isFile()) continue;
        const lower = name.toLowerCase();
        if (isCppSourceFile(name)) out.cpp.push(name);
        else if (lower.endsWith(".c")) out.c.push(name);
    }

    function cppSortKey(f) {
        const l = f.toLowerCase();
        if (l === "main.cpp") return 0;
        if (l.startsWith("main.")) return 1;
        if (/^main[^/\\]*\.(cpp|cc|cxx|pp)$/i.test(f)) return 1;
        return 2;
    }

    out.cpp.sort((a, b) => {
        const d = cppSortKey(a) - cppSortKey(b);
        return d !== 0 ? d : a.localeCompare(b);
    });
    out.c.sort((a, b) => {
        if (a.toLowerCase() === "glad.c") return -1;
        if (b.toLowerCase() === "glad.c") return 1;
        return a.localeCompare(b);
    });
    return out;
}

function mainCppUsesGlad(projectRoot) {
    const names = ["main.cpp", "main.cc", "main.cxx", "main.pp", "mainc.pp"];
    for (const name of names) {
        const full = path.join(projectRoot, "src", name);
        if (!fs.existsSync(full)) continue;
        try {
            const t = fs.readFileSync(full, "utf8");
            if (/#include\s*[<"]glad\/glad\.h[>"]/i.test(t)) return true;
        } catch {
            continue;
        }
    }
    return false;
}

/** GLAD and GLEW both load GL entry points — do not compile glad.c with GLEW. */
function mainCppUsesGlew(projectRoot) {
    const names = ["main.cpp", "main.cc", "main.cxx", "main.pp", "mainc.pp"];
    for (const name of names) {
        const full = path.join(projectRoot, "src", name);
        if (!fs.existsSync(full)) continue;
        try {
            const t = fs.readFileSync(full, "utf8");
            if (/#include\s*[<"]GL\/glew\.h[>"]/i.test(t)) return true;
        } catch {
            continue;
        }
    }
    return false;
}

function updateTasksJson(context) {
    const workspaceFolder = getOpenWorkspaceFolder();
    if (!workspaceFolder) return;

    const tasksPath = path.join(workspaceFolder.uri.fsPath, ".vscode", "tasks.json");
    const projectFullPath = getProjectFullPath(context);
    const projectRoot = projectFullPath || workspaceFolder.uri.fsPath;
    const compilerPath = (getCompilerPath(context) || "C:\\mingw64\\bin\\g++.exe").replace(/\\/g, "/");
    const gl = getOpenglConfig(context);
    const imgui = getImGuiConfig(context);
    const pathVars = getProjectPathVarsForTasks(context);
    if (!pathVars) return;

    const glew = resolveGlewBuild(context, projectRoot);

    const localInclude = path.join(projectRoot, "include");
    const localLib = path.join(projectRoot, "lib");
    const glInclude = (gl.include || "").trim();
    const glLib = (gl.lib || "").trim();

    const args = ["-fdiagnostics-color=always", "-g", "-std=c++17", "-Wno-register"];
    if (glew.mode === "static") args.push("-DGLEW_STATIC");

    args.push(`-I${pathVars.p("include")}`);
    if (glInclude && path.normalize(glInclude) !== path.normalize(localInclude)) {
        args.push(`-I${glInclude.replace(/\\/g, "/")}`);
    }

    if (imgui.enabled) {
        const imRoot = (imgui.include || imgui.sourceDir || "").trim();
        if (imRoot) args.push(`-I${imRoot.replace(/\\/g, "/")}`);
    }

    for (const p of getSdkIncludePaths(context)) {
        args.push(`-I${p.replace(/\\/g, "/")}`);
    }

    args.push(`-L${pathVars.p("lib")}`);
    if (glLib && path.normalize(glLib) !== path.normalize(localLib)) {
        args.push(`-L${glLib.replace(/\\/g, "/")}`);
    }

    for (const lp of getSdkLibPaths(context)) {
        args.push(`-L${lp.replace(/\\/g, "/")}`);
    }

    const { cpp, c } = collectExplicitSrcFiles(projectRoot);
    const srcList = [];
    for (const f of cpp) srcList.push(pathVars.p(path.join("src", f).replace(/\\/g, "/")));
    for (const f of c) srcList.push(pathVars.p(path.join("src", f).replace(/\\/g, "/")));

    if (srcList.length === 0) {
        srcList.push(pathVars.p("src/main.cpp"));
    }

    /**
     * If tasks.json was generated when only glad.c existed, srcList is non-empty and the
     * fallback above is skipped — link then has no main(). Always pull in entry from disk
     * when present but missing from the list.
     */
    const mainCandidates = ["main.cpp", "main.cc", "main.cxx", "main.pp", "mainc.pp"];
    for (const name of mainCandidates) {
        const full = path.join(projectRoot, "src", name);
        if (!fs.existsSync(full)) continue;
        const arg = pathVars.p(`src/${name}`);
        if (!srcList.includes(arg)) srcList.unshift(arg);
        break;
    }

    if (mainCppUsesGlew(projectRoot) && !mainCppUsesGlad(projectRoot)) {
        const gladArg = pathVars.p("src/glad.c");
        srcList = srcList.filter((a) => a !== gladArg);
    } else {
        const gladFull = path.join(projectRoot, "src", "glad.c");
        if (fs.existsSync(gladFull)) {
            const gladArg = pathVars.p("src/glad.c");
            if (!srcList.includes(gladArg)) srcList.push(gladArg);
        }
    }

    for (const a of srcList) args.push(a);

    if (glew.mode === "static") {
        args.push("-x", "c", glew.src.replace(/\\/g, "/"));
    }

    if (imgui.enabled && imgui.sourceDir) {
        const sd = imgui.sourceDir.replace(/\\/g, "/");
        args.push(`${sd}/imgui.cpp`);
        args.push(`${sd}/imgui_draw.cpp`);
        args.push(`${sd}/imgui_tables.cpp`);
        args.push(`${sd}/imgui_widgets.cpp`);
        args.push(`${sd}/backends/imgui_impl_glfw.cpp`);
        args.push(`${sd}/backends/imgui_impl_opengl3.cpp`);
    }

    if (gl.glfwLinkMode === "static") {
        args.push("-lglfw3");
    } else {
        args.push("-lglfw3dll");
    }

    if (mainCppUsesGlew(projectRoot)) {
        if (glew.mode === "static") {
            /* glew.c already added with -x c */
        } else if (glew.libPath) {
            args.push(glew.libPath.replace(/\\/g, "/"));
        } else {
            args.push("-lglew32");
        }
    }

    args.push("-lopengl32", "-lgdi32", "-luser32", "-lshell32");
    /** Console subsystem + entry via main() (avoids MinGW linking as GUI / WinMain). */
    args.push("-mconsole");
    args.push("-o", pathVars.p("bin/main.exe"));

    const detail = `compiler: ${compilerPath}`;

    const payload = {
        version: "2.0.0",
        tasks: [
            {
                type: "cppbuild",
                label: BUILD_TASK_LABEL,
                command: compilerPath,
                args,
                options: {
                    cwd: pathVars.cwd
                },
                problemMatcher: ["$gcc"],
                group: { kind: "build", isDefault: true },
                detail
            }
        ]
    };

    try {
        ensureDir(path.dirname(tasksPath));
        ensureDir(path.join(projectRoot, "bin"));
        fs.writeFileSync(tasksPath, JSON.stringify(payload, null, 4), "utf8");
        console.log(`taskbar: tasks.json synced (cppbuild, ${srcList.length} source file(s)) → ${projectRoot}`);
    } catch (err) {
        console.error("taskbar: could not update tasks.json", err);
    }
}

function updateLaunchJson(context) {
    const workspaceFolder = getOpenWorkspaceFolder();
    if (!workspaceFolder) return;

    const projectPath = getProjectPath(context);
    const projectFullPath = getProjectFullPath(context);
    const launchPath = path.join(workspaceFolder.uri.fsPath, ".vscode", "launch.json");
    if (!fs.existsSync(launchPath) || !projectPath || !projectFullPath) return;

    const program = normalizeSlashes(path.join(projectFullPath, "bin", "main.exe"));
    const cwd = normalizeSlashes(projectFullPath);

    try {
        const raw = fs.readFileSync(launchPath, "utf8");
        const launch = JSON.parse(raw);
        if (!launch.configurations || !Array.isArray(launch.configurations)) return;

        const config = launch.configurations[0];
        if (!config) return;

        config.program = program;
        config.cwd = cwd;
        config.preLaunchTask = BUILD_TASK_LABEL;
        if (!config.name) config.name = "Debug";

        fs.writeFileSync(launchPath, JSON.stringify(launch, null, 4), "utf8");
    } catch (err) {
        console.warn("taskbar: could not update launch.json", err);
    }
}

function syncVscodeFiles(context) {
    updateCppPropertiesIncludePath(context);
    updateTasksJson(context);
    updateLaunchJson(context);
}

function getToolchain(context) {
    return context.workspaceState.get(TOOLCHAIN_KEY, "mingw64");
}

function getCompilerPath(context) {
    const toolchain = getToolchain(context);
    const customPath = context.workspaceState.get(COMPILER_PATH_KEY);
    if (toolchain === "custom" && customPath) return customPath;

    const config = vscode.workspace.getConfiguration("taskbar");
    if (toolchain === "mingw32") return config.get("compilerPath.mingw32");
    return config.get("compilerPath.mingw64");
}

function getGdbPath(context) {
    const toolchain = getToolchain(context);
    const config = vscode.workspace.getConfiguration("taskbar");
    if (toolchain === "mingw32") {
        const gpp = config.get("compilerPath.mingw32") || "";
        if (gpp) return path.join(path.dirname(gpp), "gdb.exe");
        return "C:\\mingw32\\bin\\gdb.exe";
    }
    return config.get("gdbPath.mingw64", "C:\\mingw64\\bin\\gdb.exe");
}

function updateToolchainWidget(context) {
    const toolchain = getToolchain(context);
    const preset = toolchain === "custom" ? "Custom" : (TOOLCHAIN_PRESETS[toolchain]?.label || toolchain);
    TOOLCHAIN_WIDGET.text = `🔧 ${preset}`;
    TOOLCHAIN_WIDGET.tooltip = `Toolchain: ${preset}. Click to change.`;
}

function updateCompilerWidget(context) {
    const compilerPath = getCompilerPath(context);
    const exeName = (compilerPath && path.basename(compilerPath)) || "?";
    COMPILER_WIDGET.text = `🖥️ ${exeName}`;
    COMPILER_WIDGET.tooltip = `Compiler: ${compilerPath || "not set"}. Click to select executable.`;
}

function updateOpenglWidgets(context) {
    const gl = getOpenglConfig(context);
    const inc = gl.include;
    const lib = gl.lib;

    if (inc) {
        OPENGL_INC_WIDGET.text = `📂 GL incl: ${path.basename(inc) || inc}`;
        OPENGL_INC_WIDGET.tooltip = `OpenGL-related includes (-I):\n${inc}\nClick to set include + lib folders.`;
    } else {
        OPENGL_INC_WIDGET.text = "📂 GL incl: (unset)";
        OPENGL_INC_WIDGET.tooltip = "Set folder for GLFW/GLAD/etc. headers (-I). Click to configure.";
    }

    if (lib) {
        OPENGL_LIB_WIDGET.text = `📚 GL lib: ${path.basename(lib) || lib}`;
        OPENGL_LIB_WIDGET.tooltip = `MinGW-w64 libs (-L):\n${lib}\nClick to set include + lib folders.`;
    } else {
        OPENGL_LIB_WIDGET.text = "📚 GL lib: (unset)";
        OPENGL_LIB_WIDGET.tooltip = "Set MinGW lib folder (libglfw3, glfw3.dll). Click to configure.";
    }

    const modeLabel = gl.glfwLinkMode === "static" ? "static GLFW" : "dynamic GLFW";
    GLFW_LINK_WIDGET.text = `🔗 ${modeLabel}`;
    GLFW_LINK_WIDGET.tooltip =
        gl.glfwLinkMode === "static"
            ? "Linking -lglfw3 (no glfw3.dll). Click to switch to dynamic."
            : "Linking -lglfw3dll; copy glfw3.dll to bin when running. Click to switch to static.";
}

function updateImGuiWidget(context) {
    const imgui = getImGuiConfig(context);
    if (!imgui.enabled) {
        IMGUI_WIDGET.text = "🧩 ImGui: Off";
        IMGUI_WIDGET.tooltip = "ImGui disabled. Click to enable and set Dear ImGui folder.";
        return;
    }

    const root = imgui.sourceDir || imgui.include;
    const label = root ? path.basename(root) : "ImGui";
    IMGUI_WIDGET.text = `🧩 ${label}`;
    IMGUI_WIDGET.tooltip = `Dear ImGui: ${root || "?"}\nCompiles core + imgui_impl_glfw / imgui_impl_opengl3. Click to change.`;
}

function getProblemsForProject(context) {
    const projectPath = getProjectFullPath(context);
    const root = getWorkspaceRoot(context);
    let errors = 0;
    let warnings = 0;

    const diags = vscode.languages.getDiagnostics();
    for (const [uri, list] of diags) {
        const fsPath = uri.fsPath || "";

        if (projectPath && !fsPath.startsWith(projectPath)) continue;
        if (!projectPath && root && !fsPath.startsWith(root)) continue;

        const ext = path.extname(fsPath).toLowerCase();
        if (![".cpp", ".cxx", ".cc", ".h", ".hpp"].includes(ext)) continue;

        for (const d of list) {
            if (d.severity === vscode.DiagnosticSeverity.Error) errors++;
            else if (d.severity === vscode.DiagnosticSeverity.Warning) warnings++;
        }
    }

    return { errors, warnings };
}

function updateProblemsWidget(context) {
    const { errors, warnings } = getProblemsForProject(context);
    if (errors === 0 && warnings === 0) {
        PROBLEMS_WIDGET.text = "✓ 0 errors, 0 warnings";
        PROBLEMS_WIDGET.tooltip = "No problems in project. Click to open Problems view.";
        PROBLEMS_WIDGET.color = "white";
        return;
    }

    const parts = [];
    if (errors) parts.push(`${errors} error${errors !== 1 ? "s" : ""}`);
    if (warnings) parts.push(`${warnings} warning${warnings !== 1 ? "s" : ""}`);

    PROBLEMS_WIDGET.text = `$(warning) ${parts.join(", ")}`;
    PROBLEMS_WIDGET.tooltip = "Click to open Problems view.";
    PROBLEMS_WIDGET.color = errors > 0 ? "#f48771" : "#dcdcaa";
}

function getProjectBinPath(projectPath) {
    return path.join(projectPath, "bin");
}

function getProjectExePath(projectPath) {
    const binPath = getProjectBinPath(projectPath);
    const mainExe = path.join(binPath, "main.exe");
    const folderExe = path.join(binPath, `${path.basename(projectPath)}.exe`);
    return fs.existsSync(mainExe) ? mainExe : folderExe;
}

function copyNewProjectDlls(projectRoot) {
    const libDir = path.join(projectRoot, "lib");
    for (const src of NEW_PROJECT_MINGW_DLL_SOURCES) {
        if (!fs.existsSync(src)) continue;
        fs.copyFileSync(src, path.join(libDir, path.basename(src)));
    }
}

/** Copy GLFW DLL and MinGW runtime from configured lib folder into project bin (dynamic GLFW). */
function copyRuntimeDllsToBin(context, projectRoot) {
    const gl = getOpenglConfig(context);
    const binPath = getProjectBinPath(projectRoot);
    ensureDir(binPath);

    if (gl.lib && gl.glfwLinkMode !== "static") {
        const glfwDll = path.join(gl.lib, "glfw3.dll");
        if (fs.existsSync(glfwDll)) {
            try {
                fs.copyFileSync(glfwDll, path.join(binPath, "glfw3.dll"));
            } catch (e) {
                console.warn("taskbar: could not copy glfw3.dll", e);
            }
        }
    }

    const projLib = path.join(projectRoot, "lib");
    if (fs.existsSync(projLib)) {
        const files = fs.readdirSync(projLib);
        for (const f of files) {
            if (!f.toLowerCase().endsWith(".dll")) continue;
            const src = path.join(projLib, f);
            const dest = path.join(binPath, f);
            try {
                fs.copyFileSync(src, dest);
            } catch (e) {
                console.warn("taskbar: could not copy", f, e);
            }
        }
    }
}

async function refreshUi(context) {
    updateWorkspaceWidget(context);
    updateProjectWidget(context);
    updateProblemsWidget(context);
    updateToolchainWidget(context);
    updateCompilerWidget(context);
    updateOpenglWidgets(context);
    updateImGuiWidget(context);
    syncVscodeFiles(context);
}

function activate(context) {
    refreshUi(context);

    CreateWidget(PROBLEMS_WIDGET, PROBLEMS_WIDGET.text, PROBLEMS_WIDGET.tooltip, "taskbar.focusProblems", context);
    CreateWidget(BUILD_WIDGET, "🛠️ Build", "Build & compile", "taskbar.build", context, "#3c53ce");
    CreateWidget(RUN_WIDGET, "▶️ Run", "Run project executable", "taskbar.run", context, "#57b069");
    CreateWidget(BUILD_RUN_WIDGET, "🚀 Build & Run", "Build, compile & run", "taskbar.buildAndRun", context, "#f8812f");
    CreateWidget(BUILD_DEBUG_WIDGET, "🪲 Build & Debug", "Build, compile & debug", "taskbar.buildAndDebug", context, "#be173b");
    CreateWidget(CLEAN_WIDGET, "🗑️ Clean", "Clean bin folder and optionally build", "taskbar.cleanBuild", context, "#797dde");
    CreateWidget(DOCS_WIDGET, "📖 Docs", "Open C++ / OpenGL / ImGui docs", "taskbar.openDocs", context, "#a7c96b");
    CreateWidget(WORKSPACE_WIDGET, WORKSPACE_WIDGET.text, WORKSPACE_WIDGET.tooltip, "taskbar.setWorkspaceRoot", context, "#d8b4fe");
    CreateWidget(PROJECT_WIDGET, PROJECT_WIDGET.text, PROJECT_WIDGET.tooltip, "taskbar.setProjectPath", context);
    CreateWidget(NEW_PROJECT_WIDGET, "➕ New Project", "Create a new ImGui + OpenGL project", "taskbar.newProject", context, "#79d8a9");
    CreateWidget(TOOLCHAIN_WIDGET, TOOLCHAIN_WIDGET.text, TOOLCHAIN_WIDGET.tooltip, "taskbar.setToolchain", context, "#ffd340");
    CreateWidget(COMPILER_WIDGET, COMPILER_WIDGET.text, COMPILER_WIDGET.tooltip, "taskbar.setCompilerPath", context, "#82c7eb");
    CreateWidget(OPENGL_INC_WIDGET, OPENGL_INC_WIDGET.text, OPENGL_INC_WIDGET.tooltip, "taskbar.setOpenglPaths", context, "#4582c7");
    CreateWidget(OPENGL_LIB_WIDGET, OPENGL_LIB_WIDGET.text, OPENGL_LIB_WIDGET.tooltip, "taskbar.setOpenglPaths", context, "#5a9fd4");
    CreateWidget(GLFW_LINK_WIDGET, GLFW_LINK_WIDGET.text, GLFW_LINK_WIDGET.tooltip, "taskbar.setGlfwLinkMode", context, "#c586c0");
    CreateWidget(IMGUI_WIDGET, IMGUI_WIDGET.text, IMGUI_WIDGET.tooltip, "taskbar.setImGuiMode", context, "#e383c4");

    const diagSub = vscode.languages.onDidChangeDiagnostics(() => updateProblemsWidget(context));
    context.subscriptions.push(diagSub);

    const setWorkspaceRootCommand = vscode.commands.registerCommand("taskbar.setWorkspaceRoot", async () => {
        const defaultPath = path.join(os.homedir(), "Documents");
        const uris = await vscode.window.showOpenDialog({
            canSelectFolders: true,
            canSelectFiles: false,
            canSelectMany: false,
            defaultUri: vscode.Uri.file(defaultPath),
            title: "Select workspace root folder"
        });

        if (!uris?.length) return;

        await setWorkspaceRoot(context, uris[0].fsPath);
        await refreshUi(context);
        vscode.window.showInformationMessage(`Workspace root set to: ${uris[0].fsPath}`);
    });

    const setProjectPathCommand = vscode.commands.registerCommand("taskbar.setProjectPath", async () => {
        const root = getWorkspaceRoot(context) || path.join(os.homedir(), "Documents");
        const uris = await vscode.window.showOpenDialog({
            canSelectFolders: true,
            canSelectMany: false,
            defaultUri: vscode.Uri.file(root),
            title: "Select project folder"
        });

        if (!uris?.length) return;

        await setProjectPath(context, uris[0].fsPath);
        await refreshUi(context);
        vscode.window.showInformationMessage(`Project set to: ${uris[0].fsPath}`);
    });

    const newProjectCommand = vscode.commands.registerCommand("taskbar.newProject", async () => {
        const workspaceRoot = getWorkspaceRoot(context);
        if (!workspaceRoot) {
            vscode.window.showErrorMessage("No workspace root is set.");
            return;
        }

        const name = await vscode.window.showInputBox({
            prompt: "New project name (e.g. MyApp)",
            placeHolder: "ProjectName",
            validateInput: (value) => {
                if (!value || !value.trim()) return "Enter a name.";
                if (/[\\/:*?\"<>|]/.test(value)) return "Name cannot contain \\ / : * ? \" < > |";
                return null;
            }
        });

        if (!name || !name.trim()) return;
        const projectName = name.trim();
        const projectRoot = path.join(workspaceRoot, projectName);

        if (fs.existsSync(projectRoot)) {
            vscode.window.showErrorMessage(`Project already exists: ${projectRoot}`);
            return;
        }

        try {
            for (const dir of NEW_PROJECT_FOLDERS) {
                ensureDir(path.join(projectRoot, dir));
            }
            copyNewProjectDlls(projectRoot);
            await setProjectPath(context, projectRoot);
            await vscode.commands.executeCommand(
                "vscode.openFolder",
                vscode.Uri.file(projectRoot),
                true
            );
        } catch (err) {
            if (fs.existsSync(projectRoot)) {
                try {
                    fs.rmSync(projectRoot, { recursive: true, force: true });
                } catch {}
            }
            vscode.window.showErrorMessage(`Failed to create project: ${err.message}`);
        }
    });

    const getProjectPathForTaskCommand = vscode.commands.registerCommand("taskbar.getProjectPathForTask", () => {
        return getProjectFullPath(context) || getWorkspaceRoot(context);
    });

    const setToolchainCommand = vscode.commands.registerCommand("taskbar.setToolchain", async () => {
        const pick = await vscode.window.showQuickPick(
            [
                { label: `$(tools) ${TOOLCHAIN_PRESETS.mingw64.label}`, toolchain: "mingw64" },
                { label: `$(tools) ${TOOLCHAIN_PRESETS.mingw32.label}`, toolchain: "mingw32" },
                { label: `$(file-directory) ${TOOLCHAIN_PRESETS.custom.label}`, toolchain: "custom" }
            ],
            { placeHolder: "Select toolchain", title: "Toolchain" }
        );
        if (!pick) return;

        await context.workspaceState.update(TOOLCHAIN_KEY, pick.toolchain);

        if (pick.toolchain === "custom") {
            const uris = await vscode.window.showOpenDialog({
                canSelectFolders: false,
                canSelectMany: false,
                title: "Select compiler executable (e.g. g++.exe)",
                filters: { Executables: ["exe"], "All files": ["*"] }
            });
            if (uris?.length) {
                await context.workspaceState.update(COMPILER_PATH_KEY, uris[0].fsPath);
            }
        } else {
            await context.workspaceState.update(COMPILER_PATH_KEY, undefined);
        }

        updateToolchainWidget(context);
        updateCompilerWidget(context);
        vscode.window.showInformationMessage(`Toolchain: ${pick.toolchain}. Compiler: ${getCompilerPath(context) || "default"}`);
    });

    const setCompilerPathCommand = vscode.commands.registerCommand("taskbar.setCompilerPath", async () => {
        const uris = await vscode.window.showOpenDialog({
            canSelectFolders: false,
            canSelectMany: false,
            title: "Select compiler executable (e.g. g++.exe)",
            filters: { Executables: ["exe"], "All files": ["*"] }
        });
        if (!uris?.length) return;

        await context.workspaceState.update(COMPILER_PATH_KEY, uris[0].fsPath);
        await context.workspaceState.update(TOOLCHAIN_KEY, "custom");
        updateToolchainWidget(context);
        updateCompilerWidget(context);
        vscode.window.showInformationMessage(`Compiler set to: ${uris[0].fsPath}`);
    });

    const setOpenglPathsCommand = vscode.commands.registerCommand("taskbar.setOpenglPaths", async () => {
        const gl = getOpenglConfig(context);
        const currentInc = gl.include;
        const currentLib = gl.lib;

        const incUris = await vscode.window.showOpenDialog({
            canSelectFolders: true,
            canSelectMany: false,
            title: "OpenGL-related include folder (-I: GLFW, GLAD, etc.)",
            defaultUri: currentInc ? vscode.Uri.file(currentInc) : undefined
        });
        if (!incUris?.length) return;

        const libUris = await vscode.window.showOpenDialog({
            canSelectFolders: true,
            canSelectMany: false,
            title: "MinGW-w64 library folder (-L: .a and glfw3.dll)",
            defaultUri: currentLib ? vscode.Uri.file(currentLib) : undefined
        });
        if (!libUris?.length) return;

        await context.workspaceState.update(OPENGL_INCLUDE_KEY, incUris[0].fsPath);
        await context.workspaceState.update(OPENGL_LIB_KEY, libUris[0].fsPath);

        updateOpenglWidgets(context);
        syncVscodeFiles(context);
        vscode.window.showInformationMessage(
            `OpenGL paths set.\ninclude: ${incUris[0].fsPath}\nlib: ${libUris[0].fsPath}`
        );
    });

    const setGlfwLinkModeCommand = vscode.commands.registerCommand("taskbar.setGlfwLinkMode", async () => {
        const gl = getOpenglConfig(context);
        const pick = await vscode.window.showQuickPick(
            [
                {
                    label: "$(package) Dynamic GLFW (-lglfw3dll + glfw3.dll)",
                    mode: "dynamic",
                    picked: gl.glfwLinkMode !== "static"
                },
                {
                    label: "$(archive) Static GLFW (-lglfw3, no glfw3.dll)",
                    mode: "static",
                    picked: gl.glfwLinkMode === "static"
                }
            ],
            { placeHolder: "How to link GLFW", title: "GLFW link mode" }
        );
        if (!pick) return;

        await context.workspaceState.update(GLFW_LINK_MODE_KEY, pick.mode);
        updateOpenglWidgets(context);
        syncVscodeFiles(context);
        vscode.window.showInformationMessage(`GLFW link mode: ${pick.mode}`);
    });

    const setImGuiModeCommand = vscode.commands.registerCommand("taskbar.setImGuiMode", async () => {
        const imgui = getImGuiConfig(context);
        const pick = await vscode.window.showQuickPick(
            [
                { label: "$(check) Enable / set Dear ImGui folder", action: "enable" },
                { label: "$(close) Disable ImGui sources in build", action: "disable" }
            ],
            { placeHolder: imgui.enabled ? "ImGui is enabled." : "ImGui is disabled." }
        );
        if (!pick) return;

        if (pick.action === "disable") {
            await context.workspaceState.update(IMGUI_ENABLED_KEY, false);
            updateImGuiWidget(context);
            syncVscodeFiles(context);
            vscode.window.showInformationMessage("ImGui disabled for this workspace.");
            return;
        }

        const imguiUri = await vscode.window.showOpenDialog({
            canSelectFolders: true,
            canSelectMany: false,
            title: "Select Dear ImGui root (contains imgui.cpp and backends/)"
        });
        if (!imguiUri?.length) return;

        const root = imguiUri[0].fsPath;
        await context.workspaceState.update(IMGUI_ENABLED_KEY, true);
        await context.workspaceState.update(IMGUI_INCLUDE_KEY, root);
        await context.workspaceState.update(IMGUI_SOURCE_DIR_KEY, root);

        updateImGuiWidget(context);
        syncVscodeFiles(context);
        vscode.window.showInformationMessage(`ImGui enabled: ${root}`);
    });

    async function getBuildTask() {
        const tasks = await vscode.tasks.fetchTasks();
        const matches = (t) =>
            t.name === BUILD_TASK_LABEL || (t.definition && t.definition.label === BUILD_TASK_LABEL);
        return (
            tasks.find(matches) ||
            tasks.find((t) => t.group === vscode.TaskGroup.Build) ||
            tasks[0]
        );
    }

    const buildCommand = vscode.commands.registerCommand("taskbar.build", async () => {
        const buildTask = await getBuildTask();
        if (!buildTask) {
            vscode.window.showErrorMessage("No build task found. Add a build task in .vscode/tasks.json");
            return;
        }
        await vscode.tasks.executeTask(buildTask);
    });

    const runCommand = vscode.commands.registerCommand("taskbar.run", async () => {
        const projectPath = getProjectFullPath(context);
        if (!projectPath) {
            vscode.window.showErrorMessage("No project folder is set.");
            return;
        }

        copyRuntimeDllsToBin(context, projectPath);

        const exePath = getProjectExePath(projectPath);
        if (!exePath || !fs.existsSync(exePath)) {
            vscode.window.showErrorMessage("No executable found. Build the project first.");
            return;
        }

        const term = vscode.window.createTerminal({
            name: "Run",
            cwd: projectPath,
            shellPath: "cmd.exe",
            shellArgs: ["/c", "start", "", exePath]
        });
        term.show();
    });

    const buildAndRunCommand = vscode.commands.registerCommand("taskbar.buildAndRun", async () => {
        const buildTask = await getBuildTask();
        if (!buildTask) {
            vscode.window.showErrorMessage("No build task found. Add a build task in .vscode/tasks.json");
            return;
        }

        const taskLabel = buildTask.definition?.label || buildTask.name;
        const listener = vscode.tasks.onDidEndTaskProcess((e) => {
            const endedLabel = e.execution.task.definition?.label || e.execution.task.name;
            if (endedLabel !== taskLabel) return;
            if (e.exitCode === 0) {
                const projectPath = getProjectFullPath(context);
                const exePath = projectPath ? getProjectExePath(projectPath) : null;
                if (exePath && fs.existsSync(exePath) && projectPath) {
                    copyRuntimeDllsToBin(context, projectPath);
                    const term = vscode.window.createTerminal({
                        name: "Run",
                        cwd: projectPath,
                        shellPath: "cmd.exe",
                        shellArgs: ["/c", "start", "", exePath]
                    });
                    term.show();
                }
            }
            listener.dispose();
        });
        context.subscriptions.push(listener);
        await vscode.tasks.executeTask(buildTask);
    });

    const buildAndDebugCommand = vscode.commands.registerCommand("taskbar.buildAndDebug", async () => {
        if (buildAndDebugListener) {
            buildAndDebugListener.dispose();
            buildAndDebugListener = null;
        }

        const buildTask = await getBuildTask();
        if (!buildTask) {
            vscode.window.showErrorMessage("No build task found. Add a build task in .vscode/tasks.json");
            return;
        }

        const gdbPath = getGdbPath(context);
        const buildLabel = buildTask.definition?.label || buildTask.name;

        buildAndDebugListener = vscode.tasks.onDidEndTaskProcess((e) => {
            const ended = e.execution.task.definition?.label || e.execution.task.name;
            if (ended !== buildLabel) return;

            const projectPath = getProjectFullPath(context);
            const exePath = projectPath ? getProjectExePath(projectPath) : null;
            if (e.exitCode === 0 && exePath && fs.existsSync(exePath) && projectPath) {
                copyRuntimeDllsToBin(context, projectPath);
                vscode.debug.startDebugging(undefined, {
                    type: "cppdbg",
                    request: "launch",
                    name: "Debug (taskbar)",
                    program: exePath,
                    cwd: projectPath,
                    MIMode: "gdb",
                    miDebuggerPath: gdbPath,
                    setupCommands: [{ description: "Enable pretty-printing", text: "-enable-pretty-printing", ignoreFailures: true }]
                });
            }

            if (buildAndDebugListener) {
                buildAndDebugListener.dispose();
                buildAndDebugListener = null;
            }
        });
        context.subscriptions.push(buildAndDebugListener);
        await vscode.tasks.executeTask(buildTask);
    });

    const focusProblemsCommand = vscode.commands.registerCommand("taskbar.focusProblems", () => {
        vscode.commands.executeCommand("workbench.panel.markers.view.focus");
    });

    const cleanBuildCommand = vscode.commands.registerCommand("taskbar.cleanBuild", async () => {
        const projectPath = getProjectFullPath(context);
        if (!projectPath) {
            vscode.window.showErrorMessage("No project folder is set.");
            return;
        }

        const binPath = getProjectBinPath(projectPath);
        if (!fs.existsSync(binPath)) {
            vscode.window.showInformationMessage("No bin folder to clean.");
            return;
        }

        const pick = await vscode.window.showQuickPick(
            [
                { label: "🗑️ Clean only", action: "clean" },
                { label: "🗑️ Clean and Build", action: "cleanAndBuild" }
            ],
            { placeHolder: "Clean build output", title: "Clean" }
        );
        if (!pick) return;

        try {
            const entries = fs.readdirSync(binPath, { withFileTypes: true });
            for (const entry of entries) {
                const full = path.join(binPath, entry.name);
                if (entry.isFile()) fs.unlinkSync(full);
                else if (entry.isDirectory()) fs.rmSync(full, { recursive: true, force: true });
            }
            vscode.window.showInformationMessage("Build output cleaned.");
        } catch (err) {
            vscode.window.showErrorMessage(`Clean failed: ${err.message}`);
            return;
        }

        if (pick.action === "cleanAndBuild") {
            const buildTask = await getBuildTask();
            if (buildTask) await vscode.tasks.executeTask(buildTask);
        }
    });

    const DOCS_LINKS = [
        { label: "C++ Reference (cppreference.com)", url: "https://en.cppreference.com/w/" },
        { label: "OpenGL Registry / specs", url: "https://www.khronos.org/opengl/" },
        { label: "GLFW documentation", url: "https://www.glfw.org/documentation.html" },
        { label: "Dear ImGui", url: "https://github.com/ocornut/imgui" }
    ];

    const openDocsCommand = vscode.commands.registerCommand("taskbar.openDocs", async () => {
        const pick = await vscode.window.showQuickPick(
            DOCS_LINKS.map((d) => ({ label: d.label, url: d.url })),
            { placeHolder: "Open documentation", title: "Quick Docs" }
        );
        if (!pick) return;
        await vscode.env.openExternal(vscode.Uri.parse(pick.url));
    });

    context.subscriptions.push(
        setWorkspaceRootCommand,
        setProjectPathCommand,
        newProjectCommand,
        getProjectPathForTaskCommand,
        setToolchainCommand,
        setCompilerPathCommand,
        setOpenglPathsCommand,
        setGlfwLinkModeCommand,
        setImGuiModeCommand,
        focusProblemsCommand,
        cleanBuildCommand,
        openDocsCommand,
        buildCommand,
        runCommand,
        buildAndRunCommand,
        buildAndDebugCommand
    );
}

function deactivate() {}

module.exports = { activate, deactivate };
