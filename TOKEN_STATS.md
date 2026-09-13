# AI Token Usage

<!-- AI_USAGE_BADGES:BEGIN -->
![AI tokens](https://img.shields.io/badge/AI_tokens-16M_out_%2F_5.8B_read-blueviolet) ![est. energy](https://img.shields.io/badge/est._energy-~322_kWh-yellow) ![est. water](https://img.shields.io/badge/est._water-~966_L-blue)
(estimates, see [TOKEN_STATS.md](TOKEN_STATS.md))
<!-- AI_USAGE_BADGES:END -->

MikanXR is developed almost entirely in Claude Code sessions, across whichever Claude models were current at the time. This document records the API token usage of that work and derives an order-of-magnitude estimate of the electricity and water the inference consumed.

## How the numbers are collected

Claude Code writes one JSONL transcript per session under `~/.claude/projects/`, plus a transcript per subagent, and every assistant record carries the exact token counts the API metered. Those transcripts are local to one machine and are pruned on a retention timer, so they are a collection source rather than a source of truth.

The source of truth is `docs/ai-usage.json`, a committed ledger holding per-session, per-model counters. `tools/token_stats.py` scans the transcripts, folds them into that ledger, and renders the badges above and the generated block below from the ledger alone.

```bash
python tools/token_stats.py
```

Three properties make that safe to run at any cadence.

- Counters only ever grow. A session is merged with `max(stored, scanned)` per class, so a pruned or partially rotated transcript cannot erase recorded history, and a resumed session grows correctly.

- Attribution is automatic. A session counts toward MikanXR when the working directory recorded in its transcript is this repository or one of its worktrees. Everything else lands in the ledger's `excluded` map, and the script prints what it excluded and why while it runs. That map is authoritative and hand-editable, which is how a session the rule wrongly claimed gets retired: adding its id retires it on the next run. Work on a sibling project that happened to start from this directory is excluded that way, so the same session is never counted in two repositories.

- Rendering is a pure function of the ledger. Running the script twice with no new sessions produces no diff, and `python tools/token_stats.py --check` re-renders from the ledger and fails if the published files have drifted. CI runs that check on every push, which is the only part of this that can run off the developer's machine.

The ledger stores no absolute paths. Session ids are opaque, and the script refuses to write any string that looks like a filesystem location, so the file carries no drive letter, home directory, or user name.

The token counts are exact, because they are what the API metered. Everything under "Estimation methodology" is not.

## Estimation methodology

Anthropic publishes no per-token energy figures, so the estimate chains public reference points with stated assumptions. Treat the result as order-of-magnitude.

**Step 1: reduce the token classes to one.** The API meters classes that cost very different amounts of compute: fresh input, cache writes at each of the two cache lifetimes, cache reads, and output. They fold into "weighted output-equivalent tokens" using Anthropic's own price ratios as the compute proxy, since pricing is the closest public signal of relative serving cost. The ratios are the same across Claude models, so one table covers Opus and Fable alike.

| Token class | Price relative to base input | Weight in output-equivalent tokens |
|---|---|---|
| Output | 5 | 1 |
| Input | 1 | 1/5 |
| Cache write (1 hour) | 2 | 2/5 |
| Cache write (5 minute) | 1.25 | 1/4 |
| Cache read | 0.1 | 1/50 |

Note the consequence: cache reads dominate the weighted total even at 2 percent weight, because agentic coding re-reads the growing conversation context on every call. That is real work, attention over the cached context, and should not be dropped.

**Step 2: energy per weighted token.** Public anchors: Google reported the median Gemini Apps text prompt at 0.24 Wh (Aug 2025 technical report), Epoch AI estimated roughly 0.3 Wh for a typical GPT-4o query, and Sam Altman quoted 0.34 Wh for an average ChatGPT query. A median chat prompt is a few hundred output-equivalent tokens, which lands those figures near 1 Wh per 1000 weighted tokens. The models used here are larger than what serves a median consumer prompt, so the central assumption is 2 Wh per 1000 weighted tokens, with 0.5 and 6 as low and high bounds. These constants sit at the top of `tools/token_stats.py`. Correct them as better data appears.

**Step 3: water per kWh.** Onsite datacenter cooling runs about 1.1 L/kWh, consistent with Google's reported fleet WUE and their 0.26 mL figure for the 0.24 Wh prompt above. Including the water footprint of electricity generation raises it to roughly 3 L/kWh on a typical US grid mix.

**What the estimate excludes:** model training, which is generally estimated to be small per query relative to inference at scale but is not zero, datacenter construction, the dev machine running Claude Code and the local builds, and the GPU time spent on the depth, lighting, and tracking model work this project exists for. It also cannot know Anthropic's actual serving efficiency, batching, or datacenter locations, which is why the range spans an order of magnitude.

## Why not an off-the-shelf impact library

[EcoLogits](https://github.com/genai-impact/ecologits) was evaluated for this. It wraps live provider SDK calls and attaches an impact estimate to each response, which is a good fit for an application calling a model API directly. It does not fit here: the usage being measured comes from Claude Code transcripts rather than from calls this repository makes, there is no offline entry point that accepts recorded token counts, and it reports energy and greenhouse gas rather than water. The arithmetic above is small enough to state in full, which also makes its assumptions easy to check and to correct.

<!-- TOKEN_STATS:BEGIN -->
Generated by `tools/token_stats.py` from `docs/ai-usage.json`, covering 57 sessions through 2026-09-13.

## Per-session usage

| Session | Dates | Models | API calls | Output | Cache read |
|---|---|---|---|---|---|
| Unreal reference hand skeleton bone transforms | 2026-07-11 | claude-sonnet-5 | 1 | 2,327 | 23,803 |
| BodyStateEstimator code analysis | 2026-07-12 | claude-sonnet-5 | 6 | 5,498 | 277,599 |
| ARKit streaming to MikanXR plan | 2026-07-12 to 2026-07-30 | claude-opus-5, claude-sonnet-5 | 2,766 | 2,381,531 | 1,281,281,484 |
| LiveLink head position in Unreal/ARKit | 2026-07-14 | claude-sonnet-5 | 4 | 4,099 | 138,543 |
| Blender FBX export size issue | 2026-07-15 | claude-sonnet-5 | 24 | 23,743 | 1,362,943 |
| AngelScript vs Lua for Mikan | 2026-07-17 | claude-sonnet-5 | 5 | 1,799 | 179,616 |
| Sensor grain effect for CG compositing | 2026-07-17 | claude-sonnet-5 | 89 | 94,566 | 9,492,484 |
| Efficient Claude integration for MikanXR | 2026-07-18 | claude-sonnet-5 | 18 | 10,089 | 909,205 |
| Lua script HTTP endpoint registration | 2026-07-18 | claude-sonnet-5 | 270 | 222,891 | 45,431,122 |
| EditorWindow boilerplate refactoring | 2026-07-18 | claude-sonnet-5 | 31 | 21,707 | 3,178,012 |
| MCP server integration for MikanXR | 2026-07-22 | claude-sonnet-5 | 44 | 47,681 | 2,531,945 |
| DySample depth upsampling integration | 2026-07-25 to 2026-07-26 | claude-opus-4-8 | 413 | 465,090 | 124,537,340 |
| GStreamer event bus polling in MikanGStreamerVideoDevice | 2026-07-26 | claude-opus-4-8 | 74 | 68,142 | 8,041,655 |
| SteamVR device disconnection crash | 2026-07-26 | claude-opus-4-8 | 17 | 17,023 | 1,003,214 |
| Stencil alignment with natural features | 2026-07-27 | claude-opus-4-8 | 172 | 188,751 | 25,466,083 |
| Orthographic viewport and distance ruler | 2026-07-27 | claude-opus-4-8 | 283 | 286,166 | 69,661,824 |
| Light channel flashing in Unreal | 2026-07-27 | claude-opus-4-8 | 4 | 2,721 | 139,217 |
| MikanXR plugin request timeout handling | 2026-07-28 | claude-opus-4-8 | 59 | 91,292 | 7,807,160 |
| Doctor command | 2026-07-29 | claude-opus-4-8 | 11 | 16,111 | 659,186 |
| Mikan Claude documentation structure | 2026-08-03 | claude-fable-5 | 234 | 203,137 | 22,075,805 |
| V-RGBX CUDA device index error | 2026-08-20 to 2026-08-22 | claude-opus-5 | 483 | 394,118 | 202,001,155 |
| Depth-derived proxy geometry | 2026-08-22 to 2026-08-23 | claude-fable-5, claude-opus-5 | 405 | 413,662 | 156,858,320 |
| Depth-derived proxy geometry | 2026-08-22 | claude-fable-5, claude-opus-5 | 280 | 260,280 | 61,559,385 |
| Claude.md and reference docs review | 2026-08-23 | claude-opus-5 | 186 | 114,065 | 40,476,672 |
| Mikan automation server framework | 2026-08-23 | claude-fable-5, claude-opus-5 | 521 | 416,068 | 139,625,509 |
| Rendering order and line depth issues | 2026-08-25 | claude-fable-5 | 185 | 201,264 | 53,748,099 |
| MikanXR UI styling from MikanTrack | 2026-08-25 to 2026-08-26 | claude-fable-5, claude-opus-5, claude-sonnet-5 | 1,461 | 1,002,817 | 405,110,458 |
| 3D mesh generation for AR occlusion | 2026-08-25 to 2026-08-30 | claude-fable-5, claude-opus-5 | 119 | 162,599 | 25,002,129 |
| Daily Checkin Twitch redeem in SAMMI | 2026-08-26 to 2026-09-09 | claude-opus-5 | 75 | 157,324 | 11,143,527 |
| Wine glass icon scaling | 2026-08-26 | claude-opus-5 | 27 | 20,724 | 1,613,676 |
| CompositorNodeGraph undo/redo support | 2026-08-26 to 2026-08-27 | claude-fable-5, claude-opus-5 | 652 | 679,670 | 330,111,098 |
| Fix CefShutdown ordering in App::shutdown | 2026-08-27 | claude-fable-5 | 28 | 15,538 | 1,768,221 |
| Spout logging window management | 2026-08-27 | claude-opus-5 | 143 | 78,634 | 22,165,034 |
| Lua component binding architecture | 2026-08-28 | claude-opus-5 | 16 | 9,632 | 1,374,637 |
| iPhone ARKit merge conflicts | 2026-08-28 | claude-opus-5 | 83 | 44,199 | 8,694,207 |
| List agents | 2026-08-28 to 2026-08-30 | claude-opus-5 | 1,599 | 1,278,581 | 822,545,213 |
| Project Outliner tree view panel | 2026-09-01 to 2026-09-03 | claude-fable-5, claude-opus-5 | 591 | 522,130 | 271,776,496 |
| Homebrew robot arm simulator | 2026-09-01 | claude-fable-5 | 1 | 4,991 | 25,436 |
| Visual Studio detached window rendering bug | 2026-09-03 to 2026-09-04 | claude-opus-5 | 190 | 128,278 | 39,964,555 |
| MikanGStreamer shutdown crash | 2026-09-03 | claude-opus-5 | 53 | 40,416 | 5,099,965 |
| Project-level scripting refactor | 2026-09-03 | claude-fable-5-1, claude-opus-5, claude-sonnet-5 | 271 | 337,847 | 59,139,649 |
| Lua procedural light generation | 2026-09-04 to 2026-09-07 | claude-fable-5-1, claude-opus-5, claude-sonnet-5 | 1,144 | 1,077,907 | 338,762,427 |
| MikanXR_UEClientSource compositor rendering | 2026-09-06 | claude-fable-5-1, claude-opus-5 | 59 | 124,228 | 10,330,520 |
| Unreal MCP support evaluation | 2026-09-06 | claude-fable-5-1, claude-opus-5 | 313 | 458,103 | 109,423,110 |
| MikanXR stencil model data fetch optimization | 2026-09-07 | claude-opus-5 | 251 | 150,108 | 59,424,531 |
| Twitch channel points integration architecture | 2026-09-07 | claude-opus-5 | 134 | 70,071 | 22,944,037 |
| JasperMozuProject porting to link-engine | 2026-09-07 | claude-fable-5-1, claude-opus-5 | 202 | 425,569 | 93,107,964 |
| MikanXR cross-platform rendering | 2026-09-07 | claude-opus-5 | 24 | 25,638 | 2,022,014 |
| CEF browser window interactivity | 2026-09-07 to 2026-09-08 | claude-opus-5 | 637 | 414,758 | 296,648,741 |
| MikanXR camera position persistence | 2026-09-08 | claude-opus-5 | 272 | 145,965 | 72,989,508 |
| Fix missing CEF localization labels | 2026-09-08 | claude-opus-5 | 49 | 26,890 | 3,617,494 |
| QuadShape depth sorting in compositor | 2026-09-08 | claude-fable-5-1 | 108 | 91,524 | 18,501,042 |
| Node graph editor for materials | 2026-09-08 to 2026-09-10 | claude-fable-5-1, claude-opus-5 | 647 | 984,719 | 197,204,436 |
| Project outliner layout changes | 2026-09-10 to 2026-09-12 | claude-fable-5-1, claude-opus-5 | 260 | 307,805 | 87,869,814 |
| Project asset management system | 2026-09-12 to 2026-09-13 | claude-fable-5-1, claude-opus-5, claude-sonnet-5 | 719 | 736,874 | 169,164,630 |
| MikanXR energy/water usage badging | 2026-09-13 | claude-opus-5 | 52 | 52,847 | 5,915,713 |
| Localization contribution workflow | 2026-09-13 | claude-opus-5 | 16 | 27,634 | 1,127,725 |

## Totals per model

| Model | API calls | Input | Cache write | Cache read | Output | Weighted |
|---|---|---|---|---|---|---|
| claude-fable-5 | 2,283 | 10,515 | 8,320,899 | 687,634,646 | 2,306,620 | 19,188,785 |
| claude-fable-5-1 | 1,777 | 86,093 | 15,073,095 | 672,261,928 | 3,004,919 | 22,149,575 |
| claude-opus-4-8 | 1,033 | 15,661 | 4,592,961 | 237,315,679 | 1,135,296 | 7,529,115 |
| claude-opus-5 | 7,410 | 28,270 | 30,828,786 | 2,676,505,806 | 5,752,678 | 70,782,376 |
| claude-sonnet-5 | 4,278 | 59,524 | 22,500,097 | 1,479,337,328 | 3,358,328 | 41,294,818 |
| **total** | 16,781 | 200,063 | 81,315,838 | 5,753,055,387 | 15,557,841 | **160,944,671** |

## Energy and water estimate

| Scenario | Wh per 1k weighted tokens | Energy | Water (onsite cooling) | Water (incl. generation) |
|---|---|---|---|---|
| low | 0.5 | 80 kWh | 89 L | 241 L |
| central | 2.0 | 322 kWh | 354 L | 966 L |
| high | 6.0 | 966 kWh | 1,062 L | 2,897 L |
<!-- TOKEN_STATS:END -->
