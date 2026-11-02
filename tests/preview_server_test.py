"""Verify the public/private boundary using real HTTP, including encoded traversal."""
import functools
import http.client
import importlib.util
from pathlib import Path
import tempfile
import threading
from http.server import ThreadingHTTPServer

ROOT = Path(__file__).resolve().parents[1]
def module(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'tools' / (name + '.py'))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod

with tempfile.TemporaryDirectory() as temp:
    folder = Path(temp)
    metadata = {'branch': 'test/<script>alert(1)</script>', 'commit': '12345678abc', 'pr': '8'}
    module('prepare_preview').prepare(folder, metadata)
    assert set(p.name for p in folder.iterdir()) == {'index.html', 'version.json'}
    assert '&lt;script&gt;' in (folder / 'index.html').read_text()
    assert 'href="flash.html"' not in (folder / 'index.html').read_text()
    # Even accidental extra files inside the served folder cannot become routes.
    (folder / 'private.txt').write_text('MUST NOT LEAK')
    server = ThreadingHTTPServer(('127.0.0.1', 0), functools.partial(module('serve_preview').PreviewHandler, directory=folder))
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        for path in ['/', '/index.html?fresh=1', '/version.json']:
            conn = http.client.HTTPConnection(*server.server_address)
            conn.request('GET', path)
            response = conn.getresponse()
            assert response.status == 200 and response.getheader('Cache-Control') == 'no-store'
            assert response.read()
            conn.close()
        for path in ['/private.txt', '/.git/config', '/.env', '/core/undergrowth.c', '/tools/serve_preview.py', '/firmware/releases/manifest.json', '/flash.html', '/../index.html', '/%2e%2e/.env', '/index.html/../.env', '/build/preview/private.txt', '/web/app.js']:
            conn = http.client.HTTPConnection(*server.server_address)
            conn.request('GET', path)
            response = conn.getresponse()
            assert response.status == 404 and b'MUST NOT LEAK' not in response.read(), path
            conn.close()
        conn = http.client.HTTPConnection(*server.server_address)
        conn.request('HEAD', '/')
        response = conn.getresponse()
        assert response.status == 200 and response.read() == b''
        conn.close()
    finally:
        server.shutdown()
        server.server_close()
        thread.join()
print('PASS preview: two public files, escaped metadata, no repository/filesystem routes, GET/HEAD and uncached responses')
