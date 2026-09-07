#pragma once

// Validates the Lua state CommonScriptContext sets up:
//   - require() resolves a module in the project's scripts folder by plain
//     name, dotted submodule name, and folder with an init.lua
//   - a required module loads under the same project-relative chunk name a
//     script file gets, which is what the lrdb debugger matches breakpoints on
//   - a module missing from the project reports the scripts folder among the
//     paths it tried
//   - pushComponent hands Lua a component as its concrete class, so a subclass
//     reached through a base pointer keeps its own bindings
bool run_script_context_tests();
