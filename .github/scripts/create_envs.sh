#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 owner/repo [production_reviewer_id]" >&2
  exit 1
fi

REPO="$1"
PRODUCTION_REVIEWER_ID="${2:-60427476}"

# Staging: wait 2m, protected branches
gh api -X PUT -H "X-GitHub-Api-Version: 2022-11-28" \
  "repos/${REPO}/environments/staging" \
  --input - <<JSON
{
  "wait_timer": 2,
  "prevent_self_review": false,
  "deployment_branch_policy": {
    "protected_branches": true,
    "custom_branch_policies": false
  }
}
JSON

# Production: wait 5m, prevent self-review, 1 reviewer (default repo owner)
gh api -X PUT -H "X-GitHub-Api-Version: 2022-11-28" \
  "repos/${REPO}/environments/production" \
  --input - <<JSON
{
  "wait_timer": 5,
  "prevent_self_review": true,
  "deployment_branch_policy": {
    "protected_branches": true,
    "custom_branch_policies": false
  },
  "reviewers": [
    { "type": "User", "id": ${PRODUCTION_REVIEWER_ID} }
  ]
}
JSON
echo "Created/updated environments."
