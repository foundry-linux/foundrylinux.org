# bootstrap.sh credential caching + step 7.5

**Date:** 2026-05-18  
**Status:** Done

## Changes shipped

### Credential caching (`BOOTSTRAP_CACHE=/tmp/foundry-bootstrap.env`)

`CF_API_TOKEN`, `R2_ACCESS_KEY_ID`, and `R2_SECRET_ACCESS_KEY` were re-prompted on every
fresh run. Added:

- `BOOTSTRAP_CACHE="/tmp/foundry-bootstrap.env"` to config block
- `cache_set KEY value` helper — grep-and-replace single line in cache file, chmod 600
- Source cache before preflight; `CF_API_TOKEN` saved immediately after entry, R2 creds
  saved immediately after their `until` loops
- Init vars as `${VAR:-}` (not `""`) so cache values survive the initialisation block

### Step 7.5 — URL rewrite rule for `/` → `/index.html`

Original plan used `http_request_redirect` phase (for 301 redirects). The Cloudflare **free
plan does not allow this phase at zone level** — API returns:

```
"phase \"http_request_redirect\" not allowed at zone level"
```

Fix: use `http_request_transform` (URL rewrite) instead. The rewrite is transparent — the
browser URL stays `/` — which is fine for a landing page.

Also fixed along the way:
- `jq` syntax error when `REDIRECT_EXPR` containing literal `"` was interpolated into an
  inline jq string. Fix: `jq -n --arg expr "$EXPR"` throughout.
- All other inline JSON bodies (`r2/buckets`, `dns_records`, `domains/custom`) converted
  from escaped strings to `jq -n --arg` calls.
- Interactive `read` prompts wrapped in `until` loops — blank Enter no longer silently sets
  an empty variable.
- Script opens `xdg-open https://<CUSTOM_DOMAIN>/` on completion so the landing page is
  immediately visible.

### Additional fixes in this session

| What | Fix |
|---|---|
| R2 token type guidance | Prompt now says "Create Account API token" (not User) with explanation |
| Token name shown in prompt | `R2_TOKEN_NAME` displayed so no guessing |
| Step 7.5 error visibility | Errors from CF API now surface the `.errors[0].message` instead of silently dying |

## Verification

```bash
curl -sI https://apt.foundrylinux.org/
# HTTP/2 200  content-type: text/html
curl -s https://apt.foundrylinux.org/ | grep '<title>'
# <title>Foundry APT Repository</title>
```
