# Contributing

This document describes how work is organized in this repository: branching, commits, pull requests, and the extra rules that apply to KiCad hardware files.

## Repository layout

| Path | Contents |
|---|---|
| `hardware/` | KiCad project (`smart-knob.kicad_pro`) and project-local libraries in `hardware/lib/` |
| `firmware/` | ESP-IDF project for the ESP32-S3 |
| `docs/` | Datasheet links, notes, renders |
| `mechanical/` | Enclosure CAD |

## Tools

- **KiCad** 9 or newer
- **ESP-IDF** v6.1 (activate it in each new terminal before running `idf.py`)
- **Git** and optionally the **GitHub CLI** (`gh`)

## The core rule

**`main` always works.** All changes are made on a branch and merged through a pull request. Firmware on `main` must build; hardware on `main` must pass ERC and DRC.

## Branches

Name branches `type/short-description`, lowercase with hyphens:

| Prefix | Use for | Example |
|---|---|---|
| `fw/` | Firmware features | `fw/encoder-driver` |
| `hw/` | Schematic or PCB work | `hw/rev1-schematic` |
| `fix/` | Bug fixes (hardware or firmware) | `fix/encoder-double-count` |
| `docs/` | Documentation | `docs/build-instructions` |
| `chore/` | Tooling, CI, repo housekeeping | `chore/kibot-ci` |

Keep each branch focused on one change.

## Commit messages

Format: `prefix: imperative summary`

```
fw: add rotary encoder driver with debounce
hw: route USB-C data lines as differential pair
fix: correct encoder direction on CCW rotation
docs: add flashing instructions to README
chore: ignore ESP-IDF build output
```

- Keep the summary under ~70 characters, with no trailing period.
- Use the imperative mood: "add", not "added" or "adding".
- Describe what changed and why. Avoid messages like "wip" or "updates".
- Commit small, logical steps rather than one large change.

## Workflow

### 1. Start from an up-to-date `main`

```bash
git switch main
git pull
```

### 2. Create a branch

```bash
git switch -c fw/encoder-driver
```

### 3. Make changes and commit

```bash
git status
git diff
git add <specific files or folders>
git diff --cached        # review exactly what will be committed
git commit -m "fw: add encoder GPIO setup"
```

Stage specific paths rather than everything, and review the staged diff before committing.

### 4. Push

```bash
git push -u origin fw/encoder-driver   # first push of a new branch
git push                               # subsequent pushes
```

### 5. Open a pull request

```bash
gh pr create --fill
```

Or use **Compare & pull request** on GitHub. Fill in the checklist below in the PR description.

### 6. Merge and clean up

Merge with **Squash and merge** so each PR becomes one commit on `main`, titled with the PR title. Then:

```bash
git switch main
git pull
git branch -d fw/encoder-driver
```

## Pull request checklist

**Firmware changes**
- [ ] `idf.py build` succeeds with no new warnings
- [ ] Tested on hardware (note which board revision)
- [ ] Pin changes are reflected in `firmware/main/board_pins.h` and match the schematic net labels

**Hardware changes**
- [ ] ERC passes (or remaining warnings are explained in the PR)
- [ ] DRC passes
- [ ] No missing symbols, footprints, or 3D models
- [ ] Any pin changes are reflected in `board_pins.h`

**All changes**
- [ ] No secrets, credentials, or personal paths in the diff
- [ ] No build output, backups, or cache files committed

## Hardware rules

- **One hardware branch at a time.** Git cannot meaningfully merge two different edits to the same `.kicad_sch` or `.kicad_pcb`. Finish and merge one `hw/` branch before starting another.
- **Close KiCad before switching branches**, so it doesn't save an old version over the checked-out files.
- **Keep libraries project-local.** Custom symbols, footprints, and 3D models go in `hardware/lib/` and are referenced with `${KIPRJMOD}` paths, never absolute paths.
- **Pin assignments are shared.** Firmware pin names in `board_pins.h` match the net labels in the schematic. Change both together.

## Secrets

Never commit credentials. Wi-Fi passwords, API tokens, and similar values go in `firmware/main/secrets.h`, which is gitignored. A template with placeholder values lives in `firmware/main/secrets.example.h`; copy it to `secrets.h` and fill in your own values.

`firmware/sdkconfig` is also gitignored because it can contain credentials set through `menuconfig`. Commit configuration changes to `sdkconfig.defaults` with:

```bash
idf.py save-defconfig
```

## Releases and tags

| Tag | Meaning |
|---|---|
| `hw-vX.Y` | A board revision sent to fabrication |
| `fw-vX.Y.Z` | A firmware version flashed to real hardware |

When ordering boards, export manufacturing files into `hardware/production/hw-vX.Y/`, commit them, then tag:

```bash
git tag hw-v1.0
git push --tags
```

## Quick reference

```bash
git switch main && git pull                 # start fresh
git switch -c fw/thing                      # new branch
git add <files> && git commit -m "fw: ..."  # save progress
git push -u origin fw/thing                 # first push
gh pr create --fill                         # open PR
# squash and merge on GitHub, then:
git switch main && git pull && git branch -d fw/thing
```