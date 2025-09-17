#!/usr/bin/env bash
set -euo pipefail
REPO="${1:?owner/repo required}"

# Staging: wait 2m, protected branches
gh api -X PUT -H "X-GitHub-Api-Version: 2022-11-28" \
  "repos/${REPO}/environments/staging" \
  -f wait_timer=2 \
  -f prevent_self_review=false \
  -f deployment_branch_policy[protected_branches]=true \
  -f deployment_branch_policy[custom_branch_policies]=false

# Production: wait 5m, prevent self-review, 1 reviewer (replace USER_ID)
gh api -X PUT -H "X-GitHub-Api-Version: 2022-11-28" \
  "repos/${REPO}/environments/production" \
  -f wait_timer=5 \
  -f prevent_self_review=true \
  -f deployment_branch_policy[protected_branches]=true \
  -f deployment_branch_policy[custom_branch_policies]=false \
  --input - <<'JSON'
{ "reviewers": [ { "type": "User", "id": 1234567 } ] }
JSON
echo "Created/updated environments."
