#!/bin/bash
set -euo pipefail

REPO_URL="git@github.com:danielsentenac/opticsbenchui.git"
WORKDIR="/tmp/obui-ghpages"
SOURCE_DIR="/home/sentenac/OPTICSBENCHUI/opticsbenchui/docs/api/html"

echo "Building API docs..."
./make_api_doc.run

echo "Publishing to gh-pages..."
rm -rf "${WORKDIR}"
git clone "${REPO_URL}" "${WORKDIR}"
cd "${WORKDIR}"
git checkout --orphan gh-pages
git rm -rf .

mkdir -p api
cp -R "${SOURCE_DIR}/"* api/

git add -A
git commit -m "Publish API docs"
git push -u origin gh-pages --force

echo "Done. URL: https://danielsentenac.github.io/opticsbenchui/api/"
