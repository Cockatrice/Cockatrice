#!/bin/bash
# Publish a new Development beta tag.
# Wraps the manual flow:
#   git checkout master && git remote update -p && git pull
#   git tag <date>-Development-<version>-beta[.N]
#   git push upstream <tag>
#
# Tag convention (see cmake/getversion.cmake):
#   first beta for a version:  YYYY-MM-DD-Development-X.Y.Z-beta
#   subsequent betas:           YYYY-MM-DD-Development-X.Y.Z-beta.2, .3, ...
# Version defaults to the PROJECT VERSION in CMakeLists.txt.
#
# Usage:
#   ./publish_beta.sh [--dry-run] [--yes] [--version X.Y.Z] [--remote upstream] [--branch master]

set -euo pipefail

REMOTE="upstream"
BRANCH="master"
VERSION=""
DRY_RUN=0
ASSUME_YES=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --version X.Y.Z   Version to tag (default: parsed from CMakeLists.txt)
  --remote NAME     Remote to update and push the tag to (default: upstream)
  --branch NAME     Branch to tag from (default: master)
  --dry-run         Print what would be done without creating/pushing the tag
  --yes             Skip the confirmation prompt
  -h, --help        Show this help
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
    --version)
        VERSION="${2:?--version needs an argument like 3.1.1}"
        shift 2
        ;;
    --version=*)
        VERSION="${1#--version=}"
        shift
        ;;
    --remote)
        REMOTE="${2:?--remote needs an argument}"
        shift 2
        ;;
    --remote=*)
        REMOTE="${1#--remote=}"
        shift
        ;;
    --branch)
        BRANCH="${2:?--branch needs an argument}"
        shift 2
        ;;
    --branch=*)
        BRANCH="${1#--branch=}"
        shift
        ;;
    --dry-run)
        DRY_RUN=1
        shift
        ;;
    --yes)
        ASSUME_YES=1
        shift
        ;;
    -h | --help)
        usage
        exit 0
        ;;
    *)
        echo "Unknown option: $1" >&2
        usage >&2
        exit 1
        ;;
    esac
done

cd "$(dirname "${BASH_SOURCE[0]}")" || exit 2

if ! git rev-parse --git-dir >/dev/null 2>&1; then
    echo "Error: not inside a git repository." >&2
    exit 1
fi

if [[ -z "$VERSION" ]]; then
    VERSION="$(grep -m1 -oP 'project\(\s*"Cockatrice"\s+VERSION\s+\K[0-9]+\.[0-9]+\.[0-9]+' CMakeLists.txt || true)"
fi
if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Error: could not determine version (got '$VERSION'). Pass --version X.Y.Z." >&2
    exit 1
fi

if [[ -n "$(git status --porcelain)" ]]; then
    echo "Warning: working tree has uncommitted changes. They will not be included in the tag." >&2
    git status --short >&2
fi

echo "==> git checkout $BRANCH"
if [[ "$DRY_RUN" -eq 0 ]]; then
    git checkout "$BRANCH"
else
    echo "(dry-run) skipped"
fi

echo "==> git remote update -p"
if [[ "$DRY_RUN" -eq 0 ]]; then
    git remote update -p
else
    echo "(dry-run) skipped"
fi

echo "==> git pull --ff-only"
if [[ "$DRY_RUN" -eq 0 ]]; then
    git pull --ff-only
    git fetch --tags "$REMOTE"
else
    echo "(dry-run) skipped"
fi

DATE="$(date +%F)"
PATTERN="*-Development-${VERSION}-beta*"

# Highest beta number used so far for this version.
# A bare "-beta" (no suffix) counts as 1; "-beta.N" counts as N.
MAX_BETA=0
while IFS= read -r tag; do
    [[ -n "$tag" ]] || continue
    if [[ "$tag" =~ -beta\.([0-9]+)$ ]]; then
        NUM="${BASH_REMATCH[1]}"
    elif [[ "$tag" =~ -beta$ ]]; then
        NUM=1
    else
        continue
    fi
    if [[ "$NUM" -gt "$MAX_BETA" ]]; then
        MAX_BETA="$NUM"
    fi
done < <(git tag --list "$PATTERN")

NEXT_BETA=$((MAX_BETA + 1))
if [[ "$NEXT_BETA" -le 1 ]]; then
    NEW_TAG="${DATE}-Development-${VERSION}-beta"
else
    NEW_TAG="${DATE}-Development-${VERSION}-beta.${NEXT_BETA}"
fi

echo ""
echo "Version:      $VERSION"
echo "Latest beta:  $MAX_BETA (for $VERSION)"
echo "New tag:      $NEW_TAG"
echo "Head:         $(git rev-parse --short HEAD) $(git log -1 --oneline | head -c 120)"
echo ""

if git rev-parse -q --verify "refs/tags/${NEW_TAG}" >/dev/null; then
    echo "Error: tag $NEW_TAG already exists locally." >&2
    exit 1
fi
if git ls-remote --tags "$REMOTE" | grep -q "refs/tags/${NEW_TAG}$"; then
    echo "Error: tag $NEW_TAG already exists on $REMOTE." >&2
    exit 1
fi

if [[ "$DRY_RUN" -eq 1 ]]; then
    echo "(dry-run) would run: git tag $NEW_TAG"
    echo "(dry-run) would run: git push $REMOTE $NEW_TAG"
    exit 0
fi

if [[ "$ASSUME_YES" -eq 0 ]]; then
    read -r -p "Create tag $NEW_TAG and push to $REMOTE? [y/N] " answer
    if [[ ! "$answer" =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 1
    fi
fi

git tag "$NEW_TAG"
git push "$REMOTE" "$NEW_TAG"

echo "Done: $NEW_TAG"
