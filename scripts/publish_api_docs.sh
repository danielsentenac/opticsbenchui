#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
WORKDIR="/tmp/obui-ghpages"
SOURCE_DIR="${REPO_ROOT}/docs/api/html"
REMOTE_URL="$(git -C "${REPO_ROOT}" remote get-url origin)"

case "${REMOTE_URL}" in
  git@github.com:*)
    REPO_PATH="${REMOTE_URL#git@github.com:}"
    ;;
  https://github.com/*)
    REPO_PATH="${REMOTE_URL#https://github.com/}"
    ;;
  ssh://git@github.com/*)
    REPO_PATH="${REMOTE_URL#ssh://git@github.com/}"
    ;;
  *)
    echo "ERROR: origin remote must point to GitHub: ${REMOTE_URL}"
    exit 1
    ;;
esac

REPO_PATH="${REPO_PATH%.git}"
REPO_OWNER="${REPO_PATH%%/*}"
REPO_NAME="${REPO_PATH##*/}"
PAGES_URL="https://${REPO_OWNER}.github.io/${REPO_NAME}/api/"

echo "Building API docs..."
"${SCRIPT_DIR}/make_api_doc.run"

if [[ ! -d "${SOURCE_DIR}" ]]; then
  echo "ERROR: SOURCE_DIR not found: ${SOURCE_DIR}"
  exit 1
fi

if [[ -z "$(ls -A "${SOURCE_DIR}")" ]]; then
  echo "ERROR: SOURCE_DIR is empty: ${SOURCE_DIR}"
  exit 1
fi

echo "Publishing to gh-pages..."
rm -rf "${WORKDIR}"
git clone "${REMOTE_URL}" "${WORKDIR}"
cd "${WORKDIR}"

if git show-ref --verify --quiet refs/heads/gh-pages; then
  git checkout gh-pages
else
  git checkout --orphan gh-pages
fi

git rm -rf . >/dev/null 2>&1 || true
git clean -fdx >/dev/null 2>&1 || true

mkdir -p api
cp -R "${SOURCE_DIR}/"* api/
touch .nojekyll

git add -A

if git diff --cached --quiet; then
  echo "No changes to publish."
  exit 0
fi

commit_time="$(date -u +"%Y-%m-%d %H:%M UTC")"
git commit -m "Publish API docs (${commit_time})"
git push -u origin gh-pages --force

echo "Pushed commit: $(git rev-parse --short HEAD)"

echo "Done. URL: ${PAGES_URL}"
