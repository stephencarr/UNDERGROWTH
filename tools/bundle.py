#!/usr/bin/env python3
"""Build a dependency-free, local-file browser instrument from the exact tested core."""
from pathlib import Path
import base64,json
root=Path(__file__).resolve().parents[1]
w=root/'web'
html=(w/'index.html').read_text()
css=(w/'style.css').read_text()+'\n'+(w/'fm1-panel.css').read_text()
js=(w/'app.js').read_text()+'\n'+(w/'fm1-panel.js').read_text()+'\n'+(w/'device.js').read_text().replace('import "./fm1-panel.js";','')
wasm=base64.b64encode((w/'core.wasm').read_bytes()).decode()
old="const response=await fetch('core.wasm');if(!response.ok)throw new Error('Could not load core.wasm. Serve the complete web folder.');\n    const module=await WebAssembly.compile(await response.arrayBuffer());\n    await context.audioWorklet.addModule('worklet.js');"
new="const module=await WebAssembly.compile(Uint8Array.from(atob("+json.dumps(wasm)+"),c=>c.charCodeAt(0)));\n    const workletURL=URL.createObjectURL(new Blob(["+json.dumps((w/'worklet.js').read_text())+"],{type:'text/javascript'}));\n    try{await context.audioWorklet.addModule(workletURL);}finally{URL.revokeObjectURL(workletURL);}"
assert old in js
js=js.replace(old,new)
html=html.replace('<link rel="stylesheet" href="fm1-panel.css">','')
html=html.replace('<link rel="stylesheet" href="style.css">','<style>'+css+'</style>')
html=html.replace('href="../flash.html"','href="flash.html"')
html=html.replace('<script type="module" src="device.js"></script>','')
html=html.replace('<script type="module" src="app.js"></script>','<script type="module">'+js+'</script>')
assert 'src="app.js"' not in html
(root/'index.html').write_text(html)
print('Bundled',root/'index.html')
