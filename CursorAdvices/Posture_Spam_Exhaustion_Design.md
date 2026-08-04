# Posture spam / exhaustion (design — not implemented)

Future anti-spam for rapid posture flicking. Parked so the vertical slice stays first.

---

## Problem

If posture changes are free and instant (or only lightly delayed), rolling the direction input makes the fighter **unreadable** on purpose — best mix-up tool with no cost.

Goal: punish **spam**, not honest stance changes.

---

## Preferred direction: “exhaustion / slow-mo” mix

Combine ideas rather than only classic move-staling floats:

| Layer | Effect when spamming posture changes |
|-------|--------------------------------------|
| **Locomotion** | Lower move speed while changing / while exhausted |
| **Posture** | Extra delay / longer `PostureChanging` (frame ints, not float mystery) |
| **Offense / defense** | Longer startup (and maybe recovery) on attacks / guard while exhausted |
| **Presentation** | Clear visual (desat, slower anim rate, VFX) so the player *feels* the tax |

Read as: the fighter is **winded**, not “secret frame math happened.”

Scaling schedule still fits: first changes cheap, repeated changes in a short window stack exhaustion; idle clears it over N frames.

---

## What not to do first

- Don’t invent a complex float staling curve before hitboxes exist.  
- Don’t hard-lockout so hard that one mis-flick ruins a round (tune later).  

Current `PostureStalePenalty` floats in `FFighterSimState` are **placeholder**; replace with frame-based exhaustion when this is built.

---

## See also

- [Posture_System.md](Posture_System.md)  
- `Source/PurgatoriumLex/Combat/CombatTypes.h` (`FFighterSimState` posture fields)
