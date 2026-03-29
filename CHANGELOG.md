# Changelog

All notable changes to SimpleQuest will be documented here.
Format loosely follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [Unreleased]

### Active Development
- Connection of the visual graph layer to the runtime data layer
- Full Details panel support for questline nodes

### Upcoming
- Undo/Redo and node duplication robustness

### Known Issues
- Undo operation may crash the editor under certain conditions — under investigation

---

## [0.2.0] — 2026-03-28 — Visual Graph Editor: Schema & Connections

### Added
- Full connection validation in `UQuestlineGraphSchema::CanCreateConnection`:
    - Prevents duplicate signal paths from the same source quest node
    - Prevents parallel paths to the same destination through reroute nodes
    - Enforces exit node rules: a quest node may not route multiple outputs
      to the same exit node, directly or through reroutes
    - Enforces that a single output pin leads to at most one exit node
- Custom wire rendering via `FQuestlineConnectionDrawingPolicy`:
    - Green wires for Quest Success paths
    - Red wires for Quest Failure paths
    - White wires for Any Outcome and Activation paths
    - Dashed wire rendering for Prerequisites connections
    - Spline hover detection for all wire types
- Double-clicking a wire now inserts a Reroute node in-place
- Hotkey node placement matching standard Blueprint conventions:
    - `Q + click` — place Quest node
    - `S + click` — place Quest Success exit node
    - `F + click` — place Quest Failure exit node
    - `R + click` — place Reroute node
    - Pressing a valid key while dragging a wire places and connects
      the corresponding node immediately at the cursor
        - Node placement by hotkey aligns the cursor with the node's
          relevant input pin, matching standard Blueprint behavior
- `SQuestlineGraphPanel` wrapper widget providing graph-aware input
  handling with correct Slate focus lifecycle management

---

## [0.1.0] — 2026-03-25 — Visual Graph Editor: Scaffolding

### Added
- `UQuestlineGraphSchema` — custom graph schema for the questline editor
- Editor node types:
    - `UQuestlineNode_Entry` — questline start node (non-deletable)
    - `UQuestlineNode_Quest` — represents a single quest; pins: Activate,
      Prerequisites (input), Success, Failure, Any Outcome (output)
    - `UQuestlineNode_Exit_Success` / `UQuestlineNode_Exit_Failure` — terminal
      outcome nodes
    - `UQuestlineNode_Knot` — reroute/passthrough node with dynamic
      type propagation matching the first connected wire
- `FQuestlineGraphEditor` — asset editor toolkit hosting the graph viewport
- `UQuestlineGraph` — editor graph asset containing the `UEdGraph`
- Basic node auto-wiring (`AutowireNewNode`) on all node types:
    - Output pin drags connect to Activate by default on Quest nodes
    - Input pin drags connect from Any Outcome by default on Quest nodes
    - Exit nodes only accept connections from output pins

---

## [0.0.1] — 2025-10-22 — Event Bus

### Added
- Runtime event bus supporting decoupled quest state communication

---

## [0.0.0] — 2025-05-30 — Initial Prototype (GameDev.tv Game Jam — USS Proteus)

### Added
- Core DAG data structure for representing questline graphs at runtime
- Quest node data model with Success, Failure, and Any Outcome resolution
- Initial proof-of-concept during game jam development