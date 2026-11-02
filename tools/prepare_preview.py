#!/usr/bin/env python3
"""Package only the existing browser bundle and non-secret revision metadata."""
import html
import json
import os
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[1]

def prepare(destination, metadata):
    destination.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'index.html').read_text()
    label = 'PREVIEW · ' + ('PR #' + metadata['pr'] + ' · ' if metadata['pr'] else '') + metadata['branch'] + ' · ' + metadata['commit'][:8]
    banner = '<aside style="padding:10px 24px;background:#ffe6a0;color:#302710;font:14px monospace">' + html.escape(label) + ' · Session only</aside>'
    source = source.replace('<body>', '<body>' + banner, 1)
    # Previewing a browser build is separate from authorizing a hardware install.
    source = re.sub(r'<a\b[^>]*href="flash.html"[^>]*>.*?</a>', '<span>Browser preview · firmware separate</span>', source, flags=re.S)
    (destination / 'index.html').write_text(source)
    (destination / 'version.json').write_text(json.dumps(metadata) + '\n')

if __name__ == '__main__':
    prepare(ROOT / 'build/preview', {
        'branch': os.environ.get('RAILWAY_GIT_BRANCH', 'local'),
        'commit': os.environ.get('RAILWAY_GIT_COMMIT_SHA', 'local'),
        'pr': os.environ.get('RAILWAY_PR_NUMBER', ''),
    })
