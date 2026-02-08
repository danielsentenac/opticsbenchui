#!/bin/bash
set -euo pipefail

REPO_URL="git@github.com:danielsentenac/opticsbenchui.git"
WORKDIR="/tmp/obui-ghpages"
SOURCE_DIR="/home/sentenac/OPTICSBENCHUI/opticsbenchui/docs/api/html"

echo "Building API docs..."
./make_api_doc.run

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
git clone "${REPO_URL}" "${WORKDIR}"
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

echo "Done. URL: https://danielsentenac.github.io/opticsbenchui/api/"
