#!/usr/bin/env python3
"""Serve two explicitly allowed browser artifacts. Never serve repository paths."""
import functools
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import os
from pathlib import Path
from urllib.parse import urlsplit

PUBLIC = Path(__file__).resolve().parents[1] / 'build/preview'
ROUTES = {'/': ('index.html', 'text/html; charset=utf-8'),
          '/index.html': ('index.html', 'text/html; charset=utf-8'),
          '/version.json': ('version.json', 'application/json; charset=utf-8')}

class PreviewHandler(BaseHTTPRequestHandler):
    def __init__(self, *args, directory=PUBLIC, **kwargs):
        self.directory = directory
        super().__init__(*args, **kwargs)

    def do_GET(self):
        self.respond(True)

    def do_HEAD(self):
        self.respond(False)

    def respond(self, body):
        route = ROUTES.get(urlsplit(self.path).path)
        if route is None:
            self.send_error(404)
            return
        filename, content_type = route
        try:
            data = (self.directory / filename).read_bytes()
        except FileNotFoundError:
            self.send_error(503, 'Preview build not prepared')
            return
        self.send_response(200)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Cache-Control', 'no-store')
        self.send_header('X-Content-Type-Options', 'nosniff')
        self.end_headers()
        if body:
            self.wfile.write(data)

if __name__ == '__main__':
    for filename, _ in ROUTES.values():
        if not (PUBLIC / filename).is_file():
            raise SystemExit('Run python3 tools/prepare_preview.py before serving')
    ThreadingHTTPServer(('0.0.0.0', int(os.environ.get('PORT', '8080'))), PreviewHandler).serve_forever()
