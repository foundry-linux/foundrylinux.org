# Plan: bootstrap.sh credential caching + step 7.5 bug fixes

## Context

Two problems to fix in one commit:

1. **Credential re-entry fatigue**: `CF_API_TOKEN`, `R2_ACCESS_KEY_ID`, and
   `R2_SECRET_ACCESS_KEY` are re-prompted on every fresh run even though they've already been
   entered. Fix: cache prompted values to `/tmp/foundry-linux-bootstrap.env` (mode 600) and
   source it at startup.

2. **Step 7.5 double bug**: Two errors when creating the Cloudflare redirect rule:
   - `jq` syntax error: `REDIRECT_EXPR` contains double-quotes that break the inline jq
     string comparison. Fix: use `jq --arg` to pass the expression safely.
   - `curl: 405 Method Not Allowed`: `POST .../entrypoint/rules` doesn't exist as an
     endpoint. Fix: GET the phase entrypoint first to obtain the ruleset ID, then POST to
     `/rulesets/{ruleset_id}/rules`. If the entrypoint doesn't exist yet (no ruleset ID),
     use `PUT .../entrypoint` to create it with the rule inline.

## File to modify

`scripts/bootstrap.sh` only.

## Change 1 — Credential caching

### 1a. Add `BOOTSTRAP_CACHE` to the config block (after existing vars)

```bash
BOOTSTRAP_CACHE="/tmp/foundry-linux-bootstrap.env"
```

### 1b. Add `cache_set` helper next to `cf_api` / `r2_put_secret`

```bash
# Usage: cache_set KEY value  — writes/updates one KEY=<quoted> line in BOOTSTRAP_CACHE
cache_set() {
    local key="$1" val="$2"
    { grep -v "^${key}=" "$BOOTSTRAP_CACHE" 2>/dev/null || true
      printf '%s=%q\n' "$key" "$val"
    } > "${BOOTSTRAP_CACHE}.tmp" && mv "${BOOTSTRAP_CACHE}.tmp" "$BOOTSTRAP_CACHE"
    chmod 600 "$BOOTSTRAP_CACHE"
}
```

### 1c. Load cache before the preflight block (`command -v gpg`)

```bash
if [[ -f "$BOOTSTRAP_CACHE" ]]; then
    # shellcheck source=/dev/null
    source "$BOOTSTRAP_CACHE"
    info "Loaded cached credentials from $BOOTSTRAP_CACHE"
fi
```

### 1d. Save CF_API_TOKEN right after `export CF_API_TOKEN`

```bash
cache_set CF_API_TOKEN "$CF_API_TOKEN"
```

### 1e. Save R2 creds right after `export R2_ACCESS_KEY_ID R2_SECRET_ACCESS_KEY` (step 6)

```bash
cache_set R2_ACCESS_KEY_ID     "$R2_ACCESS_KEY_ID"
cache_set R2_SECRET_ACCESS_KEY "$R2_SECRET_ACCESS_KEY"
```

## Change 2 — Fix step 7.5 redirect rule creation

Replace the entire step 7.5 body with the correct three-case logic:

```bash
info "[7.5] Creating redirect rule: ${CUSTOM_DOMAIN}/ → /index.html"
REDIRECT_EXPR="(http.host eq \"${CUSTOM_DOMAIN}\" and http.request.uri.path eq \"/\")"

if $DRY_RUN; then
    echo "  [dry-run] PUT /zones/.../rulesets/phases/http_request_redirect/entrypoint"
else
    PHASE_JSON=$(cf_api GET \
        "/zones/${CF_ZONE_ID}/rulesets/phases/http_request_redirect/entrypoint" \
        2>/dev/null || echo '{}')
    RULESET_ID=$(echo "$PHASE_JSON" | jq -r '.result.id // empty' 2>/dev/null || true)
    EXISTING_RULE=$(echo "$PHASE_JSON" | jq -r \
        --arg expr "$REDIRECT_EXPR" \
        '.result.rules[]? | select(.expression == $expr) | .id' 2>/dev/null || true)

    RULE_BODY="{
        \"action\": \"redirect\",
        \"action_parameters\": {
            \"from_value\": {
                \"target_url\": {\"value\": \"https://${CUSTOM_DOMAIN}/index.html\"},
                \"status_code\": 301,
                \"preserve_query_string\": false
            }
        },
        \"expression\": \"${REDIRECT_EXPR}\",
        \"enabled\": true
    }"

    if [[ -n "$EXISTING_RULE" ]]; then
        ok "[7.5] Redirect rule already exists (id: ${EXISTING_RULE})"
    elif [[ -z "$RULESET_ID" ]]; then
        # Phase ruleset doesn't exist yet — create it via PUT
        cf_api PUT \
            "/zones/${CF_ZONE_ID}/rulesets/phases/http_request_redirect/entrypoint" \
            -d "{\"name\": \"Zone Redirect Rules\", \"rules\": [${RULE_BODY}]}" >/dev/null
        ok "[7.5] Redirect rule created: ${CUSTOM_DOMAIN}/ → /index.html"
    else
        # Ruleset exists — append our rule
        cf_api POST "/zones/${CF_ZONE_ID}/rulesets/${RULESET_ID}/rules" \
            -d "$RULE_BODY" >/dev/null
        ok "[7.5] Redirect rule added: ${CUSTOM_DOMAIN}/ → /index.html"
    fi
fi
```

## Verification

1. Run `bash scripts/bootstrap.sh` — enter CF_API_TOKEN once; confirm
   `/tmp/foundry-linux-bootstrap.env` is created (mode 600) and step 7.5 completes with
   `[ok]` instead of jq/curl errors.
2. Kill and re-run — confirm "Loaded cached credentials" appears, no token prompt shown.
3. `curl -sI https://apt.foundrylinux.org/` returns `HTTP/2 301` redirecting to
   `/index.html`.
4. `curl -sI https://apt.foundrylinux.org/index.html` returns `HTTP/2 200 text/html`.
