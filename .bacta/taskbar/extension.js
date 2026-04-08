const vscode = require("vscode");
const path = require("path");
const fs = require("fs");
const os = require("os");

// --- State keys ---
const PROJECT_PATH_KEY = "taskbar.projectPath";
const WORKSPACE_ROOT_KEY = "taskbar.workspaceRoot";
const TOOLCHAIN_KEY = "taskbar.toolchain";
const COMPILER_PATH_KEY = "taskbar.compilerPath";
const SFML_ENABLED_KEY = "taskbar.sfmlEnabled";
const SFML_DLL_MODULES_KEY = "taskbar.sfmlDllModules";
const IMGUI_ENABLED_KEY = "taskbar.imguiEnabled";

const TOOLCHAIN_PRESETS = {
    mingw64: { label: "MinGW 64-bit", key: "mingw64" },
    mingw32: { label: "MinGW 32-bit", key: "mingw32" },
    custom: { label: "Custom (browse for compiler)", key: "custom" }
};

const DEFAULT_WORKSPACE_ROOT = "";
const DEFAULT_PROJECT_PATH = "";
const NEW_PROJECT_FOLDERS = ["assets", "bin", "include", "lib", "src"];
const SFML_MODULES = ["system", "window", "graphics", "audio", "network"];

var BUILD_WIDGET = vscode.window.createStatusBarItem();
var RUN_WIDGET = vscode.window.createStatusBarItem();
var BUILD_RUN_WIDGET = vscode.window.createStatusBarItem();
var BUILD_DEBUG_WIDGET = vscode.window.createStatusBarItem();
var PROJECT_WIDGET = vscode.window.createStatusBarItem();
var WORKSPACE_WIDGET = vscode.window.createStatusBarItem();
var TOOLCHAIN_WIDGET = vscode.window.createStatusBarItem();
var COMPILER_WIDGET = vscode.window.createStatusBarItem();
var SFML_WIDGET = vscode.window.createStatusBarItem();
var SFML_DLL_WIDGET = vscode.window.createStatusBarItem();
var IMGUI_WIDGET = vscode.window.createStatusBarItem();
var PROBLEMS_WIDGET = vscode.window.createStatusBarItem();
var CLEAN_WIDGET = vscode.window.createStatusBarItem();
var NEW_PROJECT_WIDGET = vscode.window.createStatusBarItem();
var DOCS_WIDGET = vscode.window.createStatusBarItem();
var buildAndDebugListener = null;

const NEW_PROJECT_DLL_SOURCES = [
    "c:\\mingw64\\libexec\\gcc\\x86_64-w64-mingw32\\14.2.0\\libgcc_s_seh-1.dll",
    "c:\\mingw64\\libexec\\gcc\\x86_64-w64-mingw32\\14.2.0\\libwinpthread-1.dll",
    "c:\\mingw64\\bin\\libstdc++-6.dll",
    "c:\\SFML-3.0.2\\bin\\sfml-graphics-3.dll",
    "c:\\SFML-3.0.2\\bin\\sfml-system-3.dll",
    "c:\\SFML-3.0.2\\bin\\sfml-window-3.dll"
];

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

function updateCppPropertiesIncludePath(context) {
    const workspaceFolder = getOpenWorkspaceFolder();
    if (!workspaceFolder) return;

    const projectPath = getProjectPath(context);
    const projectFullPath = getProjectFullPath(context);
    const propsPath = path.join(workspaceFolder.uri.fsPath, ".vscode", "c_cpp_properties.json");
    if (!fs.existsSync(propsPath)) return;

    const sfml = getSfmlPaths(context);
    const imgui = getImGuiConfig(context);
    const sfmlInclude = (sfml.enabled && sfml.include) ? sfml.include : "C:/SFML-3.0.2/include";
    const projectInclude = projectFullPath
        ? normalizeSlashes(path.join(projectFullPath, "include"))
        : normalizeSlashes(path.join(projectPath || "", "include"));

    try {
        const raw = fs.readFileSync(propsPath, "utf8");
        const props = JSON.parse(raw);
        if (!props.configurations || !Array.isArray(props.configurations)) return;

        const includePath = [projectInclude];
        if (sfml.enabled) includePath.push(normalizeSlashes(sfmlInclude));
        if (imgui.enabled) {
            if (imgui.include) includePath.push(normalizeSlashes(imgui.include));
            if (imgui.sfmlInclude) includePath.push(normalizeSlashes(imgui.sfmlInclude));
        }

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

function collectCppGlobDirs(sourceRoot) {
    const dirs = new Set();

    function walk(currentDir) {
        if (!fs.existsSync(currentDir)) return;

        const entries = fs.readdirSync(currentDir, { withFileTypes: true });
        let hasCpp = false;

        for (const entry of entries) {
            const fullPath = path.join(currentDir, entry.name);

            if (entry.isDirectory()) {
                walk(fullPath);
            } else if (entry.isFile() && fullPath.toLowerCase().endsWith(".cpp")) {
                hasCpp = true;
            }
        }

        if (hasCpp) {
            dirs.add(currentDir);
        }
    }

    walk(sourceRoot);
    return [...dirs].sort((a, b) => a.localeCompare(b));
}

function updateTasksJson(context) {
    const workspaceFolder = getOpenWorkspaceFolder();
    if (!workspaceFolder) return;

    const tasksPath = path.join(workspaceFolder.uri.fsPath, ".vscode", "tasks.json");
    const projectPath = getProjectPath(context);
    const projectFullPath = getProjectFullPath(context);
    const compilerPath = getCompilerPath(context) || "C:\\mingw64\\bin\\g++.exe";
    const sfml = getSfmlPaths(context);
    const imgui = getImGuiConfig(context);
    const sfmlInclude = (sfml.include || "C:\\SFML-3.0.2\\include").replace(/\//g, "\\");
    const sfmlLib = (sfml.lib || "C:\\SFML-3.0.2\\lib").replace(/\//g, "\\");

    const args = ["-fdiagnostics-color=always", "-g", "-std=c++20"];

    if (sfml.enabled) args.push(`-I${sfmlInclude}`);
    args.push("-Iinclude");

    if (imgui.enabled) {
        if (imgui.include) args.push(`-I${imgui.include}`);
        if (imgui.sfmlInclude) args.push(`-I${imgui.sfmlInclude}`);
    }

    const projectRoot = projectFullPath || workspaceFolder.uri.fsPath;
    const sourceRoot = path.join(projectRoot, "src");
    const cppDirs = collectCppGlobDirs(sourceRoot);

    for (const dir of cppDirs) {
        const relDir = path.relative(projectRoot, dir).replace(/\\/g, "/");
        args.push(`${relDir}/*.cpp`);
    }

    if (imgui.enabled && !imgui.linkPrebuilt) {
        if (imgui.sourceDir) {
            args.push(path.join(imgui.sourceDir, "imgui.cpp"));
            args.push(path.join(imgui.sourceDir, "imgui_draw.cpp"));
            args.push(path.join(imgui.sourceDir, "imgui_tables.cpp"));
            args.push(path.join(imgui.sourceDir, "imgui_widgets.cpp"));
        }
        if (imgui.sfmlCpp) args.push(imgui.sfmlCpp);
    }

    if (sfml.enabled) {
        args.push(`-L${sfmlLib}`);
        args.push("-lsfml-graphics", "-lsfml-audio", "-lsfml-window", "-lsfml-system", "-lopengl32");
    }

    if (imgui.enabled && imgui.linkPrebuilt) {
        if (imgui.lib) args.push(`-L${imgui.lib}`);
        args.push("-limgui-sfml", "-limgui");
    }

    args.push("-o", "bin/main.exe");

    const buildDetail = imgui.enabled
        ? "Build selected project with SFML + ImGui-SFML"
        : "Build selected project with SFML";

    const payload = {
        version: "2.0.0",
        inputs: [
            {
                id: "projectPath",
                type: "command",
                command: "taskbar.getProjectPathForTask"
            }
        ],
        tasks: [
            {
                label: "Build current project (SFML)",
                type: "shell",
                command: compilerPath,
                args,
                options: {
                    cwd: projectPath ? "${input:projectPath}" : workspaceFolder.uri.fsPath
                },
                problemMatcher: ["$gcc"],
                group: { kind: "build", isDefault: true },
                detail: `${buildDetail} (auto-updated by taskbar for ${projectFullPath || projectPath || "no project"})`
            }
        ]
    };

    try {
        ensureDir(path.dirname(tasksPath));
        fs.writeFileSync(tasksPath, JSON.stringify(payload, null, 4), "utf8");
        console.log(`taskbar: tasks.json synced with ${cppFiles.length} source file(s) from ${sourceRoot}`);
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
        config.preLaunchTask = "Build current project (SFML)";
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

function getSfmlPaths(context) {
    const config = vscode.workspace.getConfiguration("taskbar");
    const enabledStored = context.workspaceState.get(SFML_ENABLED_KEY);
    const enabled = enabledStored !== undefined ? enabledStored : config.get("sfml.enabled", true);
    return {
        enabled,
        include: config.get("sfml.include"),
        lib: config.get("sfml.lib")
    };
}

function getImGuiConfig(context) {
    const config = vscode.workspace.getConfiguration("taskbar");
    const enabledStored = context.workspaceState.get(IMGUI_ENABLED_KEY);
    const enabled = enabledStored !== undefined ? enabledStored : config.get("imgui.enabled", false);
    return {
        enabled,
        include: config.get("imgui.include"),
        sfmlInclude: config.get("imguiSfml.include"),
        sourceDir: config.get("imgui.sourceDir"),
        sfmlCpp: config.get("imgui.sfmlCpp"),
        lib: config.get("imgui.lib"),
        linkPrebuilt: config.get("imgui.linkPrebuilt", false)
    };
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

function updateSfmlWidget(context) {
    const sfml = getSfmlPaths(context);
    if (!sfml.enabled) {
        SFML_WIDGET.text = "📦 No SFML";
        SFML_WIDGET.tooltip = "SFML disabled (plain C++ build). Click to set SFML or enable.";
        return;
    }

    const root = sfml.include ? path.dirname(sfml.include) : "";
    const label = root ? path.basename(root) : "SFML";
    SFML_WIDGET.text = `📦 ${label}`;
    SFML_WIDGET.tooltip = `SFML include: ${sfml.include || "?"}\nSFML lib: ${sfml.lib || "?"}. Click to set paths or disable.`;
}

function updateImGuiWidget(context) {
    const imgui = getImGuiConfig(context);
    if (!imgui.enabled) {
        IMGUI_WIDGET.text = "🧩 ImGui: Off";
        IMGUI_WIDGET.tooltip = "ImGui-SFML disabled. Click to enable and set paths.";
        return;
    }

    IMGUI_WIDGET.text = "🧩 ImGui: On";
    IMGUI_WIDGET.tooltip = `ImGui include: ${imgui.include || "?"}\nImGui-SFML include: ${imgui.sfmlInclude || "?"}\nMode: ${imgui.linkPrebuilt ? "prebuilt libs" : "compile sources"}. Click to change.`;
}

function getSfmlDllModules(context) {
    const stored = context.workspaceState.get(SFML_DLL_MODULES_KEY);
    if (Array.isArray(stored) && stored.length > 0) return stored;
    return ["system", "window", "graphics"];
}

function getSfmlBinPath(context) {
    const sfml = getSfmlPaths(context);
    if (!sfml.enabled || !sfml.include) return null;
    const root = path.dirname(sfml.include);
    return path.join(root, "bin");
}

function updateSfmlDllWidget(context) {
    const sfml = getSfmlPaths(context);
    if (!sfml.enabled) {
        SFML_DLL_WIDGET.hide();
        return;
    }

    const modules = getSfmlDllModules(context);
    SFML_DLL_WIDGET.text = modules.length === 0 ? "📦 DLLs: none" : `📦 DLLs: ${modules.join(", ")}`;
    SFML_DLL_WIDGET.tooltip = "Select which SFML DLLs to copy from SFML/bin to project output. Click to change.";
    SFML_DLL_WIDGET.show();
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
    for (const src of NEW_PROJECT_DLL_SOURCES) {
        if (!fs.existsSync(src)) continue;
        fs.copyFileSync(src, path.join(libDir, path.basename(src)));
    }
}

async function refreshUi(context) {
    updateWorkspaceWidget(context);
    updateProjectWidget(context);
    updateProblemsWidget(context);
    updateToolchainWidget(context);
    updateCompilerWidget(context);
    updateSfmlWidget(context);
    updateImGuiWidget(context);
    updateSfmlDllWidget(context);
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
    CreateWidget(DOCS_WIDGET, "📖 Docs", "Open C++ or SFML documentation", "taskbar.openDocs", context, "#a7c96b");
    CreateWidget(WORKSPACE_WIDGET, WORKSPACE_WIDGET.text, WORKSPACE_WIDGET.tooltip, "taskbar.setWorkspaceRoot", context, "#d8b4fe");
    CreateWidget(PROJECT_WIDGET, PROJECT_WIDGET.text, PROJECT_WIDGET.tooltip, "taskbar.setProjectPath", context);
    CreateWidget(NEW_PROJECT_WIDGET, "➕ New Project", "Create a new SFML project", "taskbar.newProject", context, "#79d8a9");
    CreateWidget(TOOLCHAIN_WIDGET, TOOLCHAIN_WIDGET.text, TOOLCHAIN_WIDGET.tooltip, "taskbar.setToolchain", context, "#ffd340");
    CreateWidget(COMPILER_WIDGET, COMPILER_WIDGET.text, COMPILER_WIDGET.tooltip, "taskbar.setCompilerPath", context, "#82c7eb");
    CreateWidget(SFML_WIDGET, SFML_WIDGET.text, SFML_WIDGET.tooltip, "taskbar.setSfmlPath", context, "#4582c7");
    CreateWidget(IMGUI_WIDGET, IMGUI_WIDGET.text, IMGUI_WIDGET.tooltip, "taskbar.setImGuiMode", context, "#e383c4");
    CreateWidget(SFML_DLL_WIDGET, SFML_DLL_WIDGET.text, SFML_DLL_WIDGET.tooltip, "taskbar.setSfmlDllModules", context, "#82c7eb");
    updateSfmlDllWidget(context);

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
            prompt: "New project name (e.g. MyGame)",
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

    const setSfmlPathCommand = vscode.commands.registerCommand("taskbar.setSfmlPath", async () => {
        const sfml = getSfmlPaths(context);
        const pick = await vscode.window.showQuickPick(
            [
                { label: "$(file-directory) Select SFML root folder", action: "select" },
                { label: "$(close) Disable SFML (plain C++ build)", action: "disable" }
            ],
            { placeHolder: sfml.enabled ? "SFML is enabled. Change path or disable." : "SFML is disabled. Enable and set path." }
        );
        if (!pick) return;

        const config = vscode.workspace.getConfiguration("taskbar");

        if (pick.action === "disable") {
            await context.workspaceState.update(SFML_ENABLED_KEY, false);
            updateSfmlWidget(context);
            updateSfmlDllWidget(context);
            syncVscodeFiles(context);
            vscode.window.showInformationMessage("SFML disabled. Build will compile without SFML.");
            return;
        }

        const uris = await vscode.window.showOpenDialog({
            canSelectFolders: true,
            canSelectMany: false,
            title: "Select SFML root folder (containing include/ and lib/)"
        });
        if (!uris?.length) return;

        const root = uris[0].fsPath;
        const includePath = path.join(root, "include");
        const libPath = path.join(root, "lib");
        await context.workspaceState.update(SFML_ENABLED_KEY, true);
        await config.update("sfml.include", includePath, vscode.ConfigurationTarget.Workspace);
        await config.update("sfml.lib", libPath, vscode.ConfigurationTarget.Workspace);
        updateSfmlWidget(context);
        updateSfmlDllWidget(context);
        syncVscodeFiles(context);
        vscode.window.showInformationMessage(`SFML enabled. include=${includePath}, lib=${libPath}`);
    });

    const setImGuiModeCommand = vscode.commands.registerCommand("taskbar.setImGuiMode", async () => {
        const imgui = getImGuiConfig(context);
        const pick = await vscode.window.showQuickPick(
            [
                { label: "$(check) Enable / Configure ImGui-SFML", action: "enable" },
                { label: "$(close) Disable ImGui-SFML", action: "disable" }
            ],
            { placeHolder: imgui.enabled ? "ImGui-SFML is enabled." : "ImGui-SFML is disabled." }
        );
        if (!pick) return;

        if (pick.action === "disable") {
            await context.workspaceState.update(IMGUI_ENABLED_KEY, false);
            updateImGuiWidget(context);
            syncVscodeFiles(context);
            vscode.window.showInformationMessage("ImGui-SFML disabled.");
            return;
        }

        const config = vscode.workspace.getConfiguration("taskbar");
        const imguiUri = await vscode.window.showOpenDialog({
            canSelectFolders: true,
            canSelectMany: false,
            title: "Select Dear ImGui root folder"
        });
        if (!imguiUri?.length) return;

        const imguiSfmlUri = await vscode.window.showOpenDialog({
            canSelectFolders: true,
            canSelectMany: false,
            title: "Select ImGui-SFML root folder"
        });
        if (!imguiSfmlUri?.length) return;

        const modePick = await vscode.window.showQuickPick(
            [
                { label: "Compile ImGui sources directly", mode: "compile" },
                { label: "Link prebuilt imgui/imgui-sfml libraries", mode: "prebuilt" }
            ],
            { placeHolder: "Choose ImGui build integration mode" }
        );
        if (!modePick) return;

        const imguiRoot = imguiUri[0].fsPath;
        const imguiSfmlRoot = imguiSfmlUri[0].fsPath;
        await context.workspaceState.update(IMGUI_ENABLED_KEY, true);
        await config.update("imgui.enabled", true, vscode.ConfigurationTarget.Workspace);
        await config.update("imgui.include", imguiRoot, vscode.ConfigurationTarget.Workspace);
        await config.update("imgui.sourceDir", imguiRoot, vscode.ConfigurationTarget.Workspace);
        await config.update("imguiSfml.include", imguiSfmlRoot, vscode.ConfigurationTarget.Workspace);
        await config.update("imgui.sfmlCpp", path.join(imguiSfmlRoot, "imgui-SFML.cpp"), vscode.ConfigurationTarget.Workspace);
        await config.update("imgui.linkPrebuilt", modePick.mode === "prebuilt", vscode.ConfigurationTarget.Workspace);

        if (modePick.mode === "prebuilt") {
            const libUri = await vscode.window.showOpenDialog({
                canSelectFolders: true,
                canSelectMany: false,
                title: "Select library folder containing imgui and imgui-sfml libs"
            });
            if (libUri?.length) {
                await config.update("imgui.lib", libUri[0].fsPath, vscode.ConfigurationTarget.Workspace);
            }
        }

        updateImGuiWidget(context);
        syncVscodeFiles(context);
        vscode.window.showInformationMessage("ImGui-SFML enabled and .vscode files updated.");
    });

    const setSfmlDllModulesCommand = vscode.commands.registerCommand("taskbar.setSfmlDllModules", async () => {
        const sfml = getSfmlPaths(context);
        if (!sfml.enabled) {
            vscode.window.showInformationMessage("Enable SFML first (click the SFML widget to set path).");
            return;
        }

        const current = getSfmlDllModules(context);
        const binPath = getSfmlBinPath(context);
        let options = SFML_MODULES.map((mod) => ({
            label: `sfml-${mod}.dll`,
            description: mod,
            module: mod,
            picked: current.includes(mod)
        }));

        if (binPath && fs.existsSync(binPath)) {
            const files = fs.readdirSync(binPath).filter((f) => f.startsWith("sfml-") && f.endsWith(".dll"));
            if (files.length > 0) {
                const byMod = new Map();
                for (const f of files) {
                    const mod = f.replace(/^sfml-(.+?)-\d+\.dll$/i, "$1") || f.replace(/^sfml-|-\d*\.dll$/gi, "");
                    if (!byMod.has(mod)) byMod.set(mod, f);
                }
                options = [...byMod.entries()].map(([mod, file]) => ({
                    label: file,
                    description: mod,
                    module: mod,
                    picked: current.includes(mod)
                }));
            }
        }

        const picked = await vscode.window.showQuickPick(options, {
            placeHolder: "Select SFML DLLs to copy to output (from SFML/bin)",
            title: "SFML DLLs",
            canPickMany: true,
            matchOnDescription: true
        });
        if (picked === undefined) return;

        const modules = [...new Set(picked.map((p) => p.module))];
        await context.workspaceState.update(SFML_DLL_MODULES_KEY, modules);
        updateSfmlDllWidget(context);
        vscode.window.showInformationMessage(`SFML DLLs to copy: ${modules.length ? modules.join(", ") : "none"}`);
    });

    async function getBuildTask() {
        const tasks = await vscode.tasks.fetchTasks();
        return (
            tasks.find((t) => t.name === "Build current project (SFML)") ||
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
                if (exePath && fs.existsSync(exePath)) {
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

        buildAndDebugListener = vscode.tasks.onDidEndTaskProcess((e) => {
            if (e.execution.task.name !== buildTask.name) return;

            const projectPath = getProjectFullPath(context);
            const exePath = projectPath ? getProjectExePath(projectPath) : null;
            if (e.exitCode === 0 && exePath && fs.existsSync(exePath)) {
                vscode.debug.startDebugging(undefined, {
                    type: "cppdbg",
                    request: "launch",
                    name: "Debug (taskbar)",
                    program: exePath,
                    cwd: projectPath,
                    MIMode: "gdb",
                    miDebuggerPath: "C:\\mingw64\\bin\\gdb.exe",
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
        { label: "SFML Documentation", url: "https://www.sfml-dev.org/documentation.php" },
        { label: "Dear ImGui", url: "https://github.com/ocornut/imgui" },
        { label: "ImGui-SFML", url: "https://github.com/SFML/imgui-sfml" }
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
        setSfmlPathCommand,
        setImGuiModeCommand,
        setSfmlDllModulesCommand,
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
