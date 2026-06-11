---
name: feedback-no-bare-sleep
description: "Don't use bare sleep as a timing fix; use condition-based polling with sleeps in the loop"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 786f77e8-30e0-47c9-9b9f-7917fb351bd8
---

Don't use `sleep N` as a timing fix — it's a guess that will be wrong on slower hardware and wasteful on faster hardware.

**Why:** Will explicitly asked for this; the trigger was a `sleep 5` added before `plasma-apply-wallpaperimage`. The right model is to poll for the actual readiness condition (e.g., dbus-send succeeds, file exists, process is up) with a sleep *inside* the loop, and break as soon as the condition is met.

**How to apply:** Whenever you're tempted to "wait for X to be ready" with a fixed delay:
- Identify the actual observable readiness condition (DBus registration, PID appearing, port listening, file existing)
- Write a polling loop: `while [ $i -lt N ]; do check && break; sleep 1; i=$((i+1)); done`
- Set a timeout (the loop bound) so it doesn't spin forever
- Act only after the condition passes, not after the sleep elapses

Example (plasmashell DBus readiness):
```sh
i=0
while [ $i -lt 60 ]; do
    dbus-send --session --dest=org.kde.plasmashell --print-reply \
        /PlasmaShell org.kde.PlasmaShell.version >/dev/null 2>&1 && break
    sleep 1
    i=$((i+1))
done
```
