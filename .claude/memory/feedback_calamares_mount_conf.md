---
name: feedback-calamares-mount-conf
description: "Calamares 3.3 mount.conf — correct YAML schema for extraMounts (options array, efi:true, no extraMountsEfi)"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 786f77e8-30e0-47c9-9b9f-7917fb351bd8
---

The Calamares 3.3 mount module schema (`mount.schema.yaml`) has two hard rules that cause silent failure if violated (schema has `additionalProperties: false`, so unknown keys or wrong types drop the entire extraMounts block):

**Rule 1: `options` must be a YAML array, never a scalar.**
```yaml
# WRONG — string; Python ",".join("bind") = "b,i,n,d"
options: bind
options: gid=5,mode=620

# CORRECT — array
options: ["bind"]
options: ["gid=5", "mode=620"]
```

**Rule 2: `extraMountsEfi:` does not exist.** Use `efi: true` on individual entries inside `extraMounts:`.
```yaml
# WRONG — unknown key, rejects whole config
extraMountsEfi:
  - device: /sys/firmware/efi/efivars
    ...

# CORRECT
extraMounts:
  - device: /sys/firmware/efi/efivars
    fs:     efivarfs
    mountPoint: /sys/firmware/efi/efivars
    efi:    true
```

**Why:** When schema validation fails, `extra_mounts = []` and Python logs "No extra mounts defined. Does mount.conf exist?" — then grub-install runs without /dev bind mounts and exits code 1 ("cannot find a device for /boot/efi").

**How to apply:** Any time mount.conf is written or reviewed, verify options are arrays and there is no `extraMountsEfi:` key. See `foundry-apt/packages/calamares-settings-foundry-linux/data/modules/mount.conf` for the canonical correct version (1.0.15+).
